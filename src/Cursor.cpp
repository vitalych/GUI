/// Copyright (c) 2020 Vitaly Chipounov
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.

#include "Cursor.h"
#include "Framebuffer.h"
#include "Utils.h"

namespace gui {

// TODO: faster and/xor to framebuffer
void CImageData::Draw(IFrameBuffer &fb, int _x, int _y) const {
    auto i = 0;
    for (auto y = 0; y < m_height; ++y) {
        for (auto x = 0; x < m_width; ++x, ++i) {
            auto am = m_and.get();
            bool b = (am[i / 8] & (0x80 >> (i % 8))) == 0;
            if (b) {
                fb.PutPixel(x + _x, y + _y, m_pixels.get()[i]);
            }
        }
    }
}

CImageDataPtr CCursor::ParseImageWithPalette(const CURSORDIRENTRY *cde, const BITMAPINFOHEADER *bih,
                                             const RGBQUAD *colors, const uint8_t *xorMask, const uint8_t *andMask) {
    auto bmp = std::shared_ptr<uint32_t[]>(new uint32_t[bih->biWidth * bih->biHeight]);

    switch (bih->biBitCount) {
        case 1: {
            int j = bih->biWidth * (bih->biHeight - 1);
            for (auto y = 0, i = 0; y < bih->biHeight; y++) {
                for (auto x = 0; x < bih->biWidth; x++, i++, j++) {
                    const auto &c = colors[(xorMask[i / 8] & (0x80 >> (i % 8))) != 0];
                    bmp[j] = RGB(c.rgbBlue, c.rgbGreen, c.rgbRed);
                }
                j -= bih->biWidth * 2;
            }
        } break;

        case 4: {
            int j = bih->biWidth * (bih->biHeight - 1);
            for (auto y = 0, i = 0; y < bih->biHeight; y++) {
                for (auto x = 0; x < bih->biWidth; x += 2, i += 2, j += 2) {
                    auto val = (xorMask[i / 2] & 0xF0) >> 4;
                    const auto &c0 = colors[val];
                    bmp[j] = RGB(c0.rgbRed, c0.rgbGreen, c0.rgbBlue);

                    val = xorMask[i / 2] & 0x0F;
                    const auto &c1 = colors[val];
                    bmp[j + 1] = RGB(c1.rgbRed, c1.rgbGreen, c1.rgbBlue);
                }
                j -= bih->biWidth * 2;
            }
        } break;

        case 8: {
            int j = bih->biWidth * (bih->biHeight - 1);
            for (auto y = 0, i = 0; y < bih->biHeight; y++) {
                for (auto x = 0; x < bih->biWidth; x++, i++, j++) {
                    const auto &c = colors[xorMask[i]];
                    bmp[j] = RGB(c.rgbRed, c.rgbGreen, c.rgbBlue);
                }
                j -= bih->biWidth * 2;
            }
        }

        break;
    }

    auto andMaskSize = (bih->biWidth * bih->biHeight + 7) / 8;
    auto andMask2 = std::shared_ptr<uint8_t[]>(new uint8_t[andMaskSize]);

    // The masks are stored upside down in the file.
    // We flip them for easier handling.
    // TODO: handle odd number of lines / pixels?
    for (auto y = 0; y < bih->biHeight / 2; y++) {
        for (auto x = 0; x < bih->biWidth / 8; x++) {
            auto t1 = andMask[y * bih->biWidth / 8 + x];
            auto t2 = andMask[(bih->biHeight - y - 1) * bih->biWidth / 8 + x];
            andMask2[y * bih->biWidth / 8 + x] = t2;
            andMask2[(bih->biHeight - y - 1) * bih->biWidth / 8 + x] = t1;
        }
    }

    return CImageData::Create(bih->biWidth, bih->biHeight, cde->wXHotspot, cde->wYHotspot, bmp, andMask2);
}

bool CCursor::ParseImage(const CURSORDIRENTRY *cde) {
    printf("%dx%d colors=%d xh=%d yh=%d size=%u\n", cde->bWidth, cde->bHeight, cde->bColorCount, cde->wXHotspot,
           cde->wYHotspot, cde->lBytesInRes);

    auto imageData = GetPtr<uint8_t>(cde->dwImageOffset, cde->lBytesInRes);
    if (!imageData) {
        return false;
    }

    auto bih = GetPtr<BITMAPINFOHEADER>(cde->dwImageOffset);
    if (!bih) {
        return false;
    }

    if (bih->biSize != sizeof(*bih) || bih->biPlanes != 1) {
        return false;
    }

    if (bih->biCompression != BI_RGB) {
        return false;
    }

    printf(" %dx%d %dbpp clr=%d\n", bih->biWidth, bih->biHeight, bih->biBitCount, bih->biClrUsed);

    if (bih->biHeight != cde->bHeight * 2) {
        return false;
    }

    bih->biHeight = bih->biHeight / 2;

    if (cde->wXHotspot >= cde->bHeight || cde->wYHotspot >= cde->bWidth) {
        return false;
    }

    bool hasColorTable = bih->biCompression == BI_RGB && bih->biBitCount <= 8;

    auto curOffset = cde->dwImageOffset + sizeof(*bih);

    if (hasColorTable) {
        auto paletteEntriesCount = bih->biClrUsed == 0 ? 1u << bih->biBitCount : bih->biClrUsed;
        if (paletteEntriesCount != 1u << bih->biBitCount) {
            return false;
        }

        auto paletteSize = paletteEntriesCount * sizeof(RGBQUAD);
        auto palette = GetPtr<RGBQUAD>(curOffset, paletteSize);
        if (!palette) {
            return false;
        }

        curOffset += paletteSize;
        auto xorMaskSize = bih->biWidth * bih->biHeight / (8 / (bih->biBitCount));
        auto xorMask = GetPtr<uint8_t>(curOffset, xorMaskSize);
        if (!xorMask) {
            return false;
        }

        curOffset += xorMaskSize;
        auto andMaskSize = (bih->biWidth * bih->biHeight + 7) / 8;
        auto andMask = GetPtr<uint8_t>(curOffset, andMaskSize);
        if (!andMask) {
            return false;
        }

        auto image = ParseImageWithPalette(cde, bih, palette, xorMask, andMask);
        m_images.push_back(image);
    }

    return true;
}

bool CCursor::Parse() {
    auto hdr = GetPtr<ICONDIR>(0);
    if (!hdr) {
        return false;
    }

    if (hdr->cdReserved != 0 || hdr->cdType != CURSOR) {
        return false;
    }

    auto cdeOffset = sizeof(*hdr);
    for (auto i = 0; i < hdr->cdCount; ++i) {
        auto cde = GetPtr<CURSORDIRENTRY>(cdeOffset);
        if (!cde) {
            return false;
        }

        if (!ParseImage(cde)) {
            return false;
        }

        cdeOffset += sizeof(*cde);
    }

    return true;
}

std::shared_ptr<CCursor> CCursor::Create(const std::string &fileName) {
    size_t size;
    auto data = utils::ReadFile(fileName, size);
    if (!data) {
        return nullptr;
    }

    auto ret = std::shared_ptr<CCursor>(new CCursor(std::move(data), size));
    if (!ret->Parse()) {
        return nullptr;
    }

    return ret;
}

} // namespace gui

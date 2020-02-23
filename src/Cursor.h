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

#ifndef __GUI_CURSOR_H__

#define __GUI_CURSOR_H__

#include <inttypes.h>
#include <memory>
#include <vector>

namespace gui {

enum EIconType { ICON = 1, CURSOR = 2 };

struct ICONDIR {
    uint16_t cdReserved;
    uint16_t cdType; // EIconType
    uint16_t cdCount;
    // CURSORDIRENTRY cdEntries[];
} __attribute__((packed));

struct CURSORDIRENTRY {
    uint8_t bWidth;
    uint8_t bHeight;
    uint8_t bColorCount;
    uint8_t bReserved;
    uint16_t wXHotspot;
    uint16_t wYHotspot;
    uint32_t lBytesInRes;
    uint32_t dwImageOffset;
} __attribute__((packed));

struct ANIHeader {
    uint32_t cbSizeOf;           // Num bytes in AniHeader (36 bytes)
    uint32_t cFrames;            // Number of unique Icons in this cursor
    uint32_t cSteps;             // Number of Blits before the animation cycles
    uint32_t cx, cy;             // reserved, must be zero.
    uint32_t cBitCount, cPlanes; // reserved, must be zero.
    uint32_t JifRate;            // Default Jiffies (1/60th of a second) if rate chunk not present.
    uint32_t flags;              // Animation Flag (see AF_ constants)
} __attribute__((packed));

struct BITMAPINFOHEADER {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} __attribute__((packed));

enum BiCompression {
    BI_RGB = 0x0000,
    BI_RLE8 = 0x0001,
    BI_RLE4 = 0x0002,
    BI_BITFIELDS = 0x0003,
    BI_JPEG = 0x0004,
    BI_PNG = 0x0005,
    BI_CMYK = 0x000B,
    BI_CMYKRLE8 = 0x000C,
    BI_CMYKRLE4 = 0x000D
};

struct RGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} __attribute__((packed));

class CImageData;
class IFrameBuffer;
using CImageDataPtr = std::shared_ptr<CImageData>;

class CImageData {
private:
    int m_width, m_height;
    int m_xh, m_yh;
    std::shared_ptr<uint32_t[]> m_pixels;

    // TODO: encode the mask in the alpha component
    std::shared_ptr<uint8_t[]> m_and;

    CImageData(int width, int height, int xh, int yh, std::shared_ptr<uint32_t[]> pixels,
               std::shared_ptr<uint8_t[]> andMask) {
        m_width = width;
        m_height = height;
        m_xh = xh;
        m_yh = yh;
        m_pixels = pixels;
        m_and = andMask;
    }

public:
    static CImageDataPtr Create(int width, int height, int xh, int yh, std::shared_ptr<uint32_t[]> pixels,
                                std::shared_ptr<uint8_t[]> andMask) {
        return CImageDataPtr(new CImageData(width, height, xh, yh, pixels, andMask));
    }

    void Draw(IFrameBuffer &fb, int x, int y) const;

    int GetWidth() const {
        return m_width;
    }
    int GetHeight() const {
        return m_height;
    }
    int GetXHotspot() const {
        return m_xh;
    }
    int GetYHotspot() const {
        return m_yh;
    }
};

class CCursor {
private:
    // TODO: free this data once parsing is done
    std::unique_ptr<uint8_t[]> m_data;
    int m_size;

    std::vector<CImageDataPtr> m_images;

    // TODO: deduplicate with CPI.h
    template <typename T> T *GetPtr(uint32_t offset) {
        if (offset + sizeof(T) > (uint32_t) m_size) {
            return nullptr;
        }
        return reinterpret_cast<T *>(m_data.get() + offset);
    }

    template <typename T> T *GetPtr(uint32_t offset, uint32_t size) {
        if (offset + size > (uint32_t) m_size) {
            return nullptr;
        }
        return reinterpret_cast<T *>(m_data.get() + offset);
    }

    CCursor(std::unique_ptr<uint8_t[]> data, size_t size) : m_data(std::move(data)), m_size(size) {
    }

    CImageDataPtr ParseImageWithPalette(const CURSORDIRENTRY *cde, const BITMAPINFOHEADER *bih, const RGBQUAD *colors,
                                        const uint8_t *xorMask, const uint8_t *andMask);
    bool ParseImage(const CURSORDIRENTRY *cde);
    bool Parse();

public:
    static std::shared_ptr<CCursor> Create(const std::string &fileName);

    CImageDataPtr GetCursor() const {
        if (m_images.size() == 0) {
            return nullptr;
        }
        return m_images[0];
    }
};

} // namespace gui

#endif

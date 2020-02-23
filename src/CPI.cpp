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

#include <sstream>
#include <stdio.h>
#include <string.h>

#include "CPI.h"
#include "Framebuffer.h"
#include "Utils.h"

namespace gui {
namespace font {

std::shared_ptr<CCPIFont> CCPIFont::Create(const std::string &fileName) {
    std::shared_ptr<CCPIFont> ret;

    size_t size;
    auto data = utils::ReadFile(fileName, size);
    if (!data) {
        return nullptr;
    }

    ret = std::shared_ptr<CCPIFont>(new CCPIFont(std::move(data), size));
    if (!ret->Parse()) {
        return nullptr;
    }

    return ret;
}

std::vector<std::string> CCPIFont::GetFonts() const {
    std::vector<std::string> ret;
    for (auto &it : m_fonts) {
        ret.push_back(it.first);
    }
    return ret;
}

uint32_t CCPIFont::TranslateOffset(uint32_t offset, uint32_t newOffset) {
    if (m_type == FONT) {
        return newOffset;
    } else if (m_type == FONT_NT) {
        return offset + newOffset;
    } else {
        return 0;
    }
}

bool CCPIFont::ParseCodePage(const CPICPEntryHeader *cp, uint32_t cpOffset) {
    printf("Code page %d devtype=%d %8s\n", cp->cpeCodePageID, cp->cpeDevType, cp->cpeDevSubType);

    auto fdhOffset = TranslateOffset(cpOffset, cp->cpeOffset);
    if (!fdhOffset) {
        return false;
    }

    auto fdh = GetPtr<CPIFontDataHeader>(fdhOffset);
    if (!fdh) {
        return false;
    }

    if (fdh->fdhVersion != 1) {
        return false;
    }

    printf("  #fonts: %d size: %d\n", fdh->fdhFonts, fdh->fdhLength);

    auto sfhOffset = fdhOffset + sizeof(*fdh);
    for (int i = 0; i < fdh->fdhFonts; ++i) {
        auto sfh = GetPtr<CPIScreenFontHeader>(sfhOffset);
        if (!sfh) {
            return false;
        }

        if (sfh->sfhCharacters != 256) {
            return false;
        }

        printf("     %2dx%2d #chars: %d\n", sfh->sfhWidth, sfh->sfhHeight, sfh->sfhCharacters);

        auto bmpOffset = sfhOffset + sizeof(*sfh);
        auto charSize = sfh->sfhHeight * ((sfh->sfhWidth + 7) / 8);
        auto bmpLength = sfh->sfhCharacters * charSize;

        // We don't support fonts that are not 8 pixels large
        if (sfh->sfhWidth != 8) {
            continue;
        }

        std::stringstream ss;
        ss << cp->cpeCodePageID << '_' << (int) sfh->sfhWidth << 'x' << (int) sfh->sfhHeight;

        FontInfo fi;
        fi.Width = sfh->sfhWidth;
        fi.Height = sfh->sfhHeight;
        fi.NumChars = sfh->sfhCharacters;
        fi.CharSize = charSize;
        fi.Bitmap = GetPtr<uint8_t>(bmpOffset, bmpLength);
        if (!fi.Bitmap) {
            return false;
        }
        m_fonts[ss.str()] = fi;

        sfhOffset = bmpOffset + bmpLength;
    }

    return true;
}

bool CCPIFont::Parse() {
    auto hdr = reinterpret_cast<CPIFontFileHeader *>(m_data.get());

    if (hdr->ffhFileTag[0] != 0xff) {
        return false;
    }

    if (hdr->ffhPointers != 1) {
        return false;
    }

    if (hdr->ffhPointerType != 1) {
        return false;
    }

    if (!::memcmp(&hdr->ffhFileTag[1], "FONT.NT", 7)) {
        m_type = FONT_NT;
    } else if (!::memcmp(&hdr->ffhFileTag[1], "FONT   ", 7)) {
        m_type = FONT;
    } else {
        return false;
    }

    auto fiHdr = GetPtr<CPIFontInfoHeader>(hdr->ffhOffset);
    if (!fiHdr) {
        return false;
    }

    auto numCodePages = fiHdr->fihCodePages;

    if (numCodePages == 0) {
        return true;
    }

    auto cpEntryHdrOffset = hdr->ffhOffset + sizeof(*fiHdr);

    int i = 0;
    while (i < numCodePages) {
        auto entry = GetPtr<CPICPEntryHeader>(cpEntryHdrOffset);
        if (!entry) {
            return false;
        }

        if (entry->cpeLength != sizeof(*entry)) {
            return false;
        }

        if (!ParseCodePage(entry, cpEntryHdrOffset)) {
            return false;
        }

        if (!entry->cpeNext) {
            break;
        }

        cpEntryHdrOffset = TranslateOffset(cpEntryHdrOffset, entry->cpeNext);
        if (!cpEntryHdrOffset) {
            return false;
        }

        ++i;
    }

    return true;
}

bool CCPIFont::PrintChar(const FontInfo &font, int y, uint8_t c) const {
    auto offset = font.CharSize * c;
    auto data = &font.Bitmap[offset];

    char line = data[y];
    for (auto x = font.Width - 1; x >= 0; --x) {
        if (line & (1 << x)) {
            printf("X");
        } else {
            printf(" ");
        }
    }

    return true;
}

bool CCPIFont::PrintString(const std::string &font, const std::string &text) const {
    auto it = m_fonts.find(font);
    if (it == m_fonts.end()) {
        return false;
    }

    for (int y = 0; y < it->second.Height; ++y) {
        for (auto c : text) {
            PrintChar(it->second, y, c);
        }
        printf("\n");
    }

    return true;
}

bool CCPIFont::RenderString(IFrameBuffer &fb, TPoint pos, const std::string &font, const std::string &text,
                            uint32_t color) const {
    auto it = m_fonts.find(font);
    if (it == m_fonts.end()) {
        return false;
    }

    auto &fi = it->second;

    auto starty = pos.y;

    for (int y = 0; y < it->second.Height; ++y) {
        auto startx = pos.x;
        for (uint8_t c : text) {
            auto offset = fi.CharSize * c;
            auto data = &fi.Bitmap[offset];

            char line = data[y];
            for (auto x = 0; x < fi.Width; ++x) {
                if (line & (1 << (fi.Width - x - 1))) {
                    fb.PutPixel(startx + x, starty + y, color);
                }
            }
            startx += fi.Width;
        }
    }

    return true;
}

} // namespace font
} // namespace gui

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

#ifndef __GUI_FONT_CPI_H__

#define __GUI_FONT_CPI_H__

#include <fstream>
#include <inttypes.h>
#include <ios>
#include <map>
#include <memory>
#include <vector>

#include "Framebuffer.h"

namespace gui {
namespace font {

// See format here: https://www.seasip.info/DOS/CPI/cpi.html
struct CPIFontFileHeader {
    uint8_t ffhFileTag[8];  // Font-file identifier (0ffh,"FONT   ")
    uint8_t ffhReserved[8]; // Reserved (must be 0)
    int16_t ffhPointers;    // No. of information pointers (= 1)
    uint8_t ffhPointerType; // Type of pointer (= 1)
    uint32_t ffhOffset;     // File offset of FontInfoHeader
} __attribute__((packed));

struct CPIFontInfoHeader {
    int16_t fihCodePages; // No. of code-page entries
} __attribute__((packed));

struct CPICPEntryHeader {
    int16_t cpeLength;        // Size of this structure, in uint8_ts (28)
    uint32_t cpeNext;         // Offset to next CPEntryHeader (last=0)
    int16_t cpeDevType;       // Device type (1=screen, 2=printer)
    uint8_t cpeDevSubType[8]; // Name of device & file (e.g. "EGA     ")
    int16_t cpeCodePageID;    // Code-page identifier
    uint8_t cpeReserved[6];   // Reserved (must be 0)
    uint32_t cpeOffset;       // Offset to font data for this code-page
} __attribute__((packed));

struct CPIFontDataHeader {
    int16_t fdhVersion; // 1=FONT 2=DRFONT
    int16_t fdhFonts;   // Number of fonts (max. 1 if printer)
    int16_t fdhLength;  // uint8_t size of font data
} __attribute__((packed));

struct CPIScreenFontHeader {
    uint8_t sfhHeight;     // Character height
    uint8_t sfhWidth;      // Character width
    uint8_t sfhRelHeight;  // Currently unused; must be 0
    uint8_t sfhRelWidth;   // Currently unused; must be 0
    int16_t sfhCharacters; // No. of characters defined in bitmap
} __attribute__((packed));

struct CPIPrintFontHeader {
    int16_t pfhSelType;    // Selection type for printer font
    int16_t pfhSeqLength;  // uint8_t size of control-sequence data
} __attribute__((packed)); //= 4 uint8_ts

class CCPIFont {
private:
    enum FontType { UNKNOWN, FONT, FONT_NT };
    struct FontInfo {
        int Width, Height;
        int NumChars;
        int CharSize;
        uint8_t *Bitmap;
    };

    std::unique_ptr<uint8_t[]> m_data;
    int m_size;

    FontType m_type;
    std::map<std::string, FontInfo> m_fonts;

    CCPIFont(std::unique_ptr<uint8_t[]> data, size_t size) : m_data(std::move(data)), m_size(size), m_type(UNKNOWN) {
    }

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

    uint32_t TranslateOffset(uint32_t offset, uint32_t newOffset);

    bool ParseCodePage(const CPICPEntryHeader *cp, uint32_t cpOffset);
    bool Parse();
    bool PrintChar(const FontInfo &font, int y, uint8_t c) const;

public:
    static std::shared_ptr<CCPIFont> Create(const std::string &fileName);
    std::vector<std::string> GetFonts() const;
    bool PrintString(const std::string &font, const std::string &text) const;
    bool RenderString(IFrameBuffer &fb, TPoint pos, const std::string &font, const std::string &text,
                      uint32_t color) const;
};

} // namespace font
} // namespace gui

#endif

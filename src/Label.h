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

#ifndef __GUI_LABEL_H__

#define __GUI_LABEL_H__

#include <string>

#include "Window.h"

namespace gui {

class CLabel;
using CLabelPtr = std::shared_ptr<CLabel>;

class CLabel : public CWindow {
private:
    std::string m_text;
    uint32_t m_textColor;
    bool m_verticalCenter;
    bool m_horizontalCenter;
    bool m_transparent;

public:
    const std::string &GetText() const {
        return m_text;
    }

    void SetText(const std::string &text) {
        m_text = text;
        SetDirty(true);
    }

    void SetTextColor(uint32_t color) {
        m_textColor = color;
        SetDirty(true);
    }

    void SetVCenter(bool b) {
        m_verticalCenter = b;
        SetDirty(true);
    }

    void SetHCenter(bool b) {
        m_horizontalCenter = b;
        SetDirty(true);
    }

    void SetTransparent(bool b) {
        m_transparent = b;
        SetDirty(true);
    }

    CLabel(const this_is_private &p, TRect rect) : CWindow(p, rect) {
        m_textColor = 0;
        m_verticalCenter = false;
        m_horizontalCenter = false;
        m_transparent = true;
    }

    static CLabelPtr Create(TRect rect) {
        return std::make_shared<CLabel>(this_is_private{0}, rect);
    }

    virtual void Draw(IFrameBuffer &fb) const;
};

} // namespace gui

#endif

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

#include "Button.h"
#include "Framebuffer.h"

namespace gui {

void CButton::Draw(IFrameBuffer &fb) const {
    auto width = GetWidth();
    auto height = GetHeight();
    auto rect = TRect(0, 0, width - 1, height - 1);

    if (m_pressed) {
        fb.DrawRect(rect, RGB(0xC0, 0xC0, 0xC0));
        fb.DrawEmptyRect(rect, 0);
        fb.DrawEmptyRect(TRect(1, 1, width - 2, height - 2), RGB(0xC0, 0xC0, 0xC0));
        fb.DrawEmptyRect(TRect(2, 2, width - 3, height - 3), RGB(0xA0, 0xA0, 0xA0));
    } else {
        fb.DrawRect(rect, RGB(0xC0, 0xC0, 0xC0));
        // Outer border
        fb.DrawHLine(TPoint(0, 0), width - 1, RGB(255, 255, 255));
        fb.DrawVLine(TPoint(0, 1), height - 2, RGB(255, 255, 255));

        fb.DrawHLine(TPoint(0, height - 1), width, RGB(0, 0, 0));
        fb.DrawVLine(TPoint(width - 1, 0), height - 1, RGB(0, 0, 0));

        // Inner border
        fb.DrawHLine(TPoint(1, height - 2), width - 2, RGB(128, 128, 128));
        fb.DrawVLine(TPoint(width - 2, 1), height - 3, RGB(128, 128, 128));
    }
}

void CButton::OnMouseButtonDownHandler(const MouseState &state, MouseButton button) {
    if (button == LEFT) {
        m_pressed = true;
        SetFocus();
        SetDirty(true);
    }
    CWindow::OnMouseButtonDownHandler(state, button);
}

void CButton::OnMouseButtonUpHandler(const MouseState &state, MouseButton button) {
    if (button == LEFT) {
        m_pressed = false;
        SetDirty(true);
    }
    CWindow::OnMouseButtonUpHandler(state, button);
}

void CButton::OnMouseOutHandler(const MouseState &state) {
    if (m_pressed) {
        m_pressed = false;
        SetDirty(true);
    }
    CWindow::OnMouseOutHandler(state);
}

} // namespace gui

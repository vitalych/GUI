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

#ifndef __GUI_BUTTON_H__

#define __GUI_BUTTON_H__

#include "Label.h"
#include "Window.h"

namespace gui {

class CButton;
using CButtonPtr = std::shared_ptr<CButton>;

class CButton : public CWindow {
private:
    CLabelPtr m_label;
    bool m_pressed;

    TRect GetLabelRect() const {
        return TRect(0, 0, GetWidth() - 1, GetHeight() - 1);
    }

    virtual void OnMouseButtonDownHandler(const MouseState &state, MouseButton button);
    virtual void OnMouseButtonUpHandler(const MouseState &state, MouseButton button);
    virtual void OnMouseOutHandler(const MouseState &state);

public:
    CButton(const this_is_private &p, TRect rect) : CWindow(p, rect) {
        m_pressed = false;
        m_label = CLabel::Create(GetLabelRect());
        m_label->SetText("Button");
        m_label->SetVCenter(true);
        m_label->SetHCenter(true);
        m_label->SetTextColor(0);

        InterceptChildEvents(true);
    }

    static CButtonPtr Create(TRect rect) {
        auto ret = std::make_shared<CButton>(this_is_private{0}, rect);
        ret->CWindow::AddChild(ret->m_label);
        return ret;
    }

    virtual void AddChild(const CWindowPtr &child) {
        // Can't add child to a button
    }

    virtual void Draw(IFrameBuffer &fb) const;

    virtual void SetRect(TRect r) {
        CWindow::SetRect(r);
        m_label->SetRect(GetLabelRect());
    }

    CLabelPtr GetLabel() const {
        return m_label;
    }
};

} // namespace gui

#endif

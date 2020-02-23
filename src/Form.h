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

#ifndef __GUI_FORM_H__

#define __GUI_FORM_H__

#include "Button.h"
#include "Framebuffer.h"
#include "Label.h"
#include "Rect.h"
#include "Window.h"

namespace gui {
class CForm;
using CFormPtr = std::shared_ptr<CForm>;

class CForm : public CWindow {
protected:
    CWindowPtr m_clientArea;
    CLabelPtr m_titleBar;
    CButtonPtr m_close;
    bool m_resizable;

    virtual void OnMouseDragHandler(const MouseState &state, int relx, int rely);
    virtual bool OnMouseBeginDragHandler(const MouseState &state);
    virtual void OnMouseMoveHandler(const MouseState &state);
    virtual void OnMouseButtonDownHandler(const MouseState &state, MouseButton button);

private:
    enum DragMode {
        NONE,
        MOVING,
        RESIZING_N,
        RESIZING_S,
        RESIZING_E,
        RESIZING_W,
        RESIZING_NW,
        RESIZING_NE,
        RESIZING_SW,
        RESIZING_SE
    };

    DragMode m_dragMode;

    DragMode GetResizingMode(int x, int y);

    TRect GetTitleBarRect() const {
        auto width = GetWidth();
        return TRect(TPoint(4, 4), TPoint(width - 5, 24));
    }

    TRect GetClientRect(int width, int height) const {
        return TRect(4, 25, 4 + width - 8 - 1, 25 + height - 29 - 1);
    }

    TRect GetClientRect() const {
        auto width = GetWidth();
        auto height = GetHeight();
        return GetClientRect(width, height);
    }

    TRect GetCloseRect() const {
        auto tb = GetTitleBarRect();
        auto h = tb.Height();
        tb.p0.x = tb.p1.x - h;
        tb.p0.y += 1;
        tb.p1.y -= 1;
        return tb;
    }

    void OnCloseHandler(const MouseState &) {
        SetVisible(false);
        OnClose.emit(this->shared_from_this());
    }

public:
    sigc::signal<void, CWindowPtr> OnClose;

    CForm(const this_is_private &p, TRect rect) : CWindow(p, rect) {
        m_dragMode = NONE;
        auto clientRect = GetClientRect(rect.Width(), rect.Height());
        m_clientArea = CWindow::Create(clientRect);
        m_clientArea->SetColor(RGB(128, 128, 128));

        auto tbr = GetTitleBarRect();
        tbr.p0.x += 4;
        m_titleBar = CLabel::Create(tbr);
        m_titleBar->SetText("Form");
        m_titleBar->SetVCenter(true);
        m_titleBar->SetTextColor(RGB(255, 255, 255));

        m_close = CButton::Create(GetCloseRect());
        m_close->Mouse.OnClick.connect(sigc::mem_fun(*this, &CForm::OnCloseHandler));
        auto lbl = m_close->GetLabel();
        lbl->SetText("X");

        m_resizable = true;

        InterceptChildEvents(true);
    }

    static CFormPtr Create(TRect rect) {
        auto ret = std::make_shared<CForm>(this_is_private{0}, rect);
        ret->CWindow::AddChild(ret->m_clientArea);
        ret->CWindow::AddChild(ret->m_titleBar);
        ret->CWindow::AddChild(ret->m_close);
        return ret;
    }

    virtual void AddChild(const CWindowPtr &child) {
        m_clientArea->AddChild(child);
        SetDirty(true);
    }

    virtual void Draw(IFrameBuffer &fb) const;

    virtual void SetRect(TRect r) {
        CWindow::SetRect(r);
        m_close->SetRect(GetCloseRect());
    }

    CLabelPtr GetTitleBar() const {
        return m_titleBar;
    }

    CWindowPtr GetClientArea() const {
        return m_clientArea;
    }

    void SetResizable(bool b) {
        m_resizable = b;
    }
};
} // namespace gui

#endif

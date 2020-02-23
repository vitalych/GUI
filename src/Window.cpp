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

#include "Window.h"
#include "Framebuffer.h"

namespace gui {

void CWindow::GetAbsoluteCoords(int &x, int &y) const {
    x = 0;
    y = 0;
    for (auto w = this; w != nullptr; w = w->GetParent().get()) {
        x += w->GetX();
        y += w->GetY();
    }
}

void CWindow::Draw(IFrameBuffer &fb) const {
    auto p0 = TPoint(0, 0);
    auto p1 = TPoint(GetWidth() - 1, GetHeight() - 1);
    fb.DrawRect(TRect(p0, p1), m_color);
}

void CWindow::SetFocus() {
    if (!m_parent) {
        return;
    }

    SetDirty(true);
    auto oldParent = m_parent;
    if (m_parent->FocusChild(this)) {
        oldParent->SetFocus();
    }
}

CWindowPtr WindowFromPoint(const CWindowPtr &root, int x, int y) {
    auto children = root->GetChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto child = *it;
        if (!child->Visible()) {
            continue;
        }

        int cx, cy;
        child->GetAbsoluteCoords(cx, cy);

        if (x >= cx && y >= cy && x < (cx + child->GetWidth()) && y < (cy + child->GetHeight())) {
            return WindowFromPoint(child, x, y);
        }
    }

    return root;
}

bool DrawWindow(IFrameBuffer &fb, TRect client, CWindow &wnd, bool parentDirty) {
    if (!wnd.Visible()) {
        return false;
    }

    int x, y;
    wnd.GetAbsoluteCoords(x, y);

    CClippedFrameBuffer cfb(fb, client);
    CTranslatedFrameBuffer tfb(cfb, TPoint(x, y));

    auto dirty = wnd.IsDirty() || parentDirty;
    if (dirty) {
        wnd.Draw(tfb);
    }

    auto thisRect = TRect(x, y, x + wnd.GetWidth() - 1, y + wnd.GetHeight() - 1);
    if (!client.ClipRect(thisRect)) {
        return false;
    }

    for (auto child : wnd.GetChildren()) {
        dirty |= DrawWindow(fb, thisRect, *child.get(), dirty);
    }

    wnd.SetDirty(false);
    return dirty;
}

} // namespace gui

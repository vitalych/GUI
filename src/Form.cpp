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

#include "Form.h"
#include "CPI.h"
#include "Framebuffer.h"
#include "WindowManager.h"

namespace gui {

void CForm::Draw(IFrameBuffer &fb) const {
    int width = m_rect.Width();
    int height = m_rect.Height();

    // Exterior border
    fb.DrawHLine(TPoint(0, 0), width - 1, RGB(200, 208, 212));
    fb.DrawVLine(TPoint(0, 1), height - 2, RGB(200, 208, 212));

    fb.DrawHLine(TPoint(0, height - 1), width, RGB(0, 0, 0));
    fb.DrawVLine(TPoint(width - 1, 0), height - 1, RGB(0, 0, 0));

    // Interior border white - dark-grey
    fb.DrawHLine(TPoint(1, 1), width - 3, RGB(255, 255, 255));
    fb.DrawVLine(TPoint(1, 2), height - 4, RGB(255, 255, 255));

    fb.DrawHLine(TPoint(1, height - 2), width - 2, RGB(128, 128, 128));
    fb.DrawVLine(TPoint(width - 2, 1), height - 3, RGB(128, 128, 128));

    // Client border - light grey - thickness 2
    fb.DrawHLine(TPoint(2, 2), width - 5, RGB(200, 208, 212));
    fb.DrawVLine(TPoint(2, 3), height - 6, RGB(200, 208, 212));

    fb.DrawHLine(TPoint(2, height - 3), width - 4, RGB(200, 208, 212));
    fb.DrawVLine(TPoint(width - 3, 2), height - 5, RGB(200, 208, 212));

    fb.DrawHLine(TPoint(3, 3), width - 7, RGB(200, 208, 212));
    fb.DrawVLine(TPoint(3, 4), height - 8, RGB(200, 208, 212));

    fb.DrawHLine(TPoint(3, height - 4), width - 6, RGB(200, 208, 212));
    fb.DrawVLine(TPoint(width - 4, 2), height - 6, RGB(200, 208, 212));

    // Title bar
    auto tb = GetTitleBarRect();
    fb.DrawRect(tb, RGB(0, 0, 255));
}

bool CForm::OnMouseBeginDragHandler(const MouseState &state) {
    int cx, cy;
    GetAbsoluteCoords(cx, cy);

    auto x = state.Position.x - cx;
    auto y = state.Position.y - cy;

    m_dragMode = NONE;

    auto tb = GetTitleBarRect();
    tb.p1.x -= 20;
    if (tb.Contains(x, y)) {
        m_dragMode = MOVING;
        return true;
    }

    m_dragMode = GetResizingMode(x, y);
    return m_dragMode != NONE;
}

CForm::DragMode CForm::GetResizingMode(int x, int y) {
    auto width = GetWidth();
    auto height = GetHeight();

    if (!m_resizable) {
        return NONE;
    }

    if (y < 4) {
        if (x < 4) {
            return RESIZING_NW;
        } else if (x > width - 5) {
            return RESIZING_NE;
        } else {
            return RESIZING_N;
        }
    } else if (y > height - 4) {
        if (x < 4) {
            return RESIZING_SW;
        } else if (x > width - 5) {
            return RESIZING_SE;
        } else {
            return RESIZING_S;
        }
    } else if (x < 4) {
        return RESIZING_W;
    } else if (x > width - 5) {
        return RESIZING_E;
    }

    return NONE;
}

void CForm::OnMouseDragHandler(const MouseState &state, int relx, int rely) {
    auto rect = GetRect();
    auto tb = GetTitleBarRect();
    auto minHeight = tb.p1.y + 4;
    auto minWidth = 16;

    switch (m_dragMode) {
        case NONE:
            break;
        case MOVING: {
            rect.p0.x += relx;
            rect.p1.x += relx;
            rect.p0.y += rely;
            rect.p1.y += rely;
            SetRect(rect);
        } break;
        case RESIZING_NW: {
            if (rect.p0.y + rely < rect.p1.y - minHeight) {
                if (rect.p0.x + relx < rect.p1.x - minWidth) {
                    rect.p0.x += relx;
                    rect.p0.y += rely;
                    SetRect(rect);
                }
            }
        } break;
        case RESIZING_N: {
            if (rect.p0.y + rely < rect.p1.y - minHeight) {
                rect.p0.y += rely;
                SetRect(rect);
            }
        } break;
        case RESIZING_NE: {
            if (rect.p0.y + rely < rect.p1.y - minHeight) {
                if (rect.p1.x + relx > rect.p0.x + minWidth) {
                    rect.p1.x += relx;
                    rect.p0.y += rely;
                    SetRect(rect);
                }
            }
        } break;
        case RESIZING_W: {
            if (rect.p0.x + relx < rect.p1.x - minWidth) {
                rect.p0.x += relx;
                SetRect(rect);
            }
        } break;
        case RESIZING_E: {
            if (rect.p1.x + relx > rect.p0.x + minWidth) {
                rect.p1.x += relx;
                SetRect(rect);
            }
        } break;
        case RESIZING_SW: {
            if (rect.p1.y + rely > rect.p0.y + minHeight) {
                if (rect.p0.x + relx < rect.p1.x - minWidth) {
                    rect.p0.x += relx;
                    rect.p1.y += rely;
                    SetRect(rect);
                }
            }
        } break;
        case RESIZING_S: {
            if (rect.p1.y + rely > rect.p0.y + minHeight) {
                rect.p1.y += rely;
                SetRect(rect);
            }
        } break;
        case RESIZING_SE: {
            if (rect.p1.y + rely > rect.p0.y + minHeight) {
                if (rect.p1.x + relx > rect.p0.x + minWidth) {
                    rect.p1.x += relx;
                    rect.p1.y += rely;
                    SetRect(rect);
                }
            }
        } break;
    }

    m_clientArea->SetRect(GetClientRect());

    CWindow::OnMouseDragHandler(state, relx, rely);
}

void CForm::OnMouseMoveHandler(const MouseState &state) {
    int cx, cy;
    GetAbsoluteCoords(cx, cy);

    auto x = state.Position.x - cx;
    auto y = state.Position.y - cy;

    auto resizingMode = GetResizingMode(x, y);
    ECursorType cursor;
    switch (resizingMode) {
        case RESIZING_N:
        case RESIZING_S:
            cursor = CURSOR_SIZE_NS;
            break;

        case RESIZING_E:
        case RESIZING_W:
            cursor = CURSOR_SIZE_WE;
            break;

        case RESIZING_NW:
        case RESIZING_SE:
            cursor = CURSOR_SIZE_NW;
            break;

        case RESIZING_NE:
        case RESIZING_SW:
            cursor = CURSOR_SIZE_NE;
            break;

        default:
            cursor = CURSOR_ARROW;
    }

    SetCursor(cursor);
    CWindow::OnMouseMoveHandler(state);
}

void CForm::OnMouseButtonDownHandler(const MouseState &state, MouseButton button) {
    if (button == LEFT) {
        SetFocus();
    }
    CWindow::OnMouseButtonDownHandler(state, button);
}

} // namespace gui

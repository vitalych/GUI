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

#include "WindowManager.h"
#include "Cursor.h"

namespace gui {

void CWindowManager::OnMoveHandler(const MouseState &state) {
    auto x = state.Position.x;
    auto y = state.Position.y;
    auto wnd = WindowFromPoint(m_desktop, x, y);

    if (wnd != m_prevWnd) {
        if (m_prevWnd) {
            propagateEvent(m_prevWnd, [&](CWindowPtr wnd) -> void { wnd->OnMouseOutHandler(state); });
        }
        m_prevWnd = wnd;
    }

    m_mouseDirty = true;
    m_mousePosition = state.Position;

    SetCursor(wnd->GetCursor());

    if (state.ButtonState.Left) {
        if (m_dragging) {
            if (m_dragWnd) {
                propagateEvent(m_dragWnd, [&](CWindowPtr wnd) -> void {
                    wnd->OnMouseDragHandler(state, x - m_dragOrigin.x, y - m_dragOrigin.y);
                });
                m_dragOrigin.x = x;
                m_dragOrigin.y = y;
            }
        }
    } else {
        propagateEvent(wnd, [&](CWindowPtr wnd) -> void { wnd->OnMouseMoveHandler(state); });
    }
}

void CWindowManager::OnButtonDownHandler(const MouseState &state, MouseButton b) {
    auto x = state.Position.x;
    auto y = state.Position.y;
    auto wnd = WindowFromPoint(m_desktop, x, y);

    if (b == LEFT) {
        propagateEvent(wnd, [&](CWindowPtr wnd) -> void {
            if (wnd->OnMouseBeginDragHandler(state)) {
                m_dragWnd = wnd;
                m_dragOrigin.x = x;
                m_dragOrigin.y = y;
                m_dragging = true;
            }
        });
    }

    propagateEvent(wnd, [&](CWindowPtr wnd) -> void { wnd->OnMouseButtonDownHandler(state, b); });
}

void CWindowManager::OnButtonUpHandler(const MouseState &state, MouseButton b) {
    auto x = state.Position.x;
    auto y = state.Position.y;
    auto wnd = WindowFromPoint(m_desktop, x, y);

    if (b == LEFT) {
        m_dragWnd = nullptr;
        m_dragging = false;
    }

    propagateEvent(wnd, [&](CWindowPtr wnd) -> void { wnd->OnMouseButtonUpHandler(state, b); });
}

bool CWindowManager::LoadCursors() {
    std::unordered_map<ECursorType, std::string> cursors = {{CURSOR_ARROW, "arrow.cur"},
                                                            {CURSOR_SIZE_NE, "size2_ne.cur"},
                                                            {CURSOR_SIZE_NS, "size2_ns.cur"},
                                                            {CURSOR_SIZE_NW, "size2_nw.cur"},
                                                            {CURSOR_SIZE_WE, "size2_we.cur"}};

    for (auto it : cursors) {
        auto cursorPath = m_resourcePath + "/cursors/" + it.second;
        auto cursor = CCursor::Create(cursorPath);
        if (!cursor) {
            printf("Could not load %s\n", cursorPath.c_str());
            return false;
        }
        m_cursors[it.first] = cursor;
    }

    m_cursor = m_cursors[CURSOR_ARROW];
    return true;
}

bool CWindowManager::LoadResources() {
    auto egaFont = m_resourcePath + "/fonts/ega.cpi";
    m_font = font::CCPIFont::Create(egaFont);
    if (!m_font) {
        printf("Could not read font %s\n", egaFont.c_str());
        return false;
    }

    auto fontNames = m_font->GetFonts();
    for (auto &name : fontNames) {
        printf("Available font %s\n", name.c_str());
    }

    if (!LoadCursors()) {
        return false;
    }

    return true;
}

bool CWindowManager::Draw(IFrameBuffer &fb, TRect &dirtyRect) {
    auto dirty = DrawWindow(fb, m_desktop->GetRect(), *m_desktop.get(), false);
    if (!dirty && !m_mouseDirty) {
        return false;
    }

    // TODO: handle hotspot
    auto cursor = m_cursor->GetCursor();
    if (cursor) {
        // Clear the old pointer first
        DrawWindow(fb, m_oldMouseRect, *m_desktop.get(), true);

        // Draw the new pointer
        auto x = m_mousePosition.x - cursor->GetXHotspot();
        auto y = m_mousePosition.y - cursor->GetYHotspot();
        TRect rect(x, y, x + cursor->GetWidth(), y + cursor->GetHeight());

        cursor->Draw(fb, x, y);
        dirtyRect = TRect::Union(m_oldMouseRect, rect);
        m_mouseDirty = false;
        m_oldMouseRect = rect;
    }

    if (dirty) {
        dirtyRect = m_desktop->GetRect();
    }

    return true;
}

bool CWindowManager::SetCursor(ECursorType type) {
    auto it = m_cursors.find(type);
    if (it == m_cursors.end()) {
        return false;
    }

    m_cursor = it->second;
    return true;
}

} // namespace gui

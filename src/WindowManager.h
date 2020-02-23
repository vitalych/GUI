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

#ifndef __GUI_WINDOWMGR_H__

#define __GUI_WINDOWMGR_H__

#include <unordered_map>

#include "CPI.h"
#include "Cursors.h"
#include "Window.h"

namespace gui {

class CCursor;
class CWindowManager;
using CWindowManagerPtr = std::shared_ptr<CWindowManager>;

class CWindowManager {
private:
    CMouseRawEventsPtr m_mouseRawEvents;
    CWindowPtr m_desktop;
    CWindowPtr m_dragWnd;
    CWindowPtr m_prevWnd;
    TPoint m_dragOrigin;
    bool m_dragging;
    bool m_mouseDirty;
    TPoint m_mousePosition;
    TRect m_oldMouseRect;
    std::string m_resourcePath;

    std::shared_ptr<font::CCPIFont> m_font;

    std::unordered_map<ECursorType, std::shared_ptr<CCursor>> m_cursors;
    std::shared_ptr<CCursor> m_cursor;

    CWindowManager(int width, int height, const std::string &resourcePath) {
        m_mouseRawEvents = CMouseRawEvents::Create();
        m_desktop = CWindow::Create(TRect(0, 0, width - 1, height - 1));
        m_dragWnd = nullptr;
        m_dragging = false;
        m_resourcePath = resourcePath;
        m_mouseRawEvents->OnButtonUp.connect(sigc::mem_fun(*this, &CWindowManager::OnButtonUpHandler));
        m_mouseRawEvents->OnButtonDown.connect(sigc::mem_fun(*this, &CWindowManager::OnButtonDownHandler));
        m_mouseRawEvents->OnMove.connect(sigc::mem_fun(*this, &CWindowManager::OnMoveHandler));
    }

    template <typename Wnd, typename Func> void propagateEvent(Wnd wnd, Func f) {
        f(wnd);

        while ((wnd = wnd->GetParent())) {
            if (wnd->InterceptChildEvents()) {
                f(wnd);
            }
        }
    }

    bool LoadCursors();
    bool LoadResources();

    void OnMoveHandler(const MouseState &state);
    void OnButtonDownHandler(const MouseState &state, MouseButton b);
    void OnButtonUpHandler(const MouseState &state, MouseButton b);

public:
    static CWindowManagerPtr Create(int width, int height, const std::string &resourcePath) {
        auto ret = CWindowManagerPtr(new CWindowManager(width, height, resourcePath));
        if (!ret->LoadResources()) {
            return nullptr;
        }
        return ret;
    }

    CMouseRawEventsPtr GetRawMouseEvents() const {
        return m_mouseRawEvents;
    }

    CWindowPtr GetDesktop() const {
        return m_desktop;
    }

    void Resize(int width, int height) {
        assert(width >= 0 && height >= 0);
        auto r = m_desktop->GetRect();
        r.p1.x = r.p0.x + width - 1;
        r.p1.y = r.p0.y + height - 1;
        m_desktop->SetRect(r);
    }

    bool Draw(IFrameBuffer &fb, TRect &dirtyRect);

    std::shared_ptr<font::CCPIFont> GetFont() {
        return m_font;
    }

    bool SetCursor(ECursorType type);
};
} // namespace gui

extern gui::CWindowManagerPtr g_wndMgr;

#endif

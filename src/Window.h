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

#ifndef __GUI_WINDOW_H__

#define __GUI_WINDOW_H__

#include <assert.h>
#include <inttypes.h>
#include <list>
#include <memory>
#include <stack>

#include "Cursors.h"
#include "Mouse.h"
#include "Rect.h"

namespace gui {

class CWindow;
class CFrameBuffer;
class IFrameBuffer;
class CWindowManager;
using CWindowPtr = std::shared_ptr<CWindow>;

class CWindow : public CMouseEventHandlers, public std::enable_shared_from_this<CWindow> {
    friend class CWindowManager;

public:
    using Children = std::list<CWindowPtr>;

protected:
    struct this_is_private {
        explicit this_is_private(int) {
        }
    };

    TRect m_rect;
    CWindowPtr m_parent;
    Children m_children;
    bool m_visible;
    uint32_t m_color;
    bool m_interceptChildEvents;
    bool m_dirty;
    ECursorType m_cursor;

public:
    CWindow(const this_is_private &p, TRect r) : m_rect(r) {
        assert(r.Valid());
        m_visible = true;
        m_color = 0;
        m_interceptChildEvents = false;
        m_dirty = true;
        m_cursor = CURSOR_ARROW;
    }

protected:
    bool HasFocus(const CWindow *child) const {
        if (m_children.size() == 0) {
            return false;
        }

        return (*m_children.rbegin()).get() == child;
    }

    bool FocusChild(CWindow *child) {
        if (!CWindow::RemoveChild(child)) {
            return false;
        }

        m_children.push_back(child->shared_from_this());
        return true;
    }

    virtual bool RemoveChild(CWindow *child) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            auto w = *it;
            if (w.get() == child) {
                m_children.erase(it);
                return true;
            }
        }
        return false;
    }

    bool HasChild(CWindow *child) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            auto w = *it;
            if (w.get() == child) {
                return true;
            }
        }
        return false;
    }

public:
    void SetColor(uint32_t color) {
        m_color = color;
        SetDirty(true);
    }

    void SetVisible(bool v) {
        bool redraw = v != m_visible;
        m_visible = v;
        if (redraw) {
            SetDirty(true);
            if (m_parent) {
                m_parent->SetDirty(true);
            }
        }
    }

    bool Visible() const {
        return m_visible;
    }

    void InterceptChildEvents(bool b) {
        m_interceptChildEvents = true;
    }

    bool InterceptChildEvents() const {
        return m_interceptChildEvents;
    }

    CWindowPtr GetParent() const {
        return m_parent;
    }

    int GetWidth() const {
        return m_rect.Width();
    }

    int GetHeight() const {
        return m_rect.Height();
    }

    int GetX() const {
        return m_rect.p0.x;
    }

    int GetY() const {
        return m_rect.p0.y;
    }

    const TRect &GetRect() const {
        return m_rect;
    }

    ECursorType GetCursor() const {
        return m_cursor;
    }

    void SetCursor(ECursorType c) {
        m_cursor = c;
    }

    virtual void SetRect(TRect r) {
        assert(r.Valid());
        m_rect = r;
        if (m_parent) {
            m_parent->SetDirty(true);
        }
        SetDirty(true);
    }

    bool IsDirty() const {
        return m_dirty;
    }

    void SetDirty(bool b) {
        m_dirty = b;
    }

    const Children &GetChildren() const {
        return m_children;
    }

    template <typename... T> static ::std::shared_ptr<CWindow> Create(T &&... args) {
        return ::std::make_shared<CWindow>(this_is_private{0}, ::std::forward<T>(args)...);
    }

    void GetAbsoluteCoords(int &x, int &y) const;

    virtual void Draw(IFrameBuffer &fb) const;

    virtual void SetFocus();

    bool HasFocus() const {
        if (!m_parent) {
            return true;
        }

        return m_parent->HasFocus(this);
    }

    virtual void AddChild(const CWindowPtr &child) {
        assert(!child->m_parent);
        assert(!HasChild(child.get()));
        child->m_parent = shared_from_this();
        m_children.push_back(child);
        SetDirty(true);
    }
};

CWindowPtr WindowFromPoint(const CWindowPtr &root, int x, int y);
bool DrawWindow(IFrameBuffer &fb, TRect client, CWindow &wnd, bool parentDirty);

} // namespace gui

#endif

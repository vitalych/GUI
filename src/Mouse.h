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

#ifndef __GUI_MOUSE_H__

#define __GUI_MOUSE_H__

#include <fsigc++/fsigc++.h>
#include <memory>

#include "Rect.h"

namespace gui {
class CMouseRawEvents;
class CMouseEvents;
class CMouseEventsGenerator;

using CMouseRawEventsPtr = std::shared_ptr<CMouseRawEvents>;
using CMouseEventsPtr = std::shared_ptr<CMouseEvents>;
using CMouseEventsGeneratorPtr = std::shared_ptr<CMouseEventsGenerator>;

enum MouseButton { UNKNOWN = 0, LEFT, MIDDLE, RIGHT };

struct MouseButtonState {
    bool Left, Middle, Right;
};

struct MouseState {
    TPoint Position;
    MouseButtonState ButtonState;
};

class CMouseRawEvents {
private:
    MouseState m_state;

public:
    sigc::signal<void, const MouseState &> OnMove;
    sigc::signal<void, const MouseState &, MouseButton> OnButtonDown;
    sigc::signal<void, const MouseState &, MouseButton> OnButtonUp;
    sigc::signal<void, const MouseState &> OnOut;
    sigc::signal<void, const MouseState &> OnClick;

    static CMouseRawEventsPtr Create() {
        return CMouseRawEventsPtr(new CMouseRawEvents());
    }
};

class CMouseEvents : public CMouseRawEvents {
public:
    sigc::signal<void, const MouseState &, int /* relx */, int /* rely */> OnDrag;

    static CMouseEventsPtr Create() {
        return CMouseEventsPtr(new CMouseEvents());
    }
};

class CMouseEventHandlers {
public:
    CMouseEvents Mouse;

protected:
    virtual void OnMouseMoveHandler(const MouseState &state) {
        Mouse.OnMove.emit(state);
    }

    virtual void OnMouseOutHandler(const MouseState &state) {
        Mouse.OnOut.emit(state);
    }

    virtual void OnMouseButtonDownHandler(const MouseState &state, MouseButton button) {
        Mouse.OnButtonDown.emit(state, button);
    }

    virtual void OnMouseButtonUpHandler(const MouseState &state, MouseButton button) {
        Mouse.OnButtonUp.emit(state, button);
        Mouse.OnClick.emit(state);
    }

    virtual void OnMouseDragHandler(const MouseState &state, int relx, int rely) {
        Mouse.OnDrag.emit(state, relx, rely);
    }

    virtual bool OnMouseBeginDragHandler(const MouseState &state) {
        return false;
    }
};

} // namespace gui

#endif

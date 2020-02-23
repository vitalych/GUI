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

#ifndef __GUI_RECT_H__

#define __GUI_RECT_H__

#include <assert.h>

namespace gui {
struct TPoint {
    int x, y;

    TPoint() : x(0), y(0) {
    }

    TPoint(int _x, int _y) : x(_x), y(_y) {
    }
};

struct TRect {
    TPoint p0, p1;

public:
    TRect() {
    }

    TRect(TPoint _p0, TPoint _p1) : p0(_p0), p1(_p1) {
        assert(Valid());
    }

    TRect(int x0, int y0, int x1, int y1) : p0(TPoint(x0, y0)), p1(TPoint(x1, y1)) {
    }

    enum Region { INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8 };

    inline bool Valid() const {
        return p1.x >= p0.x && p1.y >= p0.y;
    }

    bool ClipRect(TRect &r) const;

    int GetRegion(TPoint p) const;

    bool ClipStraightLine(TPoint &a, TPoint &b) const;

    bool ClipHLine(TPoint &p, int &width) const {
        if (width <= 0) {
            return false;
        }

        auto q = TPoint(p.x + width - 1, p.y);
        bool ret = ClipStraightLine(p, q);
        if (ret) {
            width = q.x - p.x + 1;
        }
        return ret;
    }

    bool ClipVLine(TPoint &p, int &height) const {
        if (height <= 0) {
            return false;
        }

        auto q = TPoint(p.x, p.y + height - 1);
        bool ret = ClipStraightLine(p, q);
        if (ret) {
            height = q.y - p.y + 1;
        }
        return ret;
    }

    inline int Width() const {
        return p1.x - p0.x + 1;
    }

    inline int Height() const {
        return p1.y - p0.y + 1;
    }

    inline bool Contains(int x, int y) const {
        return x >= p0.x && y >= p0.y && x <= p1.x && y <= p1.y;
    }

    static TRect Union(const TRect &a, const TRect &b);
};
} // namespace gui

#endif

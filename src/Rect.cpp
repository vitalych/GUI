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

#include "Rect.h"

namespace gui {

bool TRect::ClipRect(TRect &r) const {
    assert(Valid() && r.Valid());

    int r0 = GetRegion(r.p0);
    int r1 = GetRegion(r.p1);

    // Rectangle is invisible
    if (r0 & r1) {
        return false;
    }

    if (r.p0.x < p0.x) {
        r.p0.x = p0.x;
    }

    if (r.p0.y < p0.y) {
        r.p0.y = p0.y;
    }

    if (r.p1.x > p1.x) {
        r.p1.x = p1.x;
    }

    if (r.p1.y > p1.y) {
        r.p1.y = p1.y;
    }

    return true;
}

int TRect::GetRegion(TPoint p) const {
    int region = 0;
    if (p.x < p0.x) {
        region = region | LEFT;
    }
    if (p.x > p1.x) {
        region = region | RIGHT;
    }
    if (p.y < p0.y) {
        region = region | TOP;
    }
    if (p.y > p1.y) {
        region = region | BOTTOM;
    }
    return region;
}

bool TRect::ClipStraightLine(TPoint &a, TPoint &b) const {
    int r0 = GetRegion(a);
    int r1 = GetRegion(b);

    // Line is in bound
    if (!(r0 | r1)) {
        return true;
    }

    // Line is invisible
    if (r0 & r1) {
        return false;
    }

    // Clip the line
    if (a.x < p0.x) {
        a.x = p0.x;
    }

    if (a.y < p0.y) {
        a.y = p0.y;
    }

    if (b.x > p1.x) {
        b.x = p1.x;
    }

    if (b.y >= p1.y) {
        b.y = p1.y;
    }

    return true;
}

TRect TRect::Union(const TRect &a, const TRect &b) {
    TRect ret;

    ret.p0.x = a.p0.x < b.p0.x ? a.p0.x : b.p0.x;
    ret.p0.y = a.p0.y < b.p0.y ? a.p0.y : b.p0.y;
    ret.p1.x = a.p1.x > b.p1.x ? a.p1.x : b.p1.x;
    ret.p1.y = a.p1.y > b.p1.y ? a.p1.y : b.p1.y;

    return ret;
}

} // namespace gui

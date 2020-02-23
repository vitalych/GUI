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

#ifndef __GUI_GRIDLAYOUT_H__

#define __GUI_GRIDLAYOUT_H__

#include <vector>

#include "Window.h"

namespace gui {
class CGridLayout;
using CGridLayoutPtr = std::shared_ptr<CGridLayout>;
using GridLine = std::vector<CWindowPtr>;
using Grid = std::vector<GridLine>;

class CGridLayout : public CWindow {
private:
    unsigned m_cols, m_rows;
    Grid m_grid;

    // Current insertion index for AddChild()
    unsigned m_ccol, m_crow;

    bool ResizeGrid(unsigned cols, unsigned rows);
    TRect GetCellRect(unsigned col, unsigned row, unsigned colspan, unsigned rowspan) const;

public:
    CGridLayout(const this_is_private &p, TRect rect) : CWindow(p, rect) {
        m_cols = 0;
        m_rows = 0;
        m_ccol = 0;
        m_crow = 0;
    }

    static CGridLayoutPtr Create(TRect rect, unsigned cols, unsigned rows) {
        auto ret = std::make_shared<CGridLayout>(this_is_private{0}, rect);
        if (!ret->ResizeGrid(cols, rows)) {
            return nullptr;
        }

        return ret;
    }

    virtual void Draw(IFrameBuffer &fb) const;

    virtual void AddChild(const CWindowPtr &child);
    bool AddChild(const CWindowPtr &child, unsigned col, unsigned row, unsigned colspan = 1, unsigned rowspan = 1);
    virtual bool RemoveChild(CWindow *child);
};

} // namespace gui

#endif

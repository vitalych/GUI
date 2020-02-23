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

#include "GridLayout.h"

namespace gui {

void CGridLayout::Draw(IFrameBuffer &fb) const {
}

TRect CGridLayout::GetCellRect(unsigned col, unsigned row, unsigned colspan, unsigned rowspan) const {
    auto w = GetWidth();
    auto h = GetHeight();

    auto cellw = w / m_cols;
    auto cellh = h / m_rows;

    TRect ret;
    ret.p0 = TPoint(col * cellw, row * cellh);
    ret.p1 = TPoint(ret.p0.x + colspan * cellw - 1, ret.p0.y + rowspan * cellh - 1);
    return ret;
}

bool CGridLayout::ResizeGrid(unsigned cols, unsigned rows) {
    if (cols == 0 || rows == 0) {
        return false;
    }

    // Check first that we don't lose any children
    if (rows < m_rows || cols < m_cols) {
        for (auto r = rows; r < m_rows; ++r) {
            for (auto c = cols; c < m_cols; ++c) {
                if (m_grid[r][c]) {
                    return false;
                }
            }
        }
    }

    m_grid.resize(rows);
    for (auto &it : m_grid) {
        it.resize(cols);
    }

    m_cols = cols;
    m_rows = rows;

    return true;
}

void CGridLayout::AddChild(const CWindowPtr &child) {
    CWindow::AddChild(child);
    auto cell = GetCellRect(m_ccol, m_crow, 1, 1);
    child->SetRect(cell);
    assert(!m_grid[m_crow][m_ccol]);
    m_grid[m_crow][m_ccol] = child;

    ++m_ccol;
    if (m_ccol >= m_cols) {
        m_crow++;
        m_ccol = 0;
    }

    if (m_crow >= m_rows) {
        if (!ResizeGrid(m_cols, m_rows + 1)) {
            abort();
        }
    }
}

bool CGridLayout::AddChild(const CWindowPtr &child, unsigned col, unsigned row, unsigned colspan, unsigned rowspan) {
    if (!colspan || !rowspan) {
        return false;
    }

    if ((col + colspan) > m_cols || (row + rowspan) > m_rows) {
        return false;
    }

    if (m_grid[row][col]) {
        return false;
    }

    CWindow::AddChild(child);
    m_grid[row][col] = child;
    auto cell = GetCellRect(col, row, colspan, rowspan);
    child->SetRect(cell);
    return true;
}

bool CGridLayout::RemoveChild(CWindow *child) {
    if (!CWindow::RemoveChild(child)) {
        return false;
    }

    bool found = false;
    for (auto &row : m_grid) {
        for (auto &col : row) {
            if (col.get() == child) {
                col = nullptr;
                found = true;
            }
        }
    }

    assert(found);
    return true;
}

} // namespace gui

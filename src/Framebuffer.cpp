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

#include "Framebuffer.h"

namespace gui {

void IFrameBuffer::DrawEmptyRect(TRect r, uint32_t color) {
    DrawHLine(r.p0, r.Width(), color);
    DrawHLine(TPoint(r.p0.x, r.p1.y), r.Width(), color);
    DrawVLine(r.p0, r.Height(), color);
    DrawVLine(TPoint(r.p1.x, r.p0.y), r.Height(), color);
}

CFrameBuffer::CFrameBuffer(uint32_t *pixels, int width, int height, uint32_t pitch)
    : CFrameBufferBase(TRect(TPoint(0, 0), TPoint(width - 1, height - 1))) {
    m_pitch = pitch / sizeof(*m_pixels);
    assert((pitch % sizeof(*m_pixels)) == 0);
    assert((int) m_pitch >= m_rect.Width());

    if (pixels) {
        m_pixels = pixels;
        m_ownsPixels = false;
    } else {
        m_pixels = new uint32_t[m_pitch * height];
        m_ownsPixels = true;
    }
}

void CFrameBuffer::CopyRect(CFrameBufferPtr fb, TRect source, TRect dest) {
    // TODO: support resizing
    assert(source.Width() == dest.Width() && source.Height() == dest.Height());
    assert(source.Valid() && dest.Valid());

    // TODO: optimize this
    auto w = source.Width();
    auto h = source.Height();
    for (auto y = 0; y < h; ++y) {
        for (auto x = 0; x < w; ++x) {
            auto sourcePixel = fb->GetPixel(source.p0.x + x, source.p0.y + y);
            auto destPixel = GetPixel(dest.p0.x + x, dest.p0.y + y);
            destPixel = ApplyAlphaBlend(sourcePixel, destPixel);
            PutPixel(dest.p0.x + x, dest.p0.y + y, destPixel);
        }
    }
}

void CFrameBuffer::DrawRect(TRect r, uint32_t color) {
    if (!m_rect.ClipRect(r)) {
        return;
    }

    for (auto y = r.p0.y; y <= r.p1.y; ++y) {
        for (auto x = r.p0.x; x <= r.p1.x; ++x) {
            m_pixels[y * m_pitch + x] = color;
        }
    }
}

void CFrameBuffer::DrawHLine(TPoint p, int width, uint32_t color) {
    if (!m_rect.ClipHLine(p, width)) {
        return;
    }

    for (int xi = p.x; xi < p.x + width; ++xi) {
        m_pixels[p.y * m_pitch + xi] = color;
    }
}

void CFrameBuffer::DrawVLine(TPoint p, int height, uint32_t color) {
    if (!m_rect.ClipVLine(p, height)) {
        return;
    }

    for (int yi = p.y; yi < p.y + height; ++yi) {
        m_pixels[(yi) *m_pitch + p.x] = color;
    }
}

} // namespace gui

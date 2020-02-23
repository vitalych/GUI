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

#ifndef __GUI_FRAMEBUFFER_H__

#define __GUI_FRAMEBUFFER_H__

#include <assert.h>
#include <inttypes.h>
#include <memory>

#include "Rect.h"

namespace gui {

class CFrameBuffer;
class IFrameBuffer;
using CFrameBufferPtr = std::shared_ptr<CFrameBuffer>;
using IFrameBufferPtr = std::shared_ptr<IFrameBuffer>;

static inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) {
    return 0xff << 24 | r << 16 | g << 8 | b;
}

static inline uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return a << 24 | r << 16 | g << 8 | b;
}

static inline uint8_t GetAlpha(uint32_t color) {
    return (uint8_t)(color >> 24);
}

static inline uint8_t GetRed(uint32_t color) {
    return (uint8_t)(color >> 16) & 0xff;
}

static inline uint8_t GetGreen(uint32_t color) {
    return (uint8_t)(color >> 8) & 0xff;
}

static inline uint8_t GetBlue(uint32_t color) {
    return (uint8_t)(color) &0xff;
}

static inline uint32_t ApplyAlphaBlend(uint32_t source, uint32_t dest) {
    auto sa = GetAlpha(source) / 255.0;
    auto sr = GetRed(source);
    auto sg = GetGreen(source);
    auto sb = GetBlue(source);

    auto da = GetAlpha(dest) / 255.0;
    auto dr = GetRed(dest);
    auto dg = GetGreen(dest);
    auto db = GetBlue(dest);

    auto outa = sa + da * (1 - sa);
    if (outa == 0.0) {
        return RGBA(0, 0, 0, 0);
    }

    return RGBA(sr * sa + dr * da * (1 - sa) / outa, sg * sa + dg * da * (1 - sa) / outa,
                sb * sa + db * da * (1 - sa) / outa, outa * 255);
}

class IFrameBuffer {
public:
    virtual void DrawRect(TRect r, uint32_t color) = 0;
    virtual void DrawHLine(TPoint p, int width, uint32_t color) = 0;
    virtual void DrawVLine(TPoint p, int height, uint32_t color) = 0;
    virtual void PutPixel(int x, int y, uint32_t color) = 0;
    virtual const TRect &GetRect() = 0;
    virtual void CopyRect(CFrameBufferPtr fb, TRect source, TRect dest) = 0;

    void DrawEmptyRect(TRect r, uint32_t color);
};

class CFrameBufferBase : public IFrameBuffer {
protected:
    TRect m_rect;

public:
    CFrameBufferBase(TRect rect) : m_rect(rect) {
        assert(rect.Valid());
    }

    virtual const TRect &GetRect() {
        return m_rect;
    }
};

class CFrameBuffer : public CFrameBufferBase {
private:
    uint32_t *m_pixels;
    uint32_t m_pitch;
    bool m_ownsPixels;

public:
    CFrameBuffer(uint32_t *pixels, int width, int height, uint32_t pitch);

    ~CFrameBuffer() {
        if (m_ownsPixels && m_pixels) {
            delete[] m_pixels;
        }
    }

    static CFrameBufferPtr Create(uint32_t *pixels, uint32_t width, uint32_t height, uint32_t pitch) {
        return CFrameBufferPtr(new CFrameBuffer(pixels, width, height, pitch));
    }

    virtual void PutPixel(int x, int y, uint32_t color) {
        if (m_rect.Contains(x, y)) {
            m_pixels[y * m_pitch + x] = color;
        }
    }

    inline uint32_t GetPixel(int x, int y) {
        if (m_rect.Contains(x, y)) {
            return m_pixels[y * m_pitch + x];
        }
        return 0;
    }

    virtual void DrawRect(TRect r, uint32_t color);
    virtual void DrawHLine(TPoint p, int width, uint32_t color);
    virtual void DrawVLine(TPoint p, int height, uint32_t color);
    virtual void CopyRect(CFrameBufferPtr fb, TRect source, TRect dest);

    inline void Fill(uint32_t color) {
        auto count = m_rect.Width() * m_rect.Height();
        for (auto i = 0; i < count; ++i) {
            m_pixels[i] = color;
        }
    }

    inline uint32_t Pitch() const {
        return m_pitch * sizeof(*m_pixels);
    }

    uint32_t *Pixels() const {
        return m_pixels;
    }
};

class CTranslatedFrameBuffer : public CFrameBufferBase {
private:
    IFrameBuffer &m_base;
    TPoint m_p;

public:
    CTranslatedFrameBuffer(IFrameBuffer &base, TPoint p) : CFrameBufferBase(base.GetRect()), m_base(base), m_p(p) {
    }

    virtual void DrawRect(TRect r, uint32_t color) {
        auto p0 = TPoint(r.p0.x + m_p.x, r.p0.y + m_p.y);
        auto p1 = TPoint(r.p1.x + m_p.x, r.p1.y + m_p.y);
        m_base.DrawRect(TRect(p0, p1), color);
    }

    virtual void CopyRect(CFrameBufferPtr fb, TRect source, TRect dest) {
        auto p0 = TPoint(dest.p0.x + m_p.x, dest.p0.y + m_p.y);
        auto p1 = TPoint(dest.p1.x + m_p.x, dest.p1.y + m_p.y);
        m_base.CopyRect(fb, source, TRect(p0, p1));
    }

    virtual void DrawHLine(TPoint p, int width, uint32_t color) {
        m_base.DrawHLine(TPoint(p.x + m_p.x, p.y + m_p.y), width, color);
    }

    virtual void DrawVLine(TPoint p, int height, uint32_t color) {
        m_base.DrawVLine(TPoint(p.x + m_p.x, p.y + m_p.y), height, color);
    }

    virtual void PutPixel(int x, int y, uint32_t color) {
        m_base.PutPixel(x + m_p.x, y + m_p.y, color);
    }
};

class CClippedFrameBuffer : public CFrameBufferBase {
private:
    IFrameBuffer &m_base;

public:
    CClippedFrameBuffer(IFrameBuffer &base, TRect rect) : CFrameBufferBase(rect), m_base(base) {
    }

    virtual void DrawRect(TRect r, uint32_t color) {
        if (!m_rect.ClipRect(r)) {
            return;
        }
        m_base.DrawRect(r, color);
    }

    virtual void CopyRect(CFrameBufferPtr fb, TRect source, TRect dest) {
        auto cdest = dest;
        if (!m_rect.ClipRect(cdest)) {
            return;
        }
        source.p0.x += cdest.p0.x - dest.p0.x;
        source.p0.y += cdest.p0.y - dest.p0.y;
        source.p1.x += cdest.p1.x - dest.p1.x;
        source.p1.y += cdest.p1.y - dest.p1.y;

        m_base.CopyRect(fb, source, cdest);
    }

    virtual void DrawHLine(TPoint p, int width, uint32_t color) {
        if (!m_rect.ClipHLine(p, width)) {
            return;
        }
        m_base.DrawHLine(p, width, color);
    }

    virtual void DrawVLine(TPoint p, int height, uint32_t color) {
        if (!m_rect.ClipVLine(p, height)) {
            return;
        }
        m_base.DrawVLine(p, height, color);
    }

    virtual void PutPixel(int x, int y, uint32_t color) {
        if (m_rect.Contains(x, y)) {
            m_base.PutPixel(x, y, color);
        }
    }
};

} // namespace gui

#endif

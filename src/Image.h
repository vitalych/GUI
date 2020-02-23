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

#ifndef __GUI_IMAGE_H__

#define __GUI_IMAGE_H__

#include <string>

#include "Framebuffer.h"
#include "ImageLoader.h"
#include "Window.h"

namespace gui {

class CImage;
using CImagePtr = std::shared_ptr<CImage>;

class CImage : public CWindow {
private:
    CFrameBufferPtr m_image;

    bool m_verticalCenter;
    bool m_horizontalCenter;

    // For now, the image control has to be at least the same size as the actual image.
    // TODO: support arbitrary size.
    void ResizeRect() {
        if (!m_image) {
            return;
        }

        auto imageRect = m_image->GetRect();

        if (GetWidth() < imageRect.Width() || GetHeight() < imageRect.Height()) {
            auto rect = GetRect();
            rect.p1.x = rect.p0.x + imageRect.Width() - 1;
            rect.p1.y = rect.p0.y + imageRect.Height() - 1;
            SetRect(rect);
        }
    }

public:
    CImage(const this_is_private &p, TRect rect) : CWindow(p, rect) {
        m_verticalCenter = false;
        m_horizontalCenter = false;
    }

    void SetVCenter(bool b) {
        m_verticalCenter = b;
        SetDirty(true);
    }

    void SetHCenter(bool b) {
        m_horizontalCenter = b;
        SetDirty(true);
    }

    bool SetImage(const std::string &imagePath) {
        auto image = LoadImage(imagePath);
        if (!image) {
            return false;
        }

        m_image = image;
        ResizeRect();
        return true;
    }

    virtual void SetRect(TRect r) {
        CWindow::SetRect(r);
        ResizeRect();
    }

    static CImagePtr Create(TRect rect) {
        return std::make_shared<CImage>(this_is_private{0}, rect);
    }

    virtual void Draw(IFrameBuffer &fb) const;
};

} // namespace gui

#endif

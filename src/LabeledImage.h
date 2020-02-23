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

#ifndef __GUI_LABELEDIMAGE_H__

#define __GUI_LABELEDIMAGE_H__

#include "Image.h"
#include "Label.h"
#include "Window.h"

namespace gui {

class CLabeledImage;
using CLabeledImagePtr = std::shared_ptr<CLabeledImage>;

class CLabeledImage : public CWindow {
private:
    CLabelPtr m_label;
    CImagePtr m_image;
    bool m_transparent;

public:
    virtual ~CLabeledImage() {
    }

    CLabeledImage(const this_is_private &p, TRect rect, const std::string &imagePath) : CWindow(p, rect) {
        m_transparent = true;

        auto imageRect = TRect(0, 0, 63, 63);
        m_image = CImage::Create(imageRect);
        m_image->SetImage(imagePath);
        m_image->SetHCenter(true);
        m_image->SetVCenter(true);

        // The rectangle might have been resized if the image is bigger
        imageRect = m_image->GetRect();

        // Horizontally center the icon
        auto iw = imageRect.Width();
        imageRect.p0.x += (rect.Width() - iw) / 2;
        imageRect.p1.x += (rect.Width() - iw) / 2;
        m_image->SetRect(imageRect);

        // Center the label under the icon
        auto ly = imageRect.p1.y + 5;
        auto labelRect = TRect(0, ly, GetWidth() - 1, ly + 16);
        m_label = CLabel::Create(labelRect);
        m_label->SetText("Icon");
        m_label->SetHCenter(true);
        m_label->SetVCenter(true);

        InterceptChildEvents(true);
    }

    static CLabeledImagePtr Create(TRect rect, const std::string &imagePath) {
        auto ret = std::make_shared<CLabeledImage>(this_is_private{0}, rect, imagePath);
        ret->CWindow::AddChild(ret->m_label);
        ret->CWindow::AddChild(ret->m_image);
        return ret;
    }

    void SetTransparent(bool b) {
        m_transparent = b;
        SetDirty(true);
    }

    virtual void AddChild(const CWindowPtr &child) {
        // Can't add child to this control
    }

    virtual void Draw(IFrameBuffer &fb) const;

    virtual void SetRect(TRect r) {
        CWindow::SetRect(r);
        // TODO: fix this
        // m_label->SetRect(GetLabelRect());
    }

    CLabelPtr GetLabel() const {
        return m_label;
    }
};

} // namespace gui

#endif

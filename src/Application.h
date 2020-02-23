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

#ifndef __GUI_APPS_H__

#define __GUI_APPS_H__

#include <memory>
#include <string>
#include "Form.h"
#include "Window.h"

namespace gui {
class CApplication;
using CApplicationPtr = std::shared_ptr<CApplication>;

class CApplication {
public:
    virtual const std::string &GetName() const = 0;
    virtual const std::string &GetIconPath() const = 0;
    virtual CFormPtr GetMainWindow() const = 0;
};

class CCalculator;
using CCalculatorPtr = std::shared_ptr<CCalculator>;

CApplicationPtr CreateCalculator(const gui::CWindowPtr &desktop, const std::string &resourcePath);
} // namespace gui

#endif

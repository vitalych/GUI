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

#include <math.h>
#include <sstream>

#include "Application.h"
#include "Button.h"
#include "Form.h"
#include "GridLayout.h"
#include "Image.h"
#include "LabeledImage.h"
#include "Mouse.h"
#include "Window.h"

namespace gui {

class CCalculator;
using CCalculatorPtr = std::shared_ptr<CCalculator>;

class CCalculator : public CApplication {
private:
    enum Button {
        BTN_0 = 0,
        BTN_1 = 1,
        BTN_2 = 2,
        BTN_3 = 3,
        BTN_4 = 4,
        BTN_5 = 5,
        BTN_6 = 6,
        BTN_7 = 7,
        BTN_8 = 8,
        BTN_9 = 9,
        BTN_DOT = 10,
        BTN_PLUS,
        BTN_MINUS,
        BTN_MULT,
        BTN_DIV,
        BTN_EQUAL,
    };

    struct Entry {
        char Operation;
        double Number;
        Entry(char op) : Operation(op), Number(0.0) {
        }
        Entry(double n) : Operation(0), Number(n) {
        }
    };

    std::string m_appName;
    std::string m_iconPath;

    CFormPtr m_form;
    CLabelPtr m_lblResult;
    std::stack<Entry> m_data;

    static bool ParseNumber(const std::string &s, double &number) {
        auto cstr = s.c_str();
        char *endptr = nullptr;
        number = strtod(cstr, &endptr);
        if (endptr == cstr) {
            return false;
        }

        if (errno == ERANGE) {
            return false;
        }

        return true;
    }

    static int GetPrecedence(char op) {
        switch (op) {
            case '+':
                return 1;
            case '-':
                return 1;
            case '*':
                return 2;
            case '/':
                return 2;
            case '%':
                return 2;
            default:
                return 0;
        }
    }

    static double ApplyOp(double a, double b, char op) {
        switch (op) {
            case '+':
                return a + b;
            case '-':
                return a - b;
            case '*':
                return a * b;
            case '/':
                return a / b;
            case '%':
                return NAN; // TODO: fix this
            default:
                return NAN;
        }
    }

    bool Evaluate(double &result) {
        std::stack<double> values;
        std::stack<char> ops;

        while (!m_data.empty()) {
            auto e = m_data.top();
            m_data.pop();
            if (e.Operation == 0) {
                values.push(e.Number);
            } else {
                while (!ops.empty() && GetPrecedence(ops.top()) >= GetPrecedence(e.Operation)) {
                    auto op = ops.top();
                    ops.pop();
                    auto v1 = values.top();
                    values.pop();
                    auto v2 = values.top();
                    values.pop();
                    values.push(ApplyOp(v1, v2, op));
                }
                ops.push(e.Operation);
            }
        }

        while (!ops.empty()) {
            auto op = ops.top();
            ops.pop();
            auto v1 = values.top();
            values.pop();
            auto v2 = values.top();
            values.pop();
            values.push(ApplyOp(v1, v2, op));
        }

        if (values.size() != 1) {
            return false;
        }
        result = values.top();
        return true;
    }

    void Clear() {
        m_lblResult->SetText("0");
        m_data = std::stack<Entry>();
    }

    void OnClear(const MouseState &state) {
        Clear();
    }

    void OnButtonClick(const MouseState &mouse, Button c) {
        auto txt = m_lblResult->GetText();
        if (txt == "ERROR") {
            Clear();
        }

        if (c >= BTN_0 && c <= BTN_9) {
            txt = txt + (char) ('0' + c);
            if (txt[0] == '0') {
                txt = txt.substr(1, std::string::npos);
            }
            m_lblResult->SetText(txt);
        } else if (c == BTN_DOT) {
            if (txt.find('.') != std::string::npos) {
                return;
            }
            txt = txt + '.';
            m_lblResult->SetText(txt);
        } else if (c >= BTN_PLUS && c <= BTN_DIV) {
            double number;
            if (!ParseNumber(txt, number)) {
                m_lblResult->SetText("ERROR");
                return;
            }
            m_data.push(Entry(number));

            switch (c) {
                case BTN_PLUS:
                    m_data.push(Entry('+'));
                    break;
                case BTN_MINUS:
                    m_data.push(Entry('-'));
                    break;
                case BTN_MULT:
                    m_data.push(Entry('*'));
                    break;
                case BTN_DIV:
                    m_data.push(Entry('/'));
                    break;
                default:
                    m_lblResult->SetText("ERROR");
            }
            m_lblResult->SetText("0");
        } else if (c == BTN_EQUAL) {
            double number;
            if (!ParseNumber(txt, number)) {
                m_lblResult->SetText("ERROR");
                return;
            }
            m_data.push(Entry(number));

            // Evaluate
            double result;
            if (!Evaluate(result)) {
                m_lblResult->SetText("ERROR");
            }
            std::stringstream ss;
            ss << result;
            m_lblResult->SetText(ss.str());
        }
    }

public:
    CCalculator(const CWindowPtr &desktop, const std::string &resourcePath) {
        m_appName = "Calculator";
        m_iconPath = resourcePath + "/icons/calculator-64x64.png";

        auto width = 300;
        auto height = 400;

        // Center the window on the desktop
        // TODO: this should be the job of the desktop
        auto fwidth = desktop->GetWidth();
        auto fheight = desktop->GetHeight();

        auto p0 = TPoint((fwidth - width) / 2, (fheight - height) / 2);
        auto p1 = p0;
        p1.x += width - 1;
        p1.y += height - 1;

        m_form = CForm::Create(TRect(p0, p1));
        m_form->GetTitleBar()->SetText(GetName());
        m_form->SetResizable(false);
        auto clientRect = m_form->GetClientArea()->GetRect();

        auto grid = CGridLayout::Create(TRect(0, 0, clientRect.Width() - 1, clientRect.Height() - 1), 4, 6);
        m_form->AddChild(grid);

        auto dummy = TRect(0, 0, 10, 10);

        m_lblResult = CLabel::Create(dummy);
        m_lblResult->SetTransparent(false);
        m_lblResult->SetText("0");
        m_lblResult->SetColor(RGB(255, 255, 255));
        m_lblResult->SetHCenter(true);
        m_lblResult->SetVCenter(true);
        grid->AddChild(m_lblResult, 0, 0, 4, 1);

        // Create numbers
        auto i = 1;
        for (auto r = 4; r >= 2; --r) {
            for (auto c = 0; c < 3; ++c, ++i) {
                std::stringstream ss;
                ss << i;

                auto btn = CButton::Create(TRect(0, 0, 10, 10));
                btn->GetLabel()->SetText(ss.str());
                btn->Mouse.OnClick.connect(sigc::bind(sigc::mem_fun(*this, &CCalculator::OnButtonClick), (Button) i));
                grid->AddChild(btn, c, r);
            }
        }

        int numPadRowStart = 1;

        auto btnClear = CButton::Create(dummy);
        btnClear->GetLabel()->SetText("Clear");
        btnClear->Mouse.OnClick.connect(sigc::mem_fun(*this, &CCalculator::OnClear));
        grid->AddChild(btnClear, 0, numPadRowStart + 0);

        auto btnDiv = CButton::Create(dummy);
        btnDiv->GetLabel()->SetText("/");
        btnDiv->Mouse.OnClick.connect(sigc::bind(sigc::mem_fun(*this, &CCalculator::OnButtonClick), BTN_DIV));
        grid->AddChild(btnDiv, 1, numPadRowStart + 0);

        auto btnMult = CButton::Create(dummy);
        btnMult->GetLabel()->SetText("*");
        btnMult->Mouse.OnClick.connect(sigc::bind(sigc::mem_fun(*this, &CCalculator::OnButtonClick), BTN_MULT));
        grid->AddChild(btnMult, 2, numPadRowStart + 0);

        auto btnMinus = CButton::Create(dummy);
        btnMinus->GetLabel()->SetText("-");
        btnMinus->Mouse.OnClick.connect(sigc::bind(sigc::mem_fun(*this, &CCalculator::OnButtonClick), BTN_MINUS));
        grid->AddChild(btnMinus, 3, numPadRowStart + 0);

        auto btnPlus = CButton::Create(dummy);
        btnPlus->GetLabel()->SetText("+");
        btnPlus->Mouse.OnClick.connect(sigc::bind(sigc::mem_fun(*this, &CCalculator::OnButtonClick), BTN_PLUS));
        grid->AddChild(btnPlus, 3, numPadRowStart + 1, 1, 2);

        auto btnEqual = CButton::Create(dummy);
        btnEqual->GetLabel()->SetText("=");
        btnEqual->Mouse.OnClick.connect(sigc::bind(sigc::mem_fun(*this, &CCalculator::OnButtonClick), BTN_EQUAL));
        grid->AddChild(btnEqual, 3, numPadRowStart + 3, 1, 2);

        auto btnDot = CButton::Create(dummy);
        btnDot->GetLabel()->SetText(".");
        btnDot->Mouse.OnClick.connect(sigc::bind(sigc::mem_fun(*this, &CCalculator::OnButtonClick), BTN_DOT));
        grid->AddChild(btnDot, 2, numPadRowStart + 4);

        auto btnZero = CButton::Create(dummy);
        btnZero->GetLabel()->SetText("0");
        btnZero->Mouse.OnClick.connect(sigc::bind(sigc::mem_fun(*this, &CCalculator::OnButtonClick), BTN_0));
        grid->AddChild(btnZero, 0, numPadRowStart + 4, 2, 1);
    }

    virtual const std::string &GetName() const {
        return m_appName;
    }

    virtual const std::string &GetIconPath() const {
        return m_iconPath;
    }

    virtual CFormPtr GetMainWindow() const {
        return m_form;
    }
};

CApplicationPtr CreateCalculator(const CWindowPtr &desktop, const std::string &resourcePath) {
    return CCalculatorPtr(new CCalculator(desktop, resourcePath));
}

} // namespace gui

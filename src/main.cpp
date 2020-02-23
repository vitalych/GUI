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

#include <SDL.h>
#include <assert.h>
#include <chrono>
#include <inttypes.h>
#include <memory>
#include <sstream>

#include "Button.h"
#include "CPI.h"
#include "Cursor.h"
#include "Form.h"
#include "Framebuffer.h"
#include "GridLayout.h"
#include "LabeledImage.h"
#include "Mouse.h"
#include "Window.h"
#include "WindowManager.h"

#include "Application.h"

using namespace gui;
using namespace std::chrono;

static MouseButton GetSdlButton(int button) {
    switch (button) {
        case SDL_BUTTON_LEFT:
            return MouseButton::LEFT;
        case SDL_BUTTON_MIDDLE:
            return MouseButton::MIDDLE;
        case SDL_BUTTON_RIGHT:
            return MouseButton::RIGHT;
        default:
            return MouseButton::UNKNOWN;
    }
}

static MouseState GetMouseState() {
    MouseState ret;
    int x, y;
    auto buttons = SDL_GetMouseState(&x, &y);
    ret.Position.x = x;
    ret.Position.y = y;
    ret.ButtonState.Left = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
    ret.ButtonState.Middle = buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
    ret.ButtonState.Right = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
    return ret;
}

bool PollEvents(SDL_Event *Event, CMouseRawEvents &events, bool &needResizing) {
    auto mouseState = GetMouseState();

    switch (Event->type) {
        case SDL_QUIT:
            return true;
            break;

        case SDL_MOUSEBUTTONUP:
            events.OnButtonUp.emit(mouseState, GetSdlButton(Event->button.button));
            break;

        case SDL_MOUSEBUTTONDOWN:
            events.OnButtonDown.emit(mouseState, GetSdlButton(Event->button.button));
            break;

        case SDL_MOUSEMOTION:
            events.OnMove.emit(mouseState);
            break;

        case SDL_WINDOWEVENT: {
            if (Event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED || Event->window.event == SDL_WINDOWEVENT_RESIZED) {
                needResizing = true;
            }
        } break;

        default:
            printf("Unknown event %d\n", Event->type);
            break;
    }
    return false;
}

void UpdateFPS(SDL_Window *window, steady_clock::time_point &start, uint32_t &frameCount) {
    auto t2 = steady_clock::now();
    auto diff = t2 - start;
    auto ms = duration_cast<milliseconds>(diff).count();
    if (ms > 500) {
        start = t2;

        std::stringstream ss;
        ss << frameCount / (ms / 1000.0) << " FPS";
        auto str = ss.str();
        SDL_SetWindowTitle(window, str.c_str());
        start = t2;
        frameCount = 0;
    }

    ++frameCount;
}

CWindowManagerPtr g_wndMgr;

void OnIconClick(const MouseState &mouse, CLabeledImagePtr icon, CApplicationPtr app) {
    app->GetMainWindow()->SetVisible(true);
    icon->SetTransparent(false);
}

void OnAppClose(CWindowPtr form, CLabeledImagePtr icon, CApplicationPtr app) {
    icon->SetTransparent(true);
}

int main(int argc, char *argv[]) {
    int width = 1280;
    int height = 1024;

    if (argc != 2) {
        printf("Usage: %s /path/to/resources\n", argv[0]);
        return -1;
    }

    auto resourcePath = std::string(argv[1]);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window =
        SDL_CreateWindow("GUI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Surface *surface = nullptr;
    SDL_Texture *texture = nullptr;

    auto frameCount = 0u;
    auto lastPrinted = steady_clock::now();

    auto wndMgr = CWindowManager::Create(width, height, resourcePath);
    if (!wndMgr) {
        printf("Could not init window manager\n");
        return -1;
    }

    g_wndMgr = wndMgr;

    // MyApp::Create(wndMgr->GetDesktop());
    auto desktop = wndMgr->GetDesktop();
    desktop->SetColor(RGB(110, 200, 200));
    auto calc = CreateCalculator(desktop, resourcePath);

    auto imagePath = calc->GetIconPath();
    auto icon = CLabeledImage::Create(TRect(16, 16, 64 + 16 + 16, 64 + 16 + 5 + 16), imagePath);
    icon->GetLabel()->SetText(calc->GetName());
    icon->SetColor(RGB(140, 235, 242));
    icon->Mouse.OnClick.connect(sigc::bind(sigc::ptr_fun(&OnIconClick), icon, calc));
    desktop->AddChild(icon);
    desktop->AddChild(calc->GetMainWindow());
    calc->GetMainWindow()->OnClose.connect(sigc::bind(sigc::ptr_fun(&OnAppClose), icon, calc));

    SDL_ShowCursor(0);

    bool needResizing = true;

    do {
        SDL_Event Event;

        if (SDL_WaitEvent(&Event)) {
            if (PollEvents(&Event, *wndMgr->GetRawMouseEvents().get(), needResizing)) {
                break;
            }
        }

        if (needResizing) {
            if (texture) {
                SDL_DestroyTexture(texture);
            }

            surface = SDL_GetWindowSurface(window);

            SDL_GetWindowSize(window, &width, &height);
            texture = SDL_CreateTexture(renderer, surface->format->format, SDL_TEXTUREACCESS_STREAMING, width, height);

            wndMgr->Resize(surface->w, surface->h);
            needResizing = false;
        }

        TRect dirtyRect;

        void *pixels;
        int pitch;
        if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) < 0) {
            abort();
        }

        CFrameBuffer framebuffer((uint32_t *) pixels, width, height, pitch);

        auto dirty = wndMgr->Draw(framebuffer, dirtyRect);

        SDL_UnlockTexture(texture);

        if (dirty) {
            SDL_Rect srect;
            srect.x = dirtyRect.p0.x;
            srect.y = dirtyRect.p0.y;
            srect.h = dirtyRect.Height();
            srect.w = dirtyRect.Width();

            SDL_RenderCopy(renderer, texture, &srect, &srect);
            SDL_RenderPresent(renderer);
        }

        UpdateFPS(window, lastPrinted, frameCount);
    } while (true);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

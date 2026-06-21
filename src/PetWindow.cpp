#include "PetWindow.hpp"

#include <windowsx.h>
#include <cstring>
#include <algorithm>
#include <vector>

namespace {
    constexpr const wchar_t* kClassName = L"DesktopPet_PetWindow";
}

bool PetWindow::Create(HINSTANCE hInst, int width, int height) {
    m_Width  = width;
    m_Height = height;

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = &PetWindow::WndProcThunk;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // we paint via UpdateLayeredWindow
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc); // ignore failure if already registered

    const DWORD style   = WS_POPUP;
    const DWORD exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW;

    m_Hwnd = CreateWindowExW(
        exStyle, kClassName, L"DesktopPet",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInst, this);

    if (!m_Hwnd) return false;

    // Build the offscreen DIB once. 32 bpp top-down.
    HDC screenDC = GetDC(nullptr);
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth       = width;
    bmi.bmiHeader.biHeight      = -height; // negative = top-down
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    m_DibBitmap = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    m_DibPixels = static_cast<uint8_t*>(bits);

    m_MemDC = CreateCompatibleDC(screenDC);
    m_OldBitmap = SelectObject(m_MemDC, m_DibBitmap);

    ReleaseDC(nullptr, screenDC);

    ShowWindow(m_Hwnd, SW_SHOWNOACTIVATE);
    return true;
}

void PetWindow::Destroy() {
    if (m_MemDC) {
        if (m_OldBitmap) SelectObject(m_MemDC, m_OldBitmap);
        DeleteDC(m_MemDC);
        m_MemDC = nullptr;
    }
    if (m_DibBitmap) {
        DeleteObject(m_DibBitmap);
        m_DibBitmap = nullptr;
        m_DibPixels = nullptr;
    }
    if (m_Hwnd) {
        DestroyWindow(m_Hwnd);
        m_Hwnd = nullptr;
    }
}

void PetWindow::PositionBottomRight(int marginRight, int marginBottom) {
    RECT work{};
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    const int x = work.right  - m_Width  - marginRight;
    const int y = work.bottom - m_Height - marginBottom;
    SetWindowPos(m_Hwnd, HWND_TOPMOST, x, y, m_Width, m_Height,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void PetWindow::MoveToNextMonitor(int marginRight, int marginBottom) {
    struct Ctx { std::vector<HMONITOR> monitors; };
    Ctx ctx;
    EnumDisplayMonitors(nullptr, nullptr,
        [](HMONITOR mon, HDC, LPRECT, LPARAM lp) -> BOOL {
            reinterpret_cast<Ctx*>(lp)->monitors.push_back(mon);
            return TRUE;
        }, reinterpret_cast<LPARAM>(&ctx));

    if (ctx.monitors.size() <= 1) return;

    HMONITOR current = MonitorFromWindow(m_Hwnd, MONITOR_DEFAULTTONEAREST);
    auto it = std::find(ctx.monitors.begin(), ctx.monitors.end(), current);
    const size_t next = (it != ctx.monitors.end())
        ? (static_cast<size_t>(std::distance(ctx.monitors.begin(), it)) + 1) % ctx.monitors.size()
        : 0;

    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(ctx.monitors[next], &mi);

    const int x = mi.rcWork.right  - m_Width  - marginRight;
    const int y = mi.rcWork.bottom - m_Height - marginBottom;
    SetWindowPos(m_Hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}

void PetWindow::BeginFrame() {
    if (!m_DibPixels) return;
    std::memset(m_DibPixels, 0, static_cast<size_t>(m_Width) * m_Height * 4);
}

void PetWindow::BlitSprite(const uint8_t* src, int srcStride, int srcX, int srcY,
                            int frameWidth, int frameHeight,
                            int destX, int destY) {
    if (!m_DibPixels || !src) return;

    const int x0 = (destX < 0) ? 0 : destX;
    const int y0 = (destY < 0) ? 0 : destY;
    const int x1 = (destX + frameWidth  > m_Width)  ? m_Width  : destX + frameWidth;
    const int y1 = (destY + frameHeight > m_Height) ? m_Height : destY + frameHeight;

    for (int y = y0; y < y1; ++y) {
        const int sy = srcY + (y - destY);
        const uint8_t* srcRow = src + sy * srcStride;
        uint8_t* dstRow = m_DibPixels + y * (m_Width * 4);
        for (int x = x0; x < x1; ++x) {
            const int sx = srcX + (x - destX);
            const uint8_t* sp = srcRow + sx * 4;
            const uint8_t a = sp[3];
            if (a == 0) continue;
            // Premultiply source then Porter-Duff "over" onto existing content.
            const uint8_t pb = static_cast<uint8_t>((uint32_t)sp[2] * a / 255);
            const uint8_t pg = static_cast<uint8_t>((uint32_t)sp[1] * a / 255);
            const uint8_t pr = static_cast<uint8_t>((uint32_t)sp[0] * a / 255);
            const uint8_t inv_a = 255 - a;
            uint8_t* dp = dstRow + x * 4;
            dp[0] = static_cast<uint8_t>(pb + (uint32_t)dp[0] * inv_a / 255);
            dp[1] = static_cast<uint8_t>(pg + (uint32_t)dp[1] * inv_a / 255);
            dp[2] = static_cast<uint8_t>(pr + (uint32_t)dp[2] * inv_a / 255);
            dp[3] = static_cast<uint8_t>(a  + (uint32_t)dp[3] * inv_a / 255);
        }
    }
}

void PetWindow::EndFrame() {
    POINT srcPt{ 0, 0 };
    SIZE sz{ m_Width, m_Height };
    BLENDFUNCTION blend{};
    blend.BlendOp             = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat         = AC_SRC_ALPHA;

    HDC screenDC = GetDC(nullptr);
    UpdateLayeredWindow(m_Hwnd, screenDC, nullptr, &sz,
                        m_MemDC, &srcPt, 0, &blend, ULW_ALPHA);
    ReleaseDC(nullptr, screenDC);
}

LRESULT CALLBACK PetWindow::WndProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    PetWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<PetWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_Hwnd = hwnd;
    } else {
        self = reinterpret_cast<PetWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (self) return self->HandleMessage(msg, wp, lp);
    return DefWindowProcW(hwnd, msg, wp, lp);
}

LRESULT PetWindow::HitTest(int screenX, int screenY) const {
    if (!m_DibPixels) return HTTRANSPARENT;
    POINT pt{ screenX, screenY };
    if (!ScreenToClient(m_Hwnd, &pt)) return HTTRANSPARENT;
    if (pt.x < 0 || pt.y < 0 || pt.x >= m_Width || pt.y >= m_Height) return HTTRANSPARENT;
    // Alpha lives at byte 3 of the BGRA pixel.
    const uint8_t a = m_DibPixels[(pt.y * m_Width + pt.x) * 4 + 3];
    return (a > 16) ? HTCAPTION : HTTRANSPARENT;
}

LRESULT PetWindow::HandleMessage(UINT msg, WPARAM wp, LPARAM lp) {
    if (m_Handler && m_Handler(msg, wp, lp)) {
        return 0;
    }
    switch (msg) {
        case WM_NCHITTEST:
            return HitTest(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(m_Hwnd, msg, wp, lp);
}

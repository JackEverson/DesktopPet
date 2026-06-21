#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <functional>

// A borderless, always-on-top, per-pixel-alpha window for the pet sprite.
// The host loop calls Render() each frame with the source RGBA pixels of the
// current sprite frame; PetWindow converts to BGRA-premultiplied and pushes
// it onscreen via UpdateLayeredWindow.
class PetWindow {
public:
    using MessageHandler = std::function<bool(UINT msg, WPARAM, LPARAM)>;

    bool Create(HINSTANCE hInst, int width, int height);
    void Destroy();

    HWND Hwnd() const { return m_Hwnd; }

    // Multi-layer rendering: call BeginFrame, then BlitSprite once per layer
    // (back-to-front), then EndFrame to push to screen.
    void BeginFrame();
    void BlitSprite(const uint8_t* src, int srcStride, int srcX, int srcY,
                    int frameWidth, int frameHeight,
                    int destX, int destY);
    void EndFrame();

    void PositionBottomRight(int marginRight, int marginBottom);
    void MoveToNextMonitor(int marginRight, int marginBottom);

    void SetMessageHandler(MessageHandler h) { m_Handler = std::move(h); }

private:
    static LRESULT CALLBACK WndProcThunk(HWND, UINT, WPARAM, LPARAM);
    LRESULT HandleMessage(UINT, WPARAM, LPARAM);
    LRESULT HitTest(int screenX, int screenY) const;

    HWND     m_Hwnd       = nullptr;
    HDC      m_MemDC      = nullptr;
    HBITMAP  m_DibBitmap  = nullptr;
    HGDIOBJ  m_OldBitmap  = nullptr;
    uint8_t* m_DibPixels  = nullptr;
    int      m_Width      = 0;
    int      m_Height     = 0;

    MessageHandler m_Handler;
};

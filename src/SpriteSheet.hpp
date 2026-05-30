#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Horizontal-strip sprite sheet loaded as RGBA8 (channel order: R,G,B,A).
// If the PNG cannot be loaded, GenerateProcedural() can be used as a fallback
// so the application is fully usable without art assets.
class SpriteSheet {
public:
    bool Load(const std::string& path, int frameWidth, int frameHeight);
    void GenerateProcedural(int frameWidth, int frameHeight, int frameCount);

    int FrameWidth() const  { return m_FrameWidth; }
    int FrameHeight() const { return m_FrameHeight; }
    int FrameCount() const  { return m_FrameCount; }
    int SheetWidth() const  { return m_SheetWidth; }
    int SheetHeight() const { return m_SheetHeight; }

    // Stride is bytes per row of the whole sheet (= sheetWidth * 4).
    int Stride() const { return m_SheetWidth * 4; }

    // Pointer to the top-left pixel of the given frame inside m_Pixels.
    const uint8_t* FramePixels(int frameIndex) const;

private:
    std::vector<uint8_t> m_Pixels; // RGBA, row-major, top-down
    int m_SheetWidth  = 0;
    int m_SheetHeight = 0;
    int m_FrameWidth  = 0;
    int m_FrameHeight = 0;
    int m_FrameCount  = 0;
};

#include "SpriteSheet.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>
#include <cmath>
#include <cstring>

bool SpriteSheet::Load(const std::string& path, int frameWidth, int frameHeight) {
    int w = 0, h = 0, n = 0;
    stbi_uc* data = stbi_load(path.c_str(), &w, &h, &n, 4);
    if (!data) return false;
    if (h != frameHeight || w % frameWidth != 0) {
        stbi_image_free(data);
        return false;
    }

    m_SheetWidth  = w;
    m_SheetHeight = h;
    m_FrameWidth  = frameWidth;
    m_FrameHeight = frameHeight;
    m_FrameCount  = w / frameWidth;
    m_Pixels.assign(data, data + (w * h * 4));
    stbi_image_free(data);
    return true;
}

namespace {
    // Place a single RGBA pixel into the sheet at (x,y).
    inline void PutPixel(std::vector<uint8_t>& buf, int sheetW, int x, int y,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        const size_t i = (static_cast<size_t>(y) * sheetW + x) * 4;
        buf[i + 0] = r;
        buf[i + 1] = g;
        buf[i + 2] = b;
        buf[i + 3] = a;
    }

    // Draw a filled circle of the given color into the given frame rect.
    void DrawCircle(std::vector<uint8_t>& buf, int sheetW,
                    int frameOriginX, int frameW, int frameH,
                    int cx, int cy, int radius,
                    uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        const int r2 = radius * radius;
        for (int y = 0; y < frameH; ++y) {
            for (int x = 0; x < frameW; ++x) {
                const int dx = x - cx;
                const int dy = y - cy;
                if (dx * dx + dy * dy <= r2) {
                    PutPixel(buf, sheetW, frameOriginX + x, y, r, g, b, a);
                }
            }
        }
    }
}

void SpriteSheet::GenerateProcedural(int frameWidth, int frameHeight, int frameCount) {
    m_FrameWidth  = frameWidth;
    m_FrameHeight = frameHeight;
    m_FrameCount  = std::max(1, frameCount);
    m_SheetWidth  = frameWidth * m_FrameCount;
    m_SheetHeight = frameHeight;
    m_Pixels.assign(static_cast<size_t>(m_SheetWidth) * m_SheetHeight * 4, 0);

    // Fallback art: a single magenta blob with two dark eyes, so the program
    // is visibly alive when sushi.png is missing.
    const int cx = frameWidth  / 2;
    const int cy = frameHeight / 2;
    const int radius = std::min(frameWidth, frameHeight) / 2 - 4;
    DrawCircle(m_Pixels, m_SheetWidth, 0, frameWidth, frameHeight,
               cx, cy, radius, 240, 100, 160, 255);
    DrawCircle(m_Pixels, m_SheetWidth, 0, frameWidth, frameHeight,
               cx - 8, cy - 6, 2, 30, 30, 30, 255);
    DrawCircle(m_Pixels, m_SheetWidth, 0, frameWidth, frameHeight,
               cx + 8, cy - 6, 2, 30, 30, 30, 255);
}

const uint8_t* SpriteSheet::FramePixels(int frameIndex) const {
    if (frameIndex < 0 || frameIndex >= m_FrameCount) return nullptr;
    return m_Pixels.data() + (frameIndex * m_FrameWidth * 4);
}

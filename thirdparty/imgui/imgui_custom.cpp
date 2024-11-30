#include "imgui_custom.h"

ImVec4 RGBAtoIV4(int r, int g, int b, float a) {
    float newr = static_cast<float>(r) / 255.0f;
    float newg = static_cast<float>(g) / 255.0f;
    float newb = static_cast<float>(b) / 255.0f;
    float newa = a;
    return ImVec4(newr, newg, newb, newa);
}

uint32_t adjustTransparency(uint32_t color, uint8_t newTransparencyPercentage) {
    // Extract the RGB components
    uint8_t r = (color >> IM_COL32_R_SHIFT) & 0xFF;  // Red
    uint8_t g = (color >> IM_COL32_G_SHIFT) & 0xFF;  // Green
    uint8_t b = (color >> IM_COL32_B_SHIFT) & 0xFF;  // Blue

    return IM_COL32(r, g, b, newTransparencyPercentage * 255 / 100);
}

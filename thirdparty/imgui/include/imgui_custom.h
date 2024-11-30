#pragma once
#include <iostream>

#include "imgui.h"

ImVec4 RGBAtoIV4(int r, int g, int b, float a);
uint32_t adjustTransparency(uint32_t color, uint8_t newAlpha);
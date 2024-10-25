#include "imgui_custom.h"
#include <iostream>
ImVec4 RGBAtoIV4(int r, int g, int b, float a) noexcept
{
	float newr = static_cast<float>(r) / 255.0f;
	float newg = static_cast<float>(g) / 255.0f;
	float newb = static_cast<float>(b) / 255.0f;
	float newa = a;
	return ImVec4(newr, newg, newb, newa);
}
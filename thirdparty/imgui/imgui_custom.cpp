#include "imgui_custom.h"

ImVec4 RGBAtoIV4(int r, int g, int b, float a) noexcept
{
	float newr = r / 255;
	float newg = g / 255;
	float newb = b / 255;
	float newa = a;
	return ImVec4(newr, newg, newb, newa);
}
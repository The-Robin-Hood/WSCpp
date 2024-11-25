#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "stb_image.h"

namespace WSCpp::UI {
ImRect getItemRect();
ImRect rectExpanded(const ImRect& rect, float x, float y);
ImRect rectOffset(const ImRect& rect, float x, float y);
ImRect rectOffset(const ImRect& rect, ImVec2 xy);

bool beginMenubar(const ImRect& barRectangle);
void endMenubar();
bool loadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width,
                           int* out_height);
}  // namespace WSCpp::UI

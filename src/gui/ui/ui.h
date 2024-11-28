#include <GLFW/glfw3.h>

#include <map>
#include <string>
#include <vector>

#include "WSCLogger.h"
#include "imgui.h"
#include "imgui_custom.h"
#include "imgui_freetype.h"
#include "imgui_internal.h"
#include "stb_image.h"

#define WCHAR(x) (ImWchar)(x)  // cast a single character to ImWchar - to avoid warnings

namespace WSCpp {
	namespace UI {
		namespace Geometry {
			ImRect getItemRect();
			ImRect rectExpanded(const ImRect& rect, float x, float y);
			ImRect rectOffset(const ImRect& rect, float x, float y);
			ImRect rectOffset(const ImRect& rect, ImVec2 xy);
			void ShiftCursor(float x, float y);
			void ShiftCursorX(float distance);
			void ShiftCursorY(float distance);
		}  // namespace Geometry

		namespace Layout {
			void beginBaseLayout(ImFont* baseFont);
			void endBaseLayout();

			bool beginMenubar(const ImRect& barRectangle);
			void endMenubar();

		}  // namespace Layout

		namespace Resources {
			bool loadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture,
									   int* out_width, int* out_height);

			bool loadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width,
									 int* out_height);

			bool setupFonts(ImGuiIO& m_io,
							std::map<std::string, std::map<std::string, ImFont*>>& m_fonts,
							const std::string& m_fontsPath,
							const std::vector<std::vector<char>>& m_preLoadedfonts);

			bool setupLogo(const std::string& m_logoPath, const std::vector<char>& m_preLoadedlogo,
						   GLuint& m_logoTexture);
		}  // namespace Resources

		namespace Theme {
			void setup();
		}
	}  // namespace UI
}  // namespace WSCpp
#include "ui.h"

namespace WSCpp::UI::Resources {

	bool loadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture,
							   int* out_width, int* out_height) {
		// Load from file
		int image_width = 0;
		int image_height = 0;
		unsigned char* image_data = stbi_load_from_memory(
			(const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
		if (image_data == NULL) return false;

		// Create a OpenGL texture identifier
		GLuint image_texture;
		glGenTextures(1, &image_texture);
		glBindTexture(GL_TEXTURE_2D, image_texture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload pixels into texture
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
					 GL_UNSIGNED_BYTE, image_data);
		stbi_image_free(image_data);

		if (out_texture != nullptr) {
			glDeleteTextures(1, out_texture);
		}

		*out_texture = image_texture;
		*out_width = image_width;
		*out_height = image_height;

		return true;
	}

	bool loadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width,
							 int* out_height) {
		FILE* f = fopen(file_name, "rb");
		if (f == NULL) return false;
		fseek(f, 0, SEEK_END);
		size_t file_size = (size_t)ftell(f);
		if (file_size == -1) return false;
		fseek(f, 0, SEEK_SET);
		void* file_data = IM_ALLOC(file_size);
		fread(file_data, 1, file_size, f);
		bool ret = loadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
		IM_FREE(file_data);
		return ret;
	}

	bool setupFonts(ImGuiIO& m_io, std::map<std::string, std::map<std::string, ImFont*>>& m_fonts,
					const std::string& m_fontsPath,
					const std::vector<std::vector<char>>& m_preLoadedfonts) {
		WSCLog(debug, "Setting up fonts");
		std::vector<int> fontSizes = {12, 14, 16, 20, 24};
		std::vector<std::string> fontNames = {"NotoEmoji", "Inter-Regular", "Inter-SemiBold",
											  "Inter-Bold"};
		const ImWchar ranges[] = {WCHAR(0x0020),  WCHAR(0x00FF),  WCHAR(0x0100),  WCHAR(0x024F),
								  WCHAR(0x1F300), WCHAR(0x1F5FF), WCHAR(0x1F900), WCHAR(0x1F9FF),
								  WCHAR(0x2600),  WCHAR(0x26FF),  WCHAR(0x1F600), WCHAR(0x1F64F),
								  WCHAR(0x1F680), WCHAR(0x1F6FF), WCHAR(0x2700),  WCHAR(0x27BF),
								  WCHAR(0x2190),  WCHAR(0x21FF),  WCHAR(0x2B00),  WCHAR(0x2BFF),
								  WCHAR(0)};

		static ImFontConfig cfg;
		cfg.OversampleH = cfg.OversampleV = 1;
		cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
		cfg.FontDataOwnedByAtlas = false;
#ifdef __APPLE__
		cfg.RasterizerMultiply = 2.0f;
#endif
		for (int i = 0; i < fontNames.size(); i++) {
			for (auto& fontSize : fontSizes) {
#ifdef _WIN32
				ImFont* f =
					m_io.Fonts->AddFontFromMemoryTTF((void*)m_preLoadedfonts[i].data(),
													 static_cast<int>(m_preLoadedfonts[i].size()),
													 static_cast<float>(fontSize), &cfg, ranges);
#else
				std::string fontPath = m_fontsPath + "/" + fontNames[i] + ".ttf";
				ImFont* f =
					m_io.Fonts->AddFontFromFileTTF(fontPath.c_str(), fontSize, &cfg, ranges);
#endif
				if (!f) {
					WSCLog(error, "Failed to load font: " + fontNames[i]);
					return false;
				}
				m_fonts[fontNames[i]][std::to_string(fontSize)] =
					f;	// Saving the font for later use
			}
		}

		if (!m_io.Fonts->Build()) {
			WSCLog(error, "Failed to build fonts");
			return false;
		}
		return true;
	}

	bool setupLogo(const std::string& m_logoPath, const std::vector<char>& m_preLoadedlogo,
				   GLuint& m_logoTexture) {
		WSCLog(debug, "Setting up logo");
		int width, height;
#ifdef _WIN32
		if (!loadTextureFromMemory(m_preLoadedlogo.data(), m_preLoadedlogo.size(), &m_logoTexture,
								   &width, &height)) {
			WSCLog(error, "Failed to load texture from memory");
			return false;
		}
#else
		if (!loadTextureFromFile(m_logoPath.c_str(), &m_logoTexture, &width, &height)) {
			WSCLog(error, "Failed to load texture from file");
			return false;
		}
#endif
		return true;
	}

}  // namespace WSCpp::UI::Resources
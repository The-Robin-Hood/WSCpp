#include "ui.h"

namespace WSCpp::UI::Resources {

    Image::Image(BinaryData& data) {
        uint32_t width, height;
        void* decodedData = Decode(data.getData(), data.getSize(), width, height);
        if (decodedData) {
            m_Width = width;
            m_Height = height;
            m_texture = 0;
            loadTextureFromMemory(decodedData, m_Width, m_Height);
            free(decodedData);
        }
    }

    Image::~Image() { glDeleteTextures(1, &m_texture); }

    void* Image::Decode(const void* buffer, uint64_t length, uint32_t& outWidth,
                        uint32_t& outHeight) {
        int width, height, channels;
        uint8_t* data = nullptr;
        uint64_t size = 0;

        data =
            stbi_load_from_memory((const stbi_uc*)buffer, (int)length, &width, &height, &channels, 4);
        size = static_cast<uint64_t>(width * height * 4);

        outWidth = width;
        outHeight = height;

        return data;
    }

    void Image::loadTextureFromMemory(void* data, int image_width, int image_height) {
        // Create a OpenGL texture identifier
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload pixels into texture
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
    }

    bool setupFonts(ImGuiIO& m_io, std::map<std::string, std::map<std::string, ImFont*>>& m_fonts,
                    std::vector<std::shared_ptr<BinaryData>> m_preLoadedfonts) {
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
        cfg.RasterizerDensity = 2.0f;
#endif
        for (int i = 0; i < fontNames.size(); i++) {
            for (auto& fontSize : fontSizes) {
                ImFont* f = m_io.Fonts->AddFontFromMemoryTTF(
                    (void*)m_preLoadedfonts[i]->getData(), (int)m_preLoadedfonts[i]->getSize(),
                    static_cast<float>(fontSize), &cfg, ranges);
                if (!f) {
                    WSCLog(error, "Failed to load font: " + fontNames[i]);
                    return false;
                }
                m_fonts[fontNames[i]][std::to_string(fontSize)] =
                    f;  // Saving the font for later use
            }
        }

        if (!m_io.Fonts->Build()) {
            WSCLog(error, "Failed to build fonts");
            return false;
        }
        return true;
    }

    BinaryData::BinaryData(const unsigned char* data, size_t size) : size(size) {
        this->data = new unsigned char[size];
        std::memcpy(this->data, data, size);
    }

    BinaryData::~BinaryData() { delete[] data; }

}  // namespace WSCpp::UI::Resources
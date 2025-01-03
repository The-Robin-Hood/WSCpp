#pragma once
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
        inline std::map<std::string, std::map<std::string, ImFont*>>* fontsMap = nullptr;
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

            void DrawBorder(ImRect rect, float thickness, float rounding, float offsetX,
                            float offsetY, ImU32 color, ImU32 bgColor);

            void DrawIcon(const GLuint texture, float buttonWidth, float buttonHeight,
                          float iconWidth, float iconHeight);
        }  // namespace Layout

        namespace Resources {
            class BinaryData {
               public:
                BinaryData(const unsigned char* data, size_t size);
                ~BinaryData();

                const unsigned char* getData() const { return data; }
                size_t getSize() const { return size; }

               private:
                unsigned char* data;
                size_t size;
            };

            class Image {
               public:
                Image(BinaryData& data);
                ~Image();

                void loadTextureFromMemory(void* data, int image_width, int image_height);
                GLuint getTexture() const { return m_texture; }

                uint32_t GetWidth() const { return m_Width; }
                uint32_t GetHeight() const { return m_Height; }

                static void* Decode(const void* data, uint64_t length, uint32_t& outWidth,
                                    uint32_t& outHeight);

               private:
                GLuint m_texture;
                uint32_t m_Width = 0, m_Height = 0;
            };

            bool setupFonts(ImGuiIO& m_io,
                            std::map<std::string, std::map<std::string, ImFont*>>& m_fonts,
                            std::vector<std::shared_ptr<BinaryData>> m_preLoadedfonts);

        }  // namespace Resources

        namespace Theme {
            void setup();
        }

        namespace Colors {
            inline constexpr auto transparentColor = IM_COL32(0, 0, 0, 0);
            inline constexpr auto backgroundColor = IM_COL32(24, 24, 27, 255);
            inline constexpr auto foregroundColor = IM_COL32(255, 255, 255, 255);
            inline constexpr auto primaryColor = IM_COL32(250, 250, 250, 255);
            inline constexpr auto secondaryColor = IM_COL32(39, 39, 42, 255);
            inline constexpr auto disabledColor = IM_COL32(161, 161, 170, 255);
            inline constexpr auto destructiveColor = IM_COL32(239, 68, 68, 255);

            inline constexpr auto text_primaryColor = backgroundColor;
            inline constexpr auto text_destructiveColor = primaryColor;
            inline constexpr auto text_secondaryColor = primaryColor;

            inline constexpr auto menubarHeaderColor = IM_COL32(47, 47, 47, 255);
        }  // namespace Colors

        namespace Component {
            enum class variants {
                primary,
                secondary,
                destructive,
                outline,
                ghost,
            };
            enum class sizes { small, normal, large, icon };

            struct ButtonProps {
                const char* label;
                float frameRounding = 5.0f;
                ImVec2 framePadding = ImVec2(12.0f, 8.0f);
                variants variant = variants::primary;
                float frameBorderSize = variant == variants::outline ? 1.0f : 0.0f;
                ImU32 frameBgColor = variant == variants::outline ? Colors::secondaryColor
                                                                  : Colors::transparentColor;
                bool disabled = false;
            };
            struct InputProps {
                const char* label;
                bool labelVisibility = false;
                int labelSameline = 0;  // 0: left, 1: top
                const char* hint = "";
                std::string& inputText;
                ImVec2 size = ImVec2(200.0f, 100.0f);
                bool multiline = false;
                float frameRounding = 3.0f;
                ImVec2 framePadding = ImVec2(12.0f, 8.0f);
                float frameBorderSize = 1.0f;
                ImU32 frameBgColor = Colors::transparentColor;
                bool disabled = false;
                ImGuiInputTextFlags flags = 0;
            };
            struct AlertDialogProps {
                std::string id;
                std::string title;
                std::string message;
                std::string cancelButtonLabel = "Cancel";
                std::string confirmButtonLabel = "OK";
                variants confirmButtonVariant = variants::primary;
                variants cancelButtonVariant = variants::outline;
                std::function<void()> onConfirm = nullptr;
                std::function<void()> onCancel = nullptr;
            };

            void showDemoUIComponents();
            bool Button(const ButtonProps& props);
            void Input(const InputProps& props);
            void CheckBox(const char* label, bool* value);
            void AlertDialog(const AlertDialogProps& props);
        }  // namespace Component

    }  // namespace UI
}  // namespace WSCpp
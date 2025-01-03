#include "imgui_custom.h"
#include "ui.h"

namespace WSCpp::UI::Theme {

    void setup() {
        WSCLog(debug, "Setting up theme");
        // auto& style = ImGui::GetStyle();
        auto& colors = ImGui::GetStyle().Colors;

        colors[ImGuiCol_TextSelectedBg] =
            ImGui::ColorConvertU32ToFloat4(adjustTransparency(UI::Colors::primaryColor, 50));
        colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(UI::Colors::menubarHeaderColor);
        colors[ImGuiCol_HeaderHovered] =
            ImGui::ColorConvertU32ToFloat4(UI::Colors::menubarHeaderColor);
        colors[ImGuiCol_HeaderActive] =
            ImGui::ColorConvertU32ToFloat4(UI::Colors::menubarHeaderColor);
        colors[ImGuiCol_ModalWindowDimBg] =
            ImGui::ColorConvertU32ToFloat4(adjustTransparency(UI::Colors::backgroundColor, 75));

        colors[ImGuiCol_ButtonHovered] =
            ImGui::ColorConvertU32ToFloat4(adjustTransparency(UI::Colors::primaryColor, 0));
        colors[ImGuiCol_TabHovered] =
            ImGui::ColorConvertU32ToFloat4(adjustTransparency(UI::Colors::primaryColor, 25));
        colors[ImGuiCol_Tab] =
            ImGui::ColorConvertU32ToFloat4(adjustTransparency(UI::Colors::primaryColor, 0));
        colors[ImGuiCol_TabSelected] =
            ImGui::ColorConvertU32ToFloat4(adjustTransparency(UI::Colors::primaryColor, 25));
    }
}  // namespace WSCpp::UI::Theme

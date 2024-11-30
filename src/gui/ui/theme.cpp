#include "imgui_custom.h"
#include "ui.h"

namespace WSCpp::UI::Theme {

    void setup() {
        WSCLog(debug, "Setting up theme");
        auto& style = ImGui::GetStyle();
        auto& colors = ImGui::GetStyle().Colors;

        colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(UI::Colors::menubarHeaderColor);
        colors[ImGuiCol_HeaderHovered] =
            ImGui::ColorConvertU32ToFloat4(UI::Colors::menubarHeaderColor);
        colors[ImGuiCol_HeaderActive] =
            ImGui::ColorConvertU32ToFloat4(UI::Colors::menubarHeaderColor);
    }
}  // namespace WSCpp::UI::Theme

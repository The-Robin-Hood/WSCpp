#include "ui.h"

namespace WSCpp::UI::Theme {
    void setup() {
        WSCLog(debug, "Setting up theme");
        auto& style = ImGui::GetStyle();
        auto& colors = ImGui::GetStyle().Colors;

        colors[ImGuiCol_Button] = ImColor(80, 80, 80, 200);
        colors[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
        colors[ImGuiCol_ButtonActive] = ImColor(56, 56, 56, 150);
    }
}  // namespace WSCpp::UI::Theme

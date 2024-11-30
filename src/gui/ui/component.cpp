#include "ui.h"

namespace WSCpp::UI::Component {
    bool Button(const ButtonConfig& args) {
        const char* label = args.label;
        variants variant = args.variant;
        bool disabled = args.disabled;
        bool clicked = false;

        auto& style = ImGui::GetStyle();

        ImGui::BeginDisabled(disabled);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));
        switch (variant) {
            case variants::primary: {
                ImGui::PushStyleColor(ImGuiCol_Button, WSCpp::UI::Colors::primaryColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      adjustTransparency(WSCpp::UI::Colors::primaryColor, 90));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      adjustTransparency(WSCpp::UI::Colors::primaryColor, 90));
                ImGui::PushStyleColor(ImGuiCol_Text, WSCpp::UI::Colors::text_primaryColor);
                break;
            }
            case variants::secondary: {
                ImGui::PushStyleColor(ImGuiCol_Button, WSCpp::UI::Colors::secondaryColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      adjustTransparency(WSCpp::UI::Colors::secondaryColor, 90));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      adjustTransparency(WSCpp::UI::Colors::secondaryColor, 90));
                ImGui::PushStyleColor(ImGuiCol_Text, WSCpp::UI::Colors::text_secondaryColor);
                break;
            }
            case variants::destructive: {
                ImGui::PushStyleColor(ImGuiCol_Button, WSCpp::UI::Colors::destructiveColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                      adjustTransparency(WSCpp::UI::Colors::destructiveColor, 90));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                      adjustTransparency(WSCpp::UI::Colors::destructiveColor, 90));
                ImGui::PushStyleColor(ImGuiCol_Text, WSCpp::UI::Colors::text_destructiveColor);
                break;
            }
            case variants::outline: {
                style.FrameBorderSize = 1.0f;
                ImGui::PushStyleColor(ImGuiCol_Border, WSCpp::UI::Colors::secondaryColor);
                ImGui::PushStyleColor(ImGuiCol_Button, WSCpp::UI::Colors::transparentColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, WSCpp::UI::Colors::secondaryColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, WSCpp::UI::Colors::secondaryColor);
                ImGui::PushStyleColor(ImGuiCol_Text, WSCpp::UI::Colors::text_secondaryColor);
                break;
            }
            case variants::ghost: {
                ImGui::PushStyleColor(ImGuiCol_Button, WSCpp::UI::Colors::transparentColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, WSCpp::UI::Colors::secondaryColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, WSCpp::UI::Colors::secondaryColor);
                ImGui::PushStyleColor(ImGuiCol_Text, WSCpp::UI::Colors::text_secondaryColor);
                break;
            }
        }

        clicked = ImGui::Button(label);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
        if (variants::outline == variant) {
            style.FrameBorderSize = 0.0f;
            ImGui::PopStyleColor();
        }
        ImGui::EndDisabled();
        return clicked;
    }
}  // namespace WSCpp::UI::Component
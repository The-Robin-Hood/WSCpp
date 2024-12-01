#include "ui.h"

namespace WSCpp::UI::Component {
    bool Button(const ButtonProps& props) {
        bool clicked = false;

        auto& style = ImGui::GetStyle();

        ImGui::BeginDisabled(props.disabled);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));
        switch (props.variant) {
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

        clicked = ImGui::Button(props.label);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
        if (variants::outline == props.variant) {
            style.FrameBorderSize = 0.0f;
            ImGui::PopStyleColor();
        }
        ImGui::EndDisabled();
        return clicked;
    }

    void Input(const InputProps& props) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, props.frameRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, props.framePadding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, props.frameBorderSize);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::transparentColor);

        ImGui::BeginDisabled(props.disabled);
        ImGui::SetNextItemWidth(props.size.x);
        std::string hideLabel = "##" + std::string(props.label);
        std::replace(hideLabel.begin(), hideLabel.end(), ' ', '_');
        if (props.multiline && props.size.y > 0.0f) {
            ImGui::InputTextMultiline(
                hideLabel.c_str(), &props.inputText[0], props.inputText.capacity() + 1,
                ImVec2(props.size.x, (props.size.y != 0 ? ((ImGui::GetTextLineHeight() * 5) + 7)
                                                        : props.size.y)),
                props.flags);
        } else {
            ImGui::InputTextWithHint(hideLabel.c_str(), props.hint, &props.inputText[0],
                                     props.inputText.capacity() + 1, props.flags);
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor();
        ImGui::EndDisabled();
    }

    void AlertDialog(const AlertDialogProps& props) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x / 2, 0.0f),
                                 ImGuiCond_Appearing);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

        if (ImGui::BeginPopupModal(props.id.c_str(), NULL,
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoMove)) {
            
            ImGui::PushFont(fontsMap->at("Inter-Bold").at("20"));
            ImGui::TextWrapped("%s", props.title.c_str());
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            ImGui::PushStyleColor(ImGuiCol_Text, adjustTransparency(Colors::foregroundColor, 75));
            ImGui::PushFont(fontsMap->at("Inter-Regular").at("16"));
            ImGui::TextWrapped("%s", props.message.c_str());
            ImGui::PopFont();
            ImGui::PopStyleColor();
            ImGui::Dummy(ImVec2(0.0f, 16.0f));

            // Measure button sizes
            ImVec2 cancelButtonSize = ImGui::CalcTextSize(props.cancelButtonLabel.c_str());
            cancelButtonSize.x += ImGui::GetStyle().FramePadding.x * 2;
            cancelButtonSize.y += ImGui::GetStyle().FramePadding.y * 2;
            ImVec2 confirmButtonSize = ImGui::CalcTextSize(props.confirmButtonLabel.c_str());
            confirmButtonSize.x += ImGui::GetStyle().FramePadding.x * 2; 
            confirmButtonSize.y += ImGui::GetStyle().FramePadding.y * 2;

            float totalButtonWidth =
                cancelButtonSize.x + confirmButtonSize.x + ImGui::GetStyle().ItemSpacing.x;

            // Align buttons to the right of the window
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalButtonWidth -
                                 (ImGui::GetStyle().WindowPadding.x * 2) -
                                 (ImGui::GetStyle().ItemSpacing.x * 2));

            if (Button({.label = props.cancelButtonLabel.c_str(),
                        .variant = props.cancelButtonVariant})) {
                if (props.onCancel) props.onCancel();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (Button({.label = props.confirmButtonLabel.c_str(),
                        .variant = props.confirmButtonVariant})) {
                if (props.onConfirm) props.onConfirm();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);
    }

}  // namespace WSCpp::UI::Component
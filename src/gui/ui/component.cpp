#include "ui.h"
#include "imgui_stdlib.h"

namespace WSCpp::UI::Component {
    bool Button(const ButtonProps& props) {
        bool clicked = false;
        unsigned int buttonColor, buttonHoveredColor, textColor;

        ImGui::BeginDisabled(props.disabled);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, props.frameRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, props.framePadding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, props.frameBorderSize);

        switch (props.variant) {
            case variants::primary:
                buttonColor = WSCpp::UI::Colors::primaryColor;
                buttonHoveredColor = adjustTransparency(buttonColor, 90);
                textColor = WSCpp::UI::Colors::text_primaryColor;
                break;

            case variants::secondary:
                buttonColor = WSCpp::UI::Colors::secondaryColor;
                buttonHoveredColor = adjustTransparency(buttonColor, 90);
                textColor = WSCpp::UI::Colors::text_secondaryColor;
                break;

            case variants::destructive:
                buttonColor = WSCpp::UI::Colors::destructiveColor;
                buttonHoveredColor = adjustTransparency(buttonColor, 90);
                textColor = WSCpp::UI::Colors::text_destructiveColor;
                break;

            case variants::outline:
                buttonColor = WSCpp::UI::Colors::transparentColor;
                buttonHoveredColor = WSCpp::UI::Colors::secondaryColor;
                textColor = WSCpp::UI::Colors::text_secondaryColor;
                break;

            case variants::ghost:
                buttonColor = WSCpp::UI::Colors::transparentColor;
                buttonHoveredColor = WSCpp::UI::Colors::secondaryColor;
                textColor = WSCpp::UI::Colors::text_secondaryColor;
                break;
            default:
                throw std::runtime_error("Invalid button variant");
        }

        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        ImGui::PushStyleColor(ImGuiCol_Border, props.frameBgColor);

        clicked = ImGui::Button(props.label);

        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(3);
        ImGui::EndDisabled();

        return clicked;
    }

    void Input(const InputProps& props) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, props.frameRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, props.framePadding);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, props.frameBorderSize);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::transparentColor);

        if (props.labelVisibility) {
            if (props.labelSameline) {
                // padding for the label
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7);
                ImGui::Text("%s", props.label);
                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 7);
            } else {
                ImGui::Text("%s", props.label);
            }
        }

        ImGui::BeginDisabled(props.disabled);
        ImGui::SetNextItemWidth(props.size.x);
        std::string hideLabel = "##" + std::string(props.label);
        std::replace(hideLabel.begin(), hideLabel.end(), ' ', '_');
        if (props.multiline && props.size.y > 0.0f) {
            ImGui::InputTextMultiline(
                hideLabel.c_str(), &props.inputText,
                ImVec2(props.size.x, (props.size.y != 0 ? ((ImGui::GetTextLineHeight() * 5) + 7)
                                                        : props.size.y)),
                props.flags);
        } else {
            ImGui::InputTextWithHint(hideLabel.c_str(), props.hint, &props.inputText, props.flags);
        }

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor();
        ImGui::EndDisabled();
    }

    void CheckBox(const char* label, bool* value){
        ImGui::PushStyleColor(ImGuiCol_FrameBg, adjustTransparency(Colors::secondaryColor, 50));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, Colors::secondaryColor);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, Colors::secondaryColor);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, Colors::primaryColor);
        ImGui::Checkbox(label, value);
        ImGui::PopStyleColor(4);
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
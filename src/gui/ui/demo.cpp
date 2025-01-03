#include "ui.h"

namespace WSCpp::UI::Component {
    void showDemoUIComponents() {
        ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Demo Components", NULL, ImGuiWindowFlags_NoCollapse);

        if (ImGui::CollapsingHeader("Buttons")) {
            if (Button({.label = "Primary"})) {
                WSCLog(debug, "Primary Button Clicked");
            }
            if (Button({.label = "Destructive", .variant = variants::destructive})) {
                WSCLog(debug, "Destructive Button Clicked");
            }
            if (Button({.label = "Secondary", .variant = variants::secondary})) {
                WSCLog(debug, "Secondary Button Clicked");
            }
            if (Button({.label = "Outline", .variant = variants::outline})) {
                WSCLog(debug, "Outline Button Clicked");
            }
            if (Button({.label = "Ghost", .variant = variants::ghost})) {
                WSCLog(debug, "Ghost Button Clicked");
            }
            if (Button(
                    {.label = "Disabed", .variant = variants::primary, .disabled = true})) {
                WSCLog(debug, "Disabled Button Clicked");
            }
        }
        if (ImGui::CollapsingHeader("Inputs")) {
            std::string normalText = "";
            Input({.label = "Normal Input",
                   .hint = "Normal Input Field with hint text (default setting)",
                   .inputText = normalText,
                   .size = ImVec2(400.0f, 0.0f)});
        }

        if (ImGui::CollapsingHeader("Alert Dialog")) {
            if (Button({.label = "Delete", .variant = variants::destructive}))
                ImGui::OpenPopup("id");

            AlertDialog({.id = "id",
                         .title = "Delete all files?",
                         .message =
                             "Do you want to delete all files? This action cannot be undone. "
                             "Please confirm.",
                         .cancelButtonLabel = "Cancel",
                         .confirmButtonLabel = "Confirm",
                         .confirmButtonVariant = variants::destructive,
                         .onConfirm = []() { printf("Files deleted!\n"); },
                         .onCancel = []() { printf("Cancelled.\n"); }});
        }

        static bool testCheckbox = false;
        CheckBox("Checkbox", &testCheckbox);

        ImGui::End();
    }
}  // namespace WSCpp::UI::Component
#include "ui.h"

namespace WSCpp::UI {
ImRect getItemRect() { return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()); }

ImRect rectExpanded(const ImRect& rect, float x, float y) {
    ImRect result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

ImRect rectOffset(const ImRect& rect, float x, float y) {
    ImRect result = rect;
    result.Min.x += x;
    result.Min.y += y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

ImRect rectOffset(const ImRect& rect, ImVec2 xy) { return rectOffset(rect, xy.x, xy.y); }

bool beginMenubar(
    const ImRect& barRectangle) {  // modified version of imgui_internal.h BeginMenuBar
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;
    /*if (!(window->Flags & ImGuiWindowFlags_MenuBar))
        return false;*/

    IM_ASSERT(!window->DC.MenuBarAppending);
    ImGui::BeginGroup();
    ImGui::PushID("##menubar");

    const ImVec2 padding = window->WindowPadding;
    ImRect bar_rect = UI::rectOffset(barRectangle, 0.0f, padding.y);  // window->MenuBarRect();
    ImRect clip_rect(
        IM_ROUND(ImMax(window->Pos.x,
                       bar_rect.Min.x + window->WindowBorderSize + window->Pos.x - 10.0f)),
        IM_ROUND(bar_rect.Min.y + window->WindowBorderSize + window->Pos.y),
        IM_ROUND(ImMax(bar_rect.Min.x + window->Pos.x,
                       bar_rect.Max.x - ImMax(window->WindowRounding, window->WindowBorderSize))),
        IM_ROUND(bar_rect.Max.y + window->Pos.y));

    clip_rect.ClipWith(window->OuterRectClipped);
    ImGui::PushClipRect(clip_rect.Min, clip_rect.Max, false);
    window->DC.CursorPos = window->DC.CursorMaxPos =
        ImVec2(bar_rect.Min.x + window->Pos.x, bar_rect.Min.y + window->Pos.y);
    window->DC.LayoutType = ImGuiLayoutType_Horizontal;
    window->DC.IsSameLine = false;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
    window->DC.MenuBarAppending = true;
    ImGui::AlignTextToFramePadding();
    return true;
}

void endMenubar() {  // modified version of imgui_internal.h endMenuBar
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;
    ImGuiContext& g = *GImGui;
    if (ImGui::NavMoveRequestButNoResultYet() &&
        (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right) &&
        (g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu)) {
        ImGuiWindow* nav_earliest_child = g.NavWindow;
        while (nav_earliest_child->ParentWindow &&
               (nav_earliest_child->ParentWindow->Flags & ImGuiWindowFlags_ChildMenu))
            nav_earliest_child = nav_earliest_child->ParentWindow;
        if (nav_earliest_child->ParentWindow == window &&
            nav_earliest_child->DC.ParentLayoutType == ImGuiLayoutType_Horizontal &&
            (g.NavMoveFlags & ImGuiNavMoveFlags_Forwarded) == 0) {
            const ImGuiNavLayer layer = ImGuiNavLayer_Menu;
            IM_ASSERT(window->DC.NavLayersActiveMaskNext & (1 << layer));  // Sanity check
            ImGui::FocusWindow(window);
            ImGui::SetNavID(window->NavLastIds[layer], layer, 0, window->NavRectRel[layer]);
            ImGui::NavMoveRequestForward(g.NavMoveDir, g.NavMoveClipDir, g.NavMoveFlags,
                                         g.NavMoveScrollFlags);  // Repeat
        }
    }

    IM_MSVC_WARNING_SUPPRESS(6011);
    IM_ASSERT(window->DC.MenuBarAppending);
    ImGui::PopClipRect();
    ImGui::PopID();
    window->DC.MenuBarOffset.x =
        window->DC.CursorPos.x -
        window->Pos.x;  // Save horizontal position so next append can reuse it. This is kinda
                        // equivalent to a per-layer CursorPos.
    g.GroupStack.back().EmitItem = false;
    ImGui::EndGroup();  // Restore position on layer 0
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
    window->DC.MenuBarAppending = false;
}

bool loadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width,
                           int* out_height) {
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size,
                                                      &image_width, &image_height, NULL, 4);
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

    if(out_texture != nullptr) {
        glDeleteTextures(1, out_texture);
    }

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

}  // namespace WSCpp::UI
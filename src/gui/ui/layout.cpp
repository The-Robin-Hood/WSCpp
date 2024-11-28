#include "ui.h"

namespace WSCpp::UI::Layout {
	bool beginMenubar(
		const ImRect& barRectangle) {  // modified version of imgui_internal.h BeginMenuBar
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		IM_ASSERT(!window->DC.MenuBarAppending);
		ImGui::BeginGroup();
		ImGui::PushID("##menubar");

		const ImVec2 padding = window->WindowPadding;
		ImRect bar_rect =
			UI::Geometry::rectOffset(barRectangle, 0.0f, padding.y);  // window->MenuBarRect();
		ImRect clip_rect(IM_ROUND(ImMax(window->Pos.x, bar_rect.Min.x + window->WindowBorderSize +
														   window->Pos.x - 10.0f)),
						 IM_ROUND(bar_rect.Min.y + window->WindowBorderSize + window->Pos.y),
						 IM_ROUND(ImMax(bar_rect.Min.x + window->Pos.x,
										bar_rect.Max.x - ImMax(window->WindowRounding,
															   window->WindowBorderSize))),
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

	void endMenubar() {	 // modified version of imgui_internal.h endMenuBar
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
											 g.NavMoveScrollFlags);	 // Repeat
			}
		}

		IM_MSVC_WARNING_SUPPRESS(6011);
		IM_ASSERT(window->DC.MenuBarAppending);
		ImGui::PopClipRect();
		ImGui::PopID();
		window->DC.MenuBarOffset.x =
			window->DC.CursorPos.x -
			window->Pos.x;	// Save horizontal position so next append can reuse it. This is kinda
							// equivalent to a per-layer CursorPos.
		g.GroupStack.back().EmitItem = false;
		ImGui::EndGroup();	// Restore position on layer 0
		window->DC.LayoutType = ImGuiLayoutType_Vertical;
		window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
		window->DC.MenuBarAppending = false;
	}

	void beginBaseLayout(ImFont* baseFont) {
		ImGuiWindowFlags baseWindowFlags =
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, RGBAtoIV4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, RGBAtoIV4(21, 21, 21, 255));
		ImGui::Begin("BaseLayout", nullptr, baseWindowFlags);
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);

		ImGui::PushFont(baseFont);
	}

	void endBaseLayout() {
		ImGui::PopFont();
		ImGui::End();
	}

}  // namespace WSCpp::UI::Layout
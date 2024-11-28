#include "ui.h"
namespace WSCpp::UI::Geometry {
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

    void ShiftCursorX(float distance) { ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance); }

    void ShiftCursorY(float distance) { ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance); }

    void ShiftCursor(float x, float y) {
        const ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(cursor.x + x, cursor.y + y));
    }

}  // namespace WSCpp::UI::Geometry
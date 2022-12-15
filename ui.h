#pragma once
#include"imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
namespace Ui {

    extern ImFont* truetype_font;  // TrueType, Lato-Regular, 18pt (main font)
    extern ImFont* opentype_font;  // OpenType, Palatino Linotype, 17pt (sub font)
    extern ImFont* web_icon_font;  // Fork Awesome web icon font, 18pt

    void Init(void);


    bool NewInspector(void);
    void EndInspector(void);
    // scene-level helper functions
    void DrawVerticalLine(void);
    void DrawTooltip(const char* desc, float spacing = 5.0f);
    void DrawRainbowBar(const ImVec2& offset, float height);
    void PushRotation();
    void PopRotation(float radians, bool ccw = false);
}

#include"d3dApp.h"
#include"ui.h"
#include<string>
#include <imgui_internal.h>
#include <IconsForkAwesome.h>
namespace Ui {
    using namespace ImGui;
    ImFont* truetype_font;  // TrueType, Lato-Regular, 18pt (main font)
    ImFont* opentype_font;  // OpenType, Palatino Linotype, 17pt (sub font)
    ImFont* web_icon_font;  // Fork Awesome web icon font, 18pt

    // private global variables
    static ImVec2 window_center;
    static ImVec4 red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    static ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    static ImVec4 green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    static ImVec4 blue = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    static ImVec4 cyan = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);

    static const ImGuiColorEditFlags color3_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
    static const ImGuiWindowFlags invisible_window_flags = ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;
    static int rotation_start_index = 0;
    void Init() {
        using namespace ImGui;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = GetIO();
        ImGuiStyle& style = GetStyle();

        // window_center = ImVec2((float)Window::width, (float)Window::height) * 0.5f;

         // load fonts from the resource folder
        float fontsize_main = 18.0f;
        float fontsize_icon = 18.0f;  // bake icon font into the main font
        float fontsize_sub = 17.0f;

        std::string path = "";
        std::string ttf_main = path + "Lato.ttf";
        std::string ttf_sub = path + "palatino.ttf";
        std::string ttf_icon = path + FONT_ICON_FILE_NAME_FK;

        ImFontConfig config_main;
        config_main.PixelSnapH = true;
        config_main.OversampleH = 4;
        config_main.OversampleV = 4;
        config_main.RasterizerMultiply = 1.2f;  // brighten up the font to make them more readable
        config_main.GlyphExtraSpacing.x = 0.0f;

        ImFontConfig config_sub;
        config_sub.PixelSnapH = true;
        config_sub.OversampleH = 4;
        config_sub.OversampleV = 4;
        config_sub.RasterizerMultiply = 1.25f;  // brighten up the font to make them more readable
        config_sub.GlyphExtraSpacing.x = 0.0f;

        ImFontConfig config_icon;
        config_icon.MergeMode = true;
        config_icon.PixelSnapH = true;
        config_icon.OversampleH = 4;
        config_icon.OversampleV = 4;
        config_icon.RasterizerMultiply = 1.5f;  // brighten up the font to make them more readable
        config_icon.GlyphOffset.y = 0.0f;       // tweak this to vertically align with the main font
        config_icon.GlyphMinAdvanceX = fontsize_main;  // enforce monospaced icon font
        config_icon.GlyphMaxAdvanceX = fontsize_main;  // enforce monospaced icon font

        static const ImWchar icon_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };  // zero-terminated

        truetype_font = io.Fonts->AddFontFromFileTTF(ttf_main.c_str(), fontsize_main, &config_main);
        web_icon_font = io.Fonts->AddFontFromFileTTF(ttf_icon.c_str(), fontsize_icon, &config_icon, icon_ranges);
        opentype_font = io.Fonts->AddFontFromFileTTF(ttf_sub.c_str(), fontsize_sub, &config_sub);

        // build font textures
        unsigned char* pixels;
        int width, height, bytes_per_pixel;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

        // load default dark theme
        StyleColorsDark();

        // setup custom styles
        style.WindowBorderSize = 0.0f;
        style.FrameBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.TabBorderSize = 0.0f;
        style.ScrollbarSize = 18.0f;
        style.GrabMinSize = 10.0f;

        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(4.0f, 6.0f);
        style.ItemSpacing = ImVec2(10.0f, 10.0f);
        style.ItemInnerSpacing = ImVec2(10.0f, 10.0f);
        style.IndentSpacing = 16.0f;

        style.WindowRounding = 0.0f;
        style.ChildRounding = 0.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 0.0f;
        style.TabRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.ScrollbarRounding = 12.0f;

        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ColorButtonPosition = ImGuiDir_Right;

        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        style.AntiAliasedLinesUseTex = true;

        // setup custom colors
        auto& c = GetStyle().Colors;

        c[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.85f);
        c[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
        c[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);

        c[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
        c[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.75f);

        c[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
        c[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
        c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);

        c[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        c[ImGuiCol_ScrollbarGrab] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.9f);
        c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.9f);

        c[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        c[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.4f, 0.0f, 0.9f);
        c[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);

        c[ImGuiCol_Button] = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.55f, 0.0f, 0.9f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);

        c[ImGuiCol_Header] = ImVec4(0.5f, 0.0f, 1.0f, 0.5f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.0f, 1.0f, 0.8f);
        c[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.0f, 1.0f, 0.7f);

        c[ImGuiCol_Tab] = ImVec4(0.0f, 0.3f, 0.0f, 0.8f);
        c[ImGuiCol_TabHovered] = ImVec4(0.0f, 0.4f, 0.0f, 0.8f);
        c[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4f, 0.0f, 0.8f);
        c[ImGuiCol_TabUnfocused] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
    }

    bool NewInspector(void) {
        static const float w = 256.0f * 1.25f;  // tweaked for 1600 x 900 resolution
        static const float h = 612.0f * 1.25f;
        auto [m_w,m_h] = D3DApp::Get()->GetWH();
        
        SetNextWindowPos(ImVec2(m_w - w, (m_h - h) * 0.5f));
        SetNextWindowSize(ImVec2(w, h));

        static ImGuiWindowFlags inspector_flags = ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

        PushID("Inspector Window");

        if (Begin(ICON_FK_LOCATION_ARROW " Inspector", 0, inspector_flags)) {
            return true;
        }


        return false;
    }

    void EndInspector(void) {
        End();
        PopID();
    }

    void DrawVerticalLine() {
        SeparatorEx(ImGuiSeparatorFlags_Vertical);
    }

    void DrawTooltip(const char* desc, float spacing) {
        SameLine(0.0f, spacing);
        TextDisabled("(?)");

        if (IsItemHovered()) {
            PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            PushStyleColor(ImGuiCol_PopupBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            BeginTooltip();
            PushTextWrapPos(GetFontSize() * 35.0f);
            TextUnformatted(desc);
            PopTextWrapPos();
            EndTooltip();
            PopStyleColor(2);
        }
    }

    void DrawRainbowBar(const ImVec2& offset, float height) {
        // draw a rainbow bar of the given height in the current window
        // bar width will be automatic adjusted to center at the window
        // offset is in pixels relative to the window's upper left corner

        // this function is borrowed and modified from the unknown cheats forum
        // source: https://www.unknowncheats.me/forum/2550901-post1.html

        float speed = 0.0006f;
        static float static_hue = 0.0f;

        ImDrawList* draw_list = GetWindowDrawList();
        ImVec2 pos = { GetWindowPos().x + offset.x ,GetWindowPos().y + offset.y };
        float width = GetWindowWidth() - offset.x * 2.0f;

        static_hue -= speed;
        if (static_hue < -1.0f) {
            static_hue += 1.0f;
        }

        for (int i = 0; i < width; i++) {
            float hue = static_hue + (1.0f / width) * i;
            if (hue < 0.0f) hue += 1.0f;
            ImColor color = ImColor::HSV(hue, 1.0f, 1.0f);
            draw_list->AddRectFilled(ImVec2(pos.x + i, pos.y), ImVec2(pos.x + i + 1, pos.y + height), color);
        }
    }



}
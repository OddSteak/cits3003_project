#include "ImGuiManager.h"

bool ImGuiManager::disabled = false;
bool ImGuiManager::was_cursor_disabled = false;

ImGuiManager::ImGuiManager(Window window) : window(std::move(window)) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "config/imgui.ini";
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
//        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(this->window.internal(), true);
    ImGui_ImplOpenGL3_Init("#version 410");

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
//    io.Fonts->AddFontDefault();
//    io.Fonts->AddFontFromFileTTF("res/fonts/JetBrainsMono/ttf/JetBrainsMono-Regular.ttf", 13.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);
}

void ImGuiManager::new_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (!was_cursor_disabled) {
        window.set_cursor_disabled(false);
    }

    was_cursor_disabled = false;
}

void ImGuiManager::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float) window.get_window_width(), (float) window.get_window_height());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void ImGuiManager::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiManager::enable_main_window_docking() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar
                                    | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    //TODO: For save/load file menu and what not
//    window_flags |= ImGuiWindowFlags_MenuBar;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Window", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID mainDockSpaceID = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(mainDockSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    //TODO: For save/load file menu and what not
    /*if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save",                "Ctrl + S") { }
            if (ImGui::MenuItem("Load",                "Ctrl + S") { }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
*/
    ImGui::End();
}

bool ImGuiManager::want_capture_keyboard() {
    return !disabled && ImGui::GetIO().WantCaptureKeyboard;
}

// Hacky expose in global space to allow being called in WindowManager
bool want_capture_keyboard() {
    return ImGuiManager::want_capture_keyboard();
}

bool ImGuiManager::want_capture_mouse() {
    return !disabled && ImGui::GetIO().WantCaptureMouse;
}

// Hacky expose in global space to allow being called in WindowManager
bool want_capture_mouse() {
    return ImGuiManager::want_capture_mouse();
}

void ImGuiManager::set_disabled(bool set_disabled) {
    disabled = set_disabled;
}

void ImGuiManager::set_cursor_was_disabled() {
    was_cursor_disabled = true;
}

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags) {
    return InputText(label, str->data(), str->capacity(), flags | ImGuiInputTextFlags_CallbackResize,
                     [](ImGuiInputTextCallbackData* data) {
                         if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                             auto* strP = reinterpret_cast<std::string*>(data->UserData);
                             strP->resize(data->BufTextLen);
                             data->Buf = &*strP->begin();
                         }
                         return 0;
                     }, (void*) str);
}

void ImGui::HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImGui::DragDisableCursor(Window& window) {
    if (ImGui::IsItemActive()) {
        ImGuiManager::set_cursor_was_disabled();
        window.set_cursor_disabled(true);
    }
}

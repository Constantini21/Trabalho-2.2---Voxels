#include "imgui/imgui.h"

#include <string>



namespace ImGui {
    void HelpMarker(const char *desc, const bool &same_line = true);
    bool RemoveButton(const bool &same_line = true);
    bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
}
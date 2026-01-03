#pragma once
#include <cstdint>
#include <map>
#include <string>
#include "nch/rmlui-utils/sdl-webview.h"

namespace nch { class InputTextareaEx {
public:
    static void tick(SDL_Webview* webview, Rml::Element* e);

private:
    static int getTextAreaIdealHeight(Rml::Element* eTextArea, Rml::Context* rmlContext);

    static uint64_t numTextareasCreated;
    static std::map<std::string, std::string> textareaID_ValueMap;
    static std::map<std::string, std::pair<int, int>> textareaID_SizeMap;
}; }
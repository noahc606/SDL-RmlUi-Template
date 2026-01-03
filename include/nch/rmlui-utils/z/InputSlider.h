#pragma once
#include <RmlUi/Core/Element.h>
#include "nch/rmlui-utils/sdl-webview.h"

namespace nch { class InputSlider {
public:
    static void tick(SDL_Webview* sdlWebview, Rml::Element* e);
private:
    static void updateValue(Rml::Element* e, float newVal, float min, float max, float step);
    static float getValue(Rml::Element* e);
}; }
#pragma once
#include <RmlUi/Core/Element.h>
#include <nch/sdl-utils/rect.h>

namespace nch { class RmlUtils {
public:
    static nch::FRect getElementBox(Rml::Element* elem);
    static std::string getElementAttributeValue(Rml::Element* elem, std::string attrName);
private:
}; }

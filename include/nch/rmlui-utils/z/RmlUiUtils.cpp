#pragma once
#include <RmlUi/Core/Element.h>
#include <nch/math-utils/box2.h>

namespace nch { class RmlUtils {
public:
    nch::Box2f getElementBox(Rml::Element* elem);
private:
}; }

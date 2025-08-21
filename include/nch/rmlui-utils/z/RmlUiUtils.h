#include "RmlUtils.h"
using namespace nch;

Box2f RmlUtils::getElementBox(Rml::Element* elem)
{
    if(elem==nullptr) return Box2f(-1,-1,-1,-1);
    return Box2f(
        elem->GetAbsoluteLeft,
        elem->GetAbsoluteTop,
        elem->GetBox().GetSize().x,
        elem->GetBox().GetSize().y,
    );
}

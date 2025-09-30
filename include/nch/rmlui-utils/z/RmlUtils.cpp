#include "RmlUtils.h"
#include "Element.h"
#include <RmlUi/Core/Box.h>
using namespace nch;

FRect RmlUtils::getElementBox(Rml::Element* elem)
{
    if(elem==nullptr) return FRect(-1,-1,-1,-1);
    auto elemSize = elem->GetBox().GetSize(Rml::BoxArea::Border);
    
    return FRect(
        elem->GetAbsoluteLeft(),
        elem->GetAbsoluteTop(),
        elemSize.x,
        elemSize.y
    );
}

std::string RmlUtils::getElementAttributeValue(Rml::Element* elem, std::string attrName)
{
    Rml::Variant* attVal = elem->GetAttribute(attrName);
    if(attVal!=nullptr) {
        return attVal->Get<std::string>();
    }
    return "???null???";
}


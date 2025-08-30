#include "RmlUtils.h"
#include "Element.h"
using namespace nch;

FRect RmlUtils::getElementBox(Rml::Element* elem)
{
    if(elem==nullptr) return FRect(-1,-1,-1,-1);
    return FRect(
        elem->GetAbsoluteLeft(),
        elem->GetAbsoluteTop(),
        elem->GetBox().GetSize().x,
        elem->GetBox().GetSize().y
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


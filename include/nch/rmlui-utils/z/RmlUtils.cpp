#include "RmlUtils.h"
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

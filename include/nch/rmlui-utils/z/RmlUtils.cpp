#include "RmlUtils.h"
#include "Element.h"
#include <RmlUi/Core.h>
#include <RmlUi/Core/Box.h>
#include <regex>
using namespace nch;

FRect RmlUtils::getElementBox(Rml::Element* elem, Vec2f offset)
{
    if(elem==nullptr) throw std::invalid_argument("Specified 'elem' is nullptr");
    auto elemSize = elem->GetBox().GetSize(Rml::BoxArea::Border);
    
    return FRect(
        elem->GetAbsoluteLeft()+offset.x,
        elem->GetAbsoluteTop()+offset.y,
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

void RmlUtils::setAttributes(Rml::ElementPtr& elem, const std::string& attrs)
{
    //Raw string: ([a-zA-Z_:][-a-zA-Z0-9_:.]*)\s*=\s*"([^"]*)"
    std::regex attr_regex("([a-zA-Z_:][-a-zA-Z0-9_:.]*)\\s*=\\s*\"([^\"]*)\"");
    std::smatch match;
    std::string::const_iterator search_start(attrs.cbegin());

    while (std::regex_search(search_start, attrs.cend(), match, attr_regex)) {
        std::string key = match[1].str();
        std::string value = match[2].str();

        elem->SetAttribute(key, value);

        search_start = match.suffix().first;
    }
}

void RmlUtils::appendChildRml(Rml::Element* eParent, const std::string& childRml)
{
    Rml::ElementDocument* doc = eParent->GetOwnerDocument();
    std::regex selfClosingRegex(R"(^<(\w+)([^>]*)\s*/>$)", std::regex::icase);
    std::regex openTagRegex(R"(^<(\w+)([^>]*)>)", std::regex::icase);
    std::smatch match;
    std::string tagName = "", attributes = "", content = "";
    bool found = false;

    //Validate 'doc'
    if(doc==nullptr) {
        throw std::invalid_argument("Owner document of provided 'eParent' is nullptr");
    }
    //Try self-closing tag
    if(std::regex_match(childRml, match, selfClosingRegex)) {
        tagName = match[1];
        attributes = match[2];
        found = true;
    }
    //Try full tag with opening and closing
    if(!found && std::regex_search(childRml, match, openTagRegex)) {
        tagName = match[1];
        attributes = match[2];
        size_t openTagEnd = match.position(0) + match.length(0);

        std::string closeTag = "</" + tagName + ">";
        size_t closeTagStart = childRml.rfind(closeTag);

        if(closeTagStart!=std::string::npos && closeTagStart>openTagEnd) {
            content = childRml.substr(openTagEnd, closeTagStart-openTagEnd);
            found = true;
        }
    }

    if(found) {
        auto elem = doc->CreateElement(tagName);
        elem->SetInnerRML(content);
        setAttributes(elem, attributes);
        eParent->AppendChild(std::move(elem));
    }


    /*

    //Result
    if(found) {
        Rml::ElementPtr elem = doc->CreateElement(tagName);
        elem->SetInnerRML(content);
        setAttributes(std::move(elem), attributes);
        eParent->AppendChild(std::move(elem));
    } else {
        throw std::logic_error("Provided 'rml' is not a valid RML element");
    }
    */
}
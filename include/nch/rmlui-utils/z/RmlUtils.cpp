#include "RmlUtils.h"
#include "Element.h"
#include <RmlUi/Core.h>
#include <RmlUi/Core/Box.h>
#include <RmlUi/Core/Element.h>
#include <nch/cpp-utils/string-utils.h>
#include <regex>
using namespace nch;

FRect RmlUtils::getElementBox(Rml::Element* elem, Vec2f offset, Rml::BoxArea boxArea)
{
    if(elem==nullptr) throw std::invalid_argument("Specified 'elem' is nullptr");
    auto elemSize = elem->GetBox().GetSize(boxArea);
    
    return FRect(
        elem->GetAbsoluteLeft()+offset.x,
        elem->GetAbsoluteTop()+offset.y,
        elemSize.x,
        elemSize.y
    );
}
std::string RmlUtils::getElementAttribute(Rml::Element* elem, std::string attrName)
{
    if(elem==nullptr) throw std::invalid_argument("Element is nullptr");
    
    Rml::Variant* attVal = elem->GetAttribute(attrName);
    if(attVal!=nullptr) {
        return attVal->Get<std::string>();
    }
    throw std::invalid_argument(StringUtils::cat("Element does not have attribute \"", attrName, "\""));
}
bool RmlUtils::elementHasAttribute(Rml::Element* elem, std::string attrName)
{
    return elem->GetAttribute(attrName)!=nullptr;
}

std::tuple<int, int, std::string> RmlUtils::tryGetSelectedText(Rml::Element* elem)
{
    int x = 0, y = 0;
    std::string ret = "";

    Rml::ElementFormControlTextArea* textarea;
    Rml::ElementFormControlInput* input;
    if((textarea = dynamic_cast<Rml::ElementFormControlTextArea*>(elem))) {
        textarea->GetSelection(&x, &y, &ret);
    }
    if((input = dynamic_cast<Rml::ElementFormControlInput*>(elem))) {
        textarea->GetSelection(&x, &y, &ret);
    }

    return {x, y, ret};
}

int RmlUtils::getTextAreaIdealHeight(Rml::Element* eTextArea, Rml::Context* rmlContext)
{
    Rml::ElementDocument* doc = eTextArea->GetOwnerDocument();
    Rml::Element* eParent = eTextArea->GetParentNode();

    Rml::ElementPtr dummy = doc->CreateElement("dummy-text-holder");
    Rml::Element* eDummy = eParent->AppendChild(std::move(dummy)); {
        std::vector<std::string> copiedProps = {
            "max-width", "min-width", "width",
            "line-height",
            "margin-top", "margin-bottom", "margin-left", "margin-right",
            "padding-top", "padding-bottom", "padding-left", "padding-right",
        };
        for(int i = 0; i<copiedProps.size(); i++) {
            tryCopyPropertyFrom(eTextArea, eDummy, copiedProps[i]);
        }
        eDummy->SetProperty("display", "block");
        eDummy->SetProperty("background-color", "rgb(255, 255, 255)");
        eDummy->SetProperty("white-space", "pre-wrap");
        eDummy->SetProperty("word-break", "break-word");

        std::string textContent = eTextArea->GetInnerRML();
        textContent = StringUtils::replacedAllAWithB(textContent, "<", "=");
        textContent = StringUtils::replacedAllAWithB(textContent, ">", "=");
        textContent = StringUtils::replacedAllAWithB(textContent, "\n", "<br/>");
        eDummy->SetInnerRML(textContent+"x");
    }

    {
        rmlContext->Update();
        Rml::BoxArea boxArea = Rml::BoxArea::Margin;
        int ret = eDummy->GetBox().GetSize(boxArea).y;
        eParent->RemoveChild(eDummy);
        rmlContext->Update();
        return ret;
    }
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
void RmlUtils::tryCopyPropertyFrom(Rml::Element* eSrc, Rml::Element* eDst, const std::string& propertyName)
{
    if(eSrc==nullptr || eDst==nullptr) return;
    const Rml::Property* srcProp = eSrc->GetProperty(propertyName);
    if(srcProp==nullptr) return;

    eDst->SetProperty(propertyName, srcProp->ToString());
}

void RmlUtils::trySelectAllText(Rml::Element* elem)
{
    Rml::ElementFormControlTextArea* textarea;
    Rml::ElementFormControlInput* input;
    if((textarea = dynamic_cast<Rml::ElementFormControlTextArea*>(elem))) {
        textarea->Select();
    }
    if((input = dynamic_cast<Rml::ElementFormControlInput*>(elem))) {
        input->Select();
    }
}

void RmlUtils::tryClipboardPaste(Rml::Element* elem)
{

}
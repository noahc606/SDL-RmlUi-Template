#include "WebEvent.h"
#include <sstream>
#include <string>

using namespace nch;

std::map<int, std::pair<std::string, std::string>> wetDescMap = {
    {WebEvent::CLICK,        { "click",     "Clicked on" }},
    {WebEvent::DOUBLE_CLICK, { "dblclick",  "Double-clicked on" }},
    {WebEvent::MOUSE_DOWN,   { "mousedown", "Mouse pressed while on" }},
    {WebEvent::MOUSE_OVER,   { "mouseover", "Mouse over" }},
    {WebEvent::MOUSE_MOVE,   { "mousemove", "Mouse moved while on" }},
    {WebEvent::FOCUS,        { "focus",     "Focused on" }},
    {WebEvent::SUBMIT,       { "submit",    "Submitted" }},
    {WebEvent::SCROLL,       { "scroll",    "Scrolled while on" }},
    {WebEvent::RESIZE,       { "resize",    "Resized" }},
};

WebEvent::WebEvent(){}
WebEvent::WebEvent(WebEventType type, std::string elementID, Rml::Element* element) {
    WebEvent::type = type;
    WebEvent::elementID = elementID;
    WebEvent::element = element;
}
WebEvent::~WebEvent(){}

bool WebEvent::exists() {
    return type!=WebEventType::NONE;
}
WebEvent::WebEventType WebEvent::getType() {
    return type;
}
std::string WebEvent::getTypeName(int webEventType) {
    auto itr = wetDescMap.find(webEventType);
    if(itr!=wetDescMap.end()) {
        return itr->second.first;
    }
    return "???null???";
}
std::string WebEvent::getTypeName() {
    return getTypeName(type);
}
std::string WebEvent::toString()
{
    std::string verb = "???null???";
    auto itr = wetDescMap.find(type);
    if(itr!=wetDescMap.end()) {
        verb = itr->second.second;
    } else {
        verb = "Unknown action on";
    }

    std::stringstream ret;
    ret << verb << " element with ID \"" << elementID << "\"";
    return ret.str();
}
std::string WebEvent::getElementID() {
    return elementID;
}
Rml::Element* WebEvent::getElement() {
    return element;
}
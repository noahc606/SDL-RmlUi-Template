#include "WebEventHolder.h"
#include "WebEvent.h"
#include <string>
#include <nch/cpp-utils/log.h>

using namespace nch;

WebEventHolder::WebEventHolder() {
    webEvents = new std::queue<WebEvent>();
}
WebEventHolder::~WebEventHolder() {
    delete webEvents;
    webEvents = nullptr;
}

void WebEventHolder::trackEvent(WebEvent::WebEventType webEvtType, Rml::Element* elem)
{
    std::string evtName = WebEvent::getTypeName(webEvtType);
    if(evtName!="???null???") {
        elem->AddEventListener(evtName, new GenListener(webEvents, webEvtType));
        return;
    }
    Log::warn(__PRETTY_FUNCTION__, "Invalid 'webEvtType' provided (%d)", webEvtType);
}

WebEvent WebEventHolder::popEvent()
{
    //Return "NONE" event under certain conditions
    if(webEvents==nullptr || webEvents->size()==0) {
        return WebEvent();
    }

    //Remove and return object at the front of the queue
    WebEvent ret = webEvents->front();
    webEvents->pop();
    return ret;
}
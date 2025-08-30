#pragma once
#include <RmlUi/Core/Element.h>
#include "WebEvent.h"

namespace nch { class WebEventHolder {
public:
    class GenListener : public Rml::EventListener {
    public:
        std::queue<WebEvent>* webEvents;
        WebEvent::WebEventType webEvtType;
        
        GenListener(std::queue<WebEvent>* webEvents, WebEvent::WebEventType webEvtType) {
            GenListener::webEvents = webEvents;
            GenListener::webEvtType = webEvtType;
        }
        void ProcessEvent(Rml::Event& event) override {
            Rml::Element* target = event.GetTargetElement();
            if(webEvents!=nullptr) {
                webEvents->push(WebEvent(webEvtType, target->GetId()));
            }
        }
    };

    WebEventHolder();
    ~WebEventHolder();

    void trackEvent(WebEvent::WebEventType webEvtType, Rml::Element* elem);
    WebEvent popEvent();

private:
    std::queue<WebEvent>* webEvents = nullptr;
}; }
#include "InputSlider.h"
#include <RmlUi/Core/ElementDocument.h>
#include <nch/cpp-utils/string-utils.h>
#include <nch/sdl-utils/input.h>
#include "RmlUtils.h"

using namespace nch;

void InputSlider::tick(SDL_Webview* sdlWebview, Rml::Element* e)
{
    if(!sdlWebview) return;
    if(!e) return;
    auto doc = e->GetOwnerDocument();
    if(!doc) return;


    std::string btnRootStyle = "display: block; position: relative;";

    //Get relative-to-webview mousePos
    Vec2i mousePos = { Input::getMouseX(), Input::getMouseY() };
    mousePos -= Vec2i(sdlWebview->getScreenBox().r.x, sdlWebview->getScreenBox().r.y);
    mousePos -= sdlWebview->getScroll();

    //Get slider properties
    float min = 0;      try { min = std::stod(RmlUtils::getElementAttribute(e, "min")); } catch(...) {} 
    float max = 100;    try { max = std::stod(RmlUtils::getElementAttribute(e, "max")); } catch(...) {} 
    float value = 0;   try { value = std::stod(RmlUtils::getElementAttribute(e, "value")); } catch(...) {}
    float size = 0.1;   try { size = std::stod(RmlUtils::getElementAttribute(e, "size")); } catch(...) {}
    float step = 10;    try { step = std::stod(RmlUtils::getElementAttribute(e, "step")); } catch(...) {}

    //Calculate slider bounding boxes
    auto eBox = RmlUtils::getElementBox(e, {0, 0}, Rml::BoxArea::Padding);
    float x1 = eBox.r.w-size*eBox.r.w;
    float fracValue = (value-min)/(max-min);
    FRect eBtnBox = FRect(
        x1*fracValue,
        0,
        size*eBox.r.w,
        eBox.r.h
    );
    //Hovering?
    bool hovering = eBox.contains(mousePos.x, mousePos.y);

    //If this slider hasn't been built yet, so do.
    if(!RmlUtils::elementHasAttribute(e, "z-slider-built")) {
        e->SetInnerRML("");

        Rml::ElementPtr epBtn = doc->CreateElement("slider-button");
        Rml::Element* eBtn = e->AppendChild(std::move(epBtn));
        RmlUtils::setStyleFormatted(eBtn, "%s; left: -2px", btnRootStyle.c_str());

        e->SetAttribute("z-slider-built", true);
    }

    //Determine whether this slider should be controllable.
    bool controllable = false;
    if(RmlUtils::elementHasAttribute(e, "z-slider-controllable")) {
        controllable = true;
    }
    if(Input::mouseDownTime(1)==1 && hovering) {
        controllable = true;
    }
    if(!Input::isMouseDown(1)) {
        controllable = false;
    }

    //Update attribute based on 'controllable'
    if(controllable) {
        e->SetAttribute("z-slider-controllable", true);
    } else {
        e->RemoveAttribute("z-slider-controllable");
    }

    

    //Set slider style
    Rml::Element* eBtn = e->GetChild(0);
    RmlUtils::setStyleFormatted(eBtn, "%s; width: %fpx; height: %fpx; left: %fpx", btnRootStyle.c_str(), eBtnBox.r.w, eBtnBox.r.h, eBtnBox.r.x-2);
    if(controllable) {
        eBtn->SetClass("controlled", true);
    } else {
        eBtn->SetClass("controlled", false);
    }
    

    //Control slider value from mouse pos
    if(controllable) {
        float valFrac = ((float)mousePos.x-(eBox.r.x+eBtnBox.r.w/2))/(eBox.r.w-eBtnBox.r.w);
        float val = min+valFrac*(max-min);
        updateValue(e, val, min, max, step);
    }
    //Control slider value from mouse scroll
    if(hovering) {
        int mwd = Input::getMouseWheelDelta();
        if(mwd!=0) {
            updateValue(e, getValue(e)+step*mwd, min, max, step);
        }
    }
}

void InputSlider::updateValue(Rml::Element* e, float newVal, float min, float max, float step)
{
    newVal = std::round(newVal/step)*step;
    if(newVal>max) newVal = max;
    if(newVal<min) newVal = min;
    e->SetAttribute("value", StringUtils::cat(newVal));
}

float InputSlider::getValue(Rml::Element* e)
{
    try {
        return std::stod(RmlUtils::getElementAttribute(e, "value"));
    } catch(...) {}
    return -1;
}
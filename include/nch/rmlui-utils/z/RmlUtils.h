#pragma once
#include <RmlUi/Core/Element.h>
#include <nch/cpp-utils/log.h>
#include <nch/math-utils/vec2.h>
#include <nch/sdl-utils/rect.h>


namespace nch { class RmlUtils {
public:
    static nch::FRect getElementBox(Rml::Element* elem, Vec2f offset = {0, 0}, Rml::BoxArea boxArea = Rml::BoxArea::Border);
    static std::string getElementAttributeValue(Rml::Element* elem, std::string attrName);
    template<typename ... T> static void setStyleFormatted(Rml::Element* elem, const std::string& styleFormat, T ... args) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat-security"
        
        //Nullptr check
        if(elem==nullptr) {
            Log::warnv(__PRETTY_FUNCTION__, "making no changes", "Specified 'elem' is null");
            return;
        }
        //Build formatted style string
        std::string styleStr = ""; {
            int size_s = std::snprintf(nullptr, 0, styleFormat.c_str(), args ...)+1;
            if(size_s<=0) {
                Log::warnv(__PRETTY_FUNCTION__, "making no changes", "Error during formatting.");
                return;
            }
            auto size = static_cast<size_t>(size_s);
            std::unique_ptr<char[]> buf(new char[size]);
            std::snprintf(buf.get(), size, styleFormat.c_str(), args ...);
            styleStr = std::string(buf.get(), buf.get()+size-1);
        }
        //Set style attribute
        elem->SetAttribute("style", styleStr);
        
        #pragma GCC diagnostic pop
    };
    static void setAttributes(Rml::ElementPtr& elem, const std::string& attrs);
    static void appendChildRml(Rml::Element* eParent, const std::string& childRml);

private:
}; }

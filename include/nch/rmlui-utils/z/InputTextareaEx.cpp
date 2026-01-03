#include "InputTextareaEx.h"
#include <RmlUi/Core/Elements/ElementFormControlTextArea.h>
#include <nch/cpp-utils/string-utils.h>
#include <nch/rmlui-utils/rml-utils.h>
using namespace nch;

uint64_t InputTextareaEx::numTextareasCreated = 0;
std::map<std::string, std::string> InputTextareaEx::textareaID_ValueMap;
std::map<std::string, std::pair<int, int>> InputTextareaEx::textareaID_SizeMap;

void InputTextareaEx::tick(SDL_Webview* webview, Rml::Element* e) {
    if(e->GetTagName()!="textarea" || !e->HasAttribute("expanding")) return;
    auto efcta = (Rml::ElementFormControlTextArea*)e;

    if(webview==nullptr) return;
    auto doc = webview->getWorkingDocument();
    if(doc==nullptr) return;
    auto ctx = webview->getContext();

    //Assign ID if it doesn't have one
    std::string id, val; std::pair<int, int> dims;
    if(!e->HasAttribute("z-textarea-ex-id")) {
        e->SetAttribute("z-textarea-ex-id", StringUtils::cat(numTextareasCreated));
        numTextareasCreated++;
    }
    id = RmlUtils::getElementAttribute(e, "z-textarea-ex-id");
    val = efcta->GetValue();
    dims = { RmlUtils::getElementBox(e).r.w, RmlUtils::getElementBox(e).r.h };
    textareaID_ValueMap.insert({id, val});
    textareaID_SizeMap.insert({id, dims});
    
    bool update = false;
    auto itrVM = textareaID_ValueMap.find(id);
    if(itrVM->second!=val) {
        update = true;
    }
    auto itrSM = textareaID_SizeMap.find(id);
    if(itrSM->second!=dims) {
        update = true;
    }

    if(update) {
        int idealHeight = RmlUtils::getTextAreaIdealHeight(e, ctx);
        RmlUtils::setStyleFormatted(e, "height: %dpx", idealHeight);
        
        textareaID_ValueMap.erase(id);
        textareaID_ValueMap.insert({id, val});
        textareaID_SizeMap.erase(id);
        textareaID_SizeMap.insert({id, dims});
    }
}
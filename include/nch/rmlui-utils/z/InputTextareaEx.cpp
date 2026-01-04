#include "InputTextareaEx.h"
#include <RmlUi/Core/Context.h>
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
        int idealHeight = getTextAreaIdealHeight(e, ctx);
        RmlUtils::setStyleFormatted(e, "height: %dpx", idealHeight);
        
        textareaID_ValueMap.erase(id);
        textareaID_ValueMap.insert({id, val});
        textareaID_SizeMap.erase(id);
        textareaID_SizeMap.insert({id, dims});
    }
}

int InputTextareaEx::getTextAreaIdealHeight(Rml::Element* eTextArea, Rml::Context* ctx)
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
            RmlUtils::tryCopyPropertyFrom(eTextArea, eDummy, copiedProps[i]);
        }
        eDummy->SetProperty("display", "block");
        eDummy->SetProperty("background-color", "rgb(255, 255, 255)");
        eDummy->SetProperty("white-space", "pre-wrap");
        eDummy->SetProperty("word-break", "break-word");

        std::string textContent = eTextArea->GetInnerRML();
        textContent = StringUtils::replacedAllAWithB(textContent, "<", "=");
        textContent = StringUtils::replacedAllAWithB(textContent, ">", "=");
        textContent = StringUtils::replacedAllAWithB(textContent, "{", "[");
        textContent = StringUtils::replacedAllAWithB(textContent, "}", "]");
        textContent = StringUtils::replacedAllAWithB(textContent, "\n", "<br/>");
        eDummy->SetInnerRML(textContent+"x");
    }

    {
        ctx->Update();
        Rml::BoxArea boxArea = Rml::BoxArea::Margin;
        int ret = eDummy->GetBox().GetSize(boxArea).y;
        eParent->RemoveChild(eDummy);
        ctx->Update();
        return ret;
    }
}
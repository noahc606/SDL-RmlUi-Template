#include "SDL_Webview.h"
#include <RmlUi/Core.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/input.h>
#include <nch/sdl-utils/rect.h>
#include <nch/sdl-utils/texture-utils.h>
#include <sstream>


using namespace nch;

bool SDL_Webview::rmlInitialized = false;
SDL_Renderer* SDL_Webview::sdlRenderer = nullptr;
std::string SDL_Webview::sdlBasePath = "???null???";
std::string SDL_Webview::webAssetsSubpath = "???null???";
bool SDL_Webview::loggingEnabled = true;
SystemInterface_SDL* SDL_Webview::sdlSystemInterface = nullptr;
RenderInterface_SDL* SDL_Webview::sdlRenderInterface = nullptr;
std::set<SDL_Keycode> SDL_Webview::specialKeys = {
    SDLK_BACKSPACE, SDLK_RETURN, SDLK_KP_ENTER,
    SDLK_PAGEUP, SDLK_PAGEDOWN,
SDLK_END, SDLK_HOME,
SDLK_LEFT, SDLK_UP, SDLK_RIGHT, SDLK_DOWN,
SDLK_INSERT, SDLK_DELETE,
    SDLK_KP_4, SDLK_KP_8, SDLK_KP_6, SDLK_KP_2, SDLK_KP_0, SDLK_KP_PERIOD
};
Vec2i SDL_Webview::maxDimSize = {4096, 4096};

SDL_Webview::SDL_Webview(std::string p_rmlCtxID, Vec2i dimensions)
{
    SDL_Webview::dims = dimensions;
    initContext(p_rmlCtxID);
}
SDL_Webview::SDL_Webview(){}
SDL_Webview::~SDL_Webview() { destroyContext(); }

bool SDL_Webview::initContext(std::string p_rmlCtxID)
{
    if(rmlCtxID!="???null???") return false;

    if(!rmlInitialized) {
        Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "RmlUi is not initialized (did you call SDL_Webview::rmlGlobalInit()?)");
        return false;
    }
    if(rmlCtxID!="???null???") {
        Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "This webview context was already initialized (already called initContext() with this object)");
        return false;
    }

    //Set context ID, create RML context
    SDL_Webview::rmlCtxID = p_rmlCtxID;
    rmlContext = Rml::CreateContext(rmlCtxID, Rml::Vector2i(dims.x, dims.y));
    if(rmlContext==nullptr) {
        Log::errorv(__PRETTY_FUNCTION__, "Rml::CreateContext", "Failed to create RmlUi context \"%s\"", rmlCtxID.c_str());
        rmlCtxID = "???null???";
        return false;
    }
    //Create web texture
    webTex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dims.x, dims.y);
    TexUtils::clearTexture(sdlRenderer, webTex);
    setScreenPos({0, 0});

    return true;
}
void SDL_Webview::destroyContext()
{
    if(rmlContext!=nullptr && workingDocument!=nullptr) {
        rmlContext->UnloadDocument(workingDocument);
        workingDocument = nullptr;
    }
    if(rmlContext!=nullptr) {
        Rml::RemoveContext(rmlCtxID);
        rmlContext = nullptr;
    }

    workingDocumentPath = "???null???";
    if(webTex!=nullptr) {
        webTex = nullptr;
        SDL_DestroyTexture(webTex);
    }
}

void SDL_Webview::rmlGlobalInit(SDL_Renderer* p_sdlRenderer, std::string p_sdlBasePath, std::string p_webAssetsSubpath)
{
    if(rmlInitialized) {
        Log::warn(__PRETTY_FUNCTION__, "RmlUi is already globally initialized");
        return;
    }

    /* RmlUi library + subsystems init */
    {
        //Main init
        if(!Rml::Initialise()) {
            Log::errorv(__PRETTY_FUNCTION__, "Rml::Initialise()", "RmlUi init returned false");
            return;
        }
        //Subsystems init
        sdlRenderer = p_sdlRenderer;
        sdlBasePath = p_sdlBasePath;
        webAssetsSubpath = p_webAssetsSubpath;
        sdlRenderInterface = new RenderInterface_SDL(sdlRenderer);
        Rml::SetRenderInterface(sdlRenderInterface);
        sdlSystemInterface = new SystemInterface_SDL();
        Rml::SetSystemInterface(sdlSystemInterface);
        //Load basic assets

        std::string prefix = sdlBasePath+"/"+webAssetsSubpath+"/web_assets_default/";
        rmlGloballyLoadFontAbsolute(prefix+"Lato/Lato-Regular.ttf");
        rmlGloballyLoadFontAbsolute(prefix+"Lato/Lato-Italic.ttf");
        rmlGloballyLoadFontAbsolute(prefix+"Lato/Lato-Bold.ttf");
        rmlGloballyLoadFontAbsolute(prefix+"NotoEmoji-Regular.ttf", true);
    }

    rmlInitialized = true;
}
void SDL_Webview::rmlGloballyLoadFontAsset(std::string fontAssetPath, bool fallback) {
    rmlGloballyLoadFontAbsolute(sdlBasePath+"/"+webAssetsSubpath+"/web_assets/"+fontAssetPath);
}
void SDL_Webview::rmlGlobalShutdown()
{
    if(!rmlInitialized) {
        Log::warn(__PRETTY_FUNCTION__, "RmlUi is already globally shut down");
        return;
    }

    Rml::Shutdown();
    rmlInitialized = false;
}


void SDL_Webview::tick()
{


    Vec2i mousePos = { Input::getMouseX()-screenBox.r.x, Input::getMouseY()-screenBox.r.y+viewBox.r.y };
    bool cancelMouse = false;
    if(mouseDisabled) cancelMouse = true;
    if(!cancelMouse) {
        if(screenBox.contains(Input::getMouseX(), Input::getMouseY())) {
            cancelMouse = false;
        }
    }

    if(!cancelMouse) {
        //Mouse movement
        if(lastMousePos!=Vec2i(-1, 1) && lastMousePos!=mousePos) {
            rmlContext->ProcessMouseMove(mousePos.x, mousePos.y, 0);
        }

        //Mouse clicking
        std::vector<int> buttons = { 1, 2, 3 };
        for(auto i : buttons) {
            if(Input::mouseDownTime(i)==1) {
                rmlContext->ProcessMouseButtonDown(i-1, 0);
                Rml::Element* hovElem = rmlContext->GetHoverElement();
                if(hovElem!=nullptr) {
                    hovElem->Focus();
                }
            } else if(!Input::isMouseDown(i)) {
                rmlContext->ProcessMouseButtonUp(i-1, 0);
            }
        }

        //Mouse scrolling
        int mwd = Input::getMouseWheelDelta();
        if(mwd!=0) {
            injectScroll({0, mwd});
        }
    }

    //Mouse scrolling
    if(scrollEnabled) {
        int mwd = Input::getMouseWheelDelta();
        viewBox.r.y -= (mwd*scrollUnitY);
        if(viewBox.y1()<0)       { viewBox.r.y = 0; }
        if(viewBox.y2()>dims.y) { viewBox.r.y = dims.y-viewBox.r.h; }
        if(viewBox.x1()<0)       { viewBox.r.x = 0; }
        if(viewBox.x2()>dims.x) { viewBox.r.x = dims.x-viewBox.r.w; }
    }

    //Update context
	update();

	lastMousePos = mousePos;
}
void SDL_Webview::update()
{
    rmlContext->Update();
}
void SDL_Webview::render()
{   
    SDL_Texture* oldTgt = SDL_GetRenderTarget(sdlRenderer);
    SDL_SetRenderTarget(sdlRenderer, webTex); {
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0);
        SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
        SDL_RenderClear(sdlRenderer);
        rmlContext->Render();
    } SDL_SetRenderTarget(sdlRenderer, oldTgt);
}
void SDL_Webview::drawCopyAt(Rect src, Rect dst, double alpha)
{
    int res = SDL_SetTextureBlendMode(webTex, SDL_BLENDMODE_BLEND);
    if(res!=0) {
        Log::errorv(__PRETTY_FUNCTION__, "SDL_SetTextureBlendMode()", SDL_GetError());
    }

    if(alpha<0) alpha = 0;
    if(alpha>1) alpha = 1;
    SDL_SetTextureAlphaMod(webTex, (Uint8)(alpha*255));
    
    bool srcNull = src.r.w<0||src.r.h<0;
    bool dstNull = dst.r.w<0||dst.r.h<0;

    if( srcNull&& dstNull) SDL_RenderCopy(sdlRenderer, webTex, NULL, NULL);
    if( srcNull&&!dstNull) SDL_RenderCopy(sdlRenderer, webTex, NULL, &dst.r);
    if(!srcNull&& dstNull) SDL_RenderCopy(sdlRenderer, webTex, &src.r, NULL);
    if(!srcNull&&!dstNull) SDL_RenderCopy(sdlRenderer, webTex, &src.r, &dst.r);

    SDL_SetTextureAlphaMod(webTex, 255);
}
void SDL_Webview::drawCopy(Rect dst, double alpha)
{
    drawCopyAt(Rect(0, 0, -1, -1), dst, alpha);
}
void SDL_Webview::drawCopy(Vec2i pos)
{
    Rect dst(pos.x, pos.y, dims.x, dims.y);
    drawCopy(dst);
}
void SDL_Webview::drawCopy()
{
    if(screenBox.r.h>dims.y) { screenBox.r.h = dims.y; }
    if(screenBox.r.w>dims.x) { screenBox.r.w = dims.x; }
    drawCopyAt(viewBox.r, screenBox.r);
}
void SDL_Webview::drawScrollbars()
{
    Rect dst = screenBox;
    if(dst.r.h>dims.y) { dst.r.h = dims.y; }
    if(dst.r.w>dims.x) { dst.r.w = dims.x; }

    //Find 'sb0Dst' (scroll bar background)
    Rect sb0Dst = dst; {
        sb0Dst.r.w = 12;
        sb0Dst.r.x = dst.r.w-8;
    }
    //Find 'sb1Dst' (scroll bar foreground) & 'sbVisible'
    Rect sb1Dst = sb0Dst; bool sbVisible = true; {
        sb1Dst = sb0Dst;
        double sbY = (double)viewBox.r.y/dims.y*sb1Dst.r.h;
        double sbH = (double)viewBox.r.h/dims.y*sb1Dst.r.h;
        sb1Dst.r.y += sbY;
        sb1Dst.r.h = sbH;
        if(sb1Dst==sb0Dst) sbVisible = false;
    }

    if(sbVisible) {
    //Draw scroll bar background
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 127);
        SDL_RenderFillRect(sdlRenderer, &sb0Dst.r);
        //Draw scroll bar foreground
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 127);
        SDL_RenderFillRect(sdlRenderer, &sb1Dst.r);
    }
}
void SDL_Webview::events(SDL_Event& evt)
{
    if(evt.type==SDL_KEYDOWN || evt.type==SDL_TEXTINPUT) {
        //Cancel keydown/textinput in certain cases ("readonly" input elements)
        Rml::Element* focusedElem = rmlContext->GetFocusElement();
        if(focusedElem==nullptr) return;
        std::string focusedTag = focusedElem->GetTagName();
        if(focusedTag=="input" || focusedTag=="textarea") {
            if(focusedElem->HasAttribute("readonly")) return;
        }
    }

    switch(evt.type) {
        case SDL_KEYDOWN: {
            SDL_Keycode kc = evt.key.keysym.sym;
            SDL_Keymod modState = SDL_GetModState();
            if(specialKeys.find(kc)!=specialKeys.end()) {
                if(modState & KMOD_NUM) { rmlContext->ProcessKeyDown(RmlSDL::ConvertKey(kc), Rml::Input::KM_NUMLOCK); }
                else                    { rmlContext->ProcessKeyDown(RmlSDL::ConvertKey(kc), 0); }
            }
        } break;
        case SDL_TEXTINPUT: {
            //Process text input by this point
            rmlContext->ProcessTextInput(evt.text.text);
        } break;
    }
}

void SDL_Webview::setLogging(bool shouldLog) {
    loggingEnabled = shouldLog;
}
Rml::DataModelConstructor SDL_Webview::rmlCreateDataModel(std::string name, Rml::DataTypeRegister* dataTypeRegister) {
    rmlContext->RemoveDataModel(name);
    return rmlContext->CreateDataModel(name, dataTypeRegister);
}
Rml::ElementDocument* SDL_Webview::rmlLoadDocumentAsset(std::string webdocAssetPath) {
    return rmlLoadDocumentAbsolute(sdlBasePath+"/"+webAssetsSubpath+"/web_assets/"+webdocAssetPath);
}
void SDL_Webview::reload()
{
    /* Validation */
    if(workingDocument==nullptr || workingDocumentPath=="???null???") {
        Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "No document is currently loaded into this SDL_Webview");
        return;
    }

    Timer tim("webpage reload", loggingEnabled);
    rmlLoadDocumentAbsolute(workingDocumentPath);
    updateResizingBody();
}
bool SDL_Webview::resize(Vec2i dimensions)
{
    //Do nothing if dimensions weren't changed or dimensions invalid
    if(dimensions==dims) return false;
    if(dimensions.x<1 || dimensions.y<1) return false;
    if(dimensions.x>maxDimSize.x || dimensions.y>maxDimSize.y) {
        Log::error(__PRETTY_FUNCTION__, "Provided dimensions \"%dx%d\" are larger than the maximum %dx%d", dimensions.x, dimensions.y, maxDimSize.x, maxDimSize.y);
        Log::throwException(__PRETTY_FUNCTION__, "");
    }

    //Resize variable(s)
    if(screenBox.r.w<0 || screenBox.r.h<0 || (screenBox.r.w==dims.x && screenBox.r.h==dims.y)) {
        dims = dimensions;
        setScreenDims(dimensions);
    }
    dims = dimensions;

    Rect newSB = screenBox;
    if(newSB.r.w>dims.x) { newSB.r.w = dims.x; }
    if(newSB.r.h>dims.y) { newSB.r.h = dims.y; }
    setScreenBox(newSB);

    //Resize RML context and recreate texture
    rmlContext->SetDimensions({dims.x, dims.y});
    if(webTex!=nullptr) {
        SDL_DestroyTexture(webTex);    
        webTex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dims.x, dims.y);
    }

    //Misc. webpage work
    updateResizingBody();
    return true;
}
void SDL_Webview::setScreenPos(Vec2i scrPos)
{
    screenBox.r.x = scrPos.x;
    screenBox.r.y = scrPos.y;
}
void SDL_Webview::setScreenDims(Vec2i scrDims)
{
    if(scrDims.x>dims.x) scrDims.x = dims.x;
    if(scrDims.y>dims.y) scrDims.y = dims.y;
    screenBox.r.w = scrDims.x;
    screenBox.r.h = scrDims.y;
}
void SDL_Webview::setScreenBox(Rect scrBox)
{
    //Draw box cannot be larger than scroll dims
    setScreenPos({scrBox.r.x, scrBox.r.y});
    setScreenDims({scrBox.r.w, scrBox.r.h});

    //Update view box
    viewBox.r.w = screenBox.r.w;
    viewBox.r.h = screenBox.r.h;
}
void SDL_Webview::resetScrollbar()
{
    viewBox.r.y = 0;
}

void SDL_Webview::injectClick(nch::Vec2i pos, int button) {
    rmlContext->ProcessMouseMove(pos.x, pos.y, 0);
    rmlContext->ProcessMouseButtonDown(button-1, 0);
    rmlContext->ProcessMouseButtonUp(button-1, 0);
}
void SDL_Webview::injectScroll(nch::Vec2i delta) {
    rmlContext->ProcessMouseWheel({(float)delta.x, -(float)delta.y}, 0);
}
void SDL_Webview::setMouseDisabled(bool md) {
    mouseDisabled = md;
}

Rml::DataModelConstructor SDL_Webview::getWorkingDataModel(std::string name) {
    return rmlContext->GetDataModel(name);
}
Rml::ElementDocument* SDL_Webview::getWorkingDocument() {
    return workingDocument;
}
Vec2i SDL_Webview::getDims() {
    return dims;
}
Rect SDL_Webview::getScreenBox() {
    return screenBox;
}

void SDL_Webview::rmlGloballyLoadFontAbsolute(std::string fontAbsolutePath, bool fallback) {
    if(!Rml::LoadFontFace(fontAbsolutePath, fallback)) {
        Log::errorv(__PRETTY_FUNCTION__, "Rml::LoadFontFace", "Failed to load font @ \"%s\"", fontAbsolutePath.c_str());
    }
}
Rml::ElementDocument* SDL_Webview::rmlLoadDocumentAbsolute(std::string webdocAbsolutePath) {
    /* Validation */
    if(!rmlInitialized) {
        Log::warnv(__PRETTY_FUNCTION__, "returning nullptr", "RmlUi is not initialized (did you call SDL_Webview::rmlGlobalInit()?)");
        return nullptr;
    }

    /* Load */
    //Unload old document
    if(workingDocument!=nullptr) {
        rmlContext->UnloadDocument(workingDocument);
        rmlContext->Update();
    }
    //Load new document
    workingDocument = rmlContext->LoadDocument(webdocAbsolutePath);
    if(workingDocument==nullptr) {
        Log::errorv(__PRETTY_FUNCTION__, "Rml::Context::LoadDocument", "Failed to load webpage at path \"%s\"", webdocAbsolutePath.c_str());
        return nullptr;
    }
    //Successful load by this point
    workingDocumentPath = webdocAbsolutePath;
    workingDocument->Show();
    workingDocument->ReloadStyleSheet();
    return workingDocument;
    
}
void SDL_Webview::updateResizingBody()
{
    if(workingDocument!=nullptr) {
        auto eRBody = workingDocument->GetElementById("z-resizing-body");
        if(eRBody!=nullptr) {
            std::stringstream ss;
            ss << "width: " << dims.x << "px; ";
            ss << "height: " << dims.y << "px;";
            eRBody->SetAttribute("style", ss.str());
        }
    }
}
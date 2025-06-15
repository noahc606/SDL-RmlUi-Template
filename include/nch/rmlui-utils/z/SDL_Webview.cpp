#include "SDL_Webview.h"
#include <RmlUi/Core.h>
#include <nch/cpp-utils/log.h>
#include <nch/cpp-utils/timer.h>
#include <nch/sdl-utils/input.h>
#include <nch/sdl-utils/rect.h>
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL.h"

using namespace nch;

SDL_Renderer* SDL_Webview::sdlRenderer = nullptr;
std::string SDL_Webview::sdlBasePath = "???null???";
std::string SDL_Webview::webAssetsSubpath = "???null???";
bool SDL_Webview::loggingEnabled = true;

bool rmlInitialized = false;
SystemInterface_SDL* sdlSystemInterface = nullptr;
RenderInterface_SDL* sdlRenderInterface = nullptr;
std::string workingDocumentPath = "???null???";
Rml::ElementDocument* workingDocument = nullptr;


SDL_Webview::SDL_Webview(std::string rmlCtxID, Vec2i dimensions)
{
    if(!rmlInitialized) {
        Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "RmlUi is not initialized (did you call SDL_Webview::rmlGlobalInit()?)");
        return;
    }

    SDL_Webview::rmlCtxID = rmlCtxID;
    SDL_Webview::dims = dimensions;
    rmlContext = Rml::CreateContext(rmlCtxID, Rml::Vector2i(dimensions.x, dimensions.y));
    if(rmlContext==nullptr) {
        Log::errorv(__PRETTY_FUNCTION__, "Rml::CreateContext", "Failed to create RmlUi context \"%s\"", rmlCtxID.c_str());
    }

    

    webTex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dimensions.x, dimensions.y);
}
SDL_Webview::~SDL_Webview()
{
    if(rmlContext!=nullptr) {
        rmlContext->UnloadDocument(workingDocument);
        rmlContext = nullptr;
    }

    workingDocument = nullptr;
    workingDocumentPath = "???null???";

    Rml::RemoveContext(rmlCtxID);
    SDL_DestroyTexture(webTex);
}

void SDL_Webview::rmlGlobalInit(SDL_Renderer* sdlRenderer, std::string webAssetsSubpath)
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
        SDL_Webview::sdlRenderer = sdlRenderer;
        SDL_Webview::sdlBasePath = SDL_GetBasePath();
        SDL_Webview::webAssetsSubpath = webAssetsSubpath;
        sdlRenderInterface = new RenderInterface_SDL(SDL_Webview::sdlRenderer);
        Rml::SetRenderInterface(sdlRenderInterface);
        sdlSystemInterface = new SystemInterface_SDL();
        Rml::SetSystemInterface(sdlSystemInterface);
        //Load basic assets
        rmlGloballyLoadFont(sdlBasePath+"/"+webAssetsSubpath+"/web_assets_default/LatoLatin-Regular.ttf");
        rmlGloballyLoadFont(sdlBasePath+"/"+webAssetsSubpath+"/web_assets_default/NotoEmoji-Regular.ttf", true);
    }

    rmlInitialized = true;
}

void SDL_Webview::rmlGloballyLoadFont(std::string absolutePath, bool fallback)
{
    if(!Rml::LoadFontFace(absolutePath, fallback)) {
        Log::errorv(__PRETTY_FUNCTION__, "Rml::LoadFontFace", "Failed to load font @ \"%s\"", absolutePath.c_str());
    }
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
    //Mouse movement
	Vec2i mousePos = { Input::getMouseX(), Input::getMouseY() };
	if(lastMousePos!=Vec2i(-1, 1) && lastMousePos!=mousePos) {
		rmlContext->ProcessMouseMove(mousePos.x, mousePos.y, 0);
	}

    //Mouse clicking
    for(int i = 0; i<3; i++) {
        if(Input::mouseDownTime(i)==1) {
            rmlContext->ProcessMouseButtonDown(i, 0);
            rmlContext->GetHoverElement()->Focus();
        } else if(!Input::isMouseDown(i)) {
            rmlContext->ProcessMouseButtonUp(i, 0);
        }
    }

    //Update context
	rmlContext->Update();

	lastMousePos = mousePos;
}

void SDL_Webview::render()
{   
    SDL_Texture* oldTgt = SDL_GetRenderTarget(sdlRenderer);
    SDL_SetRenderTarget(sdlRenderer, webTex); {
        SDL_RenderClear(sdlRenderer);
        rmlContext->Render();
    } SDL_SetRenderTarget(sdlRenderer, oldTgt);
}

void SDL_Webview::drawCopy(Vec2i pos)
{
    Rect dst(pos.x, pos.y, dims.x, dims.y);
    
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
    SDL_RenderFillRect(sdlRenderer, &dst.r);
    
    SDL_RenderCopy(sdlRenderer, webTex, NULL, &dst.r);
}

void SDL_Webview::events(SDL_Event& evt)
{
    switch(evt.type) {
        case SDL_KEYDOWN: {
            
        } break;
        case SDL_TEXTINPUT: {
            rmlContext->ProcessTextInput(evt.text.text);
        } break;
    }
}

Rml::DataModelConstructor SDL_Webview::rmlCreateDataModel(std::string name, Rml::DataTypeRegister* dataTypeRegister) {
    rmlContext->RemoveDataModel(name);
    return rmlContext->CreateDataModel(name, dataTypeRegister);
}
Rml::ElementDocument* SDL_Webview::rmlLoadDocumentByAbsolutePath(std::string webAssetPath) {
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
    workingDocument = rmlContext->LoadDocument(webAssetPath);
    if(workingDocument==nullptr) {
        Log::errorv(__PRETTY_FUNCTION__, "Rml::Context::LoadDocument", "Failed to load webpage at path \"%s\"", webAssetPath.c_str());
        return nullptr;
    }
    //Successful load by this point
    workingDocumentPath = webAssetPath;
    workingDocument->Show();
    workingDocument->ReloadStyleSheet();
    return workingDocument;
    
}
Rml::ElementDocument* SDL_Webview::rmlLoadDocument(std::string webAsset) {
    return rmlLoadDocumentByAbsolutePath(sdlBasePath+"/"+webAssetsSubpath+"/web_assets/"+webAsset);
}

void SDL_Webview::resize(Vec2i dimensions)
{
    //Change variable
    dims = dimensions;
    //Resize context
    rmlContext->SetDimensions({dims.x, dims.y});
    //Resize texture (recreate)
    if(webTex!=nullptr) {
        SDL_DestroyTexture(webTex);    
        webTex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dims.x, dims.y);
    }
}
void SDL_Webview::reload()
{
    /* Validation */
    if(workingDocument==nullptr || workingDocumentPath=="???null???") {
        Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "No document is currently loaded into this SDL_Webview");
        return;
    }

    Timer tim("webpage reload", loggingEnabled);
    rmlLoadDocumentByAbsolutePath(workingDocumentPath);
}
void SDL_Webview::setLogging(bool loggingEnabled) {
    SDL_Webview::loggingEnabled = loggingEnabled;
}


Rml::DataModelConstructor SDL_Webview::getWorkingDataModel(std::string name) {
    return rmlContext->GetDataModel(name);
}
Rml::ElementDocument* SDL_Webview::getWorkingDocument() {
    return workingDocument;
}
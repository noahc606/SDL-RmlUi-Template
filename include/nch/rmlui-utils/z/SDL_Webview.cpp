#include "SDL_Webview.h"
#include <RmlUi/Core.h>
#include <nch/cpp-utils/log.h>
#include <nch/sdl-utils/input.h>
#include <nch/cpp-utils/timer.h>
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL.h"

using namespace nch;

SDL_Renderer* SDL_Webview::sdlRenderer = nullptr;
std::string SDL_Webview::sdlBasePath = "???nullptr???";
std::string SDL_Webview::webAssetsSubpath = "???nullptr???";
bool SDL_Webview::loggingEnabled = true;

bool rmlInitialized = false;
SystemInterface_SDL* sdlSystemInterface = nullptr;
RenderInterface_SDL* sdlRenderInterface = nullptr;
std::string workingDocumentPath = "???nullptr???";
Rml::ElementDocument* workingDocument = nullptr;


SDL_Webview::SDL_Webview(std::string rmlCtxID, Vec2i dimensions)
{
    SDL_Webview::rmlCtxID = rmlCtxID;
    rmlContext = Rml::CreateContext(rmlCtxID, Rml::Vector2i(dimensions.x, dimensions.y));
    if(rmlContext==nullptr) {
        Log::errorv(__PRETTY_FUNCTION__, "Rml::CreateContext", "Failed to create RmlUi context \"%s\"", rmlCtxID.c_str());
    }
}
SDL_Webview::~SDL_Webview()
{
    Rml::RemoveContext(rmlCtxID);
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
        Rml::LoadFontFace(sdlBasePath+"/"+webAssetsSubpath+"/web_assets_default/LatoLatin-Regular.ttf");
        Rml::LoadFontFace(sdlBasePath+"/"+webAssetsSubpath+"/web_assets_default/NotoEmoji-Regular.ttf", true);
    }

    rmlInitialized = true;
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

void SDL_Webview::draw()
{   
	rmlContext->Render();
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
    return rmlContext->CreateDataModel(name, dataTypeRegister);
}

Rml::ElementDocument* SDL_Webview::rmlLoadDocumentByAbsolutePath(std::string webAssetPath) {
    //Check whether context created
    if(rmlContext==nullptr) {
        Log::warnv(__PRETTY_FUNCTION__, "doing nothing", "RmlUi is not initialized (did you call SDL_Webview::rmlGlobalInit()?)");
        return nullptr;
    }
    //Try to load document
    Rml::ElementDocument* doc = rmlContext->LoadDocument(webAssetPath);
    if(doc==nullptr) {
        Log::errorv(__PRETTY_FUNCTION__, "Rml::Context::LoadDocument", "Failed to load webpage at path \"%s\"", webAssetPath.c_str());
        return nullptr;
    }
    //Successful load by this point
    workingDocumentPath = webAssetPath;
    workingDocument = doc;
    doc->Show();
    doc->ReloadStyleSheet();
    return doc;
}
Rml::ElementDocument* SDL_Webview::rmlLoadDocument(std::string webAsset) {
    return rmlLoadDocumentByAbsolutePath(sdlBasePath+"/"+webAssetsSubpath+"/web_assets/"+webAsset);
}

void SDL_Webview::reload()
{
    Timer tim("webpage reload", loggingEnabled);
    rmlContext->UnloadDocument(workingDocument);
    rmlLoadDocumentByAbsolutePath(workingDocumentPath);
}

void SDL_Webview::setLogging(bool loggingEnabled) {
    SDL_Webview::loggingEnabled = loggingEnabled;
}

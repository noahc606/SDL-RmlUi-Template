#pragma once
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <SDL2/SDL.h>
#include <nch/math-utils/vec2.h>
#include <nch/sdl-utils/rect.h>
#include <set>
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_SDL.h"

namespace nch { class SDL_Webview {
public:
    SDL_Webview(std::string rmlCtxID, nch::Vec2i dimensions = {1, 1});
    SDL_Webview();
    ~SDL_Webview();
    static void rmlGlobalInit(SDL_Renderer* p_sdlRenderer, std::string p_sdlBasePath, std::string p_webAssetsSubpath = "");
    static void rmlGloballyLoadFontAsset(std::string fontAssetPath, bool fallback = false);
    static void rmlGlobalShutdown();
    void initContext(std::string rmlCtxID);

    void tick(nch::Vec2i pos = nch::Vec2i(0, 0)); void update();
    void render();
    void drawCopy(nch::Rect dst, double alpha = 1.0);
    void drawCopy(nch::Vec2i pos);
    void events(SDL_Event& evt);

    Rml::DataModelConstructor rmlCreateDataModel(std::string name, Rml::DataTypeRegister* dataTypeRegister = nullptr);
    Rml::ElementDocument* rmlLoadDocumentAsset(std::string webdocAssetPath);
    bool resize(nch::Vec2i dimensions);
    void reload();
    static void setLogging(bool shouldLog);

    Rml::DataModelConstructor getWorkingDataModel(std::string name);
    Rml::ElementDocument* getWorkingDocument();
    nch::Vec2i getDims() { return dims; }

private:
    static void rmlGloballyLoadFontAbsolute(std::string fontAbsolutePath, bool fallback = false);
    Rml::ElementDocument* rmlLoadDocumentAbsolute(std::string webdocAbsolutePath);
    void updateResizingBody();
    
    static bool rmlInitialized;
    static SDL_Renderer* sdlRenderer;
    static std::string sdlBasePath;
    static std::string webAssetsSubpath;
    static bool loggingEnabled;
    static SystemInterface_SDL* sdlSystemInterface;
    static RenderInterface_SDL* sdlRenderInterface;
    static std::set<SDL_Keycode> specialKeys;

    std::string workingDocumentPath = "???null???";
    Rml::ElementDocument* workingDocument = nullptr;    
    std::string rmlCtxID = "???null???";    Rml::Context* rmlContext = nullptr;
    SDL_Texture* webTex = nullptr;          nch::Vec2i dims = {1, 1};
    nch::Vec2i lastMousePos = {-1, -1};


}; }
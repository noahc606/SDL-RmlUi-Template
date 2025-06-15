#pragma once
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <SDL2/SDL.h>
#include <nch/math-utils/vec2.h>

namespace nch { class SDL_Webview {
public:
    SDL_Webview(std::string rmlCtxID, nch::Vec2i dimensions);
    ~SDL_Webview();
    static void rmlGlobalInit(SDL_Renderer* sdlRenderer, std::string webAssetsSubpath = "");
    static void rmlGloballyLoadFont(std::string absolutePath, bool fallback = false);
    static void rmlGlobalShutdown();

    void tick();
    void render();
    void drawCopy(nch::Vec2i pos);
    void events(SDL_Event& evt);

    Rml::DataModelConstructor rmlCreateDataModel(std::string name, Rml::DataTypeRegister* dataTypeRegister = nullptr);
    Rml::ElementDocument* rmlLoadDocumentByAbsolutePath(std::string webAssetPath);
    Rml::ElementDocument* rmlLoadDocument(std::string webAsset);
    void resize(nch::Vec2i dimensions);
    void reload();
    static void setLogging(bool shouldLog);

    Rml::DataModelConstructor getWorkingDataModel(std::string name);
    Rml::ElementDocument* getWorkingDocument();


private:
    static SDL_Renderer* sdlRenderer;
    static std::string sdlBasePath;
    static std::string webAssetsSubpath;
    static bool loggingEnabled;

    nch::Vec2i lastMousePos = {-1, -1};
    std::string rmlCtxID = "???null???";
    Rml::Context* rmlContext = nullptr;
    SDL_Texture* webTex = nullptr;
    nch::Vec2i dims = {-1, -1};
}; }
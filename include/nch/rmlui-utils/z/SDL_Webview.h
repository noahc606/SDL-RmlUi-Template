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
    bool initContext(std::string rmlCtxID);
    bool initContext();
    void destroyContext();

    void tick(); void update();
    void render();
    void drawCopyAt(nch::Rect src, nch::Rect dst, double alpha = 1.0);
    void drawCopy(nch::Rect dst, double alpha = 1.0);
    void drawCopy(nch::Vec2i pos);
    void drawCopy();
    void drawScrollbars();
    void events(SDL_Event& evt);

    Rml::Context* getContext() const;
    Rml::ElementDocument* getWorkingDocument() const;
    nch::Vec2i getDims() const;
    nch::Rect getScreenBox() const;
    nch::Rect getViewBox() const;
    nch::Vec2i getScroll() const;
    nch::Vec2i getMaxScroll() const;
    bool hasForcedFocus() const;

    static void setLogging(bool shouldLog);
    Rml::ElementDocument* rmlLoadDocumentAsset(std::string webdocAssetPath);
    void reload();
    bool resize(nch::Vec2i dimensions);
    void setScreenPos(nch::Vec2i scrPos);
    void setScreenDims(nch::Vec2i scrDims);
    void setScreenBox(nch::Rect scrBox);
    void setScrollDist(int scrollDist);
    void resetScrollbar();
    void setScroll(nch::Vec2i scroll);
    void injectClick(nch::Vec2i pos, int button = 1);
    void injectScrollF(nch::Vec2f delta);
    void injectScroll(nch::Vec2i delta);
    void unfocusAll();
    void setForcedFocus(bool forcedFocus);
    void setMouseDisabled(bool md);
    void setUserCanScroll(bool ucs);
    void setReloadUsingF5(bool reloadUsingF5);
    void useAnimatedScrolling(bool use);
private:
    static void rmlGloballyLoadFontAbsolute(std::string fontAbsolutePath, bool fallback = false);
    Rml::ElementDocument* rmlLoadDocumentAbsolute(std::string webdocAbsolutePath);
    void updateResizingBody();
    void truncateViewBox();

    static SDL_RendererInfo sdlRendererInfo;
    static bool rmlInitialized;
    static SDL_Renderer* sdlRenderer;
    static std::string sdlBasePath;
    static std::string webAssetsSubpath;
    static bool loggingEnabled;
    static SystemInterface_SDL* sdlSystemInterface;
    static RenderInterface_SDL* sdlRenderInterface;
    static std::set<SDL_Keycode> specialKeys;
    static nch::Vec2i maxDimSize;
    static int globalContextCount;

    std::string workingDocumentPath = "???null???";
    Rml::ElementDocument* workingDocument = nullptr;    
    std::string rmlCtxID = "???null???";    Rml::Context* rmlContext = nullptr;
    SDL_Texture* webTex = nullptr;          nch::Vec2i dims = {1, 1};
    nch::Vec2i lastMousePos = {-1, -1};
    nch::Rect screenBox = {-1,-1,-1,-1};
    nch::Rect viewBox = {0,0,-1,-1};
    nch::Rect animViewBox = {0, 0, -1, -1};
    bool animatedScrolling = false;
    bool mouseDisabled = false;
    bool userCanScroll = true;
    int scrollDist = 26;
    bool forcedFocus = true;
    bool reloadUsingF5 = false;
}; }
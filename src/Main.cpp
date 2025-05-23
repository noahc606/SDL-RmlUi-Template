#include <RmlUi/Core.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <nch/math-utils/vec2.h>
#include <nch/rmlui-utils/sdl-webview.h>
#include <nch/sdl-utils/main-loop-driver.h>
#include <nch/sdl-utils/input.h>
#include <nch/sdl-utils/rect.h>
#include "Main.h"

using namespace nch;

SDL_Renderer* sdlRenderer;
std::string basePath;
SDL_Webview* sdlWebview;

void draw() {
    sdlWebview->draw();
}
void tick() {
    sdlWebview->tick();
}
void events(SDL_Event& evt) {
    sdlWebview->events(evt);
}

int main(int argc, char** argv)
{
    /* Initialize SDL, SDL_image, SDL_Webview */
    {
        //SDL
        assert(SDL_Init(SDL_INIT_VIDEO)==0);
        //Window
        SDL_Window* win = SDL_CreateWindow("RmlUiTests", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  640, 480, SDL_WINDOW_RESIZABLE);
        assert(win!=NULL);
        //Renderer
        sdlRenderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
        assert(sdlRenderer!=NULL);
        //Base path
        basePath = SDL_GetBasePath();
        //SDL_image
        int imgFlags = IMG_INIT_PNG;
        assert(IMG_Init(imgFlags)==imgFlags);
        //SDL_Webview (RmlUi wrapper)
        SDL_Webview::rmlGlobalInit(sdlRenderer, basePath);
        sdlWebview = new SDL_Webview("main", Vec2i(640, 480));
    }

    /* Set parameters, load, and customize web document */
    bool simpleLoad = false;
    if(!simpleLoad) {
        //Sync data
        bool showText = true;
        std::string animal = "dog";
        if(Rml::DataModelConstructor constructor = sdlWebview->rmlCreateDataModel("animals")) {
            constructor.Bind("show_text", &showText);
            constructor.Bind("animal", &animal);
        }
        //Load document (you may want to use 'basePath')
        Rml::ElementDocument* document = sdlWebview->rmlLoadDocument(basePath+"web_assets/hello_world.html");
        document->Show();
        //Customize
        Rml::Element* element = document->GetElementById("world");
        element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
        element->SetProperty("font-size", "1.5em");
    } else {
        //Load document (you may want to use 'basePath')
        Rml::ElementDocument* document = sdlWebview->rmlLoadDocument(basePath+"web_assets/hello_world.html");
        document->Show();
    }


    //MainLoopDriver ('tick', 'draw', and 'events' funcs required to drive all SDL_Webviews)
	MainLoopDriver mld(&tick, 60, &draw, 60, &events);
    //Cleanup
    delete sdlWebview;
    SDL_Webview::rmlGlobalShutdown();
    return 0;
}

std::string Main::getBasePath() {
    return basePath;
}
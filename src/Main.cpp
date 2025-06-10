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
uint64_t ticksPassed = 0;

void draw() {
    SDL_RenderClear(sdlRenderer);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderFillRect(sdlRenderer, NULL);

    sdlWebview->render();
    sdlWebview->drawCopy({0, 0});
    SDL_RenderPresent(sdlRenderer);
}
void tick() {
    sdlWebview->tick();

    ticksPassed++;

    //Dynamic customization every 1s
    std::vector<std::string> animals = { "dog", "cat", "lion", "giraffe", "horse", "goat", "pig", "cow" };
    if(ticksPassed%60==0) {        
        //Update data
        bool showText = true;
        std::string animal = animals[(ticksPassed/60)%(animals.size())];
        if(auto dmc = sdlWebview->rmlCreateDataModel("animals")) {
            dmc.Bind("show_text", &showText);
            dmc.Bind("animal", &animal);
        }

        //Data updates require reload, and reloads cause all DOM element updates until this point to be lost.
        //Alternatively you could get away with using DOM element updates for everything.
        sdlWebview->reload();

        //Re-add DOM we had at the beginning
        Rml::Element* element = sdlWebview->getWorkingDocument()->GetElementById("world");
        element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
        element->SetProperty("font-size", "1.5em");
    }
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
        SDL_Webview::rmlGlobalInit(sdlRenderer);
        sdlWebview = new SDL_Webview("main", Vec2i(640, 480));
    }

    /* Set data models, load, and customize web document */
    {
        //Update elements thru data models
        bool showText = true;
        std::string animal = "dog";
        if(auto dmc = sdlWebview->rmlCreateDataModel("animals")) {
            dmc.Bind("show_text", &showText);
            dmc.Bind("animal", &animal);
        }
        //Load document
        auto doc = sdlWebview->rmlLoadDocument("hello_world.html");
        //Update elements thru DOM
        Rml::Element* element = doc->GetElementById("world");
        element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
        element->SetProperty("font-size", "1.5em");
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
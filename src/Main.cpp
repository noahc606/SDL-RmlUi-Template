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
SDL_Webview sdlWebview;
uint64_t ticksPassed = 0;

void draw() {
    SDL_RenderClear(sdlRenderer);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderFillRect(sdlRenderer, NULL);

    sdlWebview.render();
    sdlWebview.drawCopy();
    SDL_RenderPresent(sdlRenderer);
}
void tick() {
    sdlWebview.tick();

    ticksPassed++;

    //Dynamic customization every 1s
    std::vector<std::string> animals = { "dog", "cat", "lion", "giraffe", "horse", "goat", "pig", "cow" };
    if(ticksPassed%300==0) {        
        //Update DOM
        Rml::Element* eWorld = sdlWebview.getWorkingDocument()->GetElementById("world");
        eWorld->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
        eWorld->SetProperty("font-size", "1.5em");

        Rml::Element* eAnimal = sdlWebview.getWorkingDocument()->GetElementById("animal");
        eAnimal->SetInnerRML(animals[(ticksPassed/60)%(animals.size())]);

        sdlWebview.update();
    }
}
void events(SDL_Event& evt) {
    sdlWebview.events(evt);
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
        
        if(sdlWebview.initContext()) {
            sdlWebview.rmlLoadDocumentAsset("hello_world.html");
            sdlWebview.resize({640, 480});
            sdlWebview.setReloadUsingF5(true);
        }
        
    }

    /* Set data models, load, and customize web document */
    {
        //Load document
        auto doc = sdlWebview.getWorkingDocument();
        //Update elements thru DOM
        Rml::Element* eWorld = doc->GetElementById("world");
        eWorld->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
        eWorld->SetProperty("font-size", "1.5em");
    }


    //MainLoopDriver ('tick', 'draw', and 'events' funcs required to drive all SDL_Webviews)
	MainLoopDriver mld(&tick, 60, &draw, 60, &events);
    //Cleanup
    sdlWebview.destroyContext();
    SDL_Webview::rmlGlobalShutdown();
    return 0;
}

std::string Main::getBasePath() {
    return basePath;
}
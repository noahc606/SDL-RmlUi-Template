# NCH-RmlUi-Utils
This is an NCH library module that depends on <a href="https://github.com/noahc606/nch-cpp-utils">Noah's CPP Utils</a>. The modules needed are: cpp-utils; math-utils; sdl-utils. This module depends on C++14, unlike the other ones which only need C++11.

The example application uses this library module to render RmlUi's example "Hello 🌍" webpage.

# Features
- The very useful 'SDL_Webview' that you can integrate into your SDL2 applications.
- RmlUtils: automation of common tasks.
- InputTextareaEx: An expanding text area, declared using "&lt;textarea expanding&gt;...&lt;/textarea&gt;".
- InputSlider: My own slider implementation, declared using something like "&lt;slider min="0" max="1" value="1" size="0.05" step="0.05"&gt;&lt;/slider&gt;". I added this because the original RmlUi sliders (input type=range) seem to be broken. Or I was doing something wrong.
- My own scrolling implementation, with scrolling being animated (can be turned on/off). I added this because the original RmlUi method using "overflow: ..." within divs sometimes broke the CSS in weird ways. Scrolling is per-webview, so if you want multiple scrollable areas you need just as many webviews.

# Limitations
- Webviews can only go as high as SDL_Textures can (4096px, 16384px, etc. - depends on the machine). So for an infinite-scroll setup you would need to do a lot of work yourself. But it's possible to do with the scroll functions SDL_Webview gives you.
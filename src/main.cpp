/*
 * This example code creates an SDL window and renderer, and then clears the
 * window to a different color every frame, so you'll effectively get a window
 * that's smoothly fading between colors.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */
#include <iostream>
#include <algorithm>

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "rast/color.hpp"
#include <chrono>

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Surface* surface = nullptr;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("rasterizer", 640, 480, SDL_WINDOW_RESIZABLE);
	if (!window) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
	surface = SDL_CreateSurface(640, 480, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA32);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_DestroySurface(surface);
        surface = SDL_CreateSurface(event->window.data1, event->window.data2, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    //measure time
    using clock = std::chrono::high_resolution_clock;
    static auto last_frame = clock::now();
    auto now = clock::now();
	float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame).count();
	last_frame = now;
	std::cout << dt << "\r";

    std::fill_n((rast::color::rgba8*)surface->pixels, surface->w * surface->h, rast::color::rgba8(0x00, 0x00, 0x00, 0xff));

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_BlitSurface(surface, &rect, SDL_GetWindowSurface(window), &rect);
    SDL_UpdateWindowSurface(window);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
	SDL_DestroySurface(surface);
}

/*
 * This example code creates an SDL window and renderer, and then clears the
 * window to a different color every frame, so you'll effectively get a window
 * that's smoothly fading between colors.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */
#include <iostream>
#include <algorithm>
#include <vector>

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "rast/color.hpp"
#include <chrono>
#include "rast/mesh.hpp"
#include "rast/renderer.hpp"
#include "rast/shader/constant.hpp"
#include "rast/shader/vertex_colored.hpp"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Surface* surface = nullptr;
static rast::renderer renderer;
static glm::mat4 V;
static glm::mat4 P;

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

    renderer.setViewport(0, 0, 640, 480);
    P = glm::perspective(glm::radians(70.0f), 640.0f / 480.0f, 0.1f, 1000.0f);
    V = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rast::shader::constant::color = rast::color::rgba8(51, 51, 51, 0);
	rast::shader::constant::P = P;
	rast::shader::vertex_colored::P = P;
	rast::shader::constant::V = V;
	rast::shader::vertex_colored::V = V;

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
		renderer.setViewport(0, 0, event->window.data1, event->window.data2);
		P = glm::perspective(glm::radians(70.0f), (float)event->window.data1 / (float)event->window.data2, 0.1f, 1000.0f);
        rast::shader::constant::P = P;
        rast::shader::vertex_colored::P = P;
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
    rast::image::rgba8 iv((rast::color::rgba8*)surface->pixels, surface->w, surface->h);
    static glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

    M = glm::rotate(M, dt * 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    //renderer.setM(M);
    rast::shader::vertex_colored::M = M;

    //std::vector<glm::vec3> vertex_data = rast::mesh::grid(10, 10, 1.0f);
    //std::vector<glm::vec3> vertex_data(rast::mesh::cube, rast::mesh::cube + 36);
    std::vector<rast::shader::vertex_colored::vertex::input> vertex_data = {
        { glm::vec3(1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.0f) },
		{ glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) },
		{ glm::vec3(1.0f, 0.0f, -1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) },

		{ glm::vec3(1.0f, 0.0f, -1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) },
        { glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) },
        { glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) }
    };

    //renderer.draw_triangles_glm(iv, vertex_data.data(), (rast::renderer::data_len_t)vertex_data.size(), rast::color::rgba8(51, 51, 51, 0));
    renderer.draw_array<rast::image::rgba8, rast::shader::vertex_colored>(iv, vertex_data.data(), vertex_data.data() + 6);

    //rast::shader::constant::M = glm::translate(M, glm::vec3(3.0f, 0.0f, 0.0f));
    //renderer.draw_array<rast::image::rgba8, rast::shader::constant>(iv, rast::mesh::cube, rast::mesh::cube + 36);

    //rast::shader::constant::M = glm::translate(M, glm::vec3(0.0f, 0.0f, 3.0f));
    //renderer.draw_array<rast::image::rgba8, rast::shader::constant>(iv, rast::mesh::cube, rast::mesh::cube + 36);

    //rast::shader::constant::M = glm::translate(M, glm::vec3(-3.0f, 0.0f, 0.0f));
    //renderer.draw_array<rast::image::rgba8, rast::shader::constant>(iv, rast::mesh::cube, rast::mesh::cube + 36);

    //rast::shader::constant::M = glm::translate(M, glm::vec3(0.0f, 0.0f, -3.0f));
    //renderer.draw_array<rast::image::rgba8, rast::shader::constant>(iv, rast::mesh::cube, rast::mesh::cube + 36);

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

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
#include <chrono>

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "rast/color.hpp"
#include "rast/mesh.hpp"
#include "rast/shader/constant.hpp"
#include "rast/shader/vertex_colored.hpp"
#include "rast/shader/textured.hpp"
#include "rast/texture.hpp"
#include "rast/shader/lambert_textured.hpp"
#include "rast/framebuffer.hpp"
#include "rast/renderer.hpp"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Surface* surface = nullptr;
static rast::renderer renderer;
static glm::mat4 V;
static glm::mat4 P;
static rast::image<rast::u32> depth_buffer;
static rast::image<rast::color::rgba8> texture;
static std::vector<rast::shader::lambert_textured::vertex::input> vertex_data(24);
static rast::mesh::indexed<rast::shader::lambert_textured::vertex::input> model;

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
    P = glm::perspective(glm::radians(70.0f), 640.0f / 480.0f, 0.1f, 100.0f);
    V = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rast::shader::constant::color = rast::color::rgba8(51, 51, 51, 255);

	rast::shader::constant::P = P;
	rast::shader::vertex_colored::P = P;
	rast::shader::textured::P = P;
	rast::shader::lambert_textured::P = P;

	rast::shader::constant::V = V;
	rast::shader::vertex_colored::V = V;
	rast::shader::textured::V = V;
	rast::shader::lambert_textured::V = V;

    texture = rast::image<rast::color::rgba8>::load("assets/textures/uvChecker1.png");
    rast::shader::textured::fragment::texture = rast::texture<rast::color::rgba8>::sampler(texture);
    rast::shader::lambert_textured::fragment::texture = rast::texture<rast::color::rgba8>::sampler(texture);
    depth_buffer = rast::image<rast::u32>(640, 480);
    rast::shader::lambert_textured::vertex::format(
        (glm::vec3*)rast::mesh::cube::vertices, (glm::vec3*)rast::mesh::cube::vertices + 24,
        (glm::vec3*)rast::mesh::cube::normals, (glm::vec3*)rast::mesh::cube::normals + 24,
        (glm::vec2*)rast::mesh::cube::uv, (glm::vec2*)rast::mesh::cube::uv + 24,
        vertex_data.begin()
    );
    
    model = rast::mesh::indexed<rast::shader::lambert_textured::vertex::input>("assets/models/icosphere.mesh");

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
		P = glm::perspective(glm::radians(70.0f), (float)event->window.data1 / (float)event->window.data2, 0.1f, 100.0f);
		depth_buffer = rast::image<rast::u32>(event->window.data1, event->window.data2);
        rast::shader::constant::P = P;
        rast::shader::vertex_colored::P = P;
        rast::shader::textured::P = P;
        rast::shader::lambert_textured::P = P;
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

    rast::image<rast::color::rgba8>::view iv((rast::color::rgba8*)surface->pixels, surface->w, surface->h);
    rast::image<rast::u32>::view dv(depth_buffer);
    rast::framebuffer::rgba8_depth framebuf(iv, dv);
    std::fill_n((rast::color::rgba8*)surface->pixels, surface->w * surface->h, rast::color::rgba8(0x00, 0x00, 0x00, 0xff));
    framebuf.clear_depth_buffer();
    static glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));

    M = glm::rotate(M, dt * 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    rast::shader::textured::M = M;
    rast::shader::lambert_textured::M = M;

    //std::vector<glm::vec3> vertex_data = rast::mesh::grid(10, 10, 1.0f);
  //  std::vector<rast::shader::textured::vertex::input> vertex_data = {
  //      { glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
		//{ glm::vec3(1.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f) },
  //      { glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
  //      { glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f) }
  //  };
  //  rast::u32 index_buffer[6] = {
  //      0, 2, 1,
  //      1, 2, 3
  //  };


    renderer.draw_indexed<rast::shader::lambert_textured>(framebuf, model.index_buffer.data(), model.index_buffer.data() + model.index_buffer.size(), model.vertex_buffer.data());
    rast::shader::lambert_textured::M = glm::translate(M, glm::vec3(1.0f, -1.0f, 1.0f));
    renderer.draw_indexed<rast::shader::lambert_textured>(framebuf, model.index_buffer.data(), model.index_buffer.data() + model.index_buffer.size(), model.vertex_buffer.data());

    //rast::shader::constant::M = M;
    //renderer.draw_indexed<rast::shader::constant>(iv, dv, rast::mesh::cube::indices, rast::mesh::cube::indices + 36, (glm::vec3*)rast::mesh::cube::vertices);

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

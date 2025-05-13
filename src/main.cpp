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
#include "rast/shader/deferred.hpp"
#include "rast/framebuffer.hpp"
#include "rast/renderer.hpp"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Surface* surface = nullptr;
static glm::mat4 V;
static glm::mat4 P;
static rast::image<rast::u32> depth_buffer;
using GBuffer = rast::image<rast::shader::deferred::first_pass::fragment::output>;
static GBuffer g_buffer;
static rast::image<rast::color::rgba8> texture;
static rast::mesh::indexed<rast::shader::inputs::position_normal_uv> icosphere;
static rast::mesh::indexed<rast::shader::inputs::position_normal_uv> plane;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    int width = 640;
    int height = 480;

    SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("rasterizer", width, height, SDL_WINDOW_RESIZABLE);
	if (!window) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
	surface = SDL_CreateSurface(width, height, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA32);

    P = glm::perspective(glm::radians(70.0f), (float)width / height, 0.1f, 100.0f);
    V = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rast::shader::constant::color = rast::color::rgba8(51, 51, 51, 255);

	rast::shader::constant::P = P;
	rast::shader::vertex_colored::P = P;
	rast::shader::deferred::first_pass::P = P;

	rast::shader::constant::V = V;
	rast::shader::vertex_colored::V = V;
	rast::shader::deferred::first_pass::V = V;

    texture = rast::image<rast::color::rgba8>::load("assets/textures/uvChecker1.png");
    rast::shader::textured::fragment::texture = rast::texture<rast::color::rgba8>::sampler(texture);
    rast::shader::deferred::first_pass::fragment::texture = rast::texture<rast::color::rgba8>::sampler(texture);
    depth_buffer = rast::image<rast::u32>(width, height);
    g_buffer = GBuffer(width, height);
    rast::shader::deferred::second_pass::fragment::texture = rast::texture<GBuffer::color>::sampler(g_buffer);
    
    icosphere = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>("assets/models/icosphere.mesh");
    plane = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>("assets/models/plane.mesh");

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

		P = glm::perspective(glm::radians(70.0f), (float)event->window.data1 / (float)event->window.data2, 0.1f, 100.0f);

		depth_buffer.resize(event->window.data1, event->window.data2);
        g_buffer.resize(event->window.data1, event->window.data2);

		rast::shader::deferred::second_pass::fragment::texture = rast::texture<GBuffer::color>::sampler(g_buffer);
        rast::shader::constant::P = P;
        rast::shader::vertex_colored::P = P;
        rast::shader::deferred::first_pass::P = P;
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

    // prepare framebuffers
    rast::image<rast::color::rgba8>::view iv((rast::color::rgba8*)surface->pixels, surface->w, surface->h);
    GBuffer::view gv(g_buffer);
    //rast::framebuffer::color_depth<GBuffer::color, rast::u32> framebuf(gv, depth_buffer);
    rast::framebuffer::color_depth<rast::color::rgba8, rast::u32> framebuf(iv, depth_buffer);
    framebuf.clear_depth_buffer();
    //rast::framebuffer::rgba8 noDepthFramebuffer(iv);
    //framebuf.clear_color({glm::vec3(0.0f), glm::vec3(0.0f), rast::color::rgba8(0, 0, 0, 255)});
    iv.clear(rast::color::rgba8(0, 0, 0, 255));


    // model matrix
    static glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
    M = glm::rotate(M, dt * 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    using shader = rast::shader::lambert_textured;
    rast::scissor scissor(0, 0, iv.width, iv.height);

    shader::uniform_buffer ubo;
    ubo.fragment.texture = texture;
    ubo.vertex.M = M;
    ubo.vertex.P = P;
    ubo.vertex.V = V;

    // render
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::translate(M, glm::vec3(0.0f, 0.0f, 2.0f));
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::translate(M, glm::vec3(2.0f, 0.0f, 0.0f));
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::translate(M, glm::vec3(-2.0f, 0.0f, 0.0f));
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::translate(M, glm::vec3(0.0f, 0.0f, -2.0f));
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::translate(M, glm::vec3(2.0f, 0.0f, 2.0f));
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::translate(M, glm::vec3(-2.0f, 0.0f, -2.0f));
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::translate(M, glm::vec3(2.0f, 0.0f, -2.0f));
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::translate(M, glm::vec3(-2.0f, 0.0f, 2.0f));
    rast::renderer::draw_indexed<shader>(framebuf, icosphere, ubo, scissor);
    ubo.vertex.M = glm::scale(glm::translate(M, glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(3.0f));
    rast::renderer::draw_indexed<shader>(framebuf, plane, ubo, scissor);

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

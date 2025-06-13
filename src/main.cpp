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
#include <bitset>

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#define SDL_HINT_MOUSE_RELATIVE_WARP_MOTION  "SDL_MOUSE_RELATIVE_WARP_MOTION"
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
#include "rast/clip/sutherland_hodgman.hpp"
#include "rast/clip/near_clip_far_discard.hpp"
#include "thread_pool.hpp"
#include "game/fly_cam.hpp"

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Surface* surface = nullptr;
static glm::mat4 V;
static glm::mat4 P;
using depth_format = uint32_t;
static rast::image<depth_format> depth_buffer;
using GBuffer = rast::image<rast::shader::deferred::first_pass::fragment::output>;
static GBuffer g_buffer;
static rast::image<rast::color::rgba8> texture;
static rast::mesh::indexed<rast::shader::inputs::position_normal_uv> icosphere;
static rast::mesh::indexed<rast::shader::inputs::position_normal_uv> cube;
static rast::mesh::indexed<rast::shader::inputs::position_normal_uv> plane;
//static thd::thread_pool tp(std::thread::hardware_concurrency());
static thd::thread_pool tp(std::thread::hardware_concurrency() - 2);
//static thd::thread_pool tp(1);
static std::bitset<256> pressed;
static glm::vec2 mouseDelta = glm::vec2(0.0f);

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    int width = 1920;
    int height = 1080;

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

    texture = rast::image<rast::color::rgba8>::load("assets/textures/uvChecker1.png");
    depth_buffer = rast::image<depth_format>(width, height);
    g_buffer = GBuffer(width, height);
    
    icosphere = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>("assets/models/SuzanneSmooth.mesh");
    cube = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>("assets/models/cube.mesh");
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
    }
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.scancode == SDL_SCANCODE_ESCAPE && !SDL_CursorVisible()) {
            SDL_SetWindowRelativeMouseMode(window, false);
            SDL_ShowCursor();
        }

        if (event->key.scancode < pressed.size())
			pressed.set(event->key.scancode);
    }
    if (event->type == SDL_EVENT_KEY_UP) {
        if (event->key.scancode < pressed.size())
            pressed.reset(event->key.scancode);
    }
    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        mouseDelta.x = event->motion.xrel;
        mouseDelta.y = event->motion.yrel;
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->button.button == SDL_BUTTON_LEFT && SDL_CursorVisible()) {
            SDL_SetWindowRelativeMouseMode(window, true);
            SDL_HideCursor();
        }
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
    //rast::framebuffer::color_depth<GBuffer::color, rast::uint32_t> framebuf(gv, depth_buffer);
    rast::framebuffer::color_depth<rast::color::rgba8, depth_format> framebuf(iv, depth_buffer);
    framebuf.clear_depth_buffer();
    //rast::framebuffer::rgba8 noDepthFramebuffer(iv);
    //framebuf.clear_color({glm::vec3(0.0f), glm::vec3(0.0f), rast::color::rgba8(0, 0, 0, 255)});
    iv.clear(rast::color::rgba8(0, 0, 0, 255));


    // model matrix
    static glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
    //M = glm::rotate(M, dt * 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    //M = glm::translate(M, -dt * 10.0f * glm::vec3(1.0f, 0.0f, 0.0f));

    // move camera
    static game::fly_cam flyCam;
    glm::vec3 movement = glm::vec3(0.0f, 0.0f, 0.0f);
    if (!SDL_CursorVisible()) {
		if (pressed[SDL_SCANCODE_W]) movement.z += 1.0f;
		if (pressed[SDL_SCANCODE_S]) movement.z -= 1.0f;
		if (pressed[SDL_SCANCODE_A]) movement.x += 1.0f;
		if (pressed[SDL_SCANCODE_D]) movement.x -= 1.0f;
		if (pressed[SDL_SCANCODE_SPACE]) movement.y += 1.0f;
		if (pressed[SDL_SCANCODE_LSHIFT]) movement.y -= 1.0f;
		flyCam.update(movement, glm::vec2(mouseDelta.y, -mouseDelta.x), dt);
    }
    V = glm::lookAt(flyCam.position, flyCam.position + (flyCam.rotation * glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
    mouseDelta = glm::vec2(0.0f);

    using shader = rast::shader::lambert_textured;
    using clipper = rast::sutherland_hodgman;
	rast::scissor scissor(0, 0, iv.width, iv.height);

    // vertex shader
    static std::vector<std::vector<shader::vertex::output>> intermidiate_buffer(9);
    static std::vector<rast::range<shader::vertex::output>> raster_ranges(9);
    tp.enque([PVM = P * V * M, &buffer = intermidiate_buffer[0], &range = raster_ranges[0]]() {
        buffer.resize(icosphere.index_buffer.size() * 2);
        shader::vertex::uniform_buffer ubo = { PVM };
        range.begin = buffer.data();
        range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        });
    tp.enque([PVM = P * V * glm::translate(M, glm::vec3(3.0f, 0.0f, 0.0f)), &buffer = intermidiate_buffer[1], &range = raster_ranges[1]]() {
        buffer.resize(icosphere.index_buffer.size() * 2);
        shader::vertex::uniform_buffer ubo = { PVM };
        range.begin = buffer.data();
        range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        });
    tp.enque([PVM = P * V * glm::translate(M, glm::vec3(-3.0f, 0.0f, 0.0f)), &buffer = intermidiate_buffer[2], &range = raster_ranges[2]]() {
        buffer.resize(icosphere.index_buffer.size() * 2);
        shader::vertex::uniform_buffer ubo = { PVM };
        range.begin = buffer.data();
        range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        });
    tp.enque([PVM = P * V * glm::translate(M, glm::vec3(0.0f, 0.0f, 3.0f)), &buffer = intermidiate_buffer[3], &range = raster_ranges[3]]() {
        buffer.resize(icosphere.index_buffer.size() * 2);
        shader::vertex::uniform_buffer ubo = { PVM };
        range.begin = buffer.data();
        range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        });
    tp.enque([PVM = P * V * glm::translate(M, glm::vec3(0.0f, 0.0f, -3.0f)), &buffer = intermidiate_buffer[4], &range = raster_ranges[4]]() {
        buffer.resize(icosphere.index_buffer.size() * 2);
        shader::vertex::uniform_buffer ubo = { PVM };
        range.begin = buffer.data();
        range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        });
    tp.enque([PVM = P * V * glm::translate(M, glm::vec3(3.0f, 0.0f, 3.0f)), &buffer = intermidiate_buffer[5], &range = raster_ranges[5]]() {
        buffer.resize(icosphere.index_buffer.size() * 2);
        shader::vertex::uniform_buffer ubo = { PVM };
        range.begin = buffer.data();
        range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        });
    tp.enque([PVM = P * V * glm::translate(M, glm::vec3(-3.0f, 0.0f, -3.0f)), &buffer = intermidiate_buffer[6], &range = raster_ranges[6]]() {
        buffer.resize(icosphere.index_buffer.size() * 2);
        shader::vertex::uniform_buffer ubo = { PVM };
        range.begin = buffer.data();
        range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        });
    tp.enque([PVM = P * V * glm::translate(M, glm::vec3(3.0f, 0.0f, -3.0f)), &buffer = intermidiate_buffer[7], &range = raster_ranges[7]]() {
        buffer.resize(icosphere.index_buffer.size() * 2);
        shader::vertex::uniform_buffer ubo = { PVM };
        range.begin = buffer.data();
        range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        });
    tp.enque([PV = P * V, &buffer = intermidiate_buffer[8], &range = raster_ranges[8]]() {
        //buffer.resize(icosphere.index_buffer.size() * 2);
        //shader::vertex::uniform_buffer ubo = { PV * glm::translate(M, glm::vec3(-3.0f, 0.0f, 3.0f))};
        //range.begin = buffer.data();
        //range.end = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(icosphere, ubo, buffer.data());
        buffer.resize(26 * 9);
        shader::vertex::uniform_buffer ubo = { PV * glm::translate(M, glm::vec3(-3.0f, 0.0f, 3.0f)) };
        range.begin = buffer.data();
        shader::vertex::output* iter = buffer.data();
        iter = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(cube, ubo, iter);
        ubo.PVM = PV * glm::translate(M, glm::vec3(-4.0f, 1.0f, 4.0f));
        iter = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(cube, ubo, iter);
        ubo.PVM = PV * glm::scale(glm::translate(M, glm::vec3(0.0f, -1.0f, 0.0)), glm::vec3(3.0f));
        iter = rast::renderer::run_vertex_shader_indexed<shader::vertex, clipper>(plane, ubo, iter);
        range.end = iter;
        });
    tp.wait();

    // render
    float stride = (float)iv.width / tp.thread_count();
    for (int i = 0; i < tp.thread_count(); ++i) {
        tp.enque([&framebuf, &scissor, i, stride, height = iv.height, PV = P * V]() {
			shader::uniform_buffer ubo;
			ubo.fragment.texture = texture;
			ubo.vertex.PVM = PV * M;
            rast::tile tile((int)(i * stride), 0, (int)((i + 1) * stride), height);
            rast::renderer::rasterize_ranges<shader>(framebuf, raster_ranges.begin(), raster_ranges.end(), ubo.fragment, scissor, tile);

			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(0.0f, 0.0f, 3.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(3.0f, 0.0f, 0.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(-3.0f, 0.0f, 0.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(0.0f, 0.0f, -3.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(3.0f, 0.0f, 3.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(-3.0f, 0.0f, -3.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(3.0f, 0.0f, -3.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(-3.0f, 0.0f, 3.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, icosphere, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(-4.0f, 1.0f, 4.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, cube, ubo, scissor, tile);
			//ubo.vertex.PVM = PV * glm::scale(glm::translate(M, glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(3.0f));
			//rast::renderer::draw_indexed<shader, clipper>(framebuf, plane, ubo, scissor, tile);
            });
    }
    tp.wait();

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

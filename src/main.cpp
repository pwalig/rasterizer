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
#include <string>
#include <chrono>
#include <bitset>
#include <filesystem>

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#define SDL_HINT_MOUSE_RELATIVE_WARP_MOTION  "SDL_MOUSE_RELATIVE_WARP_MOTION"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

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
#include "rast/command_buffer.hpp"
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
static rast::image<rast::color::rgba8> texture2;
static rast::mesh::indexed<rast::shader::inputs::position_normal_uv> icosphere;
static rast::mesh::indexed<rast::shader::inputs::position_normal_uv> cube;
static rast::mesh::indexed<rast::shader::inputs::position_normal_uv> plane;
//static thd::thread_pool tp(std::thread::hardware_concurrency());
static thd::thread_pool tp(std::thread::hardware_concurrency() - 2);
//static thd::thread_pool tp(1);
static std::bitset<256> pressed;
static glm::vec2 mouseDelta = glm::vec2(0.0f);

template <typename Shader>
struct model {
    rast::mesh::indexed<typename Shader::vertex::input> mesh;
    typename Shader::material material;
};

struct scene {
    inline static std::vector<rast::image<rast::color::rgba8>> textures = {};
    inline static std::vector<model<rast::shader::lambert_textured>> models = {};

    static void load(const char* filename) {
        std::filesystem::path scene_file_path = filename;
        scene_file_path.remove_filename();
		std::ifstream ifs(filename);
		rapidjson::IStreamWrapper isw(ifs);

		rapidjson::Document document;
		document.ParseStream(isw);
		assert(document.IsObject());

        // textures
        std::unordered_map<std::string, uint32_t> texture_pack_map;
        rapidjson::Value& texture_packs = document["resources"]["texture_packs"];
        for (rapidjson::Value::ConstMemberIterator texture_pack_iter = texture_packs.MemberBegin(); texture_pack_iter != texture_packs.MemberEnd(); ++texture_pack_iter) {
            texture_pack_map.insert({ texture_pack_iter->name.GetString(), static_cast<uint32_t>(textures.size())});

			std::filesystem::path texture_pack_path = texture_pack_iter->value["path"].GetString();
			if (texture_pack_path.is_relative()) {
				texture_pack_path = scene_file_path;
				texture_pack_path.append(texture_pack_iter->value["path"].GetString());
			}

			//textures.reserve(texture_pack["textures"].Size());
			for (rapidjson::SizeType i = 0; i < texture_pack_iter->value["textures"].Size(); i++) {
				std::filesystem::path tmp = texture_pack_path;
				tmp.append(texture_pack_iter->value["textures"][i].GetString());
				textures.push_back(rast::image<rast::color::rgba8>::load(
					tmp.string().c_str()
				));
			}

        }

        // meshes
        rapidjson::Value& objects = document["objects"];
        for (rapidjson::SizeType i = 0; i < objects.Size(); ++i) {
			std::filesystem::path meshes_path = objects[i]["meshes"].GetString();
			if (meshes_path.is_relative()) {
				meshes_path = scene_file_path;
				meshes_path.append(objects[i]["meshes"].GetString());
			}

			auto meshes = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>::load_multiple(
				meshes_path.string().c_str()
			);
			for (auto& mesh : meshes) {
				mesh.process<rast::shader::inputs::position_normal_uv::flip_uv>();
			}

			typename rast::shader::lambert_textured::material shader_mat;
            shader_mat.M = glm::mat4(1.0f);
            rapidjson::Value& material = objects[i]["material"];
            uint32_t albedo_id = texture_pack_map.at(material["albedo"]["pack"].GetString());
            for (rapidjson::SizeType i = 0; i < material["albedo"]["index"].Size(); ++i) {
                uint32_t index = material["albedo"]["index"][i].GetUint();
                shader_mat.texture = rast::texture<rast::color::rgba8>::sampler(textures[albedo_id + index]);
                shader_mat.M = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
                models.push_back({ meshes[i], shader_mat});
            }
        }
    }
};

struct sponza {
    inline static std::vector<rast::mesh::indexed<rast::shader::inputs::position_normal_uv>> meshes = {};
    inline static std::vector<rast::image<rast::color::rgba8>> textures = {};
    inline const static std::vector<int> mesh_to_texture = {
		0, 1, 2, 3, 4, 5, 6, 7, 5, 8,
		6, 5, 9, 4, 6, 4, 5, 6, 5, 6,
		5, 6, 5, 6, 5, 6, 5, 6, 5, 6,
		5, 6, 5, 6, 5, 4, 5, 4, 10, 4,
		10, 4, 10, 4, 9, 4, 8, 7, 5, 11,
		12, 4, 13, 0, 14, 15, 16, 14, 15, 14,
		16, 15, 13, 17, 18, 19, 18, 19, 18, 17,
		19, 18, 17, 20, 21, 20, 21, 20, 21, 20,
		21, 1, 2, 1, 2, 1, 2, 1, 2, 1,
		2, 1, 2, 1, 2, 22, 23, 3, 23, 3, 4, 24, 4
	};

    static void load(const char* filename) {
        std::filesystem::path scene_file_path = filename;
        scene_file_path.remove_filename();
		std::ifstream ifs(filename);
		rapidjson::IStreamWrapper isw(ifs);

		rapidjson::Document document;
		document.ParseStream(isw);
		assert(document.IsObject());

        // textures
        rapidjson::Value& texture_pack = document["resources"]["texture_packs"]["sponza"];
        std::filesystem::path texture_pack_path = texture_pack["path"].GetString();
        if (texture_pack_path.is_relative()) {
            texture_pack_path = scene_file_path;
            texture_pack_path.append(texture_pack["path"].GetString());
        }

        textures.reserve(texture_pack["textures"].Size());
        for (rapidjson::SizeType i = 0; i < texture_pack["textures"].Size(); i++) {
            std::filesystem::path tmp = texture_pack_path;
            tmp.append(texture_pack["textures"][i].GetString());
            textures.push_back(rast::image<rast::color::rgba8>::load(
                tmp.string().c_str()
            ));
        }

        // meshes
        rapidjson::Value& object = document["objects"][0];
        std::filesystem::path meshes_path = object["meshes"].GetString();
        if (meshes_path.is_relative()) {
            meshes_path = scene_file_path;
            meshes_path.append(object["meshes"].GetString());
        }

        meshes = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>::load_multiple(
            meshes_path.string().c_str()
        );
		for (auto& mesh : meshes) {
            mesh.process<rast::shader::inputs::position_normal_uv::flip_uv>();
		}
    }
};

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    int width = 1280;
    int height = 720;

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

    P = glm::perspective(glm::radians(70.0f), (float)width / height, 0.1f, 1000.0f);
    V = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    texture = rast::image<rast::color::rgba8>::load("assets/textures/uvChecker1.png");
    texture2 = rast::image<rast::color::rgba8>::load("assets/textures/neutral_normal.png");
    depth_buffer = rast::image<depth_format>(width, height);
    g_buffer = GBuffer(width, height);
    
    icosphere = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>::load("assets/models/SuzanneSmooth.mesh");
    icosphere.process(rast::shader::inputs::position_normal_uv::flip_uv);
    cube = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>::load("assets/models/cube.mesh");
    cube.process(rast::shader::inputs::position_normal_uv::flip_uv);
    plane = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>::load("assets/models/plane.mesh");
    plane.process(rast::shader::inputs::position_normal_uv::flip_uv);
    sponza::load("private/assets/scenes/sponza.json");
    scene::load("private/assets/scenes/sponza.json");

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

		P = glm::perspective(glm::radians(70.0f), (float)event->window.data1 / (float)event->window.data2, 0.1f, 1000.0f);

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
    iv.clear(rast::color::rgba8(25, 25, 50, 255));


    // model matrix
    static glm::mat4 M = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 0.0f)), glm::vec3(2.0f));

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

    static rast::command_buffer<shader> cmd_buffer;
    cmd_buffer.reset();
	glm::mat4 PV = P * V;

    shader::environment env;
    env.PV = PV;
    for (auto& model : scene::models) {
        cmd_buffer.draw_indexed(model.mesh, shader::get_ubo(model.material, env), scissor);
    }


	shader::uniform_buffer ubo;
 //   ubo.vertex.PVM = PV * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
	//for (int i = 0; i < sponza::meshes.size(); ++i) {
 //       ubo.fragment.texture = sponza::textures[sponza::mesh_to_texture[i]];
 //       cmd_buffer.draw_indexed(sponza::meshes[i], ubo, scissor);
 //   }
	ubo.fragment.texture = texture;
	ubo.vertex.PVM = PV * M;
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(3.0f, 0.0f, 0.0f));
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(-3.0f, 0.0f, 0.0f));
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(0.0f, 0.0f, 3.0f));
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(0.0f, 0.0f, -3.0f));
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(3.0f, 0.0f, 3.0f));
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(3.0f, 0.0f, -3.0f));
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(-3.0f, 0.0f, 3.0f));
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::translate(M, glm::vec3(-3.0f, 0.0f, -3.0f));
	cmd_buffer.draw_indexed(icosphere, ubo, scissor);
	ubo.vertex.PVM = PV * glm::scale(glm::translate(M, glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(3.0f));
	cmd_buffer.draw_indexed(plane, ubo, scissor);
	cmd_buffer.submit<clipper>(framebuf, tp);

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

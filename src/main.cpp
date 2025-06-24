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

//using GBuffer = rast::image<rast::shader::deferred::first_pass::fragment::output>;
//static GBuffer g_buffer;
//static thd::thread_pool tp(std::thread::hardware_concurrency());
static thd::thread_pool tp(std::thread::hardware_concurrency() - 2);
//static thd::thread_pool tp(1);
static std::bitset<256> pressed;
static glm::vec2 mouseDelta = glm::vec2(0.0f);

template <typename Shader>
struct material { };

template <>
struct material<rast::shader::lambert_textured> {
    uint32_t texture_id;
};

template <typename Shader>
struct model {
    rast::mesh::indexed<typename Shader::vertex::input> mesh;
    material<Shader> mat;
};

struct object {
    struct model {
        uint32_t mesh_id = 0;
        uint32_t material_id = 0;
    };
    std::vector<model> models;
    glm::mat4 M = glm::mat4(1.0f);
};

struct scene {
    using shader = rast::shader::lambert_textured;
    using mesh_type = rast::mesh::indexed<rast::shader::inputs::position_normal_uv>;

    std::vector<rast::image<rast::color::rgba8>> textures;
    std::vector<mesh_type> meshes;
    std::vector<material<shader>> materials;
    std::vector<object> objects;

    static std::filesystem::path get_full_path(const char* path, const std::filesystem::path& root) {
        std::filesystem::path res = path;
        if (res.is_relative()) {
            res = root;
            res.append(path);
        }
		return res;
    }

    void load(const char* filename) {
        std::filesystem::path scene_file_path = filename;
        scene_file_path.remove_filename();
		std::ifstream ifs(filename);
		rapidjson::IStreamWrapper isw(ifs);

		rapidjson::Document document;
		document.ParseStream(isw);
		assert(document.IsObject());

        // textures
        {
			const rapidjson::Value& textures_node = document["textures"];
            textures.reserve(textures_node.Size());
            for (rapidjson::SizeType i = 0; i < textures_node.Size(); ++i) {
                const rapidjson::Value& texture_node = textures_node[i];
                std::filesystem::path texture_path = get_full_path(texture_node.GetString(), scene_file_path);
                textures.emplace_back(rast::image<rast::color::rgba8>::load(
                    texture_path.string().c_str()
                ));
            }
        }
        // texture packs
        std::unordered_map<std::string, uint32_t> texture_pack_map;
        {
			const rapidjson::Value& texture_packs_node = document["texture_packs"];
			for (rapidjson::Value::ConstMemberIterator texture_pack_iter = texture_packs_node.MemberBegin(); texture_pack_iter != texture_packs_node.MemberEnd(); ++texture_pack_iter) {
				texture_pack_map.insert({ texture_pack_iter->name.GetString(), static_cast<uint32_t>(textures.size())});
				const rapidjson::Value& texture_pack_node = texture_pack_iter->value;

				std::filesystem::path texture_pack_path = get_full_path(
					texture_pack_node["path"].GetString(),
					scene_file_path
				);

				const rapidjson::Value& textures_node = texture_pack_node["textures"];
				for (rapidjson::SizeType i = 0; i < textures_node.Size(); i++) {
					std::filesystem::path tmp = texture_pack_path;
					tmp.append(textures_node[i].GetString());
					textures.push_back(rast::image<rast::color::rgba8>::load(
						tmp.string().c_str()
					));
				}

			}
		}

        // meshes
        {
            const rapidjson::Value& meshes_node = document["meshes"];
			for (rapidjson::SizeType i = 0; i < meshes_node.Size(); ++i) {
				std::filesystem::path mesh_path = get_full_path(
					meshes_node[i].GetString(),
					scene_file_path
				);
				if (mesh_path.extension() == ".meshes") {
					auto tmp_meshes = mesh_type::load_multiple(
						mesh_path.string().c_str()
					);
					for (auto& mesh : tmp_meshes) meshes.push_back(mesh);
				}
				else if (mesh_path.extension() == ".mesh") {
					auto tmp_mesh = mesh_type::load(
						mesh_path.string().c_str()
					);
					meshes.push_back(tmp_mesh);
				}
				else throw std::runtime_error("wrong file extension");
			}
			for (auto& mesh : meshes) {
				mesh.process<rast::shader::inputs::position_normal_uv::flip_uv>();
			}
        }


        // materials
        {
			const rapidjson::Value& materials_node = document["materials"];
			materials.reserve(materials_node.Size());
			for (rapidjson::SizeType i = 0; i < materials_node.Size(); ++i) {
				const rapidjson::Value& material_node = materials_node[i];
				const rapidjson::Value& texture_node = material_node["texture"];
				uint32_t texture_id = texture_node["index"].GetUint();
                if (texture_node.HasMember("pack")) {
                    texture_id += texture_pack_map.at(texture_node["pack"].GetString());
                }
				materials.push_back({ texture_id });
			}
        }


        // objects
        {
			const rapidjson::Value& objects_node = document["objects"];
            objects.reserve(objects_node.Size());
            for (rapidjson::SizeType i = 0; i < objects_node.Size(); ++i) {
                const rapidjson::Value& object_node = objects_node[i];
                object obj;

                if (object_node.HasMember("M")) {
                    const rapidjson::Value& M_node = object_node["M"];
                    if (M_node.HasMember("position")) {
                        obj.M = glm::translate(obj.M, glm::vec3(
                            M_node["position"][0].GetFloat(),
                            M_node["position"][1].GetFloat(),
                            M_node["position"][2].GetFloat()
                        ));
                    }
                    if (M_node.HasMember("scale")) {
                        obj.M = glm::scale(obj.M, glm::vec3(
                            M_node["scale"][0].GetFloat(),
                            M_node["scale"][1].GetFloat(),
                            M_node["scale"][2].GetFloat()
                        ));
                    }
                }

                const rapidjson::Value& models_node = object_node["models"];
                obj.models.reserve(models_node.Size());
                for (rapidjson::SizeType i = 0; i < models_node.Size(); ++i) {
                    const rapidjson::Value& model_node = models_node[i];
                    object::model om;
                    om.mesh_id = model_node["mesh"]["index"].GetUint();
                    om.material_id = model_node["material"]["index"].GetUint();
                    obj.models.push_back(om);
                }
                objects.push_back(obj);
            }
        }
    }

    template <typename Callable>
    void draw(const glm::mat4& PV, Callable&& draw_call) const {
        shader::uniform_buffer ubo;
        for (const auto& object : objects) {
            ubo.vertex.PVM = PV * object.M;
            for (const auto model : object.models) {
                ubo.fragment.texture = textures[materials[model.material_id].texture_id];
                draw_call(meshes[model.mesh_id], ubo);
            }
        }
    }
};

struct application {
    using depth_format = uint32_t;
    using DepthBuffer = rast::image<depth_format>;
    using color_format = rast::color::rgba8;
    using Framebuffer = rast::framebuffer::color_depth<color_format, depth_format>;

	SDL_Window *window = NULL;
	SDL_Surface* surface = nullptr;

    float fov = 70.0f;
    float near = 0.1f;
    float far = 1000.0f;

    glm::mat4 V = glm::lookAt(glm::vec3(5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(fov, 16.0f / 9.0f, near, far);
    scene scene;

    rast::image<depth_format> depth_buffer;
};

static application app;


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

    app.window = SDL_CreateWindow("rasterizer", width, height, SDL_WINDOW_RESIZABLE);
	if (!app.window) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
	app.surface = SDL_CreateSurface(width, height, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA32);

    app.P = glm::perspective(glm::radians(app.fov), (float)width / height, app.near, app.far);
    app.V = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    app.depth_buffer = rast::image<application::depth_format>(width, height);
    //g_buffer = GBuffer(width, height);
    
    app.scene.load("private/assets/scenes/sponza.json");
    //app.scene.load("assets/scenes/scene.json");

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_DestroySurface(app.surface);
        app.surface = SDL_CreateSurface(event->window.data1, event->window.data2, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceBlendMode(app.surface, SDL_BLENDMODE_NONE);

		app.P = glm::perspective(glm::radians(app.fov), (float)event->window.data1 / (float)event->window.data2, app.near, app.far);

		app.depth_buffer.resize<rast::resize_filter::dont_care>(event->window.data1, event->window.data2);
        //g_buffer.resize<rast::resize_filter::dont_care>(event->window.data1, event->window.data2);
    }
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.scancode == SDL_SCANCODE_ESCAPE && !SDL_CursorVisible()) {
            SDL_SetWindowRelativeMouseMode(app.window, false);
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
            SDL_SetWindowRelativeMouseMode(app.window, true);
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
    //GBuffer::view gv(g_buffer);
    //rast::framebuffer::color_depth<GBuffer::color, rast::uint32_t> framebuf(gv, depth_buffer);
    //rast::framebuffer::depth_view<rast::color::rgba8, application::depth_format> framebuf(
    //    (rast::color::rgba8*)app.surface->pixels, app.depth_buffer,
    //    app.near, app.far, 0.0f, 0.1f
    //);
    application::Framebuffer framebuf((rast::color::rgba8*)app.surface->pixels, app.depth_buffer);
    framebuf.clear_depth_buffer();
    framebuf.clear_color(rast::color::rgba8(25, 25, 50, 255));
    //rast::framebuffer::rgba8 noDepthFramebuffer((rast::color::rgba8*)surface->pixels, surface->w, surface->h);
    //framebuf.clear_color({glm::vec3(0.0f), glm::vec3(0.0f), rast::color::rgba8(0, 0, 0, 255)});


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
    app.V = glm::lookAt(flyCam.position, flyCam.position + (flyCam.rotation * glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
    mouseDelta = glm::vec2(0.0f);

    using shader = rast::shader::lambert_textured;
    using clipper = rast::sutherland_hodgman;
    using blending = rast::alpha_blend::func<rast::alpha_blend::factor::src_alpha, rast::alpha_blend::factor::one_minus_src_alpha, rast::alpha_blend::equation::add>;

    // record command buffer
    static rast::command_buffer<shader> cmd_buffer;
    cmd_buffer.reset();
	glm::mat4 PV = app.P * app.V;
    app.scene.draw(PV,
        [viewport = rast::scissor(0, 0, framebuf.width(), framebuf.height())]
        (const scene::mesh_type& mesh, const shader::uniform_buffer& ubo)
        { cmd_buffer.draw_indexed(mesh, ubo, viewport); }
    );
	cmd_buffer.submit<clipper, blending>(framebuf, tp);

    // present to screen
	tp.wait();
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = app.surface->w;
    rect.h = app.surface->h;
    SDL_BlitSurface(app.surface, &rect, SDL_GetWindowSurface(app.window), &rect);
    SDL_UpdateWindowSurface(app.window);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
	SDL_DestroySurface(app.surface);
}

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <bitset>

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#define SDL_HINT_MOUSE_RELATIVE_WARP_MOTION  "SDL_MOUSE_RELATIVE_WARP_MOTION"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define STB_IMAGE_IMPLEMENTATION
#include "rast/color.hpp"
#include "rast/mesh.hpp"
#include "rast/sampler.hpp"
#include "rast/shader/lambert_textured.hpp"
#include "rast/shader/deferred.hpp"
#include "rast/framebuffer/color_depth.hpp"
#include "rast/command_buffer.hpp"
#include "rast/clip/sutherland_hodgman.hpp"
#include "fly_cam.hpp"
#include "rast/raster/vbbox_scan.hpp"
#include <rast/thread_pool.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static rast::thread_pool tp(std::thread::hardware_concurrency() - 2);
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

	std::vector<rast::mipmapped_image<rast::color::rgba8>> textures;
	std::vector<mesh_type> meshes;
	std::vector<material<shader>> materials;
	std::vector<object> objects;

	static void append_objects(const aiNode* node, std::vector<object>& objects, aiMatrix4x4 accTransform) {
		std::cout << node->mName.C_Str() << '\n';
		std::cout << "NumMeshes: " << node->mNumMeshes << '\n';
		if (node->mNumMeshes > 0) {
			objects.push_back(object());
			for (size_t i = 0; i < node->mNumMeshes; ++i) {
				objects.back().models.push_back({ node->mMeshes[i], 0 });
				auto M = node->mTransformation * accTransform;
				objects.back().M = glm::mat4(
					M.a1, M.a2, M.a3, M.a4,
					M.b1, M.b2, M.b3, M.b4,
					M.c1, M.c2, M.c3, M.c4,
					M.d1, M.d2, M.d3, M.d4
				);
			}
		}
		std::cout << "NumChildren: " << node->mNumChildren << '\n';
		for (size_t i = 0; i < node->mNumChildren; ++i) {
			append_objects(node->mChildren[i], objects, node->mTransformation * accTransform);
		}
	}

	static scene assimp_load(const char* filename) {
		scene res;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
		if (scene == nullptr) throw std::runtime_error(std::string("could not read file ") + filename);
		std::cout << "NumMeshes: " << scene->mNumMeshes << '\n';
		std::vector<uint32_t> material_used_by_mesh(scene->mNumMeshes);
		for (size_t i = 0; i < scene->mNumMeshes; ++i) {
			const aiMesh* aimesh = scene->mMeshes[i];
			material_used_by_mesh[i] = aimesh->mMaterialIndex;
			mesh_type mesh;
			mesh.vertex_buffer.reserve(aimesh->mNumVertices);
			mesh.index_buffer.reserve(aimesh->mNumFaces);
			for (size_t j = 0; j < aimesh->mNumVertices; ++j) {
				mesh.vertex_buffer.push_back({
					glm::vec3(aimesh->mVertices[j].x, aimesh->mVertices[j].y, aimesh->mVertices[j].z),
					glm::vec3(aimesh->mNormals[j].x, aimesh->mNormals[j].y, aimesh->mNormals[j].z),
					glm::vec2(aimesh->mTextureCoords[0][j].x, aimesh->mTextureCoords[0][j].y)
				});
			}
			for (size_t j = 0; j < aimesh->mNumFaces; ++j) {
				assert(aimesh->mFaces[j].mNumIndices == 3);
				mesh.index_buffer.push_back(aimesh->mFaces[j].mIndices[0]);
				mesh.index_buffer.push_back(aimesh->mFaces[j].mIndices[1]);
				mesh.index_buffer.push_back(aimesh->mFaces[j].mIndices[2]);
			}
			res.meshes.push_back(mesh);
		}
		std::cout << "NumTextures: " << scene->mNumTextures << '\n';
		for (size_t i = 0; i < scene->mNumTextures; ++i) {
			res.textures.push_back(rast::mipmapped_image<rast::color::rgba8>(
				rast::image<rast::color::rgba8>::load(
					scene->mTextures[i]->mFilename.C_Str()
				)));
		}
		std::cout << "NumMaterials: " << scene->mNumMaterials << '\n';
		for (size_t i = 0; i < scene->mNumMaterials; ++i) {
			const aiMaterial* aimaterial = scene->mMaterials[i];
			aiString data;
			aimaterial->Get(AI_MATKEY_NAME, data);
			std::cout << '\t' << data.C_Str() << '\n';
			if (aimaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), data) == AI_SUCCESS) {
				std::cout << "\t\tdiffuse: " << data.C_Str() << '\n';
				res.textures.push_back(rast::mipmapped_image<rast::color::rgba8>(
					rast::image<rast::color::rgba8>::load(data.C_Str())
				));
			}
			if (aimaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), data) == AI_SUCCESS) std::cout << "\t\tbase color: " << data.C_Str() << '\n';
			if (aimaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), data) == AI_SUCCESS) std::cout << "\t\tnormal: " << data.C_Str() << '\n';
			if (aimaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0), data) == AI_SUCCESS) std::cout << "\t\tmetallic roughness: " << data.C_Str() << '\n';
			res.materials.push_back({ static_cast<uint32_t>(res.textures.size() - 1) });
		}
		append_objects(scene->mRootNode, res.objects, aiMatrix4x4());
		for (auto& obj : res.objects) for (auto& m : obj.models) m.material_id = material_used_by_mesh[m.mesh_id];
		return res;
	}

	template <typename Shader = shader, typename Callable>
	void draw(const glm::mat4& PV, Callable draw_call) const {
		typename Shader::uniform_buffer ubo;
		for (const auto& object : objects) {
			ubo.PVM = PV * object.M;
			for (const auto& model : object.models) {
				ubo.texture = textures[materials[model.material_id].texture_id];
				draw_call(meshes[model.mesh_id], ubo);
			}
		}
	}
};

struct application {
	using depth_format = float;
	using DepthBuffer = rast::image<depth_format>;
	using color_format = rast::color::rgba8;
	using blending = rast::alpha_blend::func<rast::alpha_blend::factor::src_alpha, rast::alpha_blend::factor::one_minus_src_alpha, rast::alpha_blend::equation::add>;
	using Framebuffer = rast::framebuffer::color_depth<color_format, rast::shader::lambert_textured::fragment::simd_shade<4>, rast::shader::lambert_textured::fragment::simd_blend<4>, rast::depth_test::less>;

	using g_format = rast::shader::deferred::first_pass::fragment::output;
	using GBuffer = rast::image<g_format>;
	using GFramebuffer = rast::framebuffer::color_depth<g_format, rast::shader::deferred::first_pass::fragment::shade, rast::alpha_blend::replace<g_format>, rast::depth_test::less>;
	GBuffer g_buffer;

	SDL_Window *window = NULL;
	SDL_Surface* surface = nullptr;

	float fov = 70.0f;
	float near = 0.1f;
	float far = 1000.0f;

	glm::mat4 V = glm::lookAt(glm::vec3(5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(fov, 16.0f / 9.0f, near, far);
	scene scene;

	rast::image<depth_format> depth_buffer;

	inline const static bool is_deffered = false;
	inline const static bool render_on_demand_only = true;
	bool request_frame = true;

	template <typename Shader, typename Framebuffer, typename ThreadPool>
	inline void draw(
		Framebuffer& framebuf,
		ThreadPool& tp
	) {
		using clipper = rast::sutherland_hodgman;
		using rasterizer = rast::raster::vbbox_scan;
		static rast::command_buffer<Shader> cmd_buffer;
		cmd_buffer.reset();
		glm::mat4 PV = P * V;
		scene.draw<Shader>(PV,
			[viewport = rast::viewport(0, 0, framebuf.width(), framebuf.height())]
			(const scene::mesh_type& mesh, const typename Shader::uniform_buffer& ubo)
			{ cmd_buffer.draw_indexed(mesh, ubo, viewport); }
		);
		cmd_buffer.template submit<rasterizer, clipper>(framebuf, tp);
	}
};

static application app;


/* This function runs once at startup. */
SDL_AppResult SDL_AppInit([[maybe_unused]]void **appstate, int argc, char ** argv)
{
	int width = 1280;
	int height = 720;

	SDL_SetAppMetadata("Software Rasterizer Program", "1.0", "com.example.pwalig.rasterizer");

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
	if constexpr(application::is_deffered) app.g_buffer = application::GBuffer(width, height);
	
	if (argc == 1) app.scene = scene::assimp_load("scene.gltf");
	else scene::assimp_load(argv[1]);

	return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent([[maybe_unused]]void *appstate, SDL_Event *event)
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
		if constexpr(application::is_deffered) app.g_buffer.resize<rast::resize_filter::dont_care>(event->window.data1, event->window.data2);
		app.request_frame = true;
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
		if (event->key.scancode == SDL_SCANCODE_L) {
			rast::shader::lambert_textured::fragment::linear = !rast::shader::lambert_textured::fragment::linear;
			app.request_frame = true;
		}
		else if (event->key.scancode == SDL_SCANCODE_M) {
			rast::shader::lambert_textured::fragment::mipmap = !rast::shader::lambert_textured::fragment::mipmap;
			app.request_frame = true;
		}
		else if (event->key.scancode == SDL_SCANCODE_K) {
			rast::shader::lambert_textured::fragment::linear_mipmap = !rast::shader::lambert_textured::fragment::linear_mipmap;
			app.request_frame = true;
		}
		else if (event->key.scancode >= SDL_SCANCODE_1 && event->key.scancode <= SDL_SCANCODE_9) {
			rast::shader::lambert_textured::fragment::mip_to_sample = event->key.scancode - SDL_SCANCODE_1;
			app.request_frame = true;
		}
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
SDL_AppResult SDL_AppIterate([[maybe_unused]]void *appstate)
{
	//measure time
	using clock = std::chrono::high_resolution_clock;
	static auto last_frame = clock::now();
	auto now = clock::now();
	float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame).count();
	last_frame = now;

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
	if (app.render_on_demand_only) {
		static bool last_frame_rendered = false;
		if (app.request_frame) app.request_frame = false;
		else if (movement == glm::vec3(0.0f, 0.0f, 0.0f) && (mouseDelta == glm::vec2(0.0f, 0.0f) || SDL_CursorVisible())) {
			if (last_frame_rendered) std::cout << dt << "\r";
			last_frame_rendered = false;
			return SDL_APP_CONTINUE;
		}
		last_frame_rendered = true;
	}
	std::cout << dt << "\r";
	app.V = glm::lookAt(flyCam.position, flyCam.position + (flyCam.rotation * glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	mouseDelta = glm::vec2(0.0f);


	// prepare framebuffers
	// deffered
	if constexpr (application::is_deffered) {
		application::GFramebuffer framebuf(app.g_buffer, app.depth_buffer);
		framebuf.clear_depth_buffer();
		framebuf.clear_color({glm::vec3(0.0f), rast::color::rgba8(0, 0, 0, 255)});
		rast::image<rast::color::rgba8>::view out_framebuf((rast::color::rgba8*)app.surface->pixels, app.surface->w, app.surface->h);

		// first pass
		app.draw<rast::shader::deferred::first_pass>(framebuf, tp);

		tp.wait();

		// second pass
		rast::shader::deferred::second_pass::fragment::uniform_buffer ubo;
		ubo.texture = rast::sampler<application::g_format>(app.g_buffer);
		rast::shade_screen_quad<rast::shader::deferred::second_pass::fragment::shade>(out_framebuf, tp, ubo);
	}
	else {
		// forward
		application::Framebuffer framebuf((rast::color::rgba8*)app.surface->pixels, app.depth_buffer);
		framebuf.clear_depth_buffer();
		framebuf.clear_color(rast::color::rgba8(25, 25, 50, 255));

		// depth view
		//rast::framebuffer::depth_view<rast::color::rgba8, application::depth_format, rast::depth_test::less> framebuf(
		//    (rast::color::rgba8*)app.surface->pixels, app.depth_buffer,
		//    app.near, app.far, 0.0f, 0.1f
		//);
		//framebuf.clear_depth_buffer();
		//framebuf.clear_color(rast::color::rgba8(0, 0, 0, 255));

		// no depth
		//rast::framebuffer::rgba8 framebuf((rast::color::rgba8*)app.surface->pixels, app.surface->w, app.surface->h);

		app.draw<rast::shader::lambert_textured>(framebuf, tp);
	}

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
void SDL_AppQuit([[maybe_unused]]void *appstate, SDL_AppResult)
{
	/* SDL will clean up the window/renderer for us. */
	SDL_DestroySurface(app.surface);
}

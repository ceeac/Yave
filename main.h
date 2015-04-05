/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef MAIN
#define MAIN

#include <n/core/Timer.h>

#include <n/math/Plane.h>
#include <n/io/File.h>

#include <SDL2/SDL.h>
#include <n/graphics/ImageLoader.h>
#include <n/graphics/GLContext.h>
#include <n/graphics/Texture.h>
#include <n/graphics/Scene.h>
#include <n/graphics/GL.h>
#include <n/graphics/StaticBuffer.h>
#include <n/graphics/TriangleBuffer.h>
#include <n/graphics/VertexArrayObject.h>
#include <n/graphics/ShaderCombinaison.h>
#include <n/graphics/Camera.h>
#include <n/graphics/StaticMesh.h>
#include <n/graphics/Material.h>
#include <n/graphics/MeshLoader.h>
#include <n/graphics/MaterialLoader.h>
#include <n/graphics/SceneRenderer.h>
#include <n/graphics/FrameBufferRenderer.h>
#include <n/graphics/FrameBuffer.h>
#include <n/graphics/GBufferRenderer.h>
#include <n/graphics/DynamicBuffer.h>
#include <n/graphics/DeferredShadingRenderer.h>
#include <n/graphics/ScreenShaderRenderer.h>
#include <n/graphics/Light.h>


#include <n/math/StaticConvexVolume.h>
#include <n/math/ConvexVolume.h>

#include "Console.h"

Vec2 mouse;
Vec2 dMouse;


class IThread : public n::concurrent::Thread
{
	public:
		IThread(uint *x) : n::concurrent::Thread(), i(x) {
		}

		virtual void run() override {
			while(true) {
				int w = -1;
				std::cin>>w;
				if(w < 0 || w > 2) {
					std::cerr<<"Invalid input"<<std::endl;
					continue;
				}
				*i = uint(w);
			}
		}

	private:
		uint *i;

};

SDL_Window *createWindow() {
	SDL_Window *mainWindow = 0;
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fatal("Unable to initialize SDL");
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	if(!(mainWindow = SDL_CreateWindow("n 2.1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN))) {
		fatal("Unable to create window");
	}
	SDL_GL_CreateContext(mainWindow);
	SDL_GL_SetSwapInterval(0);

	GLContext::getContext();

	return mainWindow;
}

bool run(SDL_Window *mainWindow) {
	SDL_GL_SwapWindow(mainWindow);
	SDL_Event e;
	bool cc = true;
	static bool m1 = false;
	dMouse = Vec2();
	while(SDL_PollEvent(&e)) {
		if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
			cc = false;
			break;
		} else if(e.type == SDL_MOUSEMOTION && m1) {
			mouse += dMouse = Vec2(e.motion.xrel, e.motion.yrel) * 0.01;
		} else if(e.type == SDL_MOUSEBUTTONDOWN) {
			m1 |= e.button.button == SDL_BUTTON_LEFT;
		} else if(e.type == SDL_MOUSEBUTTONUP) {
			m1 &= e.button.button != SDL_BUTTON_LEFT;
		}
	}
	return cc;
}

class Obj : public StaticMesh
{
	public:
		Obj(String n) : StaticMesh(MeshLoader::load<String>(n)), model(n), autoScale(0) {
		}

		Obj(const MeshInstance<> &n) : StaticMesh(n), model(""), autoScale(0) {
		}

		void setAttribs(const VertexAttribs &a) {
			attribs = a;
		}

		virtual void render(RenderQueue &qu) override {
			if(!getMeshInstance().isValid()) {
				fatal("Unable to load mesh");
			}
			if(autoScale && !getMeshInstance().isNull()) {
				setScale(autoScale / getMeshInstance().getRadius());
			}
			for(MeshInstanceBase<> *b : getMeshInstance()) {
				qu.insert(RenderBatch(getTransform().getMatrix(), b, attribs));
			}
		}

		void setAutoScale(float s) {
			autoScale = s;
		}

	private:
		VertexAttribs attribs;
		String model;
		float autoScale;
};

class RandObj : public Obj
{
	public:
		RandObj(float x = 1000) : Obj("scube.obj") {
			setPosition(Vec3(random(), random(), random()) * 1000 - x / 2);
		}
};



template<int I>
class Dummy
{
};

template<int I>
class DummyObj : public Dummy<I>, public DummyObj<I - 1>
{

	public:
		static void insert(uint max, Scene *s) {
			for(uint i = 0; i != max; i++) {
				s->insert(new DummyObj<I>());
			}
			DummyObj<I - 1>::insert(max, s);
		}
};

template<>
class DummyObj<0> : public Movable<>, public Renderable
{
	public:
		virtual void render(RenderQueue &) override {
		}

		static void insert(uint max, Scene *s) {
			for(uint i = 0; i != max; i++) {
				s->insert(new DummyObj<0>());
			}
		}

};

#endif // MAIN


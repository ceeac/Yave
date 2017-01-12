/*******************************
Copyright (c) 2016-2017 Grégoire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/
#ifndef YAVE_YAVEAPP_H
#define YAVE_YAVEAPP_H

#include <yave/yave.h>

#include <yave/Device.h>

#include <yave/material/Material.h>
#include <yave/commands/CmdBufferRecorder.h>
#include <yave/image/Image.h>
#include <yave/Swapchain.h>
#include <yave/objects/StaticMesh.h>
#include <yave/mesh/MeshInstancePool.h>

#include <yave/bindings/Binding.h>
#include <yave/bindings/DescriptorSet.h>
#include <yave/scene/SceneView.h>

#include <yave/renderers/DeferredRenderer.h>
#include <yave/renderers/ClearRenderer.h>

namespace yave {

class Window;

class YaveApp : NonCopyable {


	public:
		YaveApp(DebugParams params);
		~YaveApp();

		void init(Window* window);

		Duration draw();
		void update(math::Vec2 angles = math::Vec2(0));


	private:
		void create_command_buffers();

		void create_assets();

		Instance instance;
		Device device;

		Swapchain* swapchain;

		CmdBufferPool<CmdBufferUsage::Normal> command_pool;
		core::Vector<RecordedCmdBuffer<CmdBufferUsage::Normal>> command_buffers;

		AssetPtr<Material> material;

		MeshInstancePool mesh_pool;

		Texture mesh_texture;

		Scene* scene;
		SceneView* scene_view;
		Camera camera;

		DeferredRenderer* renderer;
};

}

#endif // YAVE_YAVEAPP_H

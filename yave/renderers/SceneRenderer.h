/*******************************
Copyright (c) 2016-2017 Gr�goire Angerand

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
#ifndef YAVE_RENDERERS_SCENERENDERER_H
#define YAVE_RENDERERS_SCENERENDERER_H

#include "CullingNode.h"

namespace yave {

class SceneRenderer : public SecondaryRenderer {

		using Recorder = CmdBufferRecorder<CmdBufferUsage::Secondary>;
	public:
		template<typename T>
		using Ptr = core::Rc<T>;

		SceneRenderer(DevicePtr dptr, const Ptr<CullingNode>& cull);

		const SceneView& scene_view() const;

		RecordedCmdBuffer<CmdBufferUsage::Secondary> process(const FrameToken& token, const Framebuffer& framebuffer) override;

	private:
		void render_renderables(Recorder& recorder, const FrameToken& token, const core::Vector<const Renderable*>& renderables);
		void render_static_meshes(Recorder& recorder, const core::Vector<const StaticMesh*>& meshes);

		void setup_instance(Recorder& recorder, const AssetPtr<StaticMeshInstance>& instance);
		void submit_batches(Recorder& recorder, AssetPtr<Material>& mat, usize offset, usize size);

		Ptr<CullingNode> _cull;

		TypedBuffer<uniform::ViewProj, BufferUsage::UniformBit> _camera_buffer;
		TypedMapping<uniform::ViewProj, MemoryFlags::CpuVisible> _camera_mapping;
		DescriptorSet _camera_set;

		TypedBuffer<math::Matrix4<>, BufferUsage::AttributeBit, MemoryFlags::CpuVisible> _matrix_buffer;
		TypedMapping<math::Matrix4<>, MemoryFlags::CpuVisible> _matrix_mapping;

		TypedBuffer<vk::DrawIndexedIndirectCommand, BufferUsage::IndirectBit, MemoryFlags::CpuVisible> _indirect_buffer;
		TypedMapping<vk::DrawIndexedIndirectCommand, MemoryFlags::CpuVisible> _indirect_mapping;

		RecordedCmdBuffer<CmdBufferUsage::Secondary> _cmd_buffer;

};

}

#endif // YAVE_RENDERERS_SCENERENDERER_H

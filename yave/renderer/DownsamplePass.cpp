/*******************************
Copyright (c) 2016-2020 Gr�goire Angerand

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

#include "DownsamplePass.h"

#include <yave/graphics/shaders/ComputeProgram.h>
#include <yave/framegraph/FrameGraphPassBuilder.h>
#include <yave/framegraph/FrameGraph.h>

namespace yave {

DownsamplePass DownsamplePass::create(FrameGraph& framegraph, FrameGraphImageId orig, usize max_downsampling) {
	const math::Vec2ui orig_size = framegraph.image_size(orig);
	const ImageFormat format = framegraph.image_format(orig);

	DownsamplePass pass;
	pass.original = orig;

	FrameGraphImageId last = orig;
	for(usize m = 1;; ++m) {
		const math::Vec2ui mip_size = math::Vec2ui(orig_size.x() >> m, orig_size.y() >> m);
		if(!mip_size.x() || !mip_size.y() || (1u << m) > max_downsampling) {
			break;
		}

		FrameGraphPassBuilder builder = framegraph.add_pass(fmt("Downsample pass x%", 1 << m));

		const auto mip = builder.declare_image(format, mip_size);

		builder.add_color_output(mip);
		builder.add_uniform_input(last);
		builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
			auto render_pass = recorder.bind_framebuffer(self->framebuffer());
			const auto* material = recorder.device()->device_resources()[DeviceResources::DownsampleMaterialTemplate];
			render_pass.bind_material(material, {self->descriptor_sets()[0]});

			VkDrawIndirectCommand command = {};
			{
				command.vertexCount = 3;
				command.instanceCount = 1;
			}
			render_pass.draw(command);
		});

		pass.mips << mip;
		last = mip;
	}

	return pass;
}

}
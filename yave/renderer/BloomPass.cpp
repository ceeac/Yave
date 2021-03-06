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

#include "BloomPass.h"
#include "ToneMappingPass.h"
#include "BlurPass.h"

#include <yave/framegraph/FrameGraphPassBuilder.h>
#include <yave/framegraph/FrameGraph.h>

namespace yave {

BloomPass BloomPass::create(FrameGraph& framegraph, FrameGraphImageId tone_mapped, const BloomPassSettings& settings) {
	const math::Vec2ui size = framegraph.image_size(tone_mapped);
	const ImageFormat format = framegraph.image_format(tone_mapped);
	const math::Vec2ui internal_size = size / 2;

	FrameGraphMutableImageId thresholded;
	{
		FrameGraphPassBuilder builder = framegraph.add_pass("Bloom pass");

		struct BloomParams {
			float power;
			float threshold;
			float rev_threshold;
		} params {
			settings.bloom_power,
			settings.bloom_threshold,
			1.0f / (1.0f - settings.bloom_threshold)
		};

		thresholded = builder.declare_image(format, internal_size);

		const auto param_buffer = builder.declare_typed_buffer<BloomParams>();

		builder.add_color_output(thresholded);
		builder.add_uniform_input(tone_mapped);
		builder.add_uniform_input(param_buffer);
		builder.map_update(param_buffer);
		builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
			self->resources().mapped_buffer(param_buffer)[0] = params;
			auto render_pass = recorder.bind_framebuffer(self->framebuffer());
			const auto* material = recorder.device()->device_resources()[DeviceResources::BloomMaterialTemplate];
			render_pass.bind_material(material, {self->descriptor_sets()[0]});
			render_pass.draw_array(3);
		});
	}

	const BlurPass blur = BlurPass::create(framegraph, thresholded);

	FrameGraphMutableImageId bloomed;
	{
		FrameGraphPassBuilder builder = framegraph.add_pass("Bloom merge pass");

		bloomed = builder.declare_copy(tone_mapped);

		builder.add_color_output(bloomed);
		builder.add_uniform_input(blur.blurred);
		builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
			auto render_pass = recorder.bind_framebuffer(self->framebuffer());
			const auto* material = recorder.device()->device_resources()[DeviceResources::ScreenBlendPassthroughMaterialTemplate];
			render_pass.bind_material(material, {self->descriptor_sets()[0]});
			render_pass.draw_array(3);
		});
	}


	BloomPass pass;
	pass.bloomed = bloomed;

	return pass;
}

}

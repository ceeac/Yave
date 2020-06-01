/*******************************
Copyright (c) 2016-2020 Grégoire Angerand

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

#include "MainWindow.h"

#include <editor/utils/crashhandler.h>
#include <editor/events/MainEventHandler.h>
#include <editor/context/EditorContext.h>

#include <yave/graphics/swapchain/Swapchain.h>
#include <yave/assets/SQLiteAssetStore.h>

#include <y/core/Chrono.h>
#include <y/concurrent/concurrent.h>

#ifdef Y_OS_WIN
#include <windows.h>
#endif

#if 0

using namespace editor;

static EditorContext* context = nullptr;

#ifdef Y_DEBUG
static bool display_console = true;
static bool debug_instance = true;
#else
static bool display_console = false;
static bool debug_instance = false;
#endif



static void hide_console() {
#ifdef Y_OS_WIN
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
}

static void parse_args(int argc, char** argv) {
	for(std::string_view arg : core::Span<const char*>(argv, argc)) {
		if(arg == "--nodebug") {
			debug_instance = false;
		}
		if(arg == "--debug") {
			debug_instance = true;
		}
		if(arg == "--console") {
			display_console = true;
		}
#ifdef Y_DEBUG
		if(arg == "--errbreak") {
			core::result::break_on_error = true;
		}
#endif
	}

	if(!display_console) {
		hide_console();
	}
	y_debug_assert([] { log_msg("Debug asserts enabled."); return true; }());
}

static void setup_logger() {
	set_log_callback([](std::string_view msg, Log type, void*) {
			if(context) {
				context->log_message(msg, type);
			}
			return !display_console;
		});
}

static Instance create_instance() {
	y_profile();
	if(!debug_instance) {
		log_msg("Vulkan debugging disabled.", Log::Warning);
	}
	return Instance(debug_instance ? DebugParams::debug() : DebugParams::none());
}

static Device create_device(Instance& instance) {
	y_profile();
	return Device(instance);
}

static EditorContext create_context(const Device& device) {
	y_profile();
	return EditorContext(&device);
}

int main(int argc, char** argv) {
	concurrent::set_thread_name("Main thread");

	parse_args(argc, argv);
	setup_logger();

	if(!crashhandler::setup_handler()) {
		log_msg("Unable to setup crash handler.", Log::Warning);
	}

	Instance instance = create_instance();


	Device device = create_device(instance);
	EditorContext ctx = create_context(device);
	context = &ctx;

	MainWindow window(&ctx);
	window.set_event_handler(std::make_unique<MainEventHandler>());
	window.show();

	for(;;) {
		if(!window.update()) {
			if(ctx.ui().confirm("Quit ?")) {
				break;
			} else {
				window.show();
			}
		}

		// 35 ms to not spam if we are capped at 30 FPS
		core::DebugTimer frame_timer("frame", core::Duration::milliseconds(35.0));

		Swapchain* swapchain = window.swapchain();
		if(swapchain && swapchain->is_valid()) {
			FrameToken frame = swapchain->next_frame();
			CmdBufferRecorder recorder(device.create_disposable_cmd_buffer());

			ctx.ui().paint(recorder, frame);

			window.present(recorder, frame);
		}

		ctx.flush_deferred();

		if(ctx.device_resources_reload_requested()) {
			device.device_resources().reload();
			ctx.resources().reload();
			ctx.set_device_resource_reloaded();
		}
	}


	if(perf::is_capturing()) {
		perf::end_capture();
	}

	set_log_callback(nullptr);
	context = nullptr;

	return 0;
}

#endif



#include <y/ecs/EntityWorld.h>
#include <yave/ecs/EntityWorld.h>

#include <y/math/random.h>

using namespace y;



template<usize I>
struct C {
	float x = I;

	y_serde3(x)
};


const usize entity_count = 1000;

template<typename W>
void add(W& world) {
	log_msg(ct_type_name<W>());
#define MAYBE_ADD(num) if(rng() & 0x1) { world.template add_components<C<num>>(id); }
	core::DebugTimer _("Create many components");
	math::FastRandom rng;
	for(usize i = 0; i != entity_count; ++i) {
		const auto id = world.create_entity();
		MAYBE_ADD(0)
		MAYBE_ADD(1)
		MAYBE_ADD(2)
		MAYBE_ADD(3)
		MAYBE_ADD(4)
		MAYBE_ADD(5)
		MAYBE_ADD(6)
		MAYBE_ADD(7)
		MAYBE_ADD(8)
		MAYBE_ADD(9)
		MAYBE_ADD(10)
		MAYBE_ADD(11)
		MAYBE_ADD(12)
		MAYBE_ADD(13)
		MAYBE_ADD(14)
		MAYBE_ADD(15)
		MAYBE_ADD(16)
		MAYBE_ADD(17)
		MAYBE_ADD(18)
		MAYBE_ADD(19)
	}
#undef MAYBE_ADD
}

template<typename V>
float iter(V&& view) {
	float sum = 0.0;
	core::DebugTimer _("Iterate 2 components");
	for(const auto& [a, b] : view) {
		sum -= a.x;
		sum += b.x;
	}
	for(usize i = 0; i != 100; ++i) {
		for(const auto& [a, b] : view) {
			sum -= a.x / (b.x + 1.0f);
		}
		sum += sum * 4.0f;
	}

	return sum;
}

int main() {
	{
		y::ecs::EntityWorld world;
		add(world);
		const float res = iter(world.view<C<7>, C<19>>());
		log_msg(fmt("result = %", res));
	}

	{
		yave::ecs::EntityWorld world;
		add(world);
		const float res = iter(world.view<const C<7>, const C<19>>().components());
		log_msg(fmt("result = %", res));
	}

}



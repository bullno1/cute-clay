#include <cute_app.h>
#include <cute_color.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_time.h>
#define REMODULE_PLUGIN_IMPLEMENTATION
#include <remodule.h>
#include "plugin_interface.h"
#include <cute.h>

REMODULE_VAR(double, counter) = 0.f;

static const char* WINDOW_TITLE = "Cute clay";

static void reinit(void);

static void
init(int argc, const char* argv[]) {
	int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
	cf_make_app(
		WINDOW_TITLE, 0, 0, 0, 640, 480, options, argv[0]
	);

	reinit();
}

static void
reinit(void) {
	cf_set_fixed_timestep(30);
	cf_app_set_vsync(true);
	cf_app_set_title(WINDOW_TITLE);
}

static void
fixed_update(void* udata) {
	counter += 0.05f;
	counter = fmodf(counter, CF_PI * 2);
}

static void
update(void) {
	cf_app_update(fixed_update);

	float lerp_counter = cf_lerp(counter - 0.05f, counter, CF_DELTA_TIME_INTERPOLANT);
	cf_draw_push_color(cf_color_red());
	cf_draw_box(
		cf_make_aabb(
			cf_v2(sinf(lerp_counter) * 150.f, 10.f),
			cf_v2(200.f + sinf(lerp_counter) * 20.f, 100.f)
		),
		2.4f,
		4.0f
	);
	cf_draw_pop_color();

	cf_app_draw_onto_screen(true);
}

static void
cleanup(void) {
	cf_destroy_app();
}

static void
register_plugin(plugin_interface_t* plugin_interface) {
	plugin_interface->init = init;
	plugin_interface->update = update;
	plugin_interface->cleanup = cleanup;
}

void
remodule_entry(remodule_op_t op, void* userdata) {
	plugin_interface_t* plugin_interface = userdata;

	switch (op) {
		case REMODULE_OP_LOAD:
			register_plugin(plugin_interface);
			break;
		case REMODULE_OP_UNLOAD:
			break;
		case REMODULE_OP_BEFORE_RELOAD:
			break;
		case REMODULE_OP_AFTER_RELOAD:
			register_plugin(plugin_interface);
			reinit();
			break;
	}
}

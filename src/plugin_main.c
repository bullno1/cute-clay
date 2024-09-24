#define REMODULE_PLUGIN_IMPLEMENTATION
#include <remodule.h>
#include "plugin_interface.h"
#include <cute.h>
#include "cute_clay.h"

static plugin_interface_t* plugin_interface = NULL;

REMODULE_VAR(bool, app_created) = false;

REMODULE_VAR(Clay_Arena, clay_memory) = { 0 };
REMODULE_VAR(bool, clay_debug) = false;
REMODULE_VAR(CF_Sprite, sprite) = { 0 };
REMODULE_VAR(htbl CF_Sprite*, sprite_instances) = NULL;

static const char* WINDOW_TITLE = "Cute clay";

static void
init(void) {
	// Cute Framework
	if (!app_created) {
		int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;
		cf_make_app(
			WINDOW_TITLE, 0, 0, 0, 1280, 720, options, plugin_interface->argv[0]
		);
		app_created = true;
	}

	cf_set_fixed_timestep(30);
	cf_app_set_vsync(true);
	cf_app_set_title(WINDOW_TITLE);

	// Clay
	uint64_t totalMemorySize = Clay_MinMemorySize();
	void* memory = clay_memory.memory != NULL ? clay_memory.memory : malloc(totalMemorySize);
	clay_memory = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, memory);
	memset(memory, 0, totalMemorySize);  // TODO: remove once fix is merged

	int width, height;
	cf_app_get_size(&width, &height);
	Clay_Initialize(clay_memory, (Clay_Dimensions){
		.width = width,
		.height = height,
	});
	Clay_SetDebugModeEnabled(clay_debug);
	Clay_SetMeasureTextFunction(cute_clay_measure_text);

	// Assets
	if (!sprite.name) {
		sprite = cf_make_demo_sprite();
	}
}

static void
fixed_update(void* udata) {
}

static void
update(void) {
	cf_app_update(fixed_update);

	// UI
	int w, h;
	cf_app_get_size(&w, &h);
	Clay_SetLayoutDimensions((Clay_Dimensions){ w, h });

	Clay_SetPointerState(
		(Clay_Vector2){ cf_mouse_x(), cf_mouse_y() },
		cf_mouse_down(CF_MOUSE_BUTTON_LEFT)
	);
	Clay_UpdateScrollContainers(
		true,
		(Clay_Vector2){ 0.f, cf_mouse_wheel_motion() },
		CF_DELTA_TIME
	);
	cf_push_font("Calibri");
	Clay_BeginLayout();

	Clay_Color root_bg = { 128, 128, 128, 255 };
	Clay_Color sidebar_bg = { 100, 100, 100, 255 };
	Clay_Color text_color = { 255, 255, 255, 255 };

	CUTE_CLAY_RECTANGLE(
		CLAY_ID("root"),
		CLAY_LAYOUT(
			.sizing = {
				.width = CLAY_SIZING_GROW(0),
				.height = CLAY_SIZING_GROW(0)
			},
			.padding = { 16, 16 },
			.childGap = 16
		),
		CLAY_RECTANGLE_CONFIG(.color = root_bg)
	) {
		CUTE_CLAY_RECTANGLE(
			CLAY_ID("SideBar"),
			CLAY_LAYOUT(
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				.sizing = {
					.width = CLAY_SIZING_FIXED(300),
					.height = CLAY_SIZING_GROW(0)
				},
				.padding = {16, 16},
				.childGap = 16
			),
			CLAY_RECTANGLE_CONFIG(.color = sidebar_bg)
		) {
			CLAY_TEXT(
				CLAY_LOCAL_ID("Title"),
				CLAY_STRING("Side bar with a long affffff titlef what is this clipping?"),
				CLAY_TEXT_CONFIG(.fontSize = 24, .textColor = text_color, .fontName = "Calibri")
			);
		}

		CUTE_CLAY_RECTANGLE(
			CLAY_ID("Content"),
			CLAY_LAYOUT(
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				.sizing = {
					.width = CLAY_SIZING_GROW(0),
					.height = CLAY_SIZING_GROW(0)
				},
				.padding = {16, 16},
				.childGap = 16
			),
			CLAY_RECTANGLE_CONFIG(.color = sidebar_bg, .cornerRadius.topLeft = 10.f)
		) {
			CLAY_TEXT(
				CLAY_LOCAL_ID("Title"),
				CLAY_STRING("<shake>Content</shake>"),
				CLAY_TEXT_CONFIG(.fontSize = 24, .textColor = text_color)
			);

			CUTE_CLAY_CONTAINER(
				CLAY_ID("Sprite container"),
				CLAY_LAYOUT(
					.layoutDirection = CLAY_LEFT_TO_RIGHT,
					.sizing = {
						.width = CLAY_SIZING_GROW(0),
						.height = CLAY_SIZING_GROW(0)
					},
					.childAlignment.x = CLAY_ALIGN_X_LEFT,
					.childGap = 10,
				)
			) {
				// Create a separate instance for each animation
				// This has to be done in a separate pass since the sprite address
				// can change.
				for (int i = 0; i < hsize(sprite.animations); ++i) {
					const CF_Animation* animation = sprite.animations[i];

					if (
						sprite_instances == NULL
						|| hget_ptr(sprite_instances, animation->name) == NULL
					) {
						hset(sprite_instances, animation->name, sprite);
						CF_Sprite* instance = hget_ptr(sprite_instances, animation->name);
						cf_sprite_play(instance, animation->name);
					}
				}

				for (int i = 0; i < hsize(sprite_instances); ++i) {
					CUTE_CLAY_BORDER(
						CLAY_LOCAL_IDI("Animation", i),
						CLAY_LAYOUT(
							.layoutDirection = CLAY_TOP_TO_BOTTOM,
							.childAlignment.x = CLAY_ALIGN_X_CENTER,
							.sizing = {
								.width = CLAY_SIZING_FIT(0),
								.height = CLAY_SIZING_FIT(0),
							},
							.padding = { 5, 5 },
						),
						CLAY_BORDER_CONFIG_OUTSIDE(
							.color = cute_clay_color(cf_color_white()),
							.width = 1,
						)
					) {
						CF_Sprite* instance = &sprite_instances[i];
						cf_sprite_update(instance);

						CLAY_IMAGE(
							CLAY_LOCAL_ID("Sprite"),
							CLAY_LAYOUT(
								.sizing = {
									.width = CLAY_SIZING_FIXED(instance->w),
									.height = CLAY_SIZING_FIXED(instance->h),
								}
							),
							Clay__StoreImageElementConfig(cute_clay_sprite(instance)),
						);

						CLAY_TEXT(
							CLAY_LOCAL_ID("Animation name"),
							((Clay_String){
								.length = strlen(instance->animation->name),
								.chars = instance->animation->name,
							}),
							CLAY_TEXT_CONFIG(.fontSize = 15, .textColor = text_color)
						);
					}
				}
			}
		}
	}

	cf_pop_font();
	Clay_RenderCommandArray clay_render_cmds = Clay_EndLayout();
	cute_clay_render(clay_render_cmds, NULL);

	if (cf_key_just_pressed(CF_KEY_F12)) {
		clay_debug = !clay_debug;
		Clay_SetDebugModeEnabled(clay_debug);
	}

	cf_app_draw_onto_screen(true);
}

static void
cleanup(void) {
	free(clay_memory.memory);
	hfree(sprite_instances);

	cf_destroy_app();
}

static void
register_plugin(void) {
	plugin_interface->init = init;
	plugin_interface->update = update;
	plugin_interface->cleanup = cleanup;
}

void
remodule_entry(remodule_op_t op, void* userdata) {
	plugin_interface = userdata;

	switch (op) {
		case REMODULE_OP_LOAD:
			register_plugin();
			break;
		case REMODULE_OP_UNLOAD:
			break;
		case REMODULE_OP_BEFORE_RELOAD:
			break;
		case REMODULE_OP_AFTER_RELOAD:
			register_plugin();
			init();
			break;
	}
}

#define CLAY_IMPLEMENTATION
#include "cute_clay.h"
#include <cute_app.h>
#include <cute_draw.h>
#include <cute_input.h>

struct cute_clay_ctx_s {
	Clay_Arena arena;
};

static cute_clay_ctx_t* cute_clay_ctx = NULL;

static inline CF_Aabb
cute_clay_aabb(Clay_BoundingBox bbox) {
	return (CF_Aabb){
		.min = {
			.x = bbox.x,
			.y = -(bbox.y + bbox.height),
		},
		.max = {
			.x = bbox.x + bbox.width,
			.y = -bbox.y,
		},
	};
}

static inline void
cute_clay_push_color(Clay_Color color) {
	cf_draw_push_color((CF_Color){
		.a = color.a / 255.f,
		.r = color.r / 255.f,
		.g = color.g / 255.f,
		.b = color.b / 255.f,
	});
}

cute_clay_ctx_t*
cute_clay_init(void) {
	uint64_t totalMemorySize = Clay_MinMemorySize();
	void* memory = cf_alloc(totalMemorySize);
	Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, memory);

	cute_clay_ctx_t* ctx = cf_alloc(sizeof(cute_clay_ctx_t));
	*ctx = (cute_clay_ctx_t){
		.arena = arena,
	};

	cute_clay_set_ctx(ctx);

	return ctx;
}

void
cute_clay_set_ctx(cute_clay_ctx_t* ctx) {
	int width, height;
	cf_app_get_size(&width, &height);

	Clay_Initialize(ctx->arena, (Clay_Dimensions){
		.width = width,
		.height = height,
	});
	Clay_SetMeasureTextFunction(cute_clay_measure_text);

	cute_clay_ctx = ctx;
}

void
cute_clay_cleanup(cute_clay_ctx_t* ctx) {
	cf_free(ctx->arena.memory);
	cf_free(ctx);
}

void
cute_clay_begin(void) {
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

	Clay_BeginLayout();
}

Clay_RenderCommandArray
cute_clay_end(void) {
	return Clay_EndLayout();
}

Clay_Dimensions
cute_clay_measure_text(Clay_String* text, Clay_TextElementConfig* config) {
	cf_push_font_size(config->fontSize);
	if (config->fontName) {
		cf_push_font(config->fontName);
	}
	CF_V2 size = cf_text_size(text->chars, text->length);
	if (config->fontName) {
		cf_pop_font();
	}
	cf_pop_font_size(config->fontSize);
	return (Clay_Dimensions){
		.width = size.x,
		.height = size.y,
	};
}

void
cute_clay_render(
	Clay_RenderCommandArray cmds,
	cute_clay_custom_renderer_t custom_redenderer
) {
	int w, h;
	cf_app_get_size(&w, &h);

	float half_width = w * 0.5f;
	float half_height = h * 0.5f;
	cf_draw_push();
	cf_draw_translate(-half_width, half_height);
	for (uint32_t i = 0; i < cmds.length; ++i) {
		Clay_RenderCommand cmd = cmds.internalArray[i];
		switch (cmd.commandType) {
			case CLAY_RENDER_COMMAND_TYPE_NONE:
				break;
			case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
				if (cmd.config.rectangleElementConfig->nine_patch == NULL) {
					cute_clay_push_color(cmd.config.rectangleElementConfig->color);
					cf_draw_box_rounded_fill(
						cute_clay_aabb(cmd.boundingBox),
						cmd.config.rectangleElementConfig->cornerRadius.topLeft
					);
					cf_draw_pop_color();
				} else {
					cute_9_patch_draw(
						cmd.config.rectangleElementConfig->nine_patch,
						cute_clay_aabb(cmd.boundingBox)
					);
				}
				break;
			case CLAY_RENDER_COMMAND_TYPE_BORDER:
				cute_clay_push_color(cmd.config.borderElementConfig->left.color);
				// TODO: Different borders
				cf_draw_box_rounded(
					cute_clay_aabb(cmd.boundingBox),
					cmd.config.borderElementConfig->top.width * 0.25f,
					cmd.config.borderElementConfig->cornerRadius.topLeft * 0.25f
				);
				cf_draw_pop_color();
				break;
			case CLAY_RENDER_COMMAND_TYPE_TEXT:
				cf_push_font_size(cmd.config.textElementConfig->fontSize);
				cute_clay_push_color(cmd.config.textElementConfig->textColor);
				cf_draw_text(
					cmd.text.chars,
					(CF_V2){ cmd.boundingBox.x, -cmd.boundingBox.y },
					cmd.text.length
				);
				cf_draw_pop_color();
				cf_pop_font_size();
				break;
			case CLAY_RENDER_COMMAND_TYPE_IMAGE:
				{
					CF_Sprite* sprite = cmd.config.imageElementConfig->imageData;
					// TODO: clip or scale?
					sprite->transform.p.x = cmd.boundingBox.x + sprite->w * 0.5f + sprite->local_offset.x;
					sprite->transform.p.y = -cmd.boundingBox.y - sprite->h * 0.5f - sprite->local_offset.y;
					cf_draw_sprite(sprite);
				}
				break;
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
				cf_draw_push_scissor((CF_Rect){
					.x = cmd.boundingBox.x,
					.y = cmd.boundingBox.y,
					.w = cmd.boundingBox.width,
					.h = cmd.boundingBox.height,
				});
				break;
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
				cf_draw_pop_scissor();
				break;
			case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
				if (custom_redenderer) {
					custom_redenderer(cmd);
				}
				break;
		}
	}
	cf_draw_pop();
}

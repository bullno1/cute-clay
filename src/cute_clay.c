#include "cute_clay.h"
#include "cute_9_patch.h"
#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_draw.h>
#include <cute_input.h>
#include <cute_hashtable.h>

#define CUTE_CLAY_ARENA_CHUNK_SIZE 4096

typedef struct {
	size_t size;
	void* data;
} cute_clay_state_hdr_t;

struct cute_clay_ctx_s {
	Clay_Arena clay_arena;
	CF_Arena current_arena;
	CF_Arena previous_arena;
	htbl cute_clay_state_hdr_t* current_states;
	htbl cute_clay_state_hdr_t* previous_states;
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
	Clay_Arena clay_arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, memory);

	cute_clay_ctx_t* ctx = cf_alloc(sizeof(cute_clay_ctx_t));
	*ctx = (cute_clay_ctx_t){
		.clay_arena = clay_arena,
	};
	ctx->current_arena = cf_make_arena(_Alignof(max_align_t), CUTE_CLAY_ARENA_CHUNK_SIZE);
	ctx->previous_arena = cf_make_arena(_Alignof(max_align_t), CUTE_CLAY_ARENA_CHUNK_SIZE);

	cute_clay_set_ctx(ctx);

	return ctx;
}

void
cute_clay_set_ctx(cute_clay_ctx_t* ctx) {
	int width, height;
	cf_app_get_size(&width, &height);

	Clay_Initialize(ctx->clay_arena, (Clay_Dimensions){
		.width = width,
		.height = height,
	});
	Clay_SetMeasureTextFunction(cute_clay_measure_text);

	cute_clay_ctx = ctx;
}

void
cute_clay_cleanup(cute_clay_ctx_t* ctx) {
	cf_destroy_arena(&ctx->previous_arena);
	cf_destroy_arena(&ctx->current_arena);
	hfree(ctx->previous_states);
	hfree(ctx->current_states);
	cf_free(ctx->clay_arena.memory);
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

	// Flip arena and states
	CF_Arena tmp_arena = cute_clay_ctx->current_arena;
	cute_clay_ctx->current_arena = cute_clay_ctx->previous_arena;
	cute_clay_ctx->previous_arena = tmp_arena;
	cf_arena_reset(&cute_clay_ctx->current_arena);

	htbl cute_clay_state_hdr_t* tmp_states = cute_clay_ctx->current_states;
	cute_clay_ctx->current_states = cute_clay_ctx->previous_states;
	cute_clay_ctx->previous_states = tmp_states;
	hclear(cute_clay_ctx->current_states);
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
					CF_V2 pivot = sprite->pivots[sprite->frame_index];
					sprite->transform.p.x = cmd.boundingBox.x + sprite->w * 0.5f + pivot.x;
					sprite->transform.p.y = -cmd.boundingBox.y - sprite->h * 0.5f - pivot.y;
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

void*
cute_clay_state(Clay_ElementId id, size_t size, void* default_value) {
	cute_clay_state_hdr_t hdr = hget(cute_clay_ctx->current_states, id.id);

	// Already created and no size change
	if (hdr.data != NULL && hdr.size == size) {
		return hdr.data;
	}

	// Try copying from last frame
	cute_clay_state_hdr_t previous_hdr = hget(cute_clay_ctx->previous_states, id.id);
	void* state = cf_arena_alloc(&cute_clay_ctx->current_arena, size);
	if (previous_hdr.data != NULL && previous_hdr.size == size) {
		memcpy(state, previous_hdr.data, size);
	} else {
		memcpy(state, default_value, size);
	}
	hset(cute_clay_ctx->current_states, id.id, (cute_clay_state_hdr_t){
		.data = state,
		.size = size,
	});

	return state;
}

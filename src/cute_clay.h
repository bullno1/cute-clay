#ifndef CUTE_CLAY_H
#define CUTE_CLAY_H

struct cute_9_patch_s;
#define CLAY_EXTEND_CONFIG_TEXT const char* fontName;
#define CLAY_EXTEND_CONFIG_RECTANGLE struct cute_9_patch_s* nine_patch;
#define CLAY_ const char* fontName;
#include <clay.h>
#include <cute_sprite.h>
#include <cute_color.h>

#define CUTE_CLAY_STATE(id, type, ...) \
	(type*)cute_clay_state(id, sizeof(type), &(type) { __VA_ARGS__ })

typedef void (*cute_clay_custom_renderer_t)(Clay_RenderCommand cmd);

typedef struct cute_clay_ctx_s cute_clay_ctx_t;

cute_clay_ctx_t*
cute_clay_init(void);

void
cute_clay_set_ctx(cute_clay_ctx_t* ctx);

void
cute_clay_cleanup(cute_clay_ctx_t* ctx);

void
cute_clay_begin(void);

Clay_RenderCommandArray
cute_clay_end(void);

Clay_Dimensions
cute_clay_measure_text(Clay_String* text, Clay_TextElementConfig* config);

void
cute_clay_render(
	Clay_RenderCommandArray cmds,
	cute_clay_custom_renderer_t custom_redenderer
);

void*
cute_clay_state(Clay_ElementId id, size_t size, void* default_value);

static inline Clay_Color
cute_clay_color(CF_Color color) {
	return (Clay_Color) {
		.r = color.r * 255.f,
		.g = color.g * 255.f,
		.b = color.b * 255.f,
		.a = color.a * 255.f,
	};
}

static inline Clay_ImageElementConfig
cute_clay_sprite(CF_Sprite* sprite) {
	return (Clay_ImageElementConfig){
		.imageData = sprite,
		.sourceDimensions = (Clay_Dimensions){
			.width = sprite->w,
			.height = sprite->h,
		}
	};
}

#endif

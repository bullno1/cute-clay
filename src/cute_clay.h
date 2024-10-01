#ifndef CUTE_CLAY_H
#define CUTE_CLAY_H

#include "cute_9_patch.h"
#define CLAY_EXTEND_CONFIG_TEXT const char* fontName;
#define CLAY_EXTEND_CONFIG_RECTANGLE cute_9_patch_t* nine_patch;
#define CLAY_ const char* fontName;
#include <clay.h>
#include <cute_color.h>
#include <cute_draw.h>

typedef void (*cute_clay_custom_renderer_t)(Clay_RenderCommand cmd);

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

Clay_Dimensions
cute_clay_measure_text(Clay_String* text, Clay_TextElementConfig* config);

void
cute_clay_render(
	Clay_RenderCommandArray cmds,
	cute_clay_custom_renderer_t custom_redenderer
);

#endif

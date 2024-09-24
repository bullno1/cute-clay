#define CLAY_EXTEND_CONFIG_TEXT const char* fontName;
#define CLAY_ const char* fontName;
#include <clay.h>
#include <cute_color.h>
#include <cute_draw.h>

#define CUTE_CLAY_LOOP_VAR CUTE_CLAY_LOOP_VAR2(cute_clay__, __LINE__)
#define CUTE_CLAY_LOOP_VAR2(lhs, rhs) CUTE_CLAY_LOOP_VAR3(lhs, rhs)
#define CUTE_CLAY_LOOP_VAR3(lhs, rhs) lhs##rhs

#define CUTE_CLAY_SCOPE(open, close, ...) \
	for (\
		int CUTE_CLAY_LOOP_VAR = (open(__VA_ARGS__), 0); \
		CUTE_CLAY_LOOP_VAR < 1; \
		++CUTE_CLAY_LOOP_VAR, close() \
	)

#define CUTE_CLAY_CONTAINER(...) \
	CUTE_CLAY_SCOPE(Clay__OpenContainerElement, Clay__CloseElementWithChildren, __VA_ARGS__)

#define CUTE_CLAY_RECTANGLE(...) \
	CUTE_CLAY_SCOPE(Clay__OpenRectangleElement, Clay__CloseElementWithChildren, __VA_ARGS__)

#define CUTE_CLAY_BORDER(...) \
	CUTE_CLAY_SCOPE(Clay__OpenBorderElement, Clay__CloseElementWithChildren, __VA_ARGS__)

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

#include <clay.h>

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

Clay_Dimensions
cute_clay_measure_text(Clay_String* text, Clay_TextElementConfig* config);

void
cute_clay_render(Clay_RenderCommandArray cmds);

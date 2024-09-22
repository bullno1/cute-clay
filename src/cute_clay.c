#define CLAY_IMPLEMENTATION
#include <cute_app.h>
#include <cute_draw.h>
#include "cute_clay.h"

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

Clay_Dimensions
cute_clay_measure_text(Clay_String* text, Clay_TextElementConfig* config) {
	cf_push_font_size(config->fontSize);
	CF_V2 size = cf_text_size(text->chars, text->length);
	cf_pop_font_size(config->fontSize);
	return (Clay_Dimensions){
		.width = size.x,
		.height = size.y,
	};
}

void
cute_clay_render(Clay_RenderCommandArray cmds) {
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
				cute_clay_push_color(cmd.config.rectangleElementConfig->color);
				cf_draw_box_rounded_fill(
					cute_clay_aabb(cmd.boundingBox),
					cmd.config.rectangleElementConfig->cornerRadius.topLeft
				);
				cf_draw_pop_color();
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
				/*cf_pop_text_clip_box();*/
				break;
			case CLAY_RENDER_COMMAND_TYPE_IMAGE:
				break;
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
				cf_render_to(cf_app_get_canvas(), false);
				cf_render_settings_push_scissor((CF_Rect){
					.x = cmd.boundingBox.x,
					.y = cmd.boundingBox.y,
					.w = cmd.boundingBox.width,
					.h = cmd.boundingBox.height,
				});
				break;
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
				cf_render_to(cf_app_get_canvas(), false);
				cf_render_settings_pop_scissor();
				break;
			case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
				break;
		}
	}
	cf_draw_pop();
}

#include "cute_9_patch.h"
#include <cute_alloc.h>
#include <cute_draw.h>
#include <cute_image.h>
#include <cute_sprite.h>

static inline CF_Sprite
cute_9_patch_make_patch(
	CF_Pixel* tmp_buf,
	CF_Image src,
	int x, int y,
	int w, int h
) {
	for (int iy = 0; iy < h; ++iy) {
		for (int ix = 0; ix < w; ++ix) {
			tmp_buf[ix + iy * w] = src.pix[(x + ix) + (y + iy) * src.w];
		}
	}

	CF_Sprite patch = cf_make_easy_sprite_from_pixels(tmp_buf, w, h);
	patch.local_offset.x = w * 0.5;
	patch.local_offset.y = -h * 0.5;
	return patch;
}

static inline void
cute_9_patch_draw_patch(
	CF_Sprite* patch,
	float x, float y,
	float x_scale, float y_scale
) {
	patch->transform.p.x = x;
	patch->transform.p.y = y;
	patch->scale.x = x_scale;
	patch->scale.y = y_scale;
	cf_draw_sprite(patch);
}

void
cute_9_patch_init(
	cute_9_patch_t* nine_patch,
	CF_Image src,
	cute_9_patch_config_t config
) {
	// TODO: if the dimensions are the same, just overwrite pixels with
	// cf_easy_sprite_update
	if (nine_patch->nw.name != NULL) {
		cf_easy_sprite_unload(&nine_patch->nw);
		cf_easy_sprite_unload(&nine_patch->n);
		cf_easy_sprite_unload(&nine_patch->ne);

		cf_easy_sprite_unload(&nine_patch->w);
		cf_easy_sprite_unload(&nine_patch->c);
		cf_easy_sprite_unload(&nine_patch->e);

		cf_easy_sprite_unload(&nine_patch->sw);
		cf_easy_sprite_unload(&nine_patch->s);
		cf_easy_sprite_unload(&nine_patch->se);

		nine_patch->nw = cf_sprite_defaults();
		nine_patch->n  = cf_sprite_defaults();
		nine_patch->ne = cf_sprite_defaults();

		nine_patch->w  = cf_sprite_defaults();
		nine_patch->c  = cf_sprite_defaults();
		nine_patch->e  = cf_sprite_defaults();

		nine_patch->sw = cf_sprite_defaults();
		nine_patch->s  = cf_sprite_defaults();
		nine_patch->se = cf_sprite_defaults();
	}

	if (!(config.left + config.right <= src.w && config.top + config.bottom <= src.h)) {
		return;
	}

	// Border size for the patches
	int p_left   = config.left;
	int p_top    = config.top;
	int p_right  = config.right;
	int p_bottom = config.bottom;
	// Width and height of the center piece
	int p_c_w = src.w - p_left - p_right;  // patch's center width
	int p_c_h = src.h - p_top - p_bottom;  // patch's center height

	CF_Pixel* img_buf = cf_alloc(src.w * src.h * sizeof(CF_Pixel));

	nine_patch->nw = cute_9_patch_make_patch(img_buf, src, 0              , 0               , p_left , p_top);
	nine_patch->n  = cute_9_patch_make_patch(img_buf, src, p_left         , 0               , p_c_w  , p_top);
	nine_patch->ne = cute_9_patch_make_patch(img_buf, src, src.w - p_right, 0               , p_right, p_top);

	nine_patch->w  = cute_9_patch_make_patch(img_buf, src, 0              , p_top           , p_left , p_c_h);
	nine_patch->c  = cute_9_patch_make_patch(img_buf, src, p_left         , p_top           , p_c_w  , p_c_h);
	nine_patch->e  = cute_9_patch_make_patch(img_buf, src, src.w - p_right, p_top           , p_right, p_c_h);

	nine_patch->sw = cute_9_patch_make_patch(img_buf, src, 0              , src.h - p_bottom, p_left , p_bottom);
	nine_patch->s  = cute_9_patch_make_patch(img_buf, src, p_left         , src.h - p_bottom, p_c_w  , p_bottom);
	nine_patch->se = cute_9_patch_make_patch(img_buf, src, src.w - p_right, src.h - p_bottom, p_right, p_bottom);

	nine_patch->config = config;
	nine_patch->center_width = p_c_w;
	nine_patch->center_height = p_c_h;

	cf_free(img_buf);
}

void
cute_9_patch_draw(cute_9_patch_t* nine_patch, CF_Aabb aabb) {
	if (nine_patch->nw.name == NULL) { return; }

	float p_left   = (float)nine_patch->config.left;
	float p_top    = (float)nine_patch->config.top;
	float p_right  = (float)nine_patch->config.right;
	float p_bottom = (float)nine_patch->config.bottom;

	float dst_width  = aabb.max.x - aabb.min.x;
	float dst_height = aabb.max.y - aabb.min.y;
	float x_scale = (dst_width - p_left - p_right) / (float)nine_patch->center_width;
	float y_scale = (dst_height - p_top - p_bottom) / (float)nine_patch->center_height;

	cute_9_patch_draw_patch(&nine_patch->nw, aabb.min.x          , aabb.max.y           , 1.f    , 1.f);
	cute_9_patch_draw_patch(&nine_patch->n , aabb.min.x + p_left , aabb.max.y           , x_scale, 1.f);
	cute_9_patch_draw_patch(&nine_patch->ne, aabb.max.x - p_right, aabb.max.y           , 1.f    , 1.f);

	cute_9_patch_draw_patch(&nine_patch->w , aabb.min.x          , aabb.max.y - p_top   , 1.f    , y_scale);
	cute_9_patch_draw_patch(&nine_patch->c , aabb.min.x + p_left , aabb.max.y - p_top   , x_scale, y_scale);
	cute_9_patch_draw_patch(&nine_patch->e , aabb.max.x - p_right, aabb.max.y - p_top   , 1.f    , y_scale);

	cute_9_patch_draw_patch(&nine_patch->sw, aabb.min.x          , aabb.min.y + p_bottom, 1.f    , 1.f);
	cute_9_patch_draw_patch(&nine_patch->s , aabb.min.x + p_left , aabb.min.y + p_bottom, x_scale, 1.f);
	cute_9_patch_draw_patch(&nine_patch->se, aabb.max.x - p_right, aabb.min.y + p_bottom, 1.f    , 1.f);
}

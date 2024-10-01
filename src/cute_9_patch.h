#ifndef CUTE_9_PATCH_H
#define CUTE_9_PATCH_H

#include <cute_image.h>
#include <cute_sprite.h>

typedef struct {
	// The size of the border in each direction
	int left, right, top, bottom;
} cute_9_patch_config_t;

typedef struct cute_9_patch_s {
	cute_9_patch_config_t config;
	int center_width;
	int center_height;

	// Sprite for each patch named after the direction + (c)enter
	struct CF_Sprite nw, n, ne;
	struct CF_Sprite  w, c,  e;
	struct CF_Sprite sw, s, se;
} cute_9_patch_t;

void
cute_9_patch_init(
	cute_9_patch_t* nine_patch,
	CF_Image source,
	cute_9_patch_config_t config
);

void
cute_9_patch_draw(const cute_9_patch_t* nine_patch, CF_Aabb aabb);

#endif

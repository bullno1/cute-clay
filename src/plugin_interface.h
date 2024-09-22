#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

typedef struct plugin_interface_s {
	void (*init)(int argc, const char* argv[]);
	void (*update)(void);
	void (*cleanup)(void);
} plugin_interface_t;

#endif

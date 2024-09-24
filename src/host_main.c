#define REMODULE_HOST_IMPLEMENTATION
#define REMODULE_MONITOR_IMPLEMENTATION
#include <remodule.h>
#include <cute_app.h>
#include <remodule_monitor.h>
#include <cute.h>
#include "plugin_interface.h"

int
main(int argc, const char* argv[]) {
	plugin_interface_t plugin_interface = {
		.argc = argc,
		.argv = argv,
	};
	remodule_t* module = remodule_load(
		"cute-clay" REMODULE_DYNLIB_EXT,
		&plugin_interface
	);
	remodule_monitor_t* monitor = remodule_monitor(module);

	plugin_interface.init();

	while (cf_app_is_running()) {
		if (plugin_interface.update != NULL) {
			plugin_interface.update();
		}

		if (remodule_check(monitor)) {
			printf("Reloaded\n");
		}
	}

	plugin_interface.cleanup();

	remodule_unmonitor(monitor);
	remodule_unload(module);

	return 0;
}

#define REMODULE_HOST_IMPLEMENTATION
#define REMODULE_MONITOR_IMPLEMENTATION
#include <remodule.h>
#include <cute_app.h>
#include <remodule_monitor.h>
#include <cute.h>
#include "plugin_interface.h"

int
main(int argc, const char* argv[]) {
	plugin_interface_t plugin_interface = { 0 };
	remodule_t* module = remodule_load(
		"cuteclay_plugin" REMODULE_DYNLIB_EXT,
		&plugin_interface
	);
	remodule_monitor_t* monitor = remodule_monitor(module);

	plugin_interface.init(argc, argv);

	while (cf_app_is_running()) {
		if (plugin_interface.update != NULL) {
			plugin_interface.update();
		}

		remodule_check(monitor);
	}

	remodule_unmonitor(monitor);
	remodule_unload(module);

	plugin_interface.cleanup();

	return 0;
}

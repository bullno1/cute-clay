#define REMODULE_HOST_IMPLEMENTATION
#include <remodule.h>
#define BRESMON_IMPLEMENTATION
#include <bresmon.h>
#include <cute_app.h>
#include <cute.h>
#include "plugin_interface.h"

static void
reload_module(const char* path, void* module) {
	printf("Reloading %s\n", path);
	remodule_reload(module);
}

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
	bresmon_t* monitor = bresmon_create(NULL);
	bresmon_watch(monitor, remodule_path(module), reload_module, module);

	plugin_interface.init();

	while (cf_app_is_running()) {
		if (plugin_interface.update != NULL) {
			plugin_interface.update();
		}

		if (bresmon_should_reload(monitor, false)) {
			printf("Reloading\n");
			bresmon_reload(monitor);
			printf("Reloaded\n");
		}
	}

	plugin_interface.cleanup();

	bresmon_destroy(monitor);
	remodule_unload(module);

	return 0;
}

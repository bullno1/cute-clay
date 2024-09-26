set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "\${ORIGIN}")

set(plugin_sources
	"plugin_main.c"
	"cute_clay.c"
	"cute_9_patch.c"
)
setup_library(cute-clay_plugin FALSE "${plugin_sources}")
target_link_libraries(cute-clay_plugin PRIVATE
	cute  # :">
	remodule
	clay
)
set_target_properties(cute-clay_plugin PROPERTIES
	OUTPUT_NAME "cute-clay"
	PREFIX ""
)
target_compile_definitions(cute-clay_plugin PRIVATE "CLAY_OVERFLOW_TRAP")

add_executable(cute-clay "host_main.c")
target_link_libraries(cute-clay PRIVATE
	cute  # :">
	remodule
)

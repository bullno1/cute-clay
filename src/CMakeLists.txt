set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "\${ORIGIN}")

setup_library(cuteclay_plugin FALSE "plugin_main.c")
target_link_libraries(cuteclay_plugin PRIVATE
	cute  # :">
	remodule
)
set_target_properties(cuteclay_plugin PROPERTIES
	OUTPUT_NAME "cuteclay_plugin"
	PREFIX ""
)

add_executable(cuteclay "host_main.c")
target_link_libraries(cuteclay PRIVATE
	cute  # :">
	remodule
)

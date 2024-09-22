# --- cute_framework ---
set(CF_FRAMEWORK_STATIC OFF CACHE BOOL "")
set(CF_FRAMEWORK_BUILD_SAMPLES OFF CACHE BOOL "")
set(CF_FRAMEWORK_BUILD_TESTS OFF CACHE BOOL "")

add_subdirectory("cute_framework" EXCLUDE_FROM_ALL)

# Make the include path system so that targets linking to cute framework can
# use a stricter warning settings.
target_include_directories(cute SYSTEM PUBLIC BEFORE "cute_framework/include")
target_include_directories(cute SYSTEM PUBLIC BEFORE "cute_framework/libraries")
target_include_directories(cute SYSTEM PUBLIC BEFORE "cute_framework/libraries/cimgui")
target_include_directories(cute SYSTEM PUBLIC BEFORE "cute_framework/libraries/cimgui/imgui")

# --- remodule --
add_library(remodule INTERFACE)
target_include_directories(remodule INTERFACE remodule)

# --- clay ---
add_library(clay INTERFACE)
target_include_directories(clay SYSTEM INTERFACE clay)

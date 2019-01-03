
project(firefox_cache)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)

#python module - debug - static /MTd
set(CMAKE_SHARED_LIBRARY_SUFFIX ".pyd")
add_library(
	${PROJECT_NAME}_pyd_debug_static
	SHARED
	pymain.cpp
	firefox_cache.cpp
)
set_target_properties(${PROJECT_NAME}_pyd_debug_static
	PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME}
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pyd_debug_static
	COMPILE_FLAGS "/MTd"
)
target_include_directories(${PROJECT_NAME}_pyd_debug_static PUBLIC
	"E:\ProgramFiles\include"
	"C:\Program Files\vcpkg\installed\x64-windows-static\include"
	"C:\Program Files\Python36\include"
)
target_link_libraries(${PROJECT_NAME}_pyd_debug_static
	"C:\Program Files\Python36\libs\python36.lib"
)


project(chrome_cache)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)

#exe - debug - static /MTd
add_executable(
	${PROJECT_NAME}_exe_debug_static
	main.cpp
	chrome_cache.cpp
)
set_target_properties(${PROJECT_NAME}_exe_debug_static
	PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME}
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/exe_debug_static
	COMPILE_FLAGS "/MTd"
)
target_include_directories(${PROJECT_NAME}_exe_debug_static PUBLIC
	"C:\Program Files\vcpkg\installed\x64-windows-static\include"
)
target_link_libraries(${PROJECT_NAME}_exe_debug_static
	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\boost_iostreams-vc140-mt-gd.lib"
	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\boost_program_options-vc140-mt-gd.lib"
	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\zlibd.lib"
)

#exe - debug - dynamic /MDd
add_executable(
	${PROJECT_NAME}_exe_debug_dynamic
	main.cpp
	chrome_cache.cpp
)
set_target_properties(${PROJECT_NAME}_exe_debug_dynamic
	PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME}
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/exe_debug_dynamic
	COMPILE_FLAGS "/MDd"
)
target_include_directories(${PROJECT_NAME}_exe_debug_dynamic PUBLIC
	"C:\Program Files\vcpkg\installed\x64-windows\include"
)
target_link_libraries(${PROJECT_NAME}_exe_debug_dynamic
	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\boost_iostreams-vc140-mt-gd.lib"
	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\boost_program_options-vc140-mt-gd.lib"
	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\zlibd.lib"
)
#browser_cache_gui用libファイル
#lib - debug - dynamic /MDd
add_library(
	${PROJECT_NAME}_lib_debug_dynamic
	STATIC
	main.cpp
	chrome_cache.cpp
)
set_target_properties(${PROJECT_NAME}_lib_debug_dynamic
	PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME}
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib_debug_dynamic
	COMPILE_FLAGS "/MDd"
)
target_include_directories(${PROJECT_NAME}_lib_debug_dynamic PUBLIC
	"C:\Program Files\vcpkg\installed\x64-windows\include"
)
target_link_libraries(${PROJECT_NAME}_lib_debug_dynamic
	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\boost_iostreams-vc140-mt-gd.lib"
	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\boost_program_options-vc140-mt-gd.lib"
	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\zlibd.lib"
)


#python module - debug - static /MTd
set(CMAKE_SHARED_LIBRARY_SUFFIX ".pyd")
add_library(
	${PROJECT_NAME}_pyd_debug_static
	SHARED
	pymain.cpp
	chrome_cache.cpp
)
set_target_properties(${PROJECT_NAME}_pyd_debug_static
	PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME}
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pyd_debug_static
	COMPILE_FLAGS "/MTd"
)
target_include_directories(${PROJECT_NAME}_pyd_debug_static PUBLIC
	"E:\ProgramFiles\include"
	"C:\Program Files\vcpkg\installed\x64-windows\include"
#	"C:\Program Files\vcpkg\installed\x64-windows-static\include"
	"C:\Program Files\Python37\include"
)
target_link_libraries(${PROJECT_NAME}_pyd_debug_static
#	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\boost_iostreams-vc140-mt-gd.lib"
#	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\zlibd.lib"
#	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\boost_iostreams-vc140-mt-gd.lib"
	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\zlibd.lib"
#	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\boost_program_options-vc140-mt-gd.lib"
	"C:\Program Files\Python37\libs\python37.lib"
)

#dll - debug - static /MTd
#set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")
#add_library(
#	${PROJECT_NAME}_dll_debug_static
#	SHARED
#	chrome_cache.cpp
#)
#set_target_properties(${PROJECT_NAME}_dll_debug_static
#	PROPERTIES
#	OUTPUT_NAME ${PROJECT_NAME}
#	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dll_debug_static
#	COMPILE_FLAGS "/MTd"
#)
#target_include_directories(${PROJECT_NAME}_dll_debug_static PUBLIC
#	"E:\ProgramFiles\include"
##	"C:\Program Files\vcpkg\installed\x64-windows\include"
#	"C:\Program Files\vcpkg\installed\x64-windows-static\include"
#)
#target_link_libraries(${PROJECT_NAME}_dll_debug_static
##	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\boost_iostreams-vc140-mt-gd.lib"
##	"C:\Program Files\vcpkg\installed\x64-windows\debug\lib\zlibd.lib"
#	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\boost_iostreams-vc140-mt-gd.lib"
#	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\zlibd.lib"
#	"C:\Program Files\vcpkg\installed\x64-windows-static\debug\lib\boost_program_options-vc140-mt-gd.lib"
#)
#
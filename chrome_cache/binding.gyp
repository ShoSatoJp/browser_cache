{
	"targets": [
		{
			"target_name": "chrome_cache",
			"sources": [
				"stdafx.hpp",
				"stdafx.cpp",
				"chrome_cache.cpp",
				"addr.hpp",
				"chrome_cache.hpp",
				"structs.hpp",
				"targetver.hpp",
				"nodemain.cpp"
			],
			"include_dirs": [
				"<!(node -e \"require('nan')\")",
				"E:/ProgramFiles/include",
				"C:/Program Files/vcpkg/installed/x64-windows-static/include"
			],
			"libraries": [
				"C:/Program Files/vcpkg/packages/zlib_x64-windows-static/debug/lib/zlibd.lib",
			],
			"cflags_cc": ["/std:c++17" ]
		}
	]
}


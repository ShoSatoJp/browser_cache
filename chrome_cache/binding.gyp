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
				"C:/Program Files/vcpkg/installed/x64-windows-static/include",
				"C:/Program Files/Python36/include"
			],
			"libraries": [
				"C:/Program Files/vcpkg/installed/x64-windows-static/lib/zlib.lib",
			],
			"cflags_cc": [ '-fno-exceptions',"/std:c++17" ]
		}
	]
}


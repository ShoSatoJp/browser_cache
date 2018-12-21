#include "stdafx.h"
#include "firefox_cache.hpp"

int main() {

	filesystem::path dir(R"(C:\Users\User\AppData\Local\Mozilla\Firefox\Profiles\qht8q8ei.default\cache2\)");
	while (true) {
		string key, path;
		cin >> key >> path;
		FirefoxCache index(dir);
		if (key == "list") {
			for (FirefoxCacheEntry f : index.records) {
				cout << f.key << endl;
			}
			cout << index.records.size() << endl;
			continue;
		}
		try {
			FirefoxCacheEntry ff = index.find(key);
			ff.save(path);
			cout << ff.file_path << endl;
		} catch (const std::exception&) {
			cout << "not found" << endl;
		}
	}
	return 0;
}


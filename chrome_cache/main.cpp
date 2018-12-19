#include "stdafx.hpp"
#include "chrome_cache.hpp"

using namespace std;

int main() {
	string cache_dir = R"(C:\Users\User\AppData\Local\Google\Chrome\User Data\Profile 3\Cache)";
	//cout << "cache_dir=" << flush;
	//getline(cin, cache_dir);
	ChromeCache cc;
RELOAD:
	try {
		cc = ChromeCache(cache_dir);
		cout << cc.count() << endl;
	} catch (const std::exception& ex) {
		cout << ex.what() << endl;
		return 0;
	}
	while (true) {
		cout << "-----------------------------------" << endl;
		string key, path;
		cout << "key=" << flush;
		getline(cin, key);
		if (key == "list") {
			cc.show_keys();
			continue;
		} else if (key == "reload") {
			cc.close();
			goto RELOAD;
		}
		cout << "path=" << flush;
		getline(cin, path);
		ChromeCacheEntry* entry = nullptr;
		if (entry = cc.get_entry(key)) {
			cc.save(path, entry);
			cout << "saved." << endl;
		} else {
			cout << "not found." << endl;
		}
	}
	cc.close();
	return 0;
}

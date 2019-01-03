#include "stdafx.hpp"
#include "chrome_cache.hpp"
#include <boost/program_options.hpp>

using namespace std;

//data length
//1 = header only. (302 Found or 204 No Content or content-length==0)
//2 = usually
//3 = what is the third data??

int main(int argc, char*argv[]) {
	const string dest = "chrome_cache_temp";
	if (argc > 1) {
		//argument parser
		using namespace boost::program_options;
		options_description description("extract browser cache.");
		description.add_options()
			("cache_dir,c", value<string>()->default_value(""), "cache directory")
			("key,k", value<string>()->default_value(""), "key(url)")
			("path,p", value<string>()->default_value(""), "path")
			("update_index,u", bool_switch()->default_value(true), "update index (=true)")
			("help,h", "help")
			("version,v", "version");
		variables_map vm;
		store(parse_command_line(argc, argv, description), vm);
		notify(vm);
		if (vm.count("help"))
			std::cout << description << std::endl;
		string cache_dir_ = vm["cache_dir"].as<string>();
		string key_ = vm["key"].as<string>();
		string path_ = vm["path"].as<string>();
		bool update_index_ = vm["update_index"].as<bool>();
		if (cache_dir_.size() && key_.size() && path_.size()) {
			try {
				ChromeCache cc(cache_dir_, dest, update_index_);
				cc.find_save(key_, path_);
			} catch (const std::exception&e) {
				cout << e.what() << endl;
			}
		} else {
			std::cout << description << std::endl;
		}
		return 0;
	} else {
		//interactive
		string cache_dir;// = R"(C:\Users\User\AppData\Local\Google\Chrome\User Data\Profile 3\Cache)";
		cout << "cache_dir=" << flush;
		getline(cin, cache_dir);
		ChromeCache cc;
	RELOAD:
		try {
			cc = ChromeCache(cache_dir, dest);
			cout << cc.count() << endl;
		} catch (const std::exception& ex) {
			cout << ex.what() << endl;
			return 0;
		}

		////////////////////////////////////////////////////////////////////////////////
		//try {
		//	for (ChromeCacheEntry *en : cc.get_entries()) {
		//		int size = en->data_addrs.size();
		//		if (size!= 2) {
		//			cout << size << "  " << en->key << endl;
		//			//cout << en->get_header().to_string() << endl;
		//		}
		//	}
		//} catch (const std::exception& e) {
		//	cout << e.what() << endl;
		//}
		//string u= "https://divnil.com/wallpaper/iphone/img/app/b/o/bokeh-images-hd-bokeh-install-bokeh-iphone-wallpaper-heart-shape-red-640x960_57928160ee11f46f602993ce22e01b13_s.jpg";
		//ChromeCacheEntry en = cc.find(u);
		//cout << en.key << endl;
		//cout <<"key_len"<< en.es->key_len << endl;
		//cout << "long_key"<<en.es->long_key << endl;
		//cout << en.get_header().to_string() << endl;
		//for (int i : en.data_lengths) {
		//	cout << i << " ";
		//}
		//cout << endl;
		//for (ChromeCacheAddress addr : en.data_addrs) {
		//	cout << addr.tostring() << endl;
		//}
		//return 0;
		////////////////////////////////////////////////////////////////////////////////

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
			try {
				cc.find_save(key, path);
			} catch (const std::exception& e) {
				cout << e.what() << endl;
			}
		}
		cc.close();
		return 0;
	}
}

#include <nan.h>
#include "node_chrome_cache.hpp"
//#include "node_chrome_cache_entry.hpp"

NAN_MODULE_INIT(init) {
	NChromeCache::Init(target);
	//NChromeCacheEntry::Init(target);
}

NODE_MODULE(chrome_cache, init);

//node-gyp configure --python "C:\Python27\python.exe"
//node-gyp build

//node-gyp --debug configure --python "C:\Python27\python.exe"
//node-gyp build --debug
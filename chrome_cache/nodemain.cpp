#include <nan.h>
#include "node_chrome_cache.hpp"
//#include "node_chrome_cache_entry.hpp"

NAN_MODULE_INIT(init) {
	NChromeCache::Init(target);
	//NChromeCacheEntry::Init(target);
}

NODE_MODULE(chrome_cache, init);
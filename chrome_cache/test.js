var c = require('./build/Release/chrome_cache');
var cc = new c.ChromeCache(String.raw `C:\Users\User\AppData\Local\Google\Chrome\User Data\Profile 3\Cache`, 'temp_cache_dir')
console.log(cc.keys().slice(0, 10));
// cc.find_save(cc.keys()[0],'./b.png');
var entry = cc.get_header(cc.keys()[0]);
console.log(entry);
// entry.save('./a')
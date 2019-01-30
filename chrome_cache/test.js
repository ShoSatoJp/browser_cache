var c = require('./build/Release/chrome_cache');
var cc = new c.ChromeCache(String.raw `C:\Users\User\AppData\Local\Google\Chrome\User Data\Profile 3\Cache`, 'temp_cache_dir')
console.log(cc.keys().slice(0,10));
// cc.find_save('http://133.162.253.166/banzai/shared_lib/img/header_group.png','./b.png');
var entry=cc.find('https://shosato.jp/');
console.log(entry.get_header());
entry.save('./a.html')
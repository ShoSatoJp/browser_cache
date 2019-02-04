var ChromeCache = require('./chrome_cache').ChromeCache

cc = new ChromeCache('C:/Users/User/AppData/Local/Google/Chrome/User Data/Profile 3/Cache/', 'temp');
console.log(cc.keys().slice(0, 10));
console.log(cc.find(cc.keys()[1]).get_header());
console.log('hoge')
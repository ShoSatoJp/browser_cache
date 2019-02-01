const fs=require('fs');


const stat=fs.statSync('chrome_cache_gui.7z')
console.log(stat.size)
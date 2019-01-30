const carlo = require('carlo');
const chrome_cache = require('./chrome_cache');
const path = require('path');
const fs = require('fs-extra');
const sizeOf = require('image-size');

function fsJSON(filename, object) {
    if (object) {
        fs.writeJSONSync(filename, object);
    } else {
        if (!fs.existsSync(filename))
            fs.writeJSONSync(filename, {});
        return fs.readJSONSync(filename);
    }
}

var cc;
const size_cache_json = 'size_cache.json';
let size_cache = fsJSON(size_cache_json);

function init_chrome_cache(dir) {
    cc = new chrome_cache.ChromeCache(dir, 'temp');
    return cc.keys();
}

function keys() {
    if (!cc) return;
    return cc.keys();
}

function find_save(key, filename, outdir) {
    if (!fs.existsSync(outdir)) fs.mkdirsSync(outdir);
    try {
        var file = path.join(outdir, filename);
        if (!fs.existsSync(file))
            cc.find_save(key, file);
        if (filename in size_cache) {
            return size_cache[filename];
        } else {
            const size = sizeOf(file);
            size_cache[filename] = {
                'height': size.height,
                'width': size.width
            };
            return size;
        }
    } catch (error) {
        return false;
    }
}

function findChromeCacheDir() {
    const chrome_userdata = process.env[process.platform == "win32" ? "USERPROFILE" : "HOME"].replace(/\\/g, '/') + '/AppData/Local/Google/Chrome/User Data/';
    const userdata_dirs = ['Default', ...Array.from(Array(5), (v, k) => 'Profile ' + (k + 1))];
    for (const dir of userdata_dirs) {
        const path = chrome_userdata + dir + '/Cache/';
        if (fs.existsSync(chrome_userdata + dir + '/Cache/')) {
            return path;
        }
    }
}


(async () => {
    try {
        // Launch the browser.
        const app = await carlo.launch();

        function exit() {
            fsJSON(size_cache_json, size_cache);
            app.exit();
            process.exit();
        }

        // Terminate Node.js process on app window closing.
        app.on('exit', exit);
        // Tell carlo where your web files are located.
        app.serveFolder('.');

        // Expose 'env' function in the web environment.
        await app.exposeFunction('init_chrome_cache', init_chrome_cache);
        await app.exposeFunction('keys', keys);
        await app.exposeFunction('find_save', find_save);
        await app.exposeFunction('fsJSON', fsJSON);
        await app.exposeFunction('findChromeCacheDir', findChromeCacheDir);
        await app.exposeFunction('exit', exit);

        // Navigate to the main page of your app.
        await app.load('index.html');
    } catch (error) {
        console.log(error)
    }
})();
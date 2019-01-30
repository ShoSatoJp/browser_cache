let liCreatingFlag;
let list = [];
let image_exts = ['jpeg', 'jpg', 'png', 'gif'];

async function reload_cache(dir) {
    return (await init_chrome_cache(dir))
        .filter(e => !e.startsWith('http://localhost:') && ~image_exts.indexOf(getExt(e)));
}

async function exit_app() {
    await exit();
}

String.prototype.hashCode = function () {
    var hash = 0,
        i, chr;
    if (this.length === 0) return hash;
    for (i = 0; i < this.length; i++) {
        chr = this.charCodeAt(i);
        hash = ((hash << 5) - hash) + chr;
        hash |= 0; // Convert to 32bit integer
    }
    return hash;
};

async function saveForm() {
    const data = [];
    document.querySelectorAll('input').forEach(e => {
        data.push(e.value);
    });
    await fsJSON('form.json', {
        data
    });
}

async function loadForm() {
    const data = (await fsJSON('form.json')).data;
    document.querySelectorAll('input').forEach((e, i) => {
        e.value = data[i];
    });
}

let images;
let images_length;
let images_index = 0;
let SEARCH_OPTIONS;
let LOAD_COUNT = 50;

async function listImages(start, count, dir, search_options = {
    height: 0,
    width: 0,
    query: null
}) {
    saveForm();
    const parent = document.querySelector('#containera');
    if (dir || !images) {
        while (parent.firstChild) parent.removeChild(parent.firstChild);
        SEARCH_OPTIONS = search_options;
        images = (await reload_cache(dir));
        if (SEARCH_OPTIONS.query.length) {
            images = images.filter(e => (new RegExp(SEARCH_OPTIONS.query)).test(e));
        }
        images_length = images.length;
        images_index = 0;
    }
    console.log(SEARCH_OPTIONS);
    // const do_size = SEARCH_OPTIONS.height || SEARCH_OPTIONS.width;

    for (let i = 0; i < count && images_index < images_length; images_index++) {
        const key = images[images_index];
        const id = key.hashCode(); //generateId(10);
        const ext = getExt(key);
        const filename = id + '.' + ext;
        try {
            var size;
            if ((new URL(key).hostname !== 'localhost') && ((size = await find_save(key, filename, './tempimg')) &&
                    size.height > SEARCH_OPTIONS.height && size.width > SEARCH_OPTIONS.width)) {
                const template = document.querySelector('#image-item-template');
                const element = template.content.cloneNode(true);
                const img = element.querySelector('img')
                img.setAttribute('src', './tempimg/' + filename);
                img.setAttribute('key', key);
                img.addEventListener('click', async function () {
                    const outdir = document.querySelector('#outputdir').value;
                    if (!outdir) alert('please specify output directory!');
                    const key = this.getAttribute('key');
                    await find_save(key, getFilename(key), outdir);
                    openNotification('Saved: ' + key);
                });
                const imgsize = element.querySelector('.img-size');
                imgsize.textContent = size.width + '×' + size.height;
                parent.appendChild(element);
                i++;
            }
        } catch {
            console.warn(key);
        }
    }
}

function getExt(key) {
    try {
        return getFilename(key).split('.').slice(-1)[0];
    } catch {
        return null;
    }
}

function getFilename(key) {
    return new URL(key).pathname.split('/').slice(-1)[0];
}

function generateId(length) {
    var text = "";
    var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (var i = 0; i < length; i++)
        text += possible.charAt(Math.floor(Math.random() * possible.length));
    return text;
}
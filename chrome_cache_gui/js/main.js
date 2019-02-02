const image_exts = ['jpeg', 'jpg', 'png', 'gif'];

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
        data.push({
            value: e.value,
            checked: e.checked
        });
    });
    await fsJSON('form.json', {
        data
    });
}

async function loadForm() {
    const data = (await fsJSON('form.json')).data;
    const inputs = document.querySelectorAll('input');
    if (inputs.length === data.length) {
        inputs.forEach((e, i) => {
            e.value = data[i].value || '';
            e.checked = data[i].checked;
        });
    }
}

let images;
let images_length;
let images_index = 0;
let SEARCH_OPTIONS;
let LOAD_COUNT = 50;
let LAST_ALIGNED_ITEM_INDEX = -1;
let ITEM_HEIGHT = 200;
const and = (a, b) => a && b;
const or = (a, b) => a || b;
async function listImages(start, count, dir, search_options = {
    height: 0,
    width: 0,
    query: null,
    use_regex: false,
    operator: 'and',
    aspect_ratio: 5,
    item_height:200,
}) {
    saveForm();
    const parent = document.querySelector('#containera');
    if (dir || !images) {
        while (parent.firstChild) parent.removeChild(parent.firstChild);
        SEARCH_OPTIONS = search_options;
        images = (await reload_cache(dir));
        if (SEARCH_OPTIONS.query.length) {
            images = images.filter(e => SEARCH_OPTIONS.use_regex ? (new RegExp(SEARCH_OPTIONS.query)).test(e) : ~e.indexOf(SEARCH_OPTIONS.query));
        }
        images_length = images.length;
        images_index = 0;
        LAST_ALIGNED_ITEM_INDEX = -1;
    }
    console.log(SEARCH_OPTIONS)
    const inverse_aspect_ratio = 1 / SEARCH_OPTIONS.aspect_ratio;
    const check_aspect_ratio = (w, h) => {
        const ratio = w / h;
        console.log(SEARCH_OPTIONS.aspect_ratio)
        return SEARCH_OPTIONS.aspect_ratio === 0 ? true : (ratio <= SEARCH_OPTIONS.aspect_ratio && ratio >= inverse_aspect_ratio);
    }
    for (let i = 0; i < count && images_index < images_length; images_index++) {
        const key = images[images_index];
        const id = key.hashCode(); //generateId(10);
        const ext = getExt(key);
        const filename = id + '.' + ext;
        const operator = SEARCH_OPTIONS.operator === 'and' ? and : or;
        try {
            var size;
            if ((new URL(key).hostname !== 'localhost') && ((size = await find_save(key, filename, './tempimg')) &&
                    operator(size.height > SEARCH_OPTIONS.height, size.width > SEARCH_OPTIONS.width) &&
                    check_aspect_ratio(size.width, size.height))) {
                const path = './tempimg/' + filename;
                const stat_ = await stat(path);
                const template = document.querySelector('#image-item-template');
                const element = template.content.cloneNode(true);
                const img = element.querySelector('img');
                img.setAttribute('src', path);
                img.setAttribute('key', key);
                img.setAttribute('height', ITEM_HEIGHT);
                img.setAttribute('data-height', size.height);
                img.setAttribute('data-width', size.width);
                img.addEventListener('click', async () => {
                    await saveImage(img);
                });
                const img_wh = element.querySelector('.img-size>.wh');
                img_wh.textContent = size.width + '×' + size.height;
                const img_size = element.querySelector('.img-size>.size');
                img_size.textContent = ' ' + bytes2bkmg(stat_.size);
                parent.appendChild(element);
                i++;
            }
        } catch (e) {
            console.warn(key, e);
        }
    }
    alignItems();
}

async function saveImage(e) {
    const outdir = document.querySelector('#outputdir').value;
    if (!outdir) return;
    const random_name = document.querySelector('#random-name').checked;
    const key = e.getAttribute('key');
    await find_save(key, random_name ? generateId(10) + '.' + getExt(key) : getFilename(key), outdir);
    openNotification(key);
}

async function saveAllImages() {
    const imgs = document.querySelectorAll('#containera img');
    for (let i = 0, len = imgs.length; i < len; i++) {
        await saveImage(imgs[i]);
    }
}


function alignItems(event) {
    const margin_per_item = 10;
    const img_height = ITEM_HEIGHT;
    const min_width = img_height * 1.25;
    const container = document.querySelector('#containera');
    const imgs = Array.from(container.querySelectorAll('img')).slice(LAST_ALIGNED_ITEM_INDEX + 1);
    const container_width = container.clientWidth;
    const last_aligned = LAST_ALIGNED_ITEM_INDEX;

    let stack = [];
    let sumwidth = 0;
    imgs.forEach((img, i) => {
        const original_width = parseInt(img.getAttribute('data-width'));
        const original_height = parseInt(img.getAttribute('data-height'));
        let img_width = img_height / original_height * original_width;
        if (img_width < min_width) img_width = min_width;
        sumwidth += margin_per_item + img_width;
        stack.push({
            img,
            img_width
        });
        if (sumwidth > container_width) {
            const sum_growth = sumwidth - container_width;
            let growth_rest = sum_growth;
            const sum = stack.map(e => e.img_width ^ 2).reduce((a, b) => a + b);
            stack.forEach(e => {
                //縮小方法の指定
                // e.img.setAttribute('width', e.img_width - growth_per_item + 'px');
                e.img.setAttribute('width', e.img_width - ((e.img_width ^ 2) / sum) * sum_growth + 'px');
            });
            //
            // stack.orderBy(e => e.img_width).forEach((e, i) => {
            //     var img_width;
            //     if (e.img_width < 200) {
            //         img_width = 200;
            //     } else {
            //         img_width = e.img_width - (growth_rest / (stack.length - i));
            //         growth_rest -= growth_rest / (stack.length - i);
            //     }
            //     e.img.setAttribute('width', img_width + 'px');
            // });
            //
            stack = [];
            sumwidth = 0;
            LAST_ALIGNED_ITEM_INDEX = last_aligned + i + 1;
        }
    });
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

function bytes2bkmg(bytes) {
    const units = ['B', 'KB', 'MB', 'GB', 'TB'];
    for (const unit of units) {
        if (bytes < 1024) {
            return Math.floor(bytes) + unit;
        }
        bytes /= 1024;
    }
}
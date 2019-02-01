Array.prototype.unique = function () {
    function onlyUnique(value, index, self) {
        return self.indexOf(value) === index;
    }
    return this.filter(onlyUnique);
};
Array.prototype.distinct = function (f) {
    var temp = this.map(x => f(x));
    return this.filter((value, index, self) => {
        return temp.indexOf(f(value)) === index;
    });
};
Array.prototype.distinct2 = function (...fs) {
    var temps = fs.map(f => this.map(x => f(x)));
    return this.filter((value, index) => {
        var result = false;
        temps.forEach((temp, i) => {
            result |= (temp.indexOf(fs[i](value)) === index);
        });
        return result;
    });
};
Array.prototype.shuffle = function () {
    var array = this;
    for (var i = array.length - 1; i > 0; i--) {
        var r = Math.floor(Math.random() * (i + 1));
        var tmp = array[i];
        array[i] = array[r];
        array[r] = tmp;
    }
    return array;
};
Array.prototype.take = function (n) {
    return this.slice(0, n);
};
Array.prototype.skip = function (n) {
    return this.slice(n);
};
Array.prototype.orderBy = function (f) {
    return this.sort((a, b) => {
        let c = 0;
        var aa = f(a),
            bb = f(b);
        if (aa > bb) {
            c = 1;
        } else if (bb > aa) {
            c = -1;
        }
        return c;
    });
};
Array.prototype.orderBy2 = function (...callbackfn) {
    return this.sort((a, b) => {
        for (let i = 0, len = callbackfn.length; i < len; i++) {
            const f = callbackfn[i];
            var aa = f(a),
                bb = f(b);
            if (aa > bb) return 1;
            if (bb > aa) return -1;
        }
        return 0;
    });
};
Array.prototype.orderByDescending = function (f) {
    return this.sort((a, b) => {
        let c = 0;
        var aa = f(a),
            bb = f(b);
        if (aa > bb) {
            c = -1;
        } else if (bb > aa) {
            c = 1;
        }
        return c;
    });
};
Array.prototype.first = function (f) {
    var length = this.length;
    for (let i = 0; i < length; i++) {
        if (f(this[i])) return this[i];
    }
    return null;
};
Array.prototype.toObject = function (keyfn) {
    var obj = {};
    this.forEach((e, i) => {
        e.index = i;
        obj[keyfn(e)] = e;
    });
    return obj;
};

function Object2Array(obj) {
    var array = [];
    for (const key in obj) {
        if (obj.hasOwnProperty(key)) {
            var t = obj[key];
            if ('index' in t) {
                array[t.index] = t;
            } else {
                array.push();
            }
        }
    }
    return array;
}

function normarizeTime(obj) {
    for (const key in obj) {
        if (obj.hasOwnProperty(key)) {
            if (obj[key] instanceof Date) {
                obj[key] = obj[key].getTime();
            }
        }
    }
    return obj;
}
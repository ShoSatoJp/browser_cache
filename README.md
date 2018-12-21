# chrome_cache
付属のバイナリはすべてC++ランタイムライブラリを含むx64版です。
## pythonモジュールとしての使用
### ビルド
以下のライブラリを適宜インクルードディレクトリに追加したり、リンクしたりする必要があります。
```
include:
    boost/algorithm/string/trim.hpp
    boost/algorithm/string/split.hpp
    boost/iostreams/filtering_stream.hpp
    boost/iostreams/filter/gzip.hpp
    boost/iostreams/copy.hpp
    pybind11/pybind11.h
    pybind11/stl.h

link:
    boost_iostreams-vc140-mt-gd.lib
    boost_program_options-vc140-mt-gd.lib
    zlibd.lib
    python36.lib
```

### 使い方
ビルドして出てきた```chrome_cache.pyd```をインポートします。
```python
from chrome_cache import ChromeCache

cc = ChromeCache("<chrome_cache_dir>", "<temp_dir>")

# ヘッダーとかも見るとき
entry = cc.find("<key(url)>")

print(entry.get_header().headers["content-type"])

entry.save("<output_path>")

# 保存するだけ
cc.find_save("<key(url)>","<output_path>")
```
```<chrome_cache_dir>```はChromeのキャッシュフォルダです。通常のキャッシュフォルダは```\AppData\Local\Google\Chrome\User Data\<user>\```です。 ```<user>``` は ```Default``` や ```Profile n``` の場合があります。  
```<temp_dir>```はファイルオープンの競合を避けるために使用するコピー先の作業用ディレクトリです。  
```<key(url)>``` は検索対象のURL、 ```<output_path>``` は保存先のパスです。※カレントディレクトリに保存する場合は接頭辞として ```./``` をつける必要があります。

## コンソールアプリとしての使用
### ビルド
static_debug構成でビルドします。
以下のライブラリを適宜インクルードディレクトリに追加したり、リンクしたりする必要があります。
```
include:
    boost/algorithm/string/trim.hpp
    boost/algorithm/string/split.hpp
    boost/iostreams/filtering_stream.hpp
    boost/iostreams/filter/gzip.hpp
    boost/iostreams/copy.hpp
    boost/program_options.hpp

link:
    boost_iostreams-vc140-mt-gd.lib
    boost_program_options-vc140-mt-gd.lib
    zlibd.lib
```

### 使い方
##### オプション付きで起動する場合
|          |                 |
| -------- | --------------- |
| ```-c``` | キャッシュフォルダ       |
| ```-k``` | URL             |
| ```-p``` | 出力パス            |
| ```-u``` | インデックスをリロードする |
##### オプションなしで起動する場合
インタラクティブに使用できます。
```key``` 入力時に ```list``` を使用してURL一覧を表示できます。また、```reload``` を使用してインデックスをリロードします。

# firefox_cache
付属のバイナリはすべてC++ランタイムライブラリを含むx64版です。
## pythonモジュールとしての使用
### ビルド
以下のライブラリを適宜インクルードディレクトリに追加したり、リンクしたりする必要があります。
```
include:
    boost/algorithm/string/trim.hpp
    boost/algorithm/string/split.hpp
    pybind11/pybind11.h
    pybind11/stl.h

link:
    python36.lib
```

### 使い方
ビルドして出てきた```firefox_cache.pyd```をインポートします。
```python
from firefox_cache import FirefoxCache

cc = FirefoxCache("<firefox_cache_dir>")

# ヘッダーとかも見るとき
entry = cc.find("<key(url)>")

print(entry.get_header().headers["content-type"])

entry.save("<output_path>")

# 保存するだけ
cc.find_save("<key(url)>","<output_path>")
```
```<firefox_cache_dir>```はFirefoxのキャッシュフォルダです。通常のキャッシュフォルダは```\AppData\Local\Mozilla\Firefox\Profiles\<user>\cache2\```です。 ```<user>``` は環境に依ります。  
```<key(url)>``` は検索対象のURL、 ```<output_path>``` は保存先のパスです。

※コンソールアプリとしての使用はサポートしていません。

import os
from firefox_cache import FirefoxCache

cache_dir = r"C:\Users\User\AppData\Local\Mozilla\Firefox\Profiles\qht8q8ei.default\cache2"
key = "https://s.aolcdn.com/hss/storage/midas/23cdfe3da345f994879224ae26769e13/202565355/OGB-INSIDER-BLOGS-GoogleLogox2-Animated.gif"
path = "./google_logo.gif"

c = FirefoxCache(cache_dir)

e = c.find(key)

h = e.get_header()

print(h.headers)
e.save(path)

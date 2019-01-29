from chrome_cache import ChromeCache
import concurrent.futures as futures

# cache_dir = r"C:\Users\User\AppData\Local\Google\Chrome\User Data\Profile 3\Cache"
cache_dir = r"E:\hoge\0\Default\Cache"
# cache_dir = r"C:\Users\User\AppData\Local\Opera Software\Opera Stable\Cache"
# key = "https://tsundora.com/image/2018/07/harukana_receive_19.jpg"
# path = "./harukana_receive_19.jpg"

cc = ChromeCache(cache_dir)

u = 'https://tsundora.com/image/2015/07/ranpo_kitan_game_of_laplace_2.jpg'

tpe=futures.ThreadPoolExecutor(2)

tpe.submit(cc.find_save,u,'e:\\hige\\乱歩奇譚\\a.jpg').result()

# e = cc.find(u)
# print(e.key)
# cc.find_save(u,'e:\\a.jpg')
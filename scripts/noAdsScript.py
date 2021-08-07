import os
import shutil
import re


PATH = os.getenv('APPDATA') + r"\Spotify\Apps"
xpui = os.path.join(PATH, "xpui")
spa = os.path.join(PATH, "xpui.spa")
bak = os.path.join(PATH, "xpui.bak")
fzip = os.path.join(PATH, "xpui.zip")
temp = os.path.join(PATH, "temp")
js = os.path.join(temp, "xpui.js")


if os.path.exists(spa) is False:
    exit()
if os.path.exists(bak):
    os.remove(bak)

shutil.copy(spa, bak)
shutil.move(spa, fzip)
shutil.unpack_archive(fzip, temp, "zip")

try:
    with open(js, "r", encoding="utf-8") as f:
        jsdata = f.read()
        UB = re.findall(r"1024&&(.+?UpgradeButton}\),)", jsdata)[0]
        LB = re.findall(r"\w+\.ads\.leaderboard\.isEnabled", jsdata)[0]
        LBedit = LB.replace("isEnabled", "isDisabled")
    with open(js, "w+", encoding="utf-8") as f:
        f.write(jsdata.replace(UB, "").replace(LB, LBedit))
except IndexError: print("[!] I did not find a button or empty block")

shutil.make_archive(xpui, 'zip', temp)
shutil.rmtree(temp)
shutil.move(fzip, spa)
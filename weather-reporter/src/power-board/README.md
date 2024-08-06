### USB programmer for Attiny85 chip is not detected
```
$> sudo vim /etc/udev/rules.d/10-usbasp.rules
SUBSYSTEM=="usb", ATTR{idVendor}=="16c0", ATTR{idProduct}=="05dc", GROUP="adm", MODE="0666", SYMLINK+="usbasp" # USBasp

sudo /etc/init.d/udev restart
```

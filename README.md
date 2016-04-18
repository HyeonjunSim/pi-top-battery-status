# pi-top-battery-status

- Displays the battery status of the pi-top battery
(a laptop based on the raspberry pi, see http://pi-top.com)
- Gives a warning if capacity is less or equal to 10%
- Shuts system down if capacity is less or equal to 5%, even if user pi does not respond to warnings 

![Alt text](screenshot.jpg?raw=true "battery charge")

The battery status is displayed on the desktop pannel.
**You must be user pi to install and use the program.**
Make sure that i2c is enabled in raspi-config.  

To install:

- Download the repository to your pi-top using the "download zip" button
- Using the file manager, go to your download folder,
 right click on "pi-top-battery-status-master.zip" and choose "Extract here"
- Open a console window and type the following commands

```
  cd Downloads
  cd pi-top-battery-status-master
  chmod +x ./install
  ./install
```
- Reboot your pi

If you want to recompile the program, you need to install

```
  sudo apt-get install libgtk-3-dev
```

You can test the compilation with

```
  touch gtk_battery.c
  make
```

To uninstall this program, edit /home/pi/.config/lxsession/LXDE-pi/autostart (remove line calling gtk_battery)
  
Please help to improve this program by tweeting to
**http://twitter.com/r_richarz** or opening an issue on this repository
if you have any problem or suggestion.

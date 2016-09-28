# pi-top-battery-status (version 1.3)

- Displays the battery status of the pi-top battery
(a laptop based on the raspberry pi, see http://pi-top.com)
- Displays the charging and discharging current in mA
- Gives a warning if capacity is less or equal to 10%
- Shuts system down if capacity reaches 5% and on battery power
- Logs all activities in /home/pi/batteryLog.txt

![Alt text](screenshot.jpg?raw=true "battery charge")

The battery status is displayed on the desktop panel.
**You must be user pi to install and use the program.**
Make sure that i2c is enabled in raspi-config.

If the icon is clicked, a window is opened which displays the following information:

![Alt text](screenshot2.png?raw=true "window")

By right-clicking on an empty space on the desktop panel and using the menu Panel settings
the desktop can be configured to show only the icons of the running programs

![Alt text](screenshot3.png?raw=true "pannel")

The display of the discharging current when running from battery can be used to get information
about battery usage at different screen brightness settings, and with additional hardware.

The log file logs detailed information about the status and health of the battery system:
- battery capacity in mAh
- charging and discharging current
- Total voltage
- Individual voltage of each of the 4 cells of the battery
 

To install:

- Download the repository to your pi-top:

```
  cd Downloads
  git clone git://github.com/rricharz/pi-top-battery-status
  sudo apt-get install wiringpi
  cd pi-top-battery-status
  chmod +x install
  ./install
```

- Reboot your pi

If you are running standard raspian instead of pi-top-os, and you get a 0% battery level display,
you might have to turn i2c on in Menu/Preferences/Raspberry Pi Configuration/Interfaces.

If you want to recompile the program, you need to install

```
  sudo apt-get install libgtk-3-dev
```

You can test the compilation with

```
  touch gtk_battery.c
  make
```

After compilation you need to install the program again.

The red, warning and shutdown levels can be adjusted in /home/pi/.config/pi-top/gtk_battery.txt.
The values have to be within the following limits:
  10 <= red = 80, 8 <= warning <= 80, 5 <= shutdown <= 78
  shutdown + 2 <= warning <= red

To uninstall this program
```
  cd ~/Downloads/pi-top-battery-status
  chmod +x ./uninstall
  ./uninstall
```


Release history:
- Version 1.0:  First stable release
- Version 1.0a: Fixed a minor bug, which could show a battery charge > 100% under rare circumstances
- Version 1.1:  Improved reliability of capacity reading. Possibility to abort automatic low battery shutdown added.
- Version 1.1a: i2c reliablity in log file, improved checking for i2c failures
- Version 1.2:  Using direct i2c access rather than calling i2cget to reduce overhead, shutdown only if discharging at low capacity
- Version 1.2a: Current and other additional information added
- Version 1.2b: Warning and shutdown level changed
- Version 1.3:  Important bug fixed which prevented automatic shutdown under certain circumstances
- Version 1.4:  Very slight adjustment of battery charge display for new PIXEL desktop

**Updating pi-top-battery-status**

If you still have the "pi-top-battery-status" in your "Downloads" folder, updating to the newest version
is very easy. Open a terminal and type:

```
  cd Downloads/pi-top-battery-status
  git pull
  chmod +x install
  ./install
```

If you do not have that folder anymore, you need to go again through the original installation instructions.


**What to do if you have problems with your battery**

See the troubleshooting guide at https://github.com/rricharz/pi-top-battery-status/blob/master/battery_problems.md


Please open an issue on this repositoryif you have a problem with the program.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. The owner of this
repository is not affiliated with pi-top.

*Other programs to display the pi-top-battery:*

- If you prefer to display the battery gauche on a Pimorini Unicorn hat
https://shop.pimoroni.com/products/unicorn-hat
instead of the desktop panel, Jez Shed has written a python script, see
https://github.com/JezShed/PiTop-Unicorn-Battery-Gauge

# pi-top-battery-status (deprecated)

> **IMPORTANT!**
> This repository contains deprecated programs and information for the original pi-top laptop.
> There is now new and improved software available for using Raspbian Stretch on the pi-top.
> See details here [up-to-date instructions for using Raspbian Stretch on the pi-top](http:github.com/rricharz/pi-top-setup).
> The software in this repository is **deprecated**. It is **not** fully compatible with the latest
> software for the pi-topSPEAKER and the pi-topPULSE, and does **not** work on the pi-top laptop version 2.
> If you have used the software in this repository up to now, it is therefore strongly recommended to
> switch to the up-to-date software (link above). The up-to-date software will also be required if you want
> to use the standard battery plugin for Raspbian Stretch (Battery pi-top / laptop). A new version of this
> standard Raspbian plugin, which will work with the pi-top laptop version 1 and 2 will be released very soon,
> and will only work if you are using the up-to-date pi-top software for Raspbian Stretch (see above).


- Displays the battery status of the pi-top battery
(a laptop based on the raspberry pi, see http://pi-top.com)
- Displays the charging and discharging current in mA
- Gives a warning if capacity is less or equal to 10%
- Shuts system down if capacity reaches 5% and on battery power
- Logs all activities in ~/batteryLog.txt

![Alt text](screenshot.jpg?raw=true "battery charge")

The battery status is displayed on the desktop panel.

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
 

**To install:**

- Download the repository to your pi-top:

```
  cd Downloads
  git clone --depth 1 git://github.com/rricharz/pi-top-battery-status
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

The red, warning and shutdown levels can be adjusted in ~/.config/pi-top/gtk_battery.txt.
The values have to be within the following limits:
  10 <= red = 80, 8 <= warning <= 80, 5 <= shutdown <= 78,
  shutdown + 2 <= warning <= red

**To uninstall this program**

```
  cd ~/Downloads/pi-top-battery-status
  chmod +x ./uninstall
  ./uninstall
```


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. The owner of this
repository is not affiliated with pi-top.

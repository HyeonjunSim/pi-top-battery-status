# pi-top-battery-status (version 1.2b)

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

The warning and shutdown levels can be adjusted in /home/pi/.config/pi-top/gtkbattery.txt.
The values have to be within the following limits: 10 <= red = 50, 8 <= warning <= 50, 5 <= shutdown <= 20

To uninstall this program, edit /home/pi/.config/lxsession/LXDE-pi/autostart (remove line calling gtk_battery)

Release history:
- Version 1.0: First stable release
- Version 1.0a: Fixed a minor bug, which could show a battery charge > 100% under rare circumstances
- Version 1.1: Improved reliability of capacity reading. Possibility to abort automatic low battery shutdown added.
- Version 1.1a: i2c reliablity in log file, improved checking for i2c failures
- Version 1.2: using direct i2c access rather than calling i2cget to reduce overhead, shutdown only if discharging at low capacity
- Version 1.2a: current and other additional information added
- Version 1.2b: warning and shutdown level changed

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

pi-to-battery-status was improved while working with Ivo Van Ursel on a project to fully understand
the pi-top-battery-pack. A summary of this project can be found in project.pdf in this repository.

There is also a program called "battery" in this repository. If you are in the downloaded
pi-top-battery-status directory, you can type

```
pkill gtk_battery
sudo ./battery status
sudo ./battery analyze
```

If you get an error message about "libpigpio", the pigpio library needs to be installed. This library is
part of the current raspian software, but does unfortunately not get installed automatically on all older
versions of Raspian and pi-topOS, if these older versions are upgraded. To install pigpio, type

```
sudo apt-get update
sudo apt-get install pigpio
```

If you get messages like "Failure: cannot read ..." using these commands, wait a short time and
repeat the command until you do not get the failure. Unfortunately the i2c bus on the Raspberry
Pi is often not very reliable due to a hardware limitation. The "pkill" command has killed the
pi-top-battery-status display program so that it does not interfere with the battery command.
After the next reboot the pi-top-battery-status display will work again.

Please help to improve this program by tweeting to
**http://twitter.com/r_richarz** or opening an issue on this repository
if you have any problem or suggestion.

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

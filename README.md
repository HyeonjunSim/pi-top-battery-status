# pi-top-battery-status

Displays the battery status of the pi-top battery (a laptop based on the raspberry pi)

The battery status is displayed on the desktop pannel

You must be user pi to install and use the program

Make sure that i2c is enabled in raspi-config 

To install:

  1. Download the depository to your computer

  2. In the local depository folder,type ./install

If you want to recompile the program, you need to install

  sudo apt-get install libgtk-3-dev

You can test the compilation with

  touch gtk_battery.c

  make
  
Please report any problems or suggestions to
  rene.richarz@bluewin.ch

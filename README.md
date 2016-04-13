# pi-top-battery-status

Displays the battery status of the pi-top battery (a laptop based on the raspberry pi).

The battery status is displayed on the desktop pannel.

You must be user pi to install and use the program.

Make sure that i2c is enabled in raspi-config.  

To install:

  1. Download the repository to your pi-top using the "download zip" button

  2. Using the file manager, go to your download folder,
 right click on "pi-top-battery-status-master.zip" and choose "Extract here"

  3. Open a console window and type the following commands

  4. cd Downloads

  5. cd pi-top-battery-status-master

  6. chmod +x ./install

  7. ./install

  8. Reboot your pi

If you want to recompile the program, you need to install

  sudo apt-get install libgtk-3-dev

You can test the compilation with

  touch gtk_battery.c

  make
  
Please report any problems or suggestions to
  rene.richarz@bluewin.ch

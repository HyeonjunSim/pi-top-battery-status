# WHAT TO DO IF YOU HAVE PROBLEMS WITH YOUR BATTERY

The following procedure can be used to analyze and solve battery problems using pi-topOS or
Raspbian Jessie, and with or without having pi-top-battery-status installed. Typical problems are
no charging, no power without the mains connected, or "fetching..." continuously displayed in pi-topOS.

Warning:! Fire hazard of Lithium Ion batteries: It is very strongly discouraged to tinker with the battery pack
or the firmware of the battery pack.

**Step 1: Does i2c work?**

The rpi communicates with the bq40z60 of the intelligent battery pack using i2c. A fully functional
i2c connection is therefore required.

Open a terminal and type
```
i2cdetect -y 1
```

If you get the answer "command not found" proceed with step 2.

If you get "could not open file /dev/i2c-1" or similar, proceed with step 3.

Otherwise, proceed with step 4.

**Step 2: Install i2c-tools**

Type
```
sudo apt-get install i2c-tools
```
Go back to step 1

**Step 3: Enable i2c**

Open the following menu "Menu->Preferences->Raspberry Pi configuration", select "Interfaces".

Is i2c already turned on?

If yes, your system is damaged. pi-top proposes to install a fresh system on the SD card. We think that
running "sudo apt-get autoremove" in pi-topOS might cause this problem.

If no, enable i2c and reboot your rpi. Then proceed with step 1.

**Step 4: analyze output of i2cdetect**

You should get something like
```
i2cdetect -y 1
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- 0b -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- --     
```
In this output, 0b is the expected response of the battery pack. If you have the pi-topSPEAKER installed, you will
also get a 73 (for mono) or similar entry.

If you get this table with the 0b, proceed to step 6. Otherwise continue with step 5.

**Step 5: Check and fix your hardware**

If you get a -- at the 0b position, the rpi cannot talk to the battery pack. Repeat the i2cdetect command a few times to
be sure that it does not see the 0b.

Shut down your rpi and turn power off. Disconnect the mains. Check the 40 pin cable between the rpi and the hub controller. Carefully look at both connectors to check for broken or
bend pins.

If you still cannot see the 0b with i2cdetect, you have a serious hardware problem. Contact support@pi-top.com.

**Step 6: Check the charging current**

You have established communication with the battery pack.

Type
```
i2cget -y 1 0x0b 0x0a w
```
If you get a "read failed", repeat the command by typing the up arrow followed by the enter key, until you get a value.

You should get a hex number, which represents the battery current. If you are not used to read hex
numbers, use your internet browser and go to www.binaryhexconverter.com/hex-to-decimal-converter and enter the
value in the hex field. If you get a value larger than 32767, subtract 65536 from this value to get a signed value.
You should have now a value for the current which should be in the range between -5000 and 5000 mA.

A positive number means your battery is charging. Typical charging currents are in the order of 1400 mA.
Everything is ok. Stop at this step and use your pi-top.

A negative number means your battery is discharging. Typical discharging currents are in the order of -330 mA. 
Proceed with step 7.

If you get zero current, proceed with step 7.

**Step 7: Why is the battery not charging?**

If you have not connected the mains cable, connect it and go back to step 6.

Type
```
i2cget -y 1 0x0b 0x0d w
```
and convert the result to decimal as described in step 6.

The result is the current capacity. If it is above 95%, your pi-top battery is not charging because your battery
is almost full.

If it is near or at 0%, there is a problem preventing the battery from charging.

**Step 8: Run the pi-top firmware upgrade**

Download and execute the pi-top firmware upgrade as follows. This will hopefully resolve your battery problem:

To download:

```
wget assets.pi-top.com/patches/pt-battery-fw-update
sudo chmod a=r+w+x pt-battery-fw-update
```

To run:

```
sudo ./pt-battery-fw-update -d
```

pi-top recommends to run it 5 -10 times successfully. You can use the up-arrow-key followed by the enter key to repeat
the command.

Once done, go back to step 6 and check the charging current. If everything is well now, I recommend that you
install pi-top-battery-status from https://github.com/rricharz/pi-top-battery-status to make sure that this problem does
not happen again. If you use Raspbian Jessie instead of pi-topOS, you also need to install the shutdown process of 
https://github.com/rricharz/pi-top-install

If it still does not work, contact support@pi-top.com


Please report any problems and suggested improvements of this procedures to r77@bluewin.ch 

Your feedback is most welcome.









# WHAT TO DO IF YOU HAVE PROBLEMS WITH YOUR BATTERY

The following procedure can be used to analyze and solve battery problems using pi-topOS or
Raspbian Jessie, and with our witout having pi-top-battery-status installed. Typical problems are
no charging, no power without the mains connected, or "fetching..." continously dispayed in pi-toOS.

**Step 1: Does i2c work**

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

Is I2C already turned on?

If yes, your system is damaged. pi-top proposes to install a fresh system on the SD card.

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

If you get a -- at the 0b position, the rpi cannot talk to the battery pack. Repeat the i2cetect command a few times to
be sure that it does not see the 0b.

Shut down your rpi and turn power off. Disconect he mains. Check the 40 pin cable between the rpi and the hub controller. Carefully look at both connectors to check for broken or
bend pins. Check also the connection between the battery pack and the hub controller. Reconnect the mains and turn your pi-top on. Go back to step 1.

If you still cannot see the 0b with i2cdetect, you have a serious hardware problem. Contact pi-top support.

**Step 6: Check the charging current**

You have establised communcation with the battery pack.

Type
```
i2cget -y 1 0x0b 0x0a w
```
If you get a "read failed", repeat the command by typing the up arrow following by the enter key, until you get a value.

You should get a hexadezimal number, which represents the battery current. If you are not used to read hexadezimal
numbers, use your internet browser and go to www.binaryhexconverter.com/hex-to-decimal-converter and enter the
value in the hex field. If you get a value larger than 32767, subtract 65536 from this value to get a signed value.
You shold have now a value for the current which should be in the range between -5000 and 5000 mA.

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

The result is the current capacity. If it is above 90%, your pi-top battery is not charging because your battery
is almost full.

If it is near or at 0%, there is a problem preventing the battery from charging. If you just want to fix the problem
proceed to step 9. If you want to know more details about the problem, go on to step 8.

**Step 8: Analyzing the problem**

Download this repository, if you have not already done so. Open a terminal and type

```
  cd Downloads
  git clone git://github.com/rricharz/pi-top-battery-status
```

Execute the battery status tool

```
  sudo apt-get install pigpio
  cd Downloads/pi-top-battery-status
  chmod +x battery
  sudo ./battery status
  sudo ./battery analyze
```
You might have to repeat the last two commands several times until you get an output without a "Failure".
If you cannot see yourself what the problem is, you can send the output of the 2 commands to r77@bluewin.ch
for a free explanation.

**Step 9: Run the pi-top firmware upgrade**

Download and execute the pi-top firmware upgrade as follows. This will hopefully resolve your battery problem:

To download:

```
wget https://s3-us-west-2.amazonaws.com/pi-top/pt-battery-fw-update_v2
```

To run:
`
```
sudo ./pt-battery-fw-update_v2 -d
```

pi-top recommends to run it 5 -10 times successfully. You can use the up-arrow-key followed by the enter key to repeat
the command.

Once done, go back to step 6 and check the charging current.








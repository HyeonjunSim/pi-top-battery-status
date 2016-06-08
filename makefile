LIBS = `pkg-config --libs gtk+-3.0`

CFLAGS = `pkg-config --cflags gtk+-3.0`

all: gtk_battery

gtk_battery: gtk_battery.c
	gcc -o gtk_battery gtk_battery.c $(LIBS) $(CFLAGS) -lwiringPi


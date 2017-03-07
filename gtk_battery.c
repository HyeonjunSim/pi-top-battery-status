/*

 * gtk_battery.c
 * display pi-top battery status

 * Copyright 2016  rricharz 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#define VERSION			"S=1.4"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <wiringPiI2C.h>

#define MAX_ANSWER_SIZE 64                     // Maximum size of answer string
#define MAX_COUNT       20                     // Maximum number of trials
#define SLEEP_TIME      500                    // time between two i2cget in microsec

#define WINDOW_WIDTH    260
#define WINDOW_HEIGHT   120
#define TEXT_OFFSET      20

#define GRAY_LEVEL      0.93

#define MAKELOG                1     // log file batteryLog in home directory (0 = no log file)

int redLevel, warningLevel, shutdownLevel;

cairo_surface_t *surface;
gint width;
gint height;
GtkWidget *MainWindow;
GtkWidget *StatusLabel1, *StatusLabel2, *StatusLabel3, *StatusLabel4, *StatusLabel5;
guint global_timeout_ref;
int lastCapacity;
int shutdownCounter;
int first;
long stat_good, stat_total;

static int lowBattery;

FILE *logFile;

int i2c_handle;

void printReg(int reg, char* unit, int min, int max)
{
	int count = 0;
	int value = 0;  
	while ((value == 0) && (count++ < MAX_COUNT)) {
		int result = i2cget(reg, &value);
		stat_total++;
		if (result == 0) {
			if ((value > max) || (value < min))
				value = 0;              // out of limits	
			else
				stat_good++;
		}
		usleep(SLEEP_TIME);
	}
	if (value == 0) {
		fprintf(logFile, " n/a %s", unit);
	}
	else
		fprintf(logFile, "%4d%s", value, unit);
}

void printLogEntry(int capacity, int current) {
	time_t rawtime;
	struct tm *timeinfo;
	char timeString[80];
	int stat_percent;
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timeString, 80, "%D %R:%S", timeinfo);
	fprintf(logFile, timeString);
	// printf("%s:  %s %d\n", timeString, s, i);
	
	if (stat_total > 0) {
		stat_percent = 100 * stat_good / stat_total;
		stat_good = 0;         // reset statistics
		stat_total = 0;
		}
	else
		stat_percent = 100;
	
	fprintf(logFile," - %s, I2C=%3d%%", VERSION, stat_percent);
	
	if (capacity)
		fprintf(logFile, ", C=%3d%%/", capacity);
	else
		fprintf(logFile, ", C=  n/a/");
	printReg(0x0F,"mAh", 0, 5000);
		
	if (current > -5000)
		fprintf(logFile, ", A=%4dmA", current);
	else
		fprintf(logFile, ", A=   n/a");
		
	fprintf(logFile, ", U=");

	printReg(0x09,"mV (", 10000, 20000);
	printReg(0x3F,"mV/", 2000, 5000);
	printReg(0x3E,"mV/", 2000, 5000);
	printReg(0x3D,"mV/", 2000, 5000);
	printReg(0x3C,"mV)", 2000, 5000);
	fprintf(logFile, "\n");
	fflush(logFile);
}

int i2cget(int address, int *data)
{
	int res = wiringPiI2CReadReg16(i2c_handle, address);
	if (res < 0)
		return -1;
	else {
		*data = res;
		return 0;
	}
}

static gboolean timer_event(GtkWidget *widget)
{
	cairo_t *cr;
	GdkPixbuf *new_pixbuf;
	int i, w;
	char str[255];
	char timeStr[255];
	char shortTimeStr[32];
	
	int capacity, current;
	int count, result;
	char *sstatus;
	int time;
	
	g_source_remove(global_timeout_ref);     // stop timer in case of tc_loop taking too long
	
	// capacity
	count = 0;
	capacity = -1;  
	while ((capacity  <  0) && (count++ < MAX_COUNT)) {
		result = i2cget(0x0d, &capacity);
		stat_total++;
		if (result == 0) {
			// if (count > 1) printf("count = %d, answer = %d\n", count, capacity);	
			if ((capacity > 100) || (capacity < 0)) {
				capacity = -1;              // capacity out of limits
			}
			else
			  stat_good++;
		}
		usleep(SLEEP_TIME);
	}

	// current
	sstatus = "unknown";
	count = 0;  
	while ((strcmp(sstatus, "unknown") == 0) && (count++ < MAX_COUNT)) {		
		result = i2cget(0x0a, &current);
		stat_total++;
		if (result == 0) {
			// if (count > 1) printf("count = %d, answer = %d\n", count, current);	
			if (current > 32767)                   // status is signed 16 bit word
				current -= 65536;
			// printf("current = %d\n", current);
			if ((current > -5000) && (current < 5000) && (current != -1)) {
				stat_good++;
				if (current < 0)
					sstatus = "discharging";
				else if (current > 0)
					sstatus = "charging";
				else
					sstatus = "external power";
			}
			else
				current = -32767;          // unknown
		}
		usleep(SLEEP_TIME);
	}
		
	// charging/discharging time
	count = 0;
	time = - 1;
	sprintf(timeStr, " ");
	sprintf(shortTimeStr, " ");
	if (strcmp(sstatus,"charging") == 0) {
		while ((time < 0) && (count++ < MAX_COUNT)) {
			result = i2cget(0x13, &time);
			stat_total++;
			// if (count > 1) printf("charging time: count = %d, answer = %s\n", count, time);
			if (result == 0) {	
				if ((time < 1) || (time >999)) {
					time = -1;                              
				}
				if (time > 0) {
					if (time <= 90) {
						sprintf(timeStr, "Estimated charging time: %d minutes\n", time);
						sprintf(shortTimeStr, "%d min", time);
					}
					else {
						sprintf(timeStr, "Estimated charging time: %.1f hours\n", (float)time / 60.0);  
						sprintf(shortTimeStr, "%.1f hours", (float)time / 60.0);
					}
					stat_good++;
				}
			}
			usleep(SLEEP_TIME);
		}
	}
	else if (strcmp(sstatus,"discharging") == 0) {
		while ((time < 0) && (count++ < MAX_COUNT)) {
			result = i2cget(0x12, &time);
			stat_total++;
			// if (count > 1) printf("discharging time: count = %d, answer = %d\n", count, time);	
			if (result == 0) {
				if ((time < 1) || (time > 960)) {
					time = -1;
				}
				if (time > 0) {
					if (time <= 90) {
						sprintf(timeStr, "Estimated life time: %d minutes\n", time);
						sprintf(shortTimeStr, "%d min", time);
					}
					else {
						sprintf(timeStr, "Estimated life time: %.1f hours\n", (double)time / 60.0);  
						sprintf(shortTimeStr, "%.1f hours", (float)time / 60.0);
					}
					stat_good++;
				}
			}
			usleep(SLEEP_TIME);
		}  
	}
	if (capacity != lastCapacity) {	
		printLogEntry(capacity, current);
		lastCapacity = capacity;
	}	
//	printf("i2c statistics: stat_good = %d, stat_total = %d, percent good %d%%\n", 
//		stat_good, stat_total, 100 * stat_good / stat_total);
	
	cr = cairo_create (surface);
	
	// fill the battery symbol
	if (capacity < 0)         // capacity out of limits
	  w = 0;
	else
	  w = (99 * capacity) / 400;
	if (strcmp(sstatus,"charging") == 0)
		cairo_set_source_rgb (cr, 1, 1, 0);
	else if (capacity <= redLevel)
		cairo_set_source_rgb (cr, 1, 0, 0);
	else if (strcmp(sstatus,"external power") == 0)
	    cairo_set_source_rgb (cr, 0.5, 0.5, 0.7);
	else
		cairo_set_source_rgb (cr, 0, 1, 0);
	cairo_rectangle (cr, 5, 4, w, 12);
	cairo_fill (cr);
	if (w < 23) {
		cairo_set_source_rgb (cr, 1, 1, 1);
		cairo_rectangle (cr, 5 + w, 4, 24 - w, 12);
		cairo_fill (cr);
	}
	
	// display the capacity figure
	cairo_set_source_rgb (cr, GRAY_LEVEL, GRAY_LEVEL, GRAY_LEVEL);
	cairo_rectangle (cr, 0, 20, 35, 15);
	cairo_fill (cr);  
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_select_font_face(cr, "Dosis", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 12);
	if (capacity >= 0) {
	  int x = 4;
	  if (capacity < 10)
	    x += 4;
	  else if (capacity > 99)
	    x -= 4;
	  cairo_move_to(cr, x, 33);
	  sprintf(str,"%2d%%",capacity);
	  cairo_show_text(cr, str);
	}
		
	// Create a new pixbuf from the modified surface and display icon
	new_pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
	gtk_window_set_icon(GTK_WINDOW(MainWindow), new_pixbuf);         
	g_object_unref(new_pixbuf);
	cairo_destroy (cr);
	
	// Update status in main Window
	sprintf(str, "<span size=\"medium\">Battery status: %s</span>", sstatus);
	gtk_label_set_markup(GTK_LABEL(StatusLabel2), str);
	if (capacity >= 0)
	  sprintf(str,"<span size=\"medium\">Capacity: %2d %%</span>", capacity);
	else
	  sprintf(str,"<span size=\"medium\">Capacity: n/a</span>");
	gtk_label_set_markup(GTK_LABEL(StatusLabel3), str);
	sprintf(str,"<span size=\"medium\">%s</span>", timeStr);
	gtk_label_set_markup(GTK_LABEL(StatusLabel4), str);
	if ((current >= -5000) && (current <= 5000) && (current != -1))
	  sprintf(str,"<span size=\"medium\">Current: %2d mA</span>", current);
	else
	  sprintf(str,"<span size=\"medium\">Current: n/a</span>", current);
	gtk_label_set_markup(GTK_LABEL(StatusLabel5), str);
	
	// display the remaining time in the title
	gtk_window_set_title(GTK_WINDOW(MainWindow), shortTimeStr);

	if (capacity > shutdownLevel)
	  shutdownCounter = 0;
	  
	// printf("Capacity = %d, lowBattery = %d, status %s\n",capacity, lowBattery, sstatus);
	
	if ((capacity > 0) && (capacity <= lowBattery) && (strcmp(sstatus,"discharging") == 0)) {

		if (capacity <= shutdownLevel) {
		    shutdownCounter++;
			sprintf(str,
				"Battery capacity very low! Automatic shutdown will happen in 2 minutes! (To abort shutdown, open a terminal and type 'pkill gtk_battery')");
			if (shutdownCounter < 2) {
				fprintf(logFile,"%s\n",str);
				fflush(logFile);
			}
		}
		else {
			sprintf(str, 
				"Battery capacity low! Automatic shutdown will happen at %d percent",
				shutdownLevel);
			fprintf(logFile,"%s\n",str);
			fflush(logFile);
		}
		
		if (shutdownCounter < 2) {
			GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(MainWindow), GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, str);		
			// open as non modal window
			g_signal_connect_swapped (dialog, "response",
			G_CALLBACK (gtk_widget_destroy), dialog);
			gtk_widget_show_all(dialog);
		}
		
		if ((capacity <= shutdownLevel) && (shutdownCounter >= 20)) {
			printLogEntry(capacity, current);
			fprintf(logFile,"SHUTTING SYSTEM DOWN\n",str);
			fflush(logFile);
			if (MAKELOG)
				fclose(logFile);
			system("sudo shutdown -h now &");
			return FALSE;        // no further checks, system will shut down anyway
		}
		
		// reduce warning level for next warning
		if (lowBattery > shutdownLevel)
			lowBattery -= 1;
			
		// avoid multiple warnings if battery is already low
		if (lowBattery > (capacity - 1))
			lowBattery = capacity - 1;
			
		// in any case, warn if capacity is below shutdown level
		if (lowBattery < shutdownLevel)
			  lowBattery = shutdownLevel;
	}
	
	
	// initialize warning again, if battery is charging
	if (strcmp(sstatus,"charging") == 0)
			lowBattery = warningLevel;
	
	// restart timer		
	global_timeout_ref = g_timeout_add(5000, (GSourceFunc) timer_event, (gpointer) MainWindow);
	
	return TRUE;
}

static void iconify(void)
{
	gtk_window_iconify(GTK_WINDOW(MainWindow));
}

int main(int argc, char *argv[])
{
	GdkPixbuf *pixbuf, *new_pixbuf;
	cairo_t *cr;
	cairo_format_t format;
	
	lastCapacity = 0;
	shutdownCounter = 0;
	stat_good = 0;
	stat_total = 0;
	first = 1;
	
	if (MAKELOG) {
		logFile = fopen("/home/pi/batteryLog.txt","a");
	}
	else
		logFile = stdout;
		
	i2c_handle = wiringPiI2CSetup(0x0b);
	if (i2c_handle < 0) {
		fprintf(logFile, "Cannot get handle for wiringPii2c\n");
		return 1;
	}
	
	FILE *confFile;
	confFile = fopen("/home/pi/.config/pi-top/gtk_battery.txt","r");
  	if (confFile == NULL) {
		fprintf(logFile,"Cannot open /home/pi/.config/pi-top/gtk_battery.txt, using defaults\n");
		redLevel = 10;
		warningLevel = 8;
		shutdownLevel = 5;
	}
	else {
		fscanf(confFile, "red=%d\n", &redLevel);
		if (redLevel < 10) redLevel = 10;
		if (redLevel > 90) redLevel = 90;
		fscanf(confFile, "warning=%d\n", &warningLevel);
		if (warningLevel < 8) warningLevel = 8;
		if (warningLevel > 90) warningLevel = 90;
		fscanf(confFile, "shutdown=%d\n", &shutdownLevel);
		if (shutdownLevel < 5) shutdownLevel = 5;
		if (shutdownLevel > 88) shutdownLevel = 88;
		if (warningLevel < shutdownLevel + 2) warningLevel = shutdownLevel + 2;
		if (redLevel < warningLevel) redLevel = warningLevel;
		
		fclose(confFile);
	}
	
	lowBattery = warningLevel;
  
	fprintf(logFile,"red=%d\n", redLevel);
	fprintf(logFile,"warning=%d\n", warningLevel);
	fprintf(logFile,"shutdown=%d\n", shutdownLevel);

	gtk_init(&argc, &argv);
	
	MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);         // Create the main windows

	// Define main window event handlers  
	gtk_widget_add_events(MainWindow, GDK_BUTTON_PRESS_MASK);
	// g_signal_connect(MainWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL); 
	g_signal_connect(MainWindow, "delete_event", G_CALLBACK(gtk_window_iconify), NULL);
	g_signal_connect(MainWindow, "button-press-event", G_CALLBACK(iconify), NULL); 
 
	// Position, size, icon and title of main window
	GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(MainWindow));
	int screenWidth = gdk_screen_get_width(screen);
	gtk_window_move(GTK_WINDOW(MainWindow), screenWidth - WINDOW_WIDTH - 2, 37);
	gtk_window_set_default_size(GTK_WINDOW(MainWindow), WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_window_set_title(GTK_WINDOW(MainWindow), "pi-top battery");
	// gtk_window_set_decorated(GTK_WINDOW(MainWindow), FALSE);
	gtk_window_set_keep_above(GTK_WINDOW(MainWindow), TRUE);
	gtk_window_iconify(GTK_WINDOW(MainWindow));
	
	// create the drawing surface and fill with icon
	char *iconPath = "/home/pi/bin/battery_icon.png";
	pixbuf = gdk_pixbuf_new_from_file (iconPath, NULL);
	if (pixbuf == NULL) {
		fprintf(logFile, "Cannot load icon (/home/pi/bin/battery_icon.png)\n", -1);
		return 1;
	}
	format = (gdk_pixbuf_get_has_alpha (pixbuf)) ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	surface = cairo_image_surface_create (format, width, height);
	g_assert (surface != NULL);
	
	// Draw icon onto the surface
	cr = cairo_create (surface);     
	gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
	cairo_paint (cr);
	cairo_destroy (cr);
	
	// Add timer event
	// Register the timer and set time in mS.
	// The timer_event() function is called repeatedly until it returns FALSE. 
	global_timeout_ref = g_timeout_add(5000, (GSourceFunc) timer_event, (gpointer) MainWindow);
	
	// Create label in main window
	GtkWidget *fixed;
	fixed = gtk_fixed_new();
	gtk_widget_set_size_request(fixed, WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_container_add(GTK_CONTAINER(MainWindow), fixed);
	gtk_widget_show(fixed);
	
	StatusLabel1 = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(StatusLabel1),
	  "<span size=\"x-large\">pi-top</span>");
	gtk_label_set_justify(GTK_LABEL(StatusLabel1), GTK_JUSTIFY_LEFT);
	gtk_fixed_put(GTK_FIXED(fixed), StatusLabel1, 64, 6);
	
	StatusLabel2 = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(StatusLabel2),
	  "<span size=\"medium\">Battery status:</span>");
	gtk_label_set_justify(GTK_LABEL(StatusLabel2), GTK_JUSTIFY_LEFT);
	gtk_fixed_put(GTK_FIXED(fixed), StatusLabel2, 8, 16 + TEXT_OFFSET);
	
	StatusLabel3 = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(StatusLabel3),
	  "<span size=\"medium\">Capacity:</span>");
	gtk_label_set_justify(GTK_LABEL(StatusLabel3), GTK_JUSTIFY_LEFT);
	gtk_fixed_put(GTK_FIXED(fixed), StatusLabel3, 8, 16 + 2 * TEXT_OFFSET);
	
	StatusLabel4 = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(StatusLabel4),
	  "<span size=\"medium\">Time:</span>");
	gtk_label_set_justify(GTK_LABEL(StatusLabel4), GTK_JUSTIFY_LEFT);
	gtk_fixed_put(GTK_FIXED(fixed), StatusLabel4, 8, 16 + 3 * TEXT_OFFSET);
	
	StatusLabel5 = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(StatusLabel5),
	  "<span size=\"medium\">Time:</span>");
	gtk_label_set_justify(GTK_LABEL(StatusLabel5), GTK_JUSTIFY_LEFT);
	gtk_fixed_put(GTK_FIXED(fixed), StatusLabel5, 8, 16 + 4 * TEXT_OFFSET);

	// display the window
	gtk_widget_show_all(MainWindow);
	
	// Call the timer function because we don't want to wait for the first time period triggered call
	timer_event(MainWindow);
	
	gtk_main();
	
	if (MAKELOG) {
	  fclose(logFile);
	}
	
	return 0;
}

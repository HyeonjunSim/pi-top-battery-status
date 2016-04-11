/*

 * gtk_battery.c
 * display pi-top battery status

 * Copyright 2016  <rene.richarz@bluewin.ch>
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

#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ANSWER_SIZE 64                     // Maximum size of answer string
#define MAX_COUNT       20                     // Maximum number of trials

#define WINDOW_WIDTH    260
#define WINDOW_HEIGHT   100
#define TEXT_OFFSET      20

#define GRAY_LEVEL      0.93

static cairo_surface_t *surface;
static gint width;
static gint height;
static GtkWidget *MainWindow;
static GtkWidget *StatusLabel1, *StatusLabel2, *StatusLabel3, *StatusLabel4;

int i2cget(char *command, char *answer)
{
	FILE *fp;
	
	fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failure to run i2cget\n");
		exit(1);
	}
	
	if (fgets(answer, MAX_ANSWER_SIZE - 1, fp) != 0) {
		pclose(fp);
		if ((answer[0] != '0') || (answer[1] != 'x')) {  // if not a hex value
			usleep(2000);
			return 1;
		
		}
		else
			return 0;	   
	}
	else {
		pclose(fp);
		return 1;
	}
}

static gboolean timer_event(GtkWidget *widget)
{
	cairo_t *cr;
	GdkPixbuf *new_pixbuf;
	int w;
	char str[255];
	char timeStr[255];
	char shortTimeStr[32];
	
	char answer[MAX_ANSWER_SIZE];
	int capacity, status;
	int count, result;
	char *sstatus;
	int time;
	
	// capacity
	count = 0;
	capacity = 0;  
	while ((result = i2cget("/usr/sbin/i2cget -y 1 0x0b 0x0d w 2>&1", answer)) && (count++ < MAX_COUNT)); 
	if (result == 0) {
		// if (count > 1) printf("count = %d, answer = %s\n", count, answer);	
		sscanf(answer, "%x", &capacity);
		// printf("Capacity = %d%%\n", capacity);
	}
	else
		printf("Cannot talk to battery pack\n");

	// status
	sstatus = "undefined";
	count = 0;  
	while ((result = i2cget("/usr/sbin/i2cget -y 1 0x0b 0x0a w 2>&1", answer)) && (count++ < MAX_COUNT));  
	if (result == 0) {
		// if (count > 1) printf("count = %d, answer = %s\n", count, answer);	
		sscanf(answer, "%x", &status);
		if (status > 32767)                   // status is signed 16 bit word
		  status -= 65536;
		// printf("status = %d\n", status);
		if ((status > -4000) && (status < 4000)) {
			if (status <= 10)
				sstatus = "discharging";
			else
				sstatus = "charging";
		}
		else
		  sstatus = "undefined";
		// printf("Status = %s\n", sstatus);
	}
	else
		printf("Cannot talk to battery pack\n");
		
	// charging/discharging time
	count = 0;
	timeStr[0] = 0;
	shortTimeStr[0] = 0;
	if (strcmp(sstatus,"charging") == 0) {
		while ((result = i2cget("/usr/sbin/i2cget -y 1 0x0b 0x13 w 2>&1", answer)) && (count++ < MAX_COUNT));
		if (result == 0) {
			// if (count > 1) printf("count = %d, answer = %s\n", count, answer);	
			sscanf(answer, "%x", &time);
			if (time <= 90) {
				sprintf(timeStr, "Estimated charging time = %d minutes\n", time);
				sprintf(shortTimeStr, "%d min", time);
			}
			else {
				sprintf(timeStr, "Estimated charging time = %.1f hours\n", (float)time / 60.0);  
				sprintf(shortTimeStr, "%.1f hours", (float)time / 60.0);
			}
		}  
		else
			printf("Cannot talk to battery pack\n");
	}
	else if (strcmp(sstatus,"discharging") == 0) {
		while ((result = i2cget("/usr/sbin/i2cget -y 1 0x0b 0x12 w 2>&1", answer)) && (count++ < MAX_COUNT));
		if (result == 0) {
			// if (count > 1) printf("count = %d, answer = %s\n", count, answer);	
			sscanf(answer, "%x", &time);
			if (time <= 90) {
				sprintf(timeStr, "Estimated life time = %d minutes\n", time);
				sprintf(shortTimeStr, "%d min", time);
			}
			else {
				sprintf(timeStr, "Estimated life time = %.1f hours\n", (double)time / 60.0);  
				sprintf(shortTimeStr, "%.1f hours", (float)time / 60.0);
			}
		}  
		else
			printf("Cannot talk to battery pack\n");
	}
	
	cr = cairo_create (surface);
	
	// fill the battery symbol
	w = (99 * capacity) / 400;
	if (strcmp(sstatus,"charging") == 0)
		cairo_set_source_rgb (cr, 1, 1, 0);
	else if (capacity < 20)
		cairo_set_source_rgb (cr, 1, 0, 0);
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
	cairo_rectangle (cr, 2, 20, 32, 15);
	cairo_fill (cr);   
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_select_font_face(cr, "Dosis", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 12);
	int x = 5;
	if (capacity < 10)
	  x += 2;
	else if (capacity > 99)
	  x -= 1;
	cairo_move_to(cr, x, 32);
	sprintf(str,"%2d %%",capacity);
	cairo_show_text(cr, str);
		
	// Create a new pixbuf from the modified surface and display icon
	new_pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
	gtk_window_set_icon(GTK_WINDOW(MainWindow), new_pixbuf);         
	g_object_unref(new_pixbuf);
	cairo_destroy (cr);
	
	// Update status in main Window
	sprintf(str, "<span size=\"medium\">Battery status: %s</span>", sstatus);
	gtk_label_set_markup(GTK_LABEL(StatusLabel2), str);
	sprintf(str,"<span size=\"medium\">Capacity: %2d %%</span>", capacity);
	gtk_label_set_markup(GTK_LABEL(StatusLabel3), str);
	sprintf(str,"<span size=\"medium\">%s</span>", timeStr);
	gtk_label_set_markup(GTK_LABEL(StatusLabel4), str);
	
	// display the remaining time in the title
	gtk_window_set_title(GTK_WINDOW(MainWindow), shortTimeStr);
	
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

	gtk_init(&argc, &argv);
		
	MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);         // Create the main windows

	// Define main window event handlers  
	gtk_widget_add_events(MainWindow, GDK_BUTTON_PRESS_MASK);
	//g_signal_connect(MainWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL); 
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
		printf("Cannot open %s\n", iconPath);
		return(1);
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
	g_timeout_add(5000, (GSourceFunc) timer_event, (gpointer) MainWindow);
	
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
	
	// display the window
	gtk_widget_show_all(MainWindow);
	// Call the timer function because we don't want to wait for the first time period triggered call
	timer_event(MainWindow);
			
	gtk_main();
	
	return 0;
}

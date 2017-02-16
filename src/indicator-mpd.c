#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <mpd/client.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#ifndef DEBUG
	#define DEBUG 0
#endif
#define INTERVAL 2
#define MAX_WIDTH 20

/* GLOBAL VARIABLES */ //because shit's on fire yo
struct mpd_connection *conn;
struct mpd_status *status;
struct mpd_song *song;
AppIndicator *indicator;

struct widgets // Gtk... other stuff
{
	GtkWidget *menu;
} widgets;

struct items // GtkMenuItem
{
	GtkWidget *prev;
	GtkWidget *next;
	GtkWidget *quit;
	GtkWidget *title;
	GtkWidget *toggle;
	GtkWidget *artist;
	GtkWidget *songid;
	GtkWidget *state;
} items;

struct details // some things we would like to get from mpd server
{
	char volume[10];
	char songid[10];
	short state;
} details;
/*END* GLOBAL VARIABLES *END*/


/* FUNCTIONS DECLARATIONS */
void logger(unsigned int count, ...);
char *get_addr_from_config();
char *shrink_to_fit(const char *source, unsigned int len);
gboolean update();
void run_toggle();
void run_next();
void run_previous();
/*END* FUNCTIONS DECLARATIONS *END*/

/* MAIN */
int main (int argc, char *argv[])
{
	conn = mpd_connection_new(get_addr_from_config(), 0, 3000);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
	{
		if(DEBUG) logger(1, "Connection: error");
		if(DEBUG) logger(1, "Exiting...");
		return 1;
	}
	else if(DEBUG) logger(1, "Connection: successful");

	gtk_init (&argc, &argv);

	widgets.menu = gtk_menu_new();

	items.artist = gtk_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL(widgets.menu), items.artist);
	gtk_widget_set_sensitive (items.artist, false);

	items.title = gtk_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL(widgets.menu), items.title);
	gtk_widget_set_sensitive (items.title, false);

	items.songid = gtk_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL(widgets.menu), items.songid);
	gtk_widget_set_sensitive (items.songid, false);

	items.state = gtk_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL(widgets.menu), items.state);
	gtk_widget_set_sensitive (items.state, false);

	items.toggle = gtk_menu_item_new_with_label("Play / Pause");
	gtk_menu_shell_append (GTK_MENU_SHELL(widgets.menu), items.toggle);
	g_signal_connect(items.toggle, "activate", G_CALLBACK(run_toggle), NULL);

	items.next = gtk_menu_item_new_with_label("Next");
	gtk_menu_shell_append (GTK_MENU_SHELL(widgets.menu), items.next);
	g_signal_connect(items.next, "activate", G_CALLBACK(run_next), NULL);

	items.prev = gtk_menu_item_new_with_label("Prev");
	gtk_menu_shell_append (GTK_MENU_SHELL(widgets.menu), items.prev);
	g_signal_connect(items.prev, "activate", G_CALLBACK(run_previous), NULL);

	items.quit = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append (GTK_MENU_SHELL(widgets.menu), items.quit);
	g_signal_connect(items.quit, "activate", gtk_main_quit, NULL);

	indicator = app_indicator_new ("indicator-mpd", "indicator-appmenu-menu-panel", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_menu (indicator, GTK_MENU(widgets.menu));
	app_indicator_set_title (indicator, "MPD Indicator");

	g_timeout_add_seconds(INTERVAL, update, 0);

	gtk_widget_show_all(widgets.menu);
	gtk_main();
	return 0;
}
/*END* MAIN *END*/

/* FUNCTIONS DEFINITIONS */
void logger(unsigned int count, ...)
{
	time_t mytime = time(0);
	va_list vl;
	printf("[%s] ", strtok(ctime(&mytime), "\n"));

	va_start(vl, count);
	for(unsigned int i = 0; i < count; i++)
	{
		printf("%s", va_arg(vl, const char *));
	}
	va_end(vl);

	printf("\n");
}

char *get_addr_from_config()
{
	char *addr = malloc(50);
	char *path = malloc(50);
	strcpy(path, getenv("HOME"));
	strcat(path, "/.config/indicator-mpd.conf");
	FILE *config = fopen(path, "r");

	if(config)
	{
		fscanf(config, "%s", addr);
		fclose(config);
		if(DEBUG) logger(2, "Config: ", path);
		if(addr)
		{
			if(DEBUG) logger(1, "Config: address found");
		}
		else if(DEBUG) logger(1, "Config: address not found");
	}
	else if(DEBUG) logger(1, "Config: error");
	return addr;
}

char *shrink_to_fit(const char *source, unsigned int len)
{
	if(strlen(source) <= len)
		return (char *)source;
	else
	{
		char *res = malloc(len + 3);
		for(unsigned int i = 0; i < len; i++)
		{
			res[i] = source[i];
		}
		res[len] = '.';
		res[len + 1] = '.';
		res[len + 2] = '.';
		res[len + 3] = '\0';
		return res;
	}
}

gboolean update()
{
	if((status = mpd_run_status(conn)) != 0)
	{
		if(details.state != mpd_status_get_state(status))
		{
			details.state = mpd_status_get_state(status);
			switch(details.state)
			{
				case MPD_STATE_UNKNOWN: //0
					gtk_menu_item_set_label(GTK_MENU_ITEM(items.state), "Unknown");
					if(DEBUG) logger(1, "State: unknown");
				break;
				case MPD_STATE_STOP: //1
					gtk_menu_item_set_label(GTK_MENU_ITEM(items.state), "Not Playing");
					if(DEBUG) logger(1, "State: not playing");
				break;
				case MPD_STATE_PLAY: //2
					gtk_menu_item_set_label(GTK_MENU_ITEM(items.state), "Playing");
					if(DEBUG) logger(1, "State: playing");
				break;
				case MPD_STATE_PAUSE: //3
					gtk_menu_item_set_label(GTK_MENU_ITEM(items.state), "Paused");
					if(DEBUG) logger(1, "State: paused");
				break;
			}
		}

		sprintf(details.songid, "%d", mpd_status_get_song_pos(status) + 1);
		if(strcmp(details.songid, gtk_menu_item_get_label(GTK_MENU_ITEM(items.songid))) != 0)
		{
			gtk_menu_item_set_label(GTK_MENU_ITEM(items.songid), details.songid);
			if(DEBUG) logger(2, "Playing: number ", details.songid);

			if((song = mpd_run_current_song(conn)) != 0)
			{
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.title), shrink_to_fit(mpd_song_get_tag(song, MPD_TAG_TITLE, 0), MAX_WIDTH));
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.artist), shrink_to_fit(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0), MAX_WIDTH));
			}
			else if(DEBUG) logger(1, "Song: error");
		}
	}
	else
	{
		if(DEBUG) logger(1, "Status: error");
		if(DEBUG) logger(1, "Connection: reconnect");
		conn = mpd_connection_new(get_addr_from_config(), 0, 3000);
	}
	return true;
}

void run_toggle()
{
	mpd_run_toggle_pause(conn);
	if(DEBUG) logger(1, "Song: toggle");
}

void run_next()
{
	mpd_run_next(conn);
	if(DEBUG) logger(1, "Song: next");
}

void run_previous()
{
	mpd_run_previous(conn);
	if(DEBUG) logger(1, "Song: prev");
}
/*END* FUNCTIONS DEFINITIONS *END*/

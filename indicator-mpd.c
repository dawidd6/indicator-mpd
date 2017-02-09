/*
 * 2017 dawidd6
 *
 *
 *
 */

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <mpd/client.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#define INTERVAL 3
#define TITLE_LENGTH 20

struct mpd_connection *conn;
struct mpd_status *status;
struct mpd_song *song;
GtkWidget *menu;
GtkWidget *item_prev, *item_next, *item_quit, *item_title, *item_toggle, *item_artist, *item_volume, *item_songid;
AppIndicator *indicator;

void logger(unsigned int count, ...) //const char * ...
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

gboolean update()
{
	if((status = mpd_run_status(conn)) != 0)
	{
		char songid[10];
		sprintf(songid, "%d", mpd_status_get_song_pos(status) + 1);
		if(strcmp(songid, gtk_menu_item_get_label(GTK_MENU_ITEM(item_songid))) != 0)
		{
			gtk_menu_item_set_label(GTK_MENU_ITEM(item_songid), songid);
			logger(2, "Now playing no. ", songid);

			if((song = mpd_run_current_song(conn)) != 0)
			{
				gtk_menu_item_set_label(GTK_MENU_ITEM(item_title), mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
				gtk_menu_item_set_label(GTK_MENU_ITEM(item_artist), mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
			}
		}
	}
	return true;
}

void run_toggle(/*GtkWidget *ignore, struct mpd_connection *conn*/)
{
	mpd_run_toggle_pause(conn);
	logger(1, "Toggled");
}

void run_next()
{
	mpd_run_next(conn);
	logger(1, "Next song");
}

void run_previous()
{
	mpd_run_previous(conn);
	logger(1, "Previous song");
}

/* Main */
int main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);

	conn = mpd_connection_new(0, 0, 3000);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
	{
		logger(1, "Connection error");
	}
	else logger(1, "Connection successful");

	menu = gtk_menu_new();

	item_artist = gtk_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_artist);
	gtk_widget_set_sensitive (item_artist, false);

	item_title = gtk_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_title);
	gtk_widget_set_sensitive (item_title, false);

	/*item_volume = gtk_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_volume);
	gtk_widget_set_sensitive (item_volume, false);*/

	item_songid = gtk_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_songid);
	gtk_widget_set_sensitive (item_songid, false);

	item_toggle = gtk_menu_item_new_with_label("Play / Pause");
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_toggle);
	g_signal_connect(item_toggle, "activate", G_CALLBACK(run_toggle), conn);

	item_next = gtk_menu_item_new_with_label("Next");
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_next);
	g_signal_connect(item_next, "activate", G_CALLBACK(run_next), conn);

	item_prev = gtk_menu_item_new_with_label("Prev");
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_prev);
	g_signal_connect(item_prev, "activate", G_CALLBACK(run_previous), conn);

	item_quit = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_quit);
	g_signal_connect(item_quit, "activate", gtk_main_quit, NULL);

	indicator = app_indicator_new ("indicator-mpd", "indicator-appmenu-menu-panel", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_menu (indicator, GTK_MENU(menu));
	app_indicator_set_title (indicator, "MPD Indicator");

	g_timeout_add_seconds(INTERVAL, update, 0);

	gtk_widget_show_all(menu);
	gtk_main();
	return 0;
}

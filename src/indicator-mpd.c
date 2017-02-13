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
#define SECOND_WORLD_LENGTH 10
#define FIRST_WORLD_LENGTH 20

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
} items;

struct details // some things we would like to get from mpd server
{
	char title[FIRST_WORLD_LENGTH];
	char artist[FIRST_WORLD_LENGTH];
	char volume[SECOND_WORLD_LENGTH];
	char songid[SECOND_WORLD_LENGTH];
} details;
/*END* GLOBAL VARIABLES *END*/


/* FUNCTIONS DECLARATIONS */
void logger(unsigned int count, ...);
char *strcatext(unsigned int count, ...);
gboolean update();
void run_toggle();
void run_next();
void run_previous();
/*END* FUNCTIONS DECLARATIONS *END*/

/* MAIN */
int main (int argc, char *argv[])
{
	/* PLAYGROUND *
	char *kuc = strcatext(2, "jeden ", "dwa");
	printf("%s\n", kuc);


	**************/

	gtk_init (&argc, &argv);

	conn = mpd_connection_new(0, 0, 3000);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
	{
		logger(1, "Connection error");
	}
	else logger(1, "Connection successful");

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
void logger(unsigned int count, ...) //const char * ... huj tam, powinno wejsc wszystko jak sie zmieni va_arg
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

/* EXPERIMENTAL */
char *strcatext(unsigned int count, ...)
{
	char *res;
	va_list vl;
	va_start(vl, count);
	for(unsigned int i = 0; i < count; i++)
	{
		const char *tmp = va_arg(vl, const char *);
		res = realloc(res, sizeof(char) * strlen(tmp));
		strcat(res, tmp);
	}
	va_end(vl);
	return res;
}
/*END* EXPERIMENTAL *END*/

gboolean update()
{
	if((status = mpd_run_status(conn)) != 0)
	{
		sprintf(details.songid, "%d", mpd_status_get_song_pos(status) + 1);
		if(strcmp(details.songid, gtk_menu_item_get_label(GTK_MENU_ITEM(items.songid))) != 0)
		{
			gtk_menu_item_set_label(GTK_MENU_ITEM(items.songid), details.songid);
			logger(2, "Now playing no. ", details.songid);

			if((song = mpd_run_current_song(conn)) != 0)
			{
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.title), mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.artist), mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
			}
			else logger(1, "Song error");
		}
	}
	else logger(1, "Status error");
	return true;
}

void run_toggle()
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
/*END* FUNCTIONS DEFINITIONS *END*/

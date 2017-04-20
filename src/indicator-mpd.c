#include <libappindicator/app-indicator.h>
#include <mpd/client.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INTERVAL 2
#define MAX_WIDTH 30

/* Vars */
struct mpd_connection *conn;
struct mpd_status *status;
struct mpd_song *song;
AppIndicator *indicator;

struct widgets
{
	GtkWidget *menu;
	GtkWidget *playlists;
} widgets;

struct items
{
	GtkWidget *prev;
	GtkWidget *next;
	GtkWidget *quit;
	GtkWidget *title;
	GtkWidget *toggle;
	GtkWidget *artist;
	GtkWidget *songid;
	GtkWidget *state;
	GtkWidget *playlists;
	GtkWidget *clear;
} items;

struct config
{
	FILE *file;
	char address[50];
	char path[50];
	unsigned int port;
	unsigned int timeout;
} config = {.port = 0, .timeout = 3000};

struct details
{
	char title[MAX_WIDTH * 2 + 4];
	char songid[10];
	short state;
} details;

/* Functions' declarations */
char *shrink_to_fit(const char *source, unsigned int len);
static gboolean update();
void config_read();
void run_toggle();
void run_next();
void run_previous();
void run_clear();
void run_play();
void populate_playlists();
void load_playlist(GtkMenuItem *item);

/* Main */
int main(int argc, char *argv[])
{
	sprintf(config.path, "%s/.config/indicator-mpd.conf", getenv("HOME"));

	config_read();

	conn = mpd_connection_new(config.address, config.port, config.timeout);
	//if(mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
	//	return 1;

	gtk_init(&argc, &argv);

	widgets.menu = gtk_menu_new();
	widgets.playlists = gtk_menu_new();

	items.artist = gtk_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.artist);
	gtk_widget_set_sensitive(items.artist, false);

	items.title = gtk_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.title);
	gtk_widget_set_sensitive(items.title, false);

	items.songid = gtk_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.songid);
	gtk_widget_set_sensitive(items.songid, false);

	items.state = gtk_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.state);
	gtk_widget_set_sensitive(items.state, false);

	items.toggle = gtk_menu_item_new_with_label("Play / Pause");
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.toggle);
	g_signal_connect(items.toggle, "activate", G_CALLBACK(run_toggle), NULL);

	items.next = gtk_menu_item_new_with_label("Next");
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.next);
	g_signal_connect(items.next, "activate", G_CALLBACK(run_next), NULL);

	items.prev = gtk_menu_item_new_with_label("Prev");
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.prev);
	g_signal_connect(items.prev, "activate", G_CALLBACK(run_previous), NULL);

	items.clear = gtk_menu_item_new_with_label("Clear");
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.clear);
	g_signal_connect(items.clear, "activate", G_CALLBACK(run_clear), NULL);

	items.playlists = gtk_menu_item_new_with_label("Playlists");
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.playlists);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(items.playlists), widgets.playlists);

	populate_playlists();

	items.quit = gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(widgets.menu), items.quit);
	g_signal_connect(items.quit, "activate", gtk_main_quit, NULL);

	indicator = app_indicator_new("indicator-mpd", "indicator-appmenu-menu-panel", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
	app_indicator_set_menu(indicator, GTK_MENU(widgets.menu));
	app_indicator_set_title(indicator, "MPD Indicator");

	g_timeout_add_seconds(INTERVAL, update, 0);

	gtk_widget_show_all(widgets.menu);
	gtk_main();
	return 0;
}

/* Functions' definitions */
void load_playlist(GtkMenuItem *item)
{
	run_clear();
	mpd_run_load(conn, gtk_menu_item_get_label(item));
	run_play();
}

void populate_playlists()
{
	mpd_send_list_playlists (conn);
	struct mpd_entity *entity;
	const struct mpd_playlist *playlist;
	while((entity = mpd_recv_entity(conn)) != NULL)
	{
		if(mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_PLAYLIST)
		{
			playlist = mpd_entity_get_playlist(entity);
			GtkWidget *tmp = gtk_menu_item_new();
			gtk_menu_shell_append (GTK_MENU_SHELL(widgets.playlists), tmp);
			gtk_menu_item_set_label(GTK_MENU_ITEM(tmp), mpd_playlist_get_path(playlist));
			g_signal_connect(tmp, "activate", G_CALLBACK(load_playlist), tmp);
		}
		mpd_entity_free(entity);
	}
}

void config_read()
{
	config.file = fopen(config.path, "r");
	char line[100];
	if(config.file)
	{
		while(fgets(line, sizeof(line), config.file) != NULL)
		{
			sscanf(line, "address = %s", config.address);
			sscanf(line, "port = %u", &config.port);
			sscanf(line, "timeout = %u", &config.timeout);
		}
		fclose(config.file);
	}
}

/*
 * This function cuts string to desired length
 * and at the end puts 3 dots
 */
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

/*
 * Window making function
 *
 * Currently not used, maybe in future
 * as config dialog
 */
/*void show_properties()
{
	GtkWidget *window;
	GtkWidget *fixed;
	GtkWidget *button_save;
	GtkWidget *label_config_address;
	GtkWidget *entry_config_address;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	fixed = gtk_fixed_new();
	button_save = gtk_button_new_with_label("Save");
	label_config_address = gtk_label_new("MPD server config_address: ");
	entry_config_address = gtk_entry_new();

	gtk_container_add(GTK_CONTAINER(window), fixed);

	gtk_widget_set_size_request(window, 400, 300);
	gtk_widget_set_size_request(button_save, 80, 20);
	gtk_widget_set_size_request(label_config_address, 150, 20);
	gtk_widget_set_size_request(entry_config_address, 380, 20);

	gtk_fixed_put(GTK_FIXED(fixed), button_save, 310, 270);
	gtk_fixed_put(GTK_FIXED(fixed), label_config_address, 10, 20);
	gtk_fixed_put(GTK_FIXED(fixed), entry_config_address, 10, 50);

	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	//gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
	gtk_window_set_title(GTK_WINDOW(window), "Properties");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	gtk_entry_set_text(GTK_ENTRY(entry_config_address), details.config_addr);

	g_signal_connect(button_save, "clicked", G_CALLBACK(config_file_write), (gpointer)gtk_entry_get_text(GTK_ENTRY(entry_config_address)));

	gtk_widget_show_all(window);
}*/

gboolean update()
{
	if((status = mpd_run_status(conn)) != 0)
	{
		switch(mpd_status_get_state(status))
		{
			case MPD_STATE_UNKNOWN: //0
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.state), "Unknown");
				app_indicator_set_label(indicator, "", NULL);
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.songid), "-");
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.title), "-");
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.artist), "-");
			break;
			case MPD_STATE_STOP: //1
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.state), "Not Playing");
				app_indicator_set_label(indicator, "", NULL);
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.songid), "-");
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.title), "-");
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.artist), "-");
			break;
			case MPD_STATE_PLAY: //2
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.state), "Playing");

				sprintf(details.songid, "#%d/%d", mpd_status_get_song_pos(status) + 1, mpd_status_get_queue_length(status));

				if((song = mpd_run_current_song(conn)) != 0)
				{
					gtk_menu_item_set_label(GTK_MENU_ITEM(items.songid), details.songid);
					sprintf(details.title, \
							"%s - %s", \
							shrink_to_fit(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0), MAX_WIDTH), \
							shrink_to_fit(mpd_song_get_tag(song, MPD_TAG_TITLE, 0), MAX_WIDTH));
					gtk_menu_item_set_label(GTK_MENU_ITEM(items.title), shrink_to_fit(mpd_song_get_tag(song, MPD_TAG_TITLE, 0), MAX_WIDTH));
					gtk_menu_item_set_label(GTK_MENU_ITEM(items.artist), shrink_to_fit(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0), MAX_WIDTH));
					app_indicator_set_label(indicator, details.title, NULL);
				}
			break;
			case MPD_STATE_PAUSE: //3
				gtk_menu_item_set_label(GTK_MENU_ITEM(items.state), "Paused");
			break;
		}
	}
	else conn = mpd_connection_new(config.address, config.port, config.timeout);
	return TRUE;
}

/*
 * Simple "run" functions
 *
 * Why not just put mpd_run_* to g_signal_connect?
 *
 * Because g_signal_connect is sending GtkWidget
 * by default as first argument to the called
 * function (i think, tested)
 */
void run_toggle()
{
	mpd_run_toggle_pause(conn);
}

void run_next()
{
	mpd_run_next(conn);
}

void run_previous()
{
	mpd_run_previous(conn);
}

void run_clear()
{
	mpd_run_clear(conn);
}

void run_play()
{
	mpd_run_play(conn);
}

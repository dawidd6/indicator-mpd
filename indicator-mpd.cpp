#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <mpd/client.h>
#include <iostream>
#include <string>


/* Variables */
short INTERVAL = 3;
struct mpd_connection *conn;
struct mpd_status *status;
struct mpd_song *song;
std::string title, artist, volume, tempstr, songid;
const char *tempchar;		
unsigned i;
GtkWidget *menu;
GtkWidget *item_prev, *item_next, *item_quit, *item_title, *item_toggle, *item_artist, *item_volume, *item_songid;
AppIndicator *indicator;

/* Functions */
void connect()
{
	conn = mpd_connection_new("192.168.1.100", 0, 3000);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
	{
		sleep(5);
		app_indicator_set_status (indicator, APP_INDICATOR_STATUS_PASSIVE);
		connect();
	}
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
}
void disconnect()
{
	mpd_connection_free(conn);
}

void run_stop()
{
	connect();
	mpd_run_stop(conn);
	disconnect();
}

void run_toggle()
{
	connect();
	mpd_run_toggle_pause(conn);
	disconnect();
}

void run_next()
{
	connect();
	mpd_run_next(conn);
	disconnect();
}

void run_prev()
{
	connect();
	mpd_run_previous(conn);
	disconnect();
}

void update_labels()
{
        gtk_menu_item_set_label(GTK_MENU_ITEM(item_artist), artist.c_str());
        gtk_menu_item_set_label(GTK_MENU_ITEM(item_title), title.c_str());
	gtk_menu_item_set_label(GTK_MENU_ITEM(item_volume), volume.c_str());
	gtk_menu_item_set_label(GTK_MENU_ITEM(item_songid), songid.c_str());
}

static gboolean get_stuff(gpointer)
{
	connect();
        mpd_command_list_begin(conn, true);                                                      
        mpd_send_status(conn);
        mpd_send_current_song(conn);                                                             
        mpd_command_list_end(conn);
        status = mpd_recv_status(conn);

		volume = std::to_string(mpd_status_get_volume(status)) + "%";
		songid = "#" + std::to_string(mpd_status_get_song_pos(status));

	mpd_status_free(status);
	mpd_response_next(conn);
	song = mpd_recv_song(conn);

		if(tempstr != songid)
        	{
			tempstr = songid;
			i = 0;
			artist = "";
			while ((tempchar = mpd_song_get_tag(song, MPD_TAG_ARTIST, i++)) != NULL) artist = artist + tempchar;
			i = 0;
			title = "";                                                                                                        
			while ((tempchar = mpd_song_get_tag(song, MPD_TAG_TITLE, i++)) != NULL) title = title + tempchar;
		}

	mpd_song_free(song);
	disconnect();

	update_labels();
	return 1;
}

/* Main */
int main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);

	menu = gtk_menu_new();

        item_artist = gtk_menu_item_new();
        gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_artist);
        gtk_widget_set_sensitive (item_artist, false);

        item_title = gtk_menu_item_new();
        gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_title);
	gtk_widget_set_sensitive (item_title, false);

	item_volume = gtk_menu_item_new();
        gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_volume);
        gtk_widget_set_sensitive (item_volume, false);

        item_songid = gtk_menu_item_new();
        gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_songid);
        gtk_widget_set_sensitive (item_songid, false);
 
        item_toggle = gtk_menu_item_new_with_label("Play / Pause");
        gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_toggle);
        g_signal_connect(item_toggle, "activate", G_CALLBACK(run_toggle), NULL);

	item_next = gtk_menu_item_new_with_label("Next");
   	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_next);
	g_signal_connect(item_next, "activate", G_CALLBACK(run_next), NULL);

	item_prev = gtk_menu_item_new_with_label("Prev");
        gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_prev);
	g_signal_connect(item_prev, "activate", G_CALLBACK(run_prev), NULL);

	item_quit = gtk_menu_item_new_with_label("Quit");
        gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_quit);
	g_signal_connect(item_quit, "activate", gtk_main_quit, NULL);
	
	indicator = app_indicator_new ("indicator-mpc", "xn-playlist-symbolic", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
	//app_indicator_set_label (indicator, "", NULL);
	app_indicator_set_menu (indicator, GTK_MENU(menu));
	app_indicator_set_title (indicator, "MPD Indicator");

	g_timeout_add_seconds(INTERVAL, get_stuff, NULL);

	gtk_widget_show_all(menu);
	gtk_main();
	return 0;
}


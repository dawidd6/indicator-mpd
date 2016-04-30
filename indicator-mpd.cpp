#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <iostream>
#include <cstdlib>
using namespace std;

/* Knowledge sources:
 * http://developer.ubuntu.com/resources/technologies/application-indicators/
 * http://developer.ubuntu.com/api/ubuntu-12.04/c/appindicator/libappindicator-app-indicator.html
 * http://zetcode.com/tutorials/gtktutorial/gtkevents/
 */

void next_song()
{
	system("mpc next");
}

void prev_song()
{
	system("mpc prev");
}

int main (int argc, char **argv)
{
	GtkWidget *menu;
	GtkWidget *item_prev, *item_next;
	AppIndicator *indicator;

	gtk_init (&argc, &argv);

	/* Menu */
	menu = gtk_menu_new();

	item_next = gtk_menu_item_new_with_label("Next");
   	gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_next);
	g_signal_connect(item_next, "activate", next_song, NULL);

	item_prev = gtk_menu_item_new_with_label("Prev");
        gtk_menu_shell_append (GTK_MENU_SHELL(menu), item_prev);
	g_signal_connect(item_prev, "activate", prev_song, NULL);
	
	/* Indicator */
	indicator = app_indicator_new ("indicator-mpc", "xn-playlist-symbolic", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
	//app_indicator_set_label (indicator, "kuc", NULL);
	app_indicator_set_menu (indicator, GTK_MENU(menu));
	//app_indicator_set_title (indicator, "SimpleClockTitle");

	gtk_widget_show_all(menu);
	gtk_main();
	return 0;
}


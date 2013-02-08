#include <gtk/gtk.h>
#include <glib-2.0/glib-gobject.h>

/* Create stats information window */
/* name | games | games won | games lost | percentage */
/* per game stats: number of moves, number of blocks left */

GtkWidget *a_Stats_new()
{
   static GtkWidget *dialog = NULL;
   GtkListStore *winners;

   if (!dialog) {
      dialog = gtk_dialog_new_with_buttons("Statistics",
                                           NULL,
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_OK,
                                           GTK_RESPONSE_NONE,
                                           NULL);

      winners = gtk_list_store_new(5, 
                         G_TYPE_STRING, 
                         G_TYPE_INT, 
                         G_TYPE_INT, 
                         G_TYPE_INT, 
                         G_TYPE_STRING);
/*      winners = gtk_clist_new(5);

      g_signal_connect_swapped(GTK_OBJECT(dialog), "response",
                               G_CALLBACK(gtk_widget_destroy), 
                               GTK_OBJECT(dialog));
*/
      gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), winners);
      gtk_widget_show(winners);
   }

   gtk_widget_show(dialog);

   return dialog;
}

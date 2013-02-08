#include <gtk/gtk.h>
#include "MainWindow.h"
#include "Prefs.h"
#include "debug.h"

Prefs *a_Prefs_new()
{
   Prefs *prefs;

   prefs = g_new0(Prefs, 1);
   prefs->grid_w = 16;
   prefs->grid_h = 21;
   prefs->block_w = 20;
   prefs->block_h = 20;
   prefs->max_colors = 5;
   prefs->outline_figures = TRUE;

   return prefs;
}

void a_Prefs_load(Prefs *prefs, const gchar *file)
{
   /* TODO */
}

/* pop-up a settings dialog */
void a_Prefs_configure(Prefs *prefs, MainWindow *mw)
{
   GtkWidget *dialog, *notebook, *page1, *page2, *page3, *page4;
   gint result;

   dialog = gtk_dialog_new();

   gtk_window_set_title(GTK_WINDOW(dialog), "Preferences");
/*
   gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                          GTK_STOCK_OK,
                          GTK_RESPONSE_ACCEPT,
                          GTK_STOCK_CANCEL,
                          GTK_RESPONSE_REJECT);
   gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
*/
   notebook = gtk_notebook_new();

   /* Page 1: Common prefs */

   page1 = gtk_vbox_new(FALSE, 0);
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page1, gtk_label_new("Common"));

   /* Page 2: Basic game prefs */

   page2 = gtk_vbox_new(FALSE, 0);
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page1, gtk_label_new("Basic"));

   /* Page 3: Advanced game prefs */

   page3 = gtk_vbox_new(FALSE, 0);
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page1, gtk_label_new("Advanced"));

   /* Page 4: Networked game prefs */

   page4 = gtk_vbox_new(FALSE, 0);
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page1, gtk_label_new("Network"));

   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), notebook);

   gtk_widget_show_all(dialog);

   result = gtk_dialog_run(GTK_DIALOG(dialog));
   if (result == GTK_RESPONSE_ACCEPT) {
         g_print("applying prefs...\n");
   }
   gtk_widget_destroy(dialog);
}

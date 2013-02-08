/*
 * Main game window - buttons, game screen, statusbar.
 */

#include <gtk/gtk.h>
#include <ctype.h>
#include "MainWindow.h"
#include "Playground.h"
#include "Prefs.h"
#include "debug.h"

/* === static data ========================================================= */

/*static?*/ MainWindow *main_window;

/* === prototypes ========================================================== */

static GtkWidget *toolbar_new(MainWindow *mw);
static void       set_button_accel(GtkButton *button, const char *key_str,
                                   GtkAccelGroup *group);
static void       win(GtkWidget *widget, guint moves, gpointer data);
static void       lose(GtkWidget *widget, guint blocks, gpointer data);
static void
undo_status_change(GtkWidget *widget, gboolean enable, gpointer data);
static void
newgame_cb(GtkWidget *obj, gpointer data);
static void
undo_cb(GtkWidget *obj, gpointer data);
static void
settings_cb(GtkWidget *obj, gpointer data);
static void
stats_cb(GtkWidget *obj, gpointer data);
static void
quit_cb(GtkObject *obj, gpointer data);

/* === public interface ==================================================== */

void a_MainWindow_new()
{
   GtkWidget *box, *handlebox, *toolbar;

   main_window = g_new0(MainWindow, 1);

   /* Default prefs */
   main_window->prefs = a_Prefs_new();

   main_window->game_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

   gtk_window_set_policy(GTK_WINDOW(main_window->game_window),
                         FALSE, FALSE, TRUE);

   gtk_signal_connect(GTK_OBJECT(main_window->game_window), "delete_event",
                      GTK_SIGNAL_FUNC(gtk_object_destroy), main_window);
   gtk_signal_connect(GTK_OBJECT(main_window->game_window), "destroy",
                      GTK_SIGNAL_FUNC(quit_cb), main_window);

   gtk_widget_realize(main_window->game_window);

   main_window->accel_group = gtk_accel_group_new();
   gtk_window_add_accel_group(GTK_WINDOW(main_window->game_window),
                              main_window->accel_group);

   gtk_window_set_title(GTK_WINDOW(main_window->game_window),
                        "Click-o-Square");

   /* create main window contents */
   box = gtk_vbox_new(FALSE, 0);
   handlebox = gtk_handle_box_new();
   toolbar = toolbar_new(main_window);
   gtk_container_add(GTK_CONTAINER(handlebox), toolbar);
   gtk_widget_show(toolbar);
   gtk_box_pack_start(GTK_BOX(box), handlebox, FALSE, FALSE, 0);
   gtk_widget_show(handlebox);
   
   /* Add game window */
   main_window->status = gtk_label_new("Welcome to Click-o-Square");
   main_window->playground = a_Playground_new(
                      main_window->prefs->grid_w, main_window->prefs->grid_h,
                      main_window->prefs->block_w, main_window->prefs->block_h,
                      main_window->prefs->max_colors,
                      GTK_LABEL(main_window->status));
   gtk_box_pack_start(GTK_BOX(box), main_window->playground, FALSE, FALSE, 0);
   gtk_widget_show(main_window->playground);

   /* Playground interaction signals */
   gtk_signal_connect(GTK_OBJECT(main_window->playground), "undo-status",
                      GTK_SIGNAL_FUNC(undo_status_change), main_window);
   gtk_signal_connect(GTK_OBJECT(main_window->playground), "win",
                      GTK_SIGNAL_FUNC(win), main_window);
   gtk_signal_connect(GTK_OBJECT(main_window->playground), "lose",
                      GTK_SIGNAL_FUNC(lose), main_window);
   
   /* Add status bar */
   gtk_box_pack_end(GTK_BOX(box), main_window->status, FALSE, FALSE, 0);
   gtk_widget_show(main_window->status);
   
   gtk_container_add(GTK_CONTAINER(main_window->game_window), box);
   gtk_widget_show(box);
   gtk_widget_show(main_window->game_window);
}

void a_MainWindow_close()
{
   /* TODO: did we release all of that crap? >8-) */
   gtk_widget_destroy(main_window->game_window);
   g_free(main_window);
   gtk_main_quit();
}

/* === signal callbacks ==================================================== */

static void
win(GtkWidget *widget, guint moves, gpointer data)
{
   MainWindow *mw;
   GtkWidget *dialog;

   mw = (MainWindow *)data;

   dialog = gtk_message_dialog_new(GTK_WINDOW(mw->game_window),
                  GTK_DIALOG_DESTROY_WITH_PARENT,
                  GTK_MESSAGE_INFO,
                  GTK_BUTTONS_CLOSE,
                  "You won! You made %d moves to achieve victory.", moves);
   gtk_dialog_run(GTK_DIALOG(dialog));
   gtk_widget_destroy(dialog);   
}

static void
lose(GtkWidget *widget, guint blocks, gpointer data)
{
   MainWindow *mw;
   GtkWidget *dialog;

   mw = (MainWindow *)data;

   dialog = gtk_message_dialog_new(GTK_WINDOW(mw->game_window),
                  GTK_DIALOG_DESTROY_WITH_PARENT,
                  GTK_MESSAGE_INFO,
                  GTK_BUTTONS_CLOSE,
                  "You lost! There are %d blocks left.", blocks);
   gtk_dialog_run(GTK_DIALOG(dialog));
   gtk_widget_destroy(dialog);   
}

static void
undo_status_change(GtkWidget *widget, gboolean enable, gpointer data)
{
   MainWindow *mw;

   DEBUG_ENTER(1, "(%p,%d,%p) {\n", widget,enable,data);

   g_return_if_fail(widget != NULL);
   g_return_if_fail(CS_IS_PLAYGROUND(widget));

   mw = (MainWindow *)data;

   gtk_widget_set_sensitive(mw->undo_move_button, enable);

   DEBUG_LEAVE(1, "}\n");
}

static void
newgame_cb(GtkWidget *obj, gpointer data)
{
   DEBUG_ENTER(1,"\n");
   a_Playground_restart(CS_PLAYGROUND(((MainWindow *)data)->playground));
}

static void
undo_cb(GtkWidget *obj, gpointer data)
{
   CsPlayground *pg;
   DEBUG_ENTER(1,"\n");
   pg = CS_PLAYGROUND(((MainWindow *)data)->playground);
   a_Playground_undo_move(pg);
}

static void
settings_cb(GtkWidget *obj, gpointer data)
{
   MainWindow *mw = (MainWindow *)data;
   DEBUG_ENTER(1,"\n");
   a_Prefs_configure(mw->prefs, mw);
}

static void
stats_cb(GtkWidget *obj, gpointer data)
{
   DEBUG_ENTER(1,"\n");
}

static void
quit_cb(GtkObject *obj, gpointer data)
{
   a_MainWindow_close();
}

/* === private methods ===================================================== */

static GtkWidget *toolbar_new(MainWindow *mw)
{
   GtkWidget *toolbar;
   gboolean labels = mw->prefs->labels;
   gboolean s = mw->prefs->small_icons;

   toolbar = gtk_toolbar_new();
   gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar), GTK_ORIENTATION_HORIZONTAL);
   gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);

   /* New game button */
   mw->new_game_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                            labels ? "New game" : NULL,
                            "Start new game", "Toolbar/New",
                            gtk_image_new_from_stock(GTK_STOCK_NEW,
                                         s ? GTK_ICON_SIZE_SMALL_TOOLBAR
                                           : GTK_ICON_SIZE_LARGE_TOOLBAR),
                            (GtkSignalFunc) newgame_cb, mw);
   gtk_widget_set_sensitive(mw->new_game_button, TRUE);
   set_button_accel(GTK_BUTTON(mw->new_game_button), "nn",
                               mw->accel_group);
   /* Undo move button */   
   mw->undo_move_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                            labels ? "Undo move" : NULL,
                            "Undo last move", "Toolbar/Undo",
                            gtk_image_new_from_stock(GTK_STOCK_UNDO,
                                         s ? GTK_ICON_SIZE_SMALL_TOOLBAR
                                           : GTK_ICON_SIZE_LARGE_TOOLBAR),
                            (GtkSignalFunc) undo_cb, mw);
   gtk_widget_set_sensitive(mw->undo_move_button, FALSE); /* inactive */
   set_button_accel(GTK_BUTTON(mw->undo_move_button), "bb",
                               mw->accel_group);
   /* Settings button */   
   mw->settings_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                            labels ? "Settings" : NULL,
                            "Suit your taste", "Toolbar/Settings",
                            gtk_image_new_from_stock(GTK_STOCK_PREFERENCES,
                                         s ? GTK_ICON_SIZE_SMALL_TOOLBAR
                                           : GTK_ICON_SIZE_LARGE_TOOLBAR),
                            (GtkSignalFunc) settings_cb, mw);
   gtk_widget_set_sensitive(mw->settings_button, TRUE);
   set_button_accel(GTK_BUTTON(mw->settings_button), "ss",
                               mw->accel_group);
   /* Stats button */   
   mw->stats_button = gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),
                            labels ? "Stats" : NULL,
                            "Game statistics", "Toolbar/Stats",
                            gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR,
                                         s ? GTK_ICON_SIZE_SMALL_TOOLBAR
                                           : GTK_ICON_SIZE_LARGE_TOOLBAR),
                            (GtkSignalFunc) stats_cb, mw);
   gtk_widget_set_sensitive(mw->stats_button, TRUE);
   set_button_accel(GTK_BUTTON(mw->stats_button), "tt",
                               mw->accel_group);

   return toolbar;
}

static void set_button_accel(GtkButton *button, const char *key_str,
                             GtkAccelGroup *group)
{
   gint accel_key = tolower(key_str[1]);
   gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", group,
                              accel_key, GDK_MOD1_MASK, GTK_ACCEL_LOCKED);
   gtk_widget_add_accelerator(GTK_WIDGET(button), "clicked", group,
                              accel_key, GDK_MOD2_MASK, GTK_ACCEL_LOCKED);
}

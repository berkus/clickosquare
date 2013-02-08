/*
 * Main game window - buttons, game screen, statusbar.
 */

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <sys/types.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

typedef struct _MainWindow MainWindow;

#include "Prefs.h"

/* main_window contains all widgets to create game main window */
struct _MainWindow
{
   GtkWidget *game_window;      /* root game window */
   GtkWidget *playground;       /* the playgame field (pieces rendering) */
   GtkWidget *status;           /* status bar */

   /* Toolbar buttons */
   GtkWidget *new_game_button;  /* start new game  */
   GtkWidget *undo_move_button; /* undo last move  */
   GtkWidget *settings_button;  /* game settings   */
   GtkWidget *stats_button;     /* show statistics */

   GtkAccelGroup *accel_group;  /* keyboard accelerators for the above */

   Prefs     *prefs;

   /* widgets for dialog boxes off main window */
   GtkWidget *game_settings_window;
   GtkWidget *statistics_window;
};

void  a_MainWindow_new();
void  a_MainWindow_close();

#endif /* __MAINWINDOW_H__ */

/*
 * Game preferences.
 */

#ifndef __PREFS_H__
#define __PREFS_H__

#include <gtk/gtk.h>

typedef struct _Prefs Prefs;

struct _Prefs
{
   /* Common prefs */
   gboolean labels;          /* show toolbar buttons labels */
   gboolean small_icons;     /* toolbar buttons with small icons */
   /* Game prefs */
   guint grid_w, grid_h;     /* game field size */
   guint block_w, block_h; /* game element size */
   gboolean outline_figures; /* outline continuos blocks */
   gint max_colors;          /* max # of square colors in game */
};

Prefs *a_Prefs_new();
void   a_Prefs_load(Prefs *prefs, const gchar *file);
/* pop-up a settings dialog */
void   a_Prefs_configure(Prefs *prefs, MainWindow *mw);

#endif /* __PREFS_H__ */

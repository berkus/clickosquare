/*
 * Click-o-square - a remake of Clickomania windows game.
 *
 * $Id: Click-o-Square.c,v 1.1.1.1 2002/07/14 18:38:34 madfire Exp $
 *
 * Copyright (c) 2002, Stanislav Karchebny <berk@madfire.net>
 * Distributed under BSD License.
 */

#include <gtk/gtk.h>
#include "MainWindow.h"

gint main(int argc, char **argv)
{
   /* Initialize i18n support */
   g_print("Setting locale to %s\n", gtk_set_locale());

   /* Initialize the widget set */
   gtk_init(&argc,&argv);

   /* This is needed for blocks drawing code */
   gdk_rgb_init();

   /* Create the main window */
   a_MainWindow_new();

   /* Run main event loop */
   gtk_main();

   /* Leave */
   gtk_exit(0);
   return 0;
}

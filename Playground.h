/*
 * Playground widget - a game field and business logic.
 *
 * GtkObject
 * +--GtkWidget
 *    +--CsPlayground
 *
 */

#ifndef __PLAYGROUND_H__
#define __PLAYGROUND_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtk/gtk.h>
#include "MainWindow.h"

#define CS_TYPE_PLAYGROUND           (a_Cs_Playground_get_type())
#define CS_PLAYGROUND(obj)           GTK_CHECK_CAST(obj, \
                                        CS_TYPE_PLAYGROUND, CsPlayground)
#define CS_PLAYGROUND_CLASS(klass)   GTK_CHECK_CLASS_CAST(klass, \
                                        CS_TYPE_PLAYGROUND, CsPlaygroundClass)
#define CS_IS_PLAYGROUND(obj)        GTK_CHECK_TYPE(obj, CS_TYPE_PLAYGROUND)

struct _CsPlayground;
struct _CsPlaygroundClass;

typedef struct _CsPlayground      CsPlayground;
typedef struct _CsPlaygroundClass CsPlaygroundClass;

/* get widget type */
GtkType  a_Cs_Playground_get_type(void);

/* create new instance */
GtkWidget *a_Playground_new(guint grid_w,
                            guint grid_h,
                            guint block_w,
                            guint block_h,
                            guint num_colors,
                            GtkLabel *status);

void a_Playground_restart(CsPlayground *pg); /* start new game */
void a_Playground_undo_move(CsPlayground *pg); /* undo last move */

void a_Playground_set_grid_size(CsPlayground *pg,
                                guint grid_w, guint grid_h);
void a_Playground_set_block_size(CsPlayground *pg,
                                 guint block_w, guint block_h);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PLAYGROUND_H__ */

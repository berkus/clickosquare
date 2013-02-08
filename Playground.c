/*
 * Playground widget - a game field and business logic.
 */

#include <gtk/gtk.h>
#include "Playground.h"
#include "debug.h"

/* === widget class ======================================================== */

typedef struct _GridBlock GridBlock;
typedef struct _GridBlockAnim GridBlockAnim;

struct _GridBlockAnim
{
   /* animated blocks support */
   int running     :1; /* block is running to its new home */
   int x_offset,
       y_offset;       /* block offset from its current position */
};

struct _GridBlock
{
   int color;          /* block "color" - one of 1..maxcolors */
   int top_line    :1; /* draw a divider line on top */
   int bottom_line :1; /* -- "" -- "" -- on bottom */
   int left_line   :1; /* -- "" -- "" -- on left side */
   int right_line  :1; /* -- "" -- "" -- on right side */
   GridBlockAnim   animation;
};

struct _CsPlayground
{
   GtkWidget   object;      /* parent object */

   GtkLabel   *status;
   GridBlock **grid;        /* blocks */
   GridBlock **undo;        /* prev state */
   gint        undo_nblocks;
   gboolean    undo_present;
   guint       grid_w, grid_h;
   guint       block_w, block_h;
   /* number of blocks in-deck, ranges from grid_w*grid_h..zero */
   guint       ncolors;
   gint        nblocks;
   gint        nmoves;  /* number of moves made */
   volatile gboolean    game_on; /* game is in progress */
};

struct _CsPlaygroundClass
{
   GtkWidgetClass parent_class;

/*   void (*grid_resized)();
   void (*block_resized)();
   void (*block_destroyed)();*/
   void (*undo_status)(CsPlayground *pg, gboolean enabled);
   void (*win)(CsPlayground *pg, gint moves_taken);
   void (*lose)(CsPlayground *pg, gint blocks_left);
};

/* === signal data ========================================================= */

enum {
/*   GRID_RESIZED, */    /* grid size has changed, causes game restart */
/*   BLOCK_RESIZED, */   /* block size has changed, causes window redraw */
/*   BLOCK_DESTROYED, */ /* useful for networked play */
   UNDO_STATUS,      /* emitted to indicate status of undo buffer */
   WIN,              /* all blocks were destroyed */
   LOSE,             /* there are no more blocks that can be destroyed */
   LAST_SIGNAL
};

static gint playground_signals[LAST_SIGNAL] = { 0 };

/* === prototypes ========================================================== */

static void CsPlayground_init(CsPlayground *pg);
static void CsPlayground_class_init(CsPlaygroundClass *klass);
static void CsPlayground_destroy(GtkObject *object);
static void CsPlayground_realize(GtkWidget *widget);
static void CsPlayground_size_request(GtkWidget *widget,
                                      GtkRequisition *reqisition);
static void CsPlayground_size_allocate(GtkWidget *widget,
                                       GtkAllocation *allocation);
static gboolean CsPlayground_expose_event(GtkWidget *widget,
                                          GdkEventExpose *event);
static gboolean CsPlayground_button_press(GtkWidget *widget,
                                          GdkEventButton *event);
static void calculate_outline(CsPlayground *pg);
static void allocate_grid(CsPlayground *pg, guint grid_w, guint grid_h);
static void destroy_grid(CsPlayground *pg);
static void copy_grid(GridBlock **from, GridBlock **to, guint w, guint h);

/* === public interface ==================================================== */

GtkWidget *a_Playground_new(guint grid_w,
                            guint grid_h,
                            guint block_w,
                            guint block_h,
                            guint ncolors,
                            GtkLabel *status)
{
   CsPlayground *pg;

   DEBUG_ENTER(1, "(%d,%d,%d,%d,%d.%p) {\n",
               grid_w, grid_h, block_w, block_h, ncolors, status);

   pg = CS_PLAYGROUND(gtk_type_new(CS_TYPE_PLAYGROUND));
   pg->status = status;
   pg->block_w = block_w;
   pg->block_h = block_h;
   pg->ncolors = ncolors;

   a_Playground_set_grid_size(pg, grid_w, grid_h);

   DEBUG_LEAVE(1, "} > %p\n", GTK_WIDGET(pg));

   return GTK_WIDGET(pg);
}

void a_Playground_set_grid_size(CsPlayground *pg,
                                guint grid_w, guint grid_h)
{
   if (pg->grid_w != grid_w || pg->grid_h != grid_h)
   {
      destroy_grid(pg);
      allocate_grid(pg, grid_w, grid_h);
      a_Playground_restart(pg);
   }
}

void a_Playground_restart(CsPlayground *pg)
{
   int row, col;

   pg->nblocks = pg->grid_w * pg->grid_h;
   pg->nmoves  = 0;

   /* initialize grid */
   for (row = 0; row < pg->grid_h; row++)
      for (col = 0; col < pg->grid_w; col++)
         pg->grid[row][col].color = g_random_int_range(1, pg->ncolors+1);

   pg->game_on = TRUE;
   pg->undo_present = FALSE;

   gtk_widget_queue_draw(GTK_WIDGET(pg));
}

void a_Playground_undo_move(CsPlayground *pg)
{
   if (pg->undo_present)
   {
      copy_grid(pg->undo, pg->grid, pg->grid_w, pg->grid_h);
      pg->nblocks = pg->undo_nblocks;
      pg->undo_present = FALSE;

      gtk_signal_emit(GTK_OBJECT(pg), playground_signals[UNDO_STATUS],
                      pg->undo_present);

      gtk_widget_queue_draw(GTK_WIDGET(pg));

      pg->nmoves--;

      if (!pg->game_on) pg->game_on = TRUE; /* re-enable game */
   }
}

/* === type construction =================================================== */

static GtkWidgetClass *parent_class;

GtkType a_Cs_Playground_get_type()
{
   static GtkType type = 0;

   DEBUG_ENTER(1, "() {\n");

   if (!type) {
      GtkTypeInfo info = {
         "CsPlayground",
         sizeof(CsPlayground),
         sizeof(CsPlaygroundClass),
         (GtkClassInitFunc) CsPlayground_class_init,
         (GtkObjectInitFunc) CsPlayground_init,
         NULL,
         NULL,
         (GtkClassInitFunc) NULL
      };

      /* create new type based on GtkWidget */
      type = gtk_type_unique(GTK_TYPE_WIDGET, &info);
   }

   DEBUG_LEAVE(1, "} > %08x\n", type);

   return type;
}

static void
CsPlayground_init(CsPlayground *pg)
{
   DEBUG_ENTER(1, "(%p) {\n", pg);
   pg->grid = NULL;
   pg->grid_w = 0;
   pg->grid_h = 0;
   pg->block_w = 0;
   pg->block_h = 0;
   pg->nblocks = 0;
   pg->ncolors = 0;
   /* do not grab focus */
   GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(pg), GTK_CAN_FOCUS);
   DEBUG_LEAVE(1, "}\n");
}

static void
CsPlayground_class_init(CsPlaygroundClass *klass)
{
   GtkObjectClass *object_class;
   GtkWidgetClass *widget_class;
   DEBUG_ENTER(1, "(%p) {\n", klass);

   parent_class = gtk_type_class(GTK_TYPE_WIDGET);

   object_class = GTK_OBJECT_CLASS(klass);
   object_class->destroy = CsPlayground_destroy;

   widget_class = (GtkWidgetClass *)klass;
   widget_class->realize             = CsPlayground_realize;
   widget_class->expose_event        = CsPlayground_expose_event;
   widget_class->size_request        = CsPlayground_size_request;
   widget_class->size_allocate       = CsPlayground_size_allocate;
   widget_class->button_press_event  = CsPlayground_button_press;

   playground_signals[UNDO_STATUS] = gtk_signal_new("undo-status",
               GTK_RUN_FIRST,
               GTK_CLASS_TYPE(object_class),
               GTK_SIGNAL_OFFSET(CsPlaygroundClass, undo_status),
               gtk_marshal_VOID__BOOLEAN,
               GTK_TYPE_NONE,
               1, GTK_TYPE_BOOL);
   playground_signals[WIN] = gtk_signal_new("win",
               GTK_RUN_FIRST,
               GTK_CLASS_TYPE(object_class),
               GTK_SIGNAL_OFFSET(CsPlaygroundClass, win),
               gtk_marshal_VOID__INT,
               GTK_TYPE_NONE,
               1, GTK_TYPE_INT);
   playground_signals[LOSE] = gtk_signal_new("lose",
               GTK_RUN_FIRST,
               GTK_CLASS_TYPE(object_class),
               GTK_SIGNAL_OFFSET(CsPlaygroundClass, lose),
               gtk_marshal_VOID__INT,
               GTK_TYPE_NONE,
               1, GTK_TYPE_INT);

   klass->undo_status = NULL; /* no default action */

   DEBUG_LEAVE(1, "}\n");
}

/* === private methods ===================================================== */

static void
make_move(CsPlayground *pg)
{
   copy_grid(pg->grid, pg->undo, pg->grid_w, pg->grid_h);
   pg->undo_nblocks = pg->nblocks;
   pg->undo_present = TRUE;

   pg->nmoves++;

   gtk_signal_emit(GTK_OBJECT(pg), playground_signals[UNDO_STATUS],
                   pg->undo_present);
}

static void
copy_grid(GridBlock **from, GridBlock **to, guint w, guint h)
{
   int row, col;

   for (row = 0; row < h; row++)
      for (col = 0; col < w; col++)
         to[row][col] = from[row][col];
}

static void
allocate_grid(CsPlayground *pg, guint grid_w, guint grid_h)
{
   int row;

   pg->grid_w = grid_w;
   pg->grid_h = grid_h;

   /* play grid */
   pg->grid = g_new0(GridBlock *, grid_h);
   for (row = 0; row < grid_h; row++)
      pg->grid[row] = g_new0(GridBlock, grid_w);
   /* undo buffer */
   pg->undo = g_new0(GridBlock *, grid_h);
   for (row = 0; row < grid_h; row++)
      pg->undo[row] = g_new0(GridBlock, grid_w);
}

static void
destroy_grid(CsPlayground *pg)
{
   int row;

   if (pg->grid)
   {
      for (row = 0; row < pg->grid_h; row++)
         g_free(pg->grid[row]);

      g_free(pg->grid);
      pg->grid = NULL;
   }

   if (pg->undo)
   {
      for (row = 0; row < pg->grid_h; row++)
         g_free(pg->undo[row]);

      g_free(pg->undo);
      pg->undo = NULL;
   }
}

static void
CsPlayground_destroy(GtkObject *object)
{
   CsPlayground *pg;

   DEBUG_ENTER(1, "(%p) {\n", object);

   g_return_if_fail(object != NULL);
   g_return_if_fail(CS_IS_PLAYGROUND(object));

   pg = CS_PLAYGROUND(object);

   destroy_grid(pg);

   if (GTK_OBJECT_CLASS(parent_class)->destroy)
      (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);

   DEBUG_LEAVE(1, "}\n");
}

static void
CsPlayground_realize(GtkWidget *widget)
{
   CsPlayground *pg;
   GdkWindowAttr attr;
   gint attr_mask;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(CS_IS_PLAYGROUND(widget));

   GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
   pg = CS_PLAYGROUND(widget);

   attr.x = widget->allocation.x;
   attr.y = widget->allocation.y;
   attr.width = widget->allocation.width;
   attr.height = widget->allocation.height;
   attr.wclass = GDK_INPUT_OUTPUT;
   attr.window_type = GDK_WINDOW_CHILD;
   attr.event_mask = gtk_widget_get_events(widget) |
      GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK;
   attr.visual = gtk_widget_get_visual(widget);
   attr.colormap = gtk_widget_get_colormap(widget);

   attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
   widget->window = gdk_window_new(widget->parent->window, &attr, attr_mask);
   widget->style  = gtk_style_attach(widget->style, widget->window);
   gdk_window_set_user_data(widget->window, widget);
   gtk_style_set_background(widget->style, widget->window, GTK_STATE_ACTIVE);
}

static gboolean
CsPlayground_expose_event(GtkWidget *widget,
                          GdkEventExpose *event)
{
   CsPlayground *pg;
   GdkGC *gc;
   GdkGCValues gcv;
   GdkGCValuesMask gcv_mask;
   gchar *status;
   int row, col, x, y;
   static int colors[] = {
      0x000000,
      0xff0000,
      0x00ff00,
      0x0000ff,
      0xffff00,
      0xff00ff,
      0x00ffff,
      0xffffff
   };
   static int outline_color = 0x000000;

   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(CS_IS_PLAYGROUND(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);

   if (event->count > 0)
      return FALSE;

   DEBUG_MSG(1, "expose_event()\n");

   pg = CS_PLAYGROUND(widget);

   calculate_outline(pg);

   gdk_window_clear_area(widget->window,
                         0, 0,
                         widget->allocation.width,
                         widget->allocation.height);

   gdk_gc_get_values(widget->style->bg_gc[widget->state], &gcv);
   gcv.fill = GDK_SOLID;
   gcv.function = GDK_COPY;
   gcv_mask = GDK_GC_FOREGROUND | GDK_GC_FILL | GDK_GC_FUNCTION;
   gc = gdk_gc_new_with_values(widget->window, &gcv, gcv_mask);

   for (row = 0; row < pg->grid_h; row++)
      for (col = 0; col < pg->grid_w; col++) {

         x = col * pg->block_w;
         y = row * pg->block_h;

         /* create color from block "color" */
         gdk_rgb_gc_set_foreground(gc, 
                            colors[pg->grid[row][col].color 
                                   % (sizeof(colors)/sizeof(colors[0]))]);

         gdk_draw_rectangle(widget->window,
                            gc,
                            TRUE,
                            x, y,
                            pg->block_w, pg->block_h);

         /* Draw block outline */
         gdk_rgb_gc_set_foreground(gc, outline_color);

         if (pg->grid[row][col].top_line)
            gdk_draw_line(widget->window, gc, x,y,x+pg->block_w,y);
         if (pg->grid[row][col].bottom_line)
            gdk_draw_line(widget->window, gc, x,y+pg->block_h,x+pg->block_w,y+pg->block_h);
         if (pg->grid[row][col].left_line)
            gdk_draw_line(widget->window, gc, x,y,x,y+pg->block_h);
         if (pg->grid[row][col].right_line)
            gdk_draw_line(widget->window, gc, x+pg->block_w,y,x+pg->block_w,y+pg->block_h);
      }

   gdk_gc_unref(gc);

   /* update status */
   status = g_strdup_printf("%d blocks left | move %d", pg->nblocks, pg->nmoves);
   gtk_label_set_text(pg->status, status);
   g_free(status);

   return FALSE;
}

static void
CsPlayground_size_request(GtkWidget *widget,
                          GtkRequisition *requisition)
{
   CsPlayground *pg;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(CS_IS_PLAYGROUND(widget));
   g_return_if_fail(requisition != NULL);

   pg = CS_PLAYGROUND(widget);
   requisition->width  = pg->grid_w * pg->block_w;
   requisition->height = pg->grid_h * pg->block_h;
}

static void
CsPlayground_size_allocate(GtkWidget *widget,
                           GtkAllocation *allocation)
{
   CsPlayground *pg;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(CS_IS_PLAYGROUND(widget));
   g_return_if_fail(allocation != NULL);

   widget->allocation = *allocation;
   if (GTK_WIDGET_REALIZED(widget))
   {
      pg = CS_PLAYGROUND(widget);
      gdk_window_move_resize(widget->window,
                             allocation->x, allocation->y,
                             allocation->width, allocation->height);
   }
}

/* compress any found holes as we go */
static void
condense_row(CsPlayground *pg, int col)
{
   int row, torow;

   DEBUG_ENTER(1, "(%p,%d) {\n", pg,col);

   row = torow = pg->grid_h-1;

   while (row >= 0)
   {
      DEBUG_MSG(1, "first loop: %d\n", torow);
      while (torow >= 0 && pg->grid[torow][col].color)
         torow--;
      if (torow < 0) break;

      row = torow - 1;

      DEBUG_MSG(1,"condense row %d torow %d\n", row, torow);

      while (row >= 0 && !pg->grid[row][col].color)
         row--;
      if (row < 0) break;

      pg->grid[torow][col].color = pg->grid[row][col].color;
      pg->grid[row][col].color = 0;
   }

   DEBUG_LEAVE(1, "}\n");
}

static void
condense_field(CsPlayground *pg)
{
   int row, col, col2, w;

   DEBUG_ENTER(1, "(%p) {\n", pg);

   /* first, kill blocks down each column */
   for (col = 0; col < pg->grid_w; col++)
   {
      DEBUG_MSG(1, "in column %d\n", col);

      condense_row(pg, col);
   }

/* try to find completely empty columns, and if so remove them,
   condensing everything to the left */
   w = pg->grid_w;

   for (col = 0; col < w; )
   {
      for (row = 0; row < pg->grid_h; row++)
      {
         if (pg->grid[row][col].color) break;
      }

      if (row == pg->grid_h && col < w-1) /* condense */
      {
         DEBUG_MSG(1, "condensing column %d\n", col);

         for (col2 = col+1; col2 < w; col2++)
            for (row = 0; row < pg->grid_h; row++)
               pg->grid[row][col2-1] = pg->grid[row][col2];
         /* clear out last column */
         for (row = 0; row < pg->grid_h; row++)
            pg->grid[row][w-1].color = 0;
         w--;
      }
      else col++; /* if we condensed we need to re-check column */
   }

   DEBUG_LEAVE(1, "}\n");
}

static void
calculate_outline(CsPlayground *pg)
{
   int row, col;

   for (row = 0; row < pg->grid_h; row++)
      for (col = 0; col < pg->grid_w; col++)
      {
         pg->grid[row][col].top_line =
         pg->grid[row][col].bottom_line =
         pg->grid[row][col].left_line =
         pg->grid[row][col].right_line = 0;

         if (row == 0 
          || pg->grid[row][col].color != pg->grid[row-1][col].color)
            pg->grid[row][col].top_line = 1;

         if (row == pg->grid_h-1
          || pg->grid[row][col].color != pg->grid[row+1][col].color)
            pg->grid[row][col].bottom_line = 1;

         if (col == 0 
          || pg->grid[row][col].color != pg->grid[row][col-1].color)
            pg->grid[row][col].left_line = 1;

         if (col == pg->grid_w-1
          || pg->grid[row][col].color != pg->grid[row][col+1].color)
            pg->grid[row][col].right_line = 1;
      }
}

static guint
neighborhood(CsPlayground *pg, int row, int col)
{
   guint nbs = 0;

   if (!pg->grid[row][col].color) /* deleted blocks do not have neighbours */
      return nbs;

   if (col > 0 && pg->grid[row][col].color == pg->grid[row][col-1].color)
      nbs++;
   if (col < pg->grid_w-1 && pg->grid[row][col].color == pg->grid[row][col+1].color)
      nbs++;
   if (row > 0 && pg->grid[row][col].color == pg->grid[row-1][col].color)
      nbs++;
   if (row < pg->grid_h-1 && pg->grid[row][col].color == pg->grid[row+1][col].color)
      nbs++;

   DEBUG_MSG(1, "block %d,%d has %d neighbors\n", row,col,nbs);

   return nbs;
}

/* kill blocks around recursively */
static void
kill_around(CsPlayground *pg, int row, int col, int color)
{
   DEBUG_ENTER(1, "(%p,%d,%d,%d) {\n", pg, row,col, color);

   if (color == pg->grid[row][col].color)
   {
      DEBUG_MSG(1, "killing block %d,%d\n", row,col);
      pg->grid[row][col].color = 0; /* kill this block */
      pg->nblocks--;
      /* TODO: emit "block-destroyed" */
   }
   else 
   {
      DEBUG_LEAVE(1, "} [nokill]\n");
      return; /* if we didn't kill this - stop seeking */
   }

   /* we do it like floodfill does */
   if (col > 0 && color == pg->grid[row][col-1].color)
      kill_around(pg,row,col-1,color);
   if (col < pg->grid_w-1 && color == pg->grid[row][col+1].color)
      kill_around(pg,row,col+1,color);
   if (row > 0 && color == pg->grid[row-1][col].color)
      kill_around(pg,row-1,col,color);
   if (row < pg->grid_h-1 && color == pg->grid[row+1][col].color)
      kill_around(pg,row+1,col,color);

   DEBUG_LEAVE(1, "}\n");
}

static void
try_killing_around(CsPlayground *pg, int row, int col)
{
   DEBUG_ENTER(1, "(%p,%d,%d) {\n", pg, row, col);

   if (neighborhood(pg,row,col))
   {
      make_move(pg); /* remember undo data */
      kill_around(pg, row, col, pg->grid[row][col].color);
      condense_field(pg);
      gtk_widget_queue_draw(GTK_WIDGET(pg)); /* refresh */
   }

   DEBUG_LEAVE(1, "}\n");
}

static gboolean
check_winning_position(CsPlayground *pg)
{
   if (pg->nblocks == 0) {
      gtk_signal_emit(GTK_OBJECT(pg), playground_signals[WIN], pg->nmoves);
      pg->game_on = FALSE;
      return TRUE;
   }
   return FALSE;
}

static gboolean
check_losing_position(CsPlayground *pg)
{
   int row, col;
   gboolean more_moves = FALSE;

   for (row = 0; row < pg->grid_h; row++)
      for (col = 0; col < pg->grid_w; col++)
         if (neighborhood(pg,row,col))
            more_moves = TRUE;

   if (!more_moves) {
      gtk_signal_emit(GTK_OBJECT(pg), playground_signals[LOSE], pg->nblocks);
      pg->game_on = FALSE;
   }

   return !more_moves;
}

static gboolean
CsPlayground_button_press(GtkWidget *widget,
                          GdkEventButton *event)
{
   CsPlayground *pg;
   int row, col;

   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(CS_IS_PLAYGROUND(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);

   pg = CS_PLAYGROUND(widget);
   row = event->y / pg->block_h;
   col = event->x / pg->block_w;

   if (pg->game_on) {
      DEBUG_MSG(1,"clicked block %d,%d\n", row, col);
      try_killing_around(pg, row, col);
      if (!check_winning_position(pg))
         check_losing_position(pg);
   }

   return FALSE;
}

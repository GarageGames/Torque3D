/* Toy widget for GTK+ for displaying MNG animations.
 *
 * Copyright (C) 2000 The Free Software Foundation
 *
 * Author(s): Volodymyr Babin <vb :at: dwuj.ichf.edu.pl>
 *
 * This code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtkmain.h>
#include "gtk-mng-view.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

/* MNG callbacks */

static mng_ptr
mng_malloc_callback (mng_size_t how_many)
{
  return (mng_ptr) g_new0 (gchar, how_many);
}

static void
mng_free_callback (mng_ptr pointer, mng_size_t number)
{
  g_free (pointer);
}

static mng_bool
mng_open_stream_callback (mng_handle mng_h)
{
  return MNG_TRUE;
}

static mng_bool
mng_close_stream_callback (mng_handle mng_h)
{
  return MNG_TRUE;
}

static mng_bool
mng_read_data_callback (mng_handle mng_h,
			mng_ptr buffer,
			mng_uint32 bytes_requested,
        		mng_uint32 * bytes_read)
{
  guint available_mng_food;

  GtkMngView * mng_view = GTK_MNG_VIEW (mng_get_userdata (mng_h));

  available_mng_food = mng_view->bytes_to_eat - mng_view->bytes_eaten;
  if (available_mng_food > 0 && mng_view->mng_food != NULL)
    {
      * bytes_read = (mng_uint32) MIN ((mng_uint32) available_mng_food, bytes_requested);
      memcpy (buffer, mng_view->mng_food + mng_view->bytes_eaten, * bytes_read);
      mng_view->bytes_eaten += * bytes_read;
      return MNG_TRUE;
    }
  else
    return MNG_FALSE;
}

static mng_bool
mng_process_header_callback (mng_handle mng_h,
			     mng_uint32 width,
			     mng_uint32 height)
{
  GtkMngView * mng_view;

  mng_view = GTK_MNG_VIEW (mng_get_userdata (mng_h));

  mng_view->width = width;
  mng_view->height = height;

  g_free (mng_view->MNG_drawing_buffer);
  mng_view->MNG_drawing_buffer = g_new0 (guchar, 3 * width * height);

  gtk_widget_queue_resize (GTK_WIDGET (mng_view));
  return MNG_TRUE;
}

static gboolean
gtk_mng_view_animator (GtkMngView * mng_view)
{
  mng_retcode retcode;

  retcode = mng_display_resume (mng_view->MNG_handle);

  if (retcode == MNG_NOERROR)
    {
      mng_view->timeout_ID = 0;
      return FALSE;
    }
  else if (retcode == MNG_NEEDTIMERWAIT)
    return FALSE;
  else
    g_warning ("mng_display_resume() return not good value");

  return FALSE;
}

static mng_bool
mng_set_timer_callback (mng_handle mng_h,
			mng_uint32 delay)
{
  GtkMngView * mng_view;

  mng_view = GTK_MNG_VIEW (mng_get_userdata (mng_h));
  mng_view->timeout_ID = gtk_timeout_add (delay,
					  (GtkFunction) gtk_mng_view_animator,
					  mng_view);
  return MNG_TRUE;
}

static mng_uint32
mng_get_tickcount_callback (mng_handle mng_h)
{
  gdouble seconds;
  gulong microseconds;

  GtkMngView * mng_view;

  mng_view = GTK_MNG_VIEW (mng_get_userdata (mng_h));
  seconds = g_timer_elapsed (mng_view->timer,
			     &microseconds);

  return ((mng_uint32) (seconds*1000.0 + ((gdouble) microseconds)/1000.0));
}

static mng_ptr
mng_get_canvas_line_callback (mng_handle mng_h,
			      mng_uint32 line)
{
  GtkMngView * mng_view;

  mng_view = GTK_MNG_VIEW (mng_get_userdata (mng_h));
  return mng_view->MNG_drawing_buffer + 3 * line * mng_view->width;
}

static void gtk_mng_view_paint (GtkMngView *, GdkRectangle *);

static mng_bool
mng_refresh_callback (mng_handle mng_h,
		      mng_uint32 x,
		      mng_uint32 y,
		      mng_uint32 width,
		      mng_uint32 height)
{
  GtkMngView * mng_view;

  mng_view = GTK_MNG_VIEW (mng_get_userdata (mng_h));

  if (GTK_WIDGET_REALIZED (mng_view))
      {
        GdkRectangle rectangle;

        rectangle.x = x;
	rectangle.y = y;
	rectangle.width = width;
	rectangle.height = height;
        gtk_mng_view_paint (mng_view, &rectangle);
      }
  return MNG_TRUE;
}

static gboolean
gtk_mng_view_init_libmng (GtkMngView * mng_view)
{
  GtkWidget * widget;

  g_return_val_if_fail (IS_GTK_MNG_VIEW (mng_view), FALSE);

  if (mng_view->MNG_handle)
    mng_cleanup (&mng_view->MNG_handle);

  mng_view->MNG_handle = mng_initialize (mng_view,
					 mng_malloc_callback,
					 mng_free_callback,
					 MNG_NULL);

  if (mng_view->MNG_handle == MNG_NULL)
    return FALSE;

  if (mng_setcb_openstream (mng_view->MNG_handle, mng_open_stream_callback) != MNG_NOERROR ||
      mng_setcb_closestream (mng_view->MNG_handle, mng_close_stream_callback) != MNG_NOERROR ||
      mng_setcb_readdata (mng_view->MNG_handle, mng_read_data_callback) != MNG_NOERROR ||
      mng_setcb_processheader (mng_view->MNG_handle, mng_process_header_callback) != MNG_NOERROR ||
      mng_setcb_settimer (mng_view->MNG_handle, mng_set_timer_callback) != MNG_NOERROR ||
      mng_setcb_gettickcount (mng_view->MNG_handle, mng_get_tickcount_callback) != MNG_NOERROR ||
      mng_setcb_getcanvasline (mng_view->MNG_handle, mng_get_canvas_line_callback) != MNG_NOERROR ||
      mng_setcb_refresh (mng_view->MNG_handle, mng_refresh_callback) != MNG_NOERROR)
    {
      mng_cleanup (&mng_view->MNG_handle);
      return FALSE;
    }

  mng_set_canvasstyle (mng_view->MNG_handle, MNG_CANVAS_RGB8);

  widget = GTK_WIDGET (mng_view);

  if (!GTK_WIDGET_REALIZED (widget))
    gtk_widget_realize (widget);

  mng_set_bgcolor (mng_view->MNG_handle,
		   widget->style->bg[GTK_STATE_NORMAL].red,
		   widget->style->bg[GTK_STATE_NORMAL].green,
		   widget->style->bg[GTK_STATE_NORMAL].blue);
  return TRUE;
}

/* GTK+ widget methods */

static GtkWidgetClass * parent_class = NULL;

static void
gtk_mng_view_finalize (GObject * obj)
{
  GtkMngView * mng_view = GTK_MNG_VIEW (obj);

  g_timer_destroy (mng_view->timer);

  if (mng_view->timeout_ID)
    gtk_timeout_remove (mng_view->timeout_ID);

  g_free (mng_view->MNG_drawing_buffer);

  if (mng_view->MNG_handle)
    mng_cleanup (&mng_view->MNG_handle);

  G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
gtk_mng_view_size_request (GtkWidget * widget, GtkRequisition * requisition)
{
  GtkMngView * mng_view;

  g_return_if_fail (IS_GTK_MNG_VIEW (widget));
  g_return_if_fail (requisition != NULL);

  mng_view = (GtkMngView *) widget;

  requisition->width = mng_view->width;
  requisition->height = mng_view->height;
}

static void
gtk_mng_view_size_allocate (GtkWidget * widget, GtkAllocation * allocation)
{
  g_return_if_fail (IS_GTK_MNG_VIEW (widget));
  g_return_if_fail (allocation != NULL);

  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
			    allocation->x,
			    allocation->y,
			    allocation->width,
			    allocation->height);
}

static void
gtk_mng_view_paint (GtkMngView * mng_view,
		    GdkRectangle * area)
{
  GtkWidget * widget;
  guint rowstride;
  guchar * buffer;
  register guchar * ptr;
  register guchar * bptr;

  widget = GTK_WIDGET (mng_view);

  g_assert (GTK_WIDGET_REALIZED (widget));

  rowstride = 3 * area->width;
  buffer = g_new (guchar, rowstride * area->height);


  bptr = buffer;
  ptr = mng_view->MNG_drawing_buffer
	+ 3 * (area->y * mng_view->width + area->x);

  while (bptr < buffer + rowstride * area->height)
    {
      memcpy (bptr, ptr, rowstride);
      bptr += rowstride;
      ptr += 3 * mng_view->width;
    }

  gdk_draw_rgb_image (widget->window,
		      widget->style->white_gc,
		      area->x,
		      area->y,
		      area->width,
		      area->height,
		      GDK_RGB_DITHER_NORMAL,
		      buffer,
		      rowstride);

  g_free (buffer);
  gdk_flush ();
}

static gboolean
gtk_mng_view_expose (GtkWidget * widget, GdkEventExpose * event)
{
  g_return_val_if_fail (IS_GTK_MNG_VIEW (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (GTK_WIDGET_REALIZED (widget))
    {
      GdkRectangle dummy;
      GdkRectangle rectangle;
      GtkMngView * mng_view;

      mng_view = GTK_MNG_VIEW (widget);
      dummy.x = dummy.y = 0;
      dummy.width = mng_view->width;
      dummy.height = mng_view->height;

      if (gdk_rectangle_intersect (&dummy, &event->area, &rectangle))
        gtk_mng_view_paint (mng_view, &rectangle);
    }
  return FALSE;
}

static void
gtk_mng_view_realize (GtkWidget * widget)
{
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (IS_GTK_MNG_VIEW (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= GDK_EXPOSURE_MASK;
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
			    	   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);
  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void
gtk_mng_view_init (GtkMngView * mng_view)
{
  g_return_if_fail (IS_GTK_MNG_VIEW (mng_view));

  GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (mng_view), GTK_NO_WINDOW);

  mng_view->MNG_handle = NULL;
  mng_view->MNG_drawing_buffer = NULL;
  mng_view->timeout_ID = 0;
  mng_view->timer = g_timer_new ();
  g_timer_start (mng_view->timer);
  mng_view->mng_food = NULL;
}

static void
gtk_mng_view_class_init (GtkMngViewClass * klass)
{
  GObjectClass   * object_class;
  GtkWidgetClass * widget_class;

  parent_class = g_type_class_peek_parent (klass);

  object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = gtk_mng_view_finalize;

  widget_class = GTK_WIDGET_CLASS (klass);
  widget_class->size_request = gtk_mng_view_size_request;
  widget_class->size_allocate = gtk_mng_view_size_allocate;
  widget_class->expose_event = gtk_mng_view_expose;
  widget_class->realize = gtk_mng_view_realize;
}

GtkType
gtk_mng_view_get_type (void)
{
  static GtkType type = 0;

  if (!type)
    {
      static const GtkTypeInfo type_info =
      {
	"GtkMngView",
	sizeof (GtkMngView),
	sizeof (GtkMngViewClass),
	(GtkClassInitFunc) gtk_mng_view_class_init,
	(GtkObjectInitFunc) gtk_mng_view_init,
	NULL, NULL,
	(GtkClassInitFunc) NULL
      };
      type = gtk_type_unique (GTK_TYPE_WIDGET, &type_info);
    }
  return type;
}

GtkWidget *
gtk_mng_view_new (void)
{
  return GTK_WIDGET (gtk_type_new (GTK_MNG_VIEW_TYPE));
}

gboolean
gtk_mng_view_load_mng_from_memory (GtkMngView * mng_view,
				   guchar * data_to_eat,
				   guint data_size)
{
  g_return_val_if_fail (IS_GTK_MNG_VIEW (mng_view), FALSE);
  g_return_val_if_fail (data_size > 27, FALSE);
  g_return_val_if_fail (data_to_eat != NULL, FALSE);

  if (data_to_eat[0] != 0x8a ||
      data_to_eat[1] != 'M' ||
      data_to_eat[2] != 'N' ||
      data_to_eat[3] != 'G' ||
      data_to_eat[4] != 0x0d ||
      data_to_eat[5] != 0x0a ||
      data_to_eat[6] != 0x1a ||
      data_to_eat[7] != 0x0a)
  {
    g_warning ("not mng format");
    return FALSE;
  }

  if (gtk_mng_view_init_libmng (mng_view))
    {
      mng_view->bytes_to_eat = data_size;
      mng_view->bytes_eaten = 0;
      mng_view->mng_food = data_to_eat;

      if (mng_read (mng_view->MNG_handle) != MNG_NOERROR)
        {
	  g_warning ("libmng read error");
	  mng_cleanup (&mng_view->MNG_handle);
	  return FALSE;
	}
      else
        return mng_display (mng_view->MNG_handle);
    }
  else
    {
      g_warning ("error initializing libmng");
      return FALSE;
    }
  return TRUE;
}

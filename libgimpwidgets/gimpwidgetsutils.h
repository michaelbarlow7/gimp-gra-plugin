/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpwidgetsutils.h
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if !defined (__GIMP_WIDGETS_H_INSIDE__) && !defined (GIMP_WIDGETS_COMPILATION)
#error "Only <libgimpwidgets/gimpwidgets.h> can be included directly."
#endif

#ifndef __GIMP_WIDGETS_UTILS_H__
#define __GIMP_WIDGETS_UTILS_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


GtkWidget          * gimp_table_attach_aligned       (GtkTable          *table,
                                                      gint               column,
                                                      gint               row,
                                                      const gchar       *label_text,
                                                      gfloat             xalign,
                                                      gfloat             yalign,
                                                      GtkWidget         *widget,
                                                      gint               colspan,
                                                      gboolean           left_align);

void                 gimp_label_set_attributes       (GtkLabel          *label,
                                                      ...);

gint                 gimp_widget_get_monitor         (GtkWidget         *widget);
gint                 gimp_get_monitor_at_pointer     (GdkScreen        **screen);

GimpColorProfile   * gimp_widget_get_color_profile   (GtkWidget         *widget);

GimpColorTransform   gimp_widget_get_color_transform (GtkWidget         *widget,
                                                      GimpColorConfig   *config,
                                                      GimpColorProfile  *src_profile,
                                                      const Babl       **src_format,
                                                      const Babl       **dest_format);


G_END_DECLS

#endif /* __GIMP_WIDGETS_UTILS_H__ */

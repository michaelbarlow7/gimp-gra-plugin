/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpwidgets.c
 * Copyright (C) 2000 Michael Natterer <mitch@gimp.org>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <lcms2.h>

#include <gegl.h>
#include <gtk/gtk.h>

#ifdef G_OS_WIN32
#include <windows.h>
#endif

#ifdef GDK_WINDOWING_QUARTZ
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>
#endif

#include "libgimpcolor/gimpcolor.h"
#include "libgimpconfig/gimpconfig.h"

#include "gimpwidgetstypes.h"

#include "gimp3migration.h"
#include "gimpsizeentry.h"
#include "gimpwidgetsutils.h"

#include "libgimp/libgimp-intl.h"


/**
 * SECTION: gimpwidgetsutils
 * @title: GimpWidgetsUtils
 * @short_description: A collection of helper functions.
 *
 * A collection of helper functions.
 **/


static GtkWidget *
find_mnemonic_widget (GtkWidget *widget,
                      gint       level)
{
  gboolean can_focus;

  g_object_get (widget, "can-focus", &can_focus, NULL);

  if (GTK_WIDGET_GET_CLASS (widget)->activate_signal ||
      can_focus                                      ||
      GTK_WIDGET_GET_CLASS (widget)->mnemonic_activate !=
      GTK_WIDGET_CLASS (g_type_class_peek (GTK_TYPE_WIDGET))->mnemonic_activate)
    {
      return widget;
    }

  if (GIMP_IS_SIZE_ENTRY (widget))
    {
      GimpSizeEntry *entry = GIMP_SIZE_ENTRY (widget);

      return gimp_size_entry_get_help_widget (entry,
                                              entry->number_of_fields - 1);
    }
  else if (GTK_IS_CONTAINER (widget))
    {
      GtkWidget *mnemonic_widget = NULL;
      GList     *children;
      GList     *list;

      children = gtk_container_get_children (GTK_CONTAINER (widget));

      for (list = children; list; list = g_list_next (list))
        {
          mnemonic_widget = find_mnemonic_widget (list->data, level + 1);

          if (mnemonic_widget)
            break;
        }

      g_list_free (children);

      return mnemonic_widget;
    }

  return NULL;
}

/**
 * gimp_table_attach_aligned:
 * @table:      The #GtkTable the widgets will be attached to.
 * @column:     The column to start with.
 * @row:        The row to attach the widgets.
 * @label_text: The text for the #GtkLabel which will be attached left of
 *              the widget.
 * @xalign:     The horizontal alignment of the #GtkLabel.
 * @yalign:     The vertival alignment of the #GtkLabel.
 * @widget:     The #GtkWidget to attach right of the label.
 * @colspan:    The number of columns the widget will use.
 * @left_align: %TRUE if the widget should be left-aligned.
 *
 * Note that the @label_text can be %NULL and that the widget will be
 * attached starting at (@column + 1) in this case, too.
 *
 * Returns: The created #GtkLabel.
 **/
GtkWidget *
gimp_table_attach_aligned (GtkTable    *table,
                           gint         column,
                           gint         row,
                           const gchar *label_text,
                           gfloat       xalign,
                           gfloat       yalign,
                           GtkWidget   *widget,
                           gint         colspan,
                           gboolean     left_align)
{
  GtkWidget *label = NULL;

  if (label_text)
    {
      GtkWidget *mnemonic_widget;

      label = gtk_label_new_with_mnemonic (label_text);
      gtk_misc_set_alignment (GTK_MISC (label), xalign, yalign);
      gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
      gtk_table_attach (table, label,
                        column, column + 1,
                        row, row + 1,
                        GTK_FILL, GTK_FILL, 0, 0);
      gtk_widget_show (label);

      mnemonic_widget = find_mnemonic_widget (widget, 0);

      if (mnemonic_widget)
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), mnemonic_widget);
    }

  if (left_align)
    {
      GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

      gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
      gtk_widget_show (widget);

      widget = hbox;
    }

  gtk_table_attach (table, widget,
                    column + 1, column + 1 + colspan,
                    row, row + 1,
                    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

  gtk_widget_show (widget);

  return label;
}

/**
 * gimp_label_set_attributes:
 * @label: a #GtkLabel
 * @...:   a list of PangoAttrType and value pairs terminated by -1.
 *
 * Sets Pango attributes on a #GtkLabel in a more convenient way than
 * gtk_label_set_attributes().
 *
 * This function is useful if you want to change the font attributes
 * of a #GtkLabel. This is an alternative to using PangoMarkup which
 * is slow to parse and akward to handle in an i18n-friendly way.
 *
 * The attributes are set on the complete label, from start to end. If
 * you need to set attributes on part of the label, you will have to
 * use the PangoAttributes API directly.
 *
 * Since: 2.2
 **/
void
gimp_label_set_attributes (GtkLabel *label,
                           ...)
{
  PangoAttribute *attr  = NULL;
  PangoAttrList  *attrs;
  va_list         args;

  g_return_if_fail (GTK_IS_LABEL (label));

  attrs = pango_attr_list_new ();

  va_start (args, label);

  do
    {
      PangoAttrType attr_type = va_arg (args, PangoAttrType);

      if (attr_type == -1)
        attr_type = PANGO_ATTR_INVALID;

      switch (attr_type)
        {
        case PANGO_ATTR_LANGUAGE:
          attr = pango_attr_language_new (va_arg (args, PangoLanguage *));
          break;

        case PANGO_ATTR_FAMILY:
          attr = pango_attr_family_new (va_arg (args, const gchar *));
          break;

        case PANGO_ATTR_STYLE:
          attr = pango_attr_style_new (va_arg (args, PangoStyle));
          break;

        case PANGO_ATTR_WEIGHT:
          attr = pango_attr_weight_new (va_arg (args, PangoWeight));
          break;

        case PANGO_ATTR_VARIANT:
          attr = pango_attr_variant_new (va_arg (args, PangoVariant));
          break;

        case PANGO_ATTR_STRETCH:
          attr = pango_attr_stretch_new (va_arg (args, PangoStretch));
          break;

        case PANGO_ATTR_SIZE:
          attr = pango_attr_size_new (va_arg (args, gint));
          break;

        case PANGO_ATTR_FONT_DESC:
          attr = pango_attr_font_desc_new (va_arg (args,
                                                   const PangoFontDescription *));
          break;

        case PANGO_ATTR_FOREGROUND:
          {
            const PangoColor *color = va_arg (args, const PangoColor *);

            attr = pango_attr_foreground_new (color->red,
                                              color->green,
                                              color->blue);
          }
          break;

        case PANGO_ATTR_BACKGROUND:
          {
            const PangoColor *color = va_arg (args, const PangoColor *);

            attr = pango_attr_background_new (color->red,
                                              color->green,
                                              color->blue);
          }
          break;

        case PANGO_ATTR_UNDERLINE:
          attr = pango_attr_underline_new (va_arg (args, PangoUnderline));
          break;

        case PANGO_ATTR_STRIKETHROUGH:
          attr = pango_attr_strikethrough_new (va_arg (args, gboolean));
          break;

        case PANGO_ATTR_RISE:
          attr = pango_attr_rise_new (va_arg (args, gint));
          break;

        case PANGO_ATTR_SCALE:
          attr = pango_attr_scale_new (va_arg (args, gdouble));
          break;

        default:
          g_warning ("%s: invalid PangoAttribute type %d",
                     G_STRFUNC, attr_type);
        case PANGO_ATTR_INVALID:
          attr = NULL;
          break;
        }

      if (attr)
        {
          attr->start_index = 0;
          attr->end_index   = -1;
          pango_attr_list_insert (attrs, attr);
        }
    }
  while (attr);

  va_end (args);

  gtk_label_set_attributes (label, attrs);
  pango_attr_list_unref (attrs);
}

gint
gimp_widget_get_monitor (GtkWidget *widget)
{
  GdkWindow     *window;
  GdkScreen     *screen;
  GtkAllocation  allocation;
  gint           x, y;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), 0);

  window = gtk_widget_get_window (widget);

  if (! window)
    return gimp_get_monitor_at_pointer (&screen);

  screen = gtk_widget_get_screen (widget);

  gdk_window_get_origin (window, &x, &y);
  gtk_widget_get_allocation (widget, &allocation);

  if (! gtk_widget_get_has_window (widget))
    {
      x += allocation.x;
      y += allocation.y;
    }

  x += allocation.width  / 2;
  y += allocation.height / 2;

  return gdk_screen_get_monitor_at_point (screen, x, y);
}

gint
gimp_get_monitor_at_pointer (GdkScreen **screen)
{
  gint x, y;

  g_return_val_if_fail (screen != NULL, 0);

  gdk_display_get_pointer (gdk_display_get_default (),
                           screen, &x, &y, NULL);

  return gdk_screen_get_monitor_at_point (*screen, x, y);
}

GimpColorProfile *
gimp_widget_get_color_profile (GtkWidget *widget)
{
  GimpColorProfile *profile = NULL;
  GdkScreen        *screen;
  gint              monitor;

  g_return_val_if_fail (widget == NULL || GTK_IS_WIDGET (widget), NULL);

  if (widget)
    {
      screen  = gtk_widget_get_screen (widget);
      monitor = gimp_widget_get_monitor (widget);
    }
  else
    {
      screen  = gdk_screen_get_default ();
      monitor = 0;
    }

#if defined GDK_WINDOWING_X11
  {
    GdkAtom  type    = GDK_NONE;
    gint     format  = 0;
    gint     nitems  = 0;
    gchar   *atom_name;
    guchar  *data    = NULL;

    if (monitor > 0)
      atom_name = g_strdup_printf ("_ICC_PROFILE_%d", monitor);
    else
      atom_name = g_strdup ("_ICC_PROFILE");

    if (gdk_property_get (gdk_screen_get_root_window (screen),
                          gdk_atom_intern (atom_name, FALSE),
                          GDK_NONE,
                          0, 64 * 1024 * 1024, FALSE,
                          &type, &format, &nitems, &data) && nitems > 0)
      {
        profile = gimp_color_profile_new_from_icc_profile (data, nitems,
                                                           NULL);
        g_free (data);
      }

    g_free (atom_name);
  }
#elif defined GDK_WINDOWING_QUARTZ
  {
    CMProfileRef prof = NULL;

    CMGetProfileByAVID (monitor, &prof);

    if (prof)
      {
        CFDataRef data;

        data = CMProfileCopyICCData (NULL, prof);
        CMCloseProfile (prof);

        if (data)
          {
            UInt8 *buffer = g_malloc (CFDataGetLength (data));

            /* We cannot use CFDataGetBytesPtr(), because that returns
             * a const pointer where cmsOpenProfileFromMem wants a
             * non-const pointer.
             */
            CFDataGetBytes (data, CFRangeMake (0, CFDataGetLength (data)),
                            buffer);

            profile = gimp_color_profile_new_from_icc_profile (data,
                                                               CFDataGetLength (data),
                                                               NULL);

            g_free (buffer);
            CFRelease (data);
          }
      }
  }
#elif defined G_OS_WIN32
  {
    HDC hdc = GetDC (NULL);

    if (hdc)
      {
        gchar  *path;
        gint32  len = 0;

        GetICMProfile (hdc, &len, NULL);
        path = g_new (gchar, len);

        if (GetICMProfile (hdc, &len, path))
          {
            GFile *file = g_file_new_for_path (path);

            profile = gimp_color_profile_new_from_file (file, NULL);
            g_object_unref (file);
          }

        g_free (path);
        ReleaseDC (NULL, hdc);
      }
  }
#endif

  return profile;
}

static GimpColorProfile *
get_display_profile (GtkWidget       *widget,
                     GimpColorConfig *config)
{
  GimpColorProfile *profile = NULL;

  if (config->display_profile_from_gdk)
    profile = gimp_widget_get_color_profile (widget);

  if (! profile)
    profile = gimp_color_config_get_display_color_profile (config, NULL);

  if (! profile)
    profile = gimp_color_profile_new_srgb ();

  return profile;
}

GimpColorTransform
gimp_widget_get_color_transform (GtkWidget         *widget,
                                 GimpColorConfig   *config,
                                 GimpColorProfile  *src_profile,
                                 const Babl       **src_format,
                                 const Babl       **dest_format)
{
  GimpColorTransform  transform     = NULL;
  GimpColorProfile   *dest_profile  = NULL;
  GimpColorProfile   *proof_profile = NULL;
  cmsHPROFILE         src_lcms;
  cmsHPROFILE         dest_lcms;
  cmsUInt32Number     lcms_src_format;
  cmsUInt32Number     lcms_dest_format;
  cmsUInt16Number     alarmCodes[cmsMAXCHANNELS] = { 0, };

  g_return_val_if_fail (widget == NULL || GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (src_profile), NULL);
  g_return_val_if_fail (GIMP_IS_COLOR_CONFIG (config), NULL);
  g_return_val_if_fail (src_format != NULL, NULL);
  g_return_val_if_fail (dest_format != NULL, NULL);

  switch (config->mode)
    {
    case GIMP_COLOR_MANAGEMENT_OFF:
      return NULL;

    case GIMP_COLOR_MANAGEMENT_SOFTPROOF:
      proof_profile = gimp_color_config_get_printer_color_profile (config, NULL);
      /*  fallthru  */

    case GIMP_COLOR_MANAGEMENT_DISPLAY:
      dest_profile = get_display_profile (widget, config);
      break;
    }

  src_lcms  = gimp_color_profile_get_lcms_profile (src_profile);
  dest_lcms = gimp_color_profile_get_lcms_profile (dest_profile);

  *src_format  = gimp_color_profile_get_format (*src_format,  &lcms_src_format);
  *dest_format = gimp_color_profile_get_format (*dest_format, &lcms_dest_format);

  if (proof_profile)
    {
      cmsHPROFILE     proof_lcms;
      cmsUInt32Number softproof_flags = cmsFLAGS_SOFTPROOFING;

      proof_lcms = gimp_color_profile_get_lcms_profile (proof_profile);

      if (config->simulation_use_black_point_compensation)
        {
          softproof_flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
        }

      if (config->simulation_gamut_check)
        {
          guchar r, g, b;

          softproof_flags |= cmsFLAGS_GAMUTCHECK;

          gimp_rgb_get_uchar (&config->out_of_gamut_color, &r, &g, &b);

          alarmCodes[0] = (cmsUInt16Number) r * 256;
          alarmCodes[1] = (cmsUInt16Number) g * 256;
          alarmCodes[2] = (cmsUInt16Number) b * 256;

          cmsSetAlarmCodes (alarmCodes);
        }

      transform = cmsCreateProofingTransform (src_lcms,  lcms_src_format,
                                              dest_lcms, lcms_dest_format,
                                              proof_lcms,
                                              config->simulation_intent,
                                              config->display_intent,
                                              softproof_flags);

      g_object_unref (proof_profile);
    }
  else if (! gimp_color_profile_is_equal (src_profile, dest_profile))
    {
      cmsUInt32Number display_flags = 0;

      if (config->display_use_black_point_compensation)
        {
          display_flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
        }

      transform = cmsCreateTransform (src_lcms,  lcms_src_format,
                                      dest_lcms, lcms_dest_format,
                                      config->display_intent,
                                      display_flags);
    }

  g_object_unref (dest_profile);

  return transform;
}

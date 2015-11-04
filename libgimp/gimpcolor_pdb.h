/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpcolor_pdb.h
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

/* NOTE: This file is auto-generated by pdbgen.pl */

#if !defined (__GIMP_H_INSIDE__) && !defined (GIMP_COMPILATION)
#error "Only <libgimp/gimp.h> can be included directly."
#endif

#ifndef __GIMP_COLOR_PDB_H__
#define __GIMP_COLOR_PDB_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


GIMP_DEPRECATED_FOR(gimp_drawable_brightness_contrast)
gboolean gimp_brightness_contrast (gint32                drawable_ID,
                                   gint                  brightness,
                                   gint                  contrast);
GIMP_DEPRECATED_FOR(gimp_drawable_levels)
gboolean gimp_levels              (gint32                drawable_ID,
                                   GimpHistogramChannel  channel,
                                   gint                  low_input,
                                   gint                  high_input,
                                   gdouble               gamma,
                                   gint                  low_output,
                                   gint                  high_output);
GIMP_DEPRECATED_FOR(gimp_drawable_levels_stretch)
gboolean gimp_levels_auto         (gint32                drawable_ID);
GIMP_DEPRECATED_FOR(gimp_drawable_levels_stretch)
gboolean gimp_levels_stretch      (gint32                drawable_ID);
GIMP_DEPRECATED_FOR(gimp_drawable_posterize)
gboolean gimp_posterize           (gint32                drawable_ID,
                                   gint                  levels);
GIMP_DEPRECATED_FOR(gimp_drawable_desaturate)
gboolean gimp_desaturate          (gint32                drawable_ID);
GIMP_DEPRECATED_FOR(gimp_drawable_desaturate)
gboolean gimp_desaturate_full     (gint32                drawable_ID,
                                   GimpDesaturateMode    desaturate_mode);
GIMP_DEPRECATED_FOR(gimp_drawable_equalize)
gboolean gimp_equalize            (gint32                drawable_ID,
                                   gboolean              mask_only);
gboolean gimp_invert              (gint32                drawable_ID);
GIMP_DEPRECATED_FOR(gimp_drawable_curves_spline)
gboolean gimp_curves_spline       (gint32                drawable_ID,
                                   GimpHistogramChannel  channel,
                                   gint                  num_points,
                                   const guint8         *control_pts);
GIMP_DEPRECATED_FOR(gimp_drawable_curves_explicit)
gboolean gimp_curves_explicit     (gint32                drawable_ID,
                                   GimpHistogramChannel  channel,
                                   gint                  num_bytes,
                                   const guint8         *curve);
gboolean gimp_color_balance       (gint32                drawable_ID,
                                   GimpTransferMode      transfer_mode,
                                   gboolean              preserve_lum,
                                   gdouble               cyan_red,
                                   gdouble               magenta_green,
                                   gdouble               yellow_blue);
GIMP_DEPRECATED_FOR(gimp_drawable_colorize_hsl)
gboolean gimp_colorize            (gint32                drawable_ID,
                                   gdouble               hue,
                                   gdouble               saturation,
                                   gdouble               lightness);
gboolean gimp_histogram           (gint32                drawable_ID,
                                   GimpHistogramChannel  channel,
                                   gint                  start_range,
                                   gint                  end_range,
                                   gdouble              *mean,
                                   gdouble              *std_dev,
                                   gdouble              *median,
                                   gdouble              *pixels,
                                   gdouble              *count,
                                   gdouble              *percentile);
GIMP_DEPRECATED_FOR(gimp_drawable_hue_saturation)
gboolean gimp_hue_saturation      (gint32                drawable_ID,
                                   GimpHueRange          hue_range,
                                   gdouble               hue_offset,
                                   gdouble               lightness,
                                   gdouble               saturation);
gboolean gimp_threshold           (gint32                drawable_ID,
                                   gint                  low_threshold,
                                   gint                  high_threshold);


G_END_DECLS

#endif /* __GIMP_COLOR_PDB_H__ */

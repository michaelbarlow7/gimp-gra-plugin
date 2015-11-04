/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpimagecolorprofile_pdb.c
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

#include "config.h"

#include <string.h>

#include "gimp.h"


/**
 * SECTION: gimpimagecolorprofile
 * @title: gimpimagecolorprofile
 * @short_description: Operations on an image's color profile.
 *
 * Operations on an image's color profile.
 **/


/**
 * _gimp_image_get_color_profile:
 * @image_ID: The image.
 * @num_bytes: Number of bytes in the color_profile array.
 *
 * Returns the image's color profile
 *
 * This procedure returns the image's color profile, or NULL if the
 * image has no color profile assigned.
 *
 * Returns: The image's serialized color profile. The returned value
 * must be freed with g_free().
 *
 * Since: 2.10
 **/
guint8 *
_gimp_image_get_color_profile (gint32  image_ID,
                               gint   *num_bytes)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  guint8 *profile_data = NULL;

  return_vals = gimp_run_procedure ("gimp-image-get-color-profile",
                                    &nreturn_vals,
                                    GIMP_PDB_IMAGE, image_ID,
                                    GIMP_PDB_END);

  *num_bytes = 0;

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      *num_bytes = return_vals[1].data.d_int32;
      profile_data = g_new (guint8, *num_bytes);
      memcpy (profile_data,
              return_vals[2].data.d_int8array,
              *num_bytes * sizeof (guint8));
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return profile_data;
}

/**
 * _gimp_image_get_effective_color_profile:
 * @image_ID: The image.
 * @num_bytes: Number of bytes in the color_profile array.
 *
 * Returns the color profile that is used for the image
 *
 * This procedure returns the color profile that is actually used for
 * this image, which is the profile returned by
 * gimp_image_get_color_profile() if the image has a profile assigned,
 * or a generated default RGB profile. If the image is not RGB or
 * INDEXED, NULL is returned.
 *
 * Returns: The image's serialized color profile. The returned value
 * must be freed with g_free().
 *
 * Since: 2.10
 **/
guint8 *
_gimp_image_get_effective_color_profile (gint32  image_ID,
                                         gint   *num_bytes)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  guint8 *profile_data = NULL;

  return_vals = gimp_run_procedure ("gimp-image-get-effective-color-profile",
                                    &nreturn_vals,
                                    GIMP_PDB_IMAGE, image_ID,
                                    GIMP_PDB_END);

  *num_bytes = 0;

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      *num_bytes = return_vals[1].data.d_int32;
      profile_data = g_new (guint8, *num_bytes);
      memcpy (profile_data,
              return_vals[2].data.d_int8array,
              *num_bytes * sizeof (guint8));
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return profile_data;
}

/**
 * _gimp_image_set_color_profile:
 * @image_ID: The image.
 * @num_bytes: Number of bytes in the color_profile array.
 * @color_profile: The new serialized color profile.
 *
 * Sets the image's color profile
 *
 * This procedure sets the image's color profile, or unsets it if NULL
 * is passed as 'color_profile'. This procedure does no color
 * conversion.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.10
 **/
gboolean
_gimp_image_set_color_profile (gint32        image_ID,
                               gint          num_bytes,
                               const guint8 *color_profile)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-image-set-color-profile",
                                    &nreturn_vals,
                                    GIMP_PDB_IMAGE, image_ID,
                                    GIMP_PDB_INT32, num_bytes,
                                    GIMP_PDB_INT8ARRAY, color_profile,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_image_set_color_profile_from_file:
 * @image_ID: The image.
 * @uri: The URI of the file containing the new color profile.
 *
 * Sets the image's color profile from an ICC file
 *
 * This procedure sets the image's color profile from a file containing
 * an ICC profile, or unsets it if NULL is passed as 'uri'. This
 * procedure does no color conversion.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.10
 **/
gboolean
gimp_image_set_color_profile_from_file (gint32       image_ID,
                                        const gchar *uri)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-image-set-color-profile-from-file",
                                    &nreturn_vals,
                                    GIMP_PDB_IMAGE, image_ID,
                                    GIMP_PDB_STRING, uri,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * _gimp_image_convert_color_profile:
 * @image_ID: The image.
 * @num_bytes: Number of bytes in the color_profile array.
 * @color_profile: The serialized color profile.
 * @intent: Rendering intent.
 * @bpc: Black point compensation.
 *
 * Convert the image's layers to a color profile
 *
 * This procedure converts from the image's color profile (or the
 * default RGB profile if none is set) to the given color profile. Only
 * RGB color profiles are accepted.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.10
 **/
gboolean
_gimp_image_convert_color_profile (gint32                    image_ID,
                                   gint                      num_bytes,
                                   const guint8             *color_profile,
                                   GimpColorRenderingIntent  intent,
                                   gboolean                  bpc)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-image-convert-color-profile",
                                    &nreturn_vals,
                                    GIMP_PDB_IMAGE, image_ID,
                                    GIMP_PDB_INT32, num_bytes,
                                    GIMP_PDB_INT8ARRAY, color_profile,
                                    GIMP_PDB_INT32, intent,
                                    GIMP_PDB_INT32, bpc,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_image_convert_color_profile_from_file:
 * @image_ID: The image.
 * @uri: The URI of the file containing the new color profile.
 * @intent: Rendering intent.
 * @bpc: Black point compensation.
 *
 * Convert the image's layers to a color profile
 *
 * This procedure converts from the image's color profile (or the
 * default RGB profile if none is set) to an ICC profile precified by
 * 'uri'. Only RGB color profiles are accepted.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.10
 **/
gboolean
gimp_image_convert_color_profile_from_file (gint32                    image_ID,
                                            const gchar              *uri,
                                            GimpColorRenderingIntent  intent,
                                            gboolean                  bpc)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-image-convert-color-profile-from-file",
                                    &nreturn_vals,
                                    GIMP_PDB_IMAGE, image_ID,
                                    GIMP_PDB_STRING, uri,
                                    GIMP_PDB_INT32, intent,
                                    GIMP_PDB_INT32, bpc,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

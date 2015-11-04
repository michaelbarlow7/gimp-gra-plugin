/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * gimpcolorprofile.c
 * Copyright (C) 2014  Michael Natterer <mitch@gimp.org>
 *                     Elle Stone <ellestone@ninedegreesbelow.com>
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

#include <string.h>

#include <lcms2.h>

#include <gio/gio.h>
#include <gegl.h>

#include "libgimpbase/gimpbase.h"

#include "gimpcolortypes.h"

#include "gimpcolorprofile.h"

#include "libgimp/libgimp-intl.h"


/**
 * SECTION: gimpcolorprofile
 * @title: GimpColorProfile
 * @short_description: Definitions and Functions relating to LCMS.
 *
 * Definitions and Functions relating to LCMS.
 **/

/**
 * GimpColorProfile:
 *
 * Simply a typedef to #gpointer, but actually is a cmsHPROFILE. It's
 * used in public GIMP APIs in order to avoid having to include LCMS
 * headers.
 **/


struct _GimpColorProfilePrivate
{
  cmsHPROFILE  lcms_profile;
  guint8      *data;
  gsize        length;

  gchar       *description;
  gchar       *manufacturer;
  gchar       *model;
  gchar       *copyright;
  gchar       *label;
  gchar       *summary;
};


static void   gimp_color_profile_finalize (GObject *object);


G_DEFINE_TYPE (GimpColorProfile, gimp_color_profile,
               G_TYPE_OBJECT);

#define parent_class gimp_color_profile_parent_class


static GQuark
gimp_color_profile_error_quark (void)
{
  static GQuark quark = 0;

  if (G_UNLIKELY (quark == 0))
    quark = g_quark_from_static_string ("gimp-color-profile-error-quark");

  return quark;
}

static void
gimp_color_profile_class_init (GimpColorProfileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gimp_color_profile_finalize;

  g_type_class_add_private (klass, sizeof (GimpColorProfilePrivate));
}

static void
gimp_color_profile_init (GimpColorProfile *profile)
{
  profile->priv = G_TYPE_INSTANCE_GET_PRIVATE (profile,
                                               GIMP_TYPE_COLOR_PROFILE,
                                               GimpColorProfilePrivate);
}

static void
gimp_color_profile_finalize (GObject *object)
{
  GimpColorProfile *profile = GIMP_COLOR_PROFILE (object);

  if (profile->priv->lcms_profile)
    {
      cmsCloseProfile (profile->priv->lcms_profile);
      profile->priv->lcms_profile = NULL;
    }

  if (profile->priv->data)
    {
      g_free (profile->priv->data);
      profile->priv->data   = NULL;
      profile->priv->length = 0;
    }

  g_free (profile->priv->description);
  g_free (profile->priv->manufacturer);
  g_free (profile->priv->model);
  g_free (profile->priv->copyright);
  g_free (profile->priv->label);
  g_free (profile->priv->summary);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


/**
 * gimp_color_profile_new_from_file:
 * @file:  a #GFile
 * @error: return location for #GError
 *
 * This function opens an ICC color profile from @file.
 *
 * Return value: the #GimpColorProfile, or %NULL. On error, %NULL is
 *               returned and @error is set.
 *
 * Since: 2.10
 **/
GimpColorProfile *
gimp_color_profile_new_from_file (GFile   *file,
                                  GError **error)
{
  GimpColorProfile *profile      = NULL;
  cmsHPROFILE       lcms_profile = NULL;
  guint8           *data         = NULL;
  gsize             length       = 0;
  gchar            *path;

  g_return_val_if_fail (G_IS_FILE (file), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  path = g_file_get_path (file);

  if (path)
    {
      GMappedFile  *mapped;

      mapped = g_mapped_file_new (path, FALSE, error);

      if (! mapped)
        return NULL;

      length = g_mapped_file_get_length (mapped);
      data   = g_memdup (g_mapped_file_get_contents (mapped), length);

      lcms_profile = cmsOpenProfileFromMem (data, length);

      g_mapped_file_unref (mapped);
    }
  else
    {
      GFileInfo *info;

      info = g_file_query_info (file,
                                G_FILE_ATTRIBUTE_STANDARD_SIZE,
                                G_FILE_QUERY_INFO_NONE,
                                NULL, error);
      if (info)
        {
          GInputStream *input;

          length = g_file_info_get_size (info);
          data   = g_malloc (length);

          g_object_unref (info);

          input = G_INPUT_STREAM (g_file_read (file, NULL, error));

          if (input)
            {
              gsize bytes_read;

              if (g_input_stream_read_all (input, data, length,
                                           &bytes_read, NULL, error) &&
                  bytes_read == length)
                {
                  lcms_profile = cmsOpenProfileFromMem (data, length);
                }

              g_object_unref (input);
            }
        }
    }

  if (lcms_profile)
    {
      profile = g_object_new (GIMP_TYPE_COLOR_PROFILE, NULL);

      profile->priv->lcms_profile = lcms_profile;
      profile->priv->data         = data;
      profile->priv->length       = length;
    }
  else
    {
      if (data)
        g_free (data);

      if (error && *error == NULL)
        {
          g_set_error (error, gimp_color_profile_error_quark (), 0,
                       _("'%s' does not appear to be an ICC color profile"),
                       gimp_file_get_utf8_name (file));
        }
    }

  return profile;
}

/**
 * gimp_color_profile_new_from_icc_profile:
 * @data:   pointer to memory containing an ICC profile
 * @length: lenght of the profile in memory, in bytes
 * @error:  return location for #GError
 *
 * This function opens an ICC color profile from memory. On error,
 * %NULL is returned and @error is set.
 *
 * Return value: the #GimpColorProfile, or %NULL.
 *
 * Since: 2.10
 **/
GimpColorProfile *
gimp_color_profile_new_from_icc_profile (const guint8  *data,
                                         gsize          length,
                                         GError       **error)
{
  cmsHPROFILE       lcms_profile;
  GimpColorProfile *profile = NULL;

  g_return_val_if_fail (data != NULL, NULL);
  g_return_val_if_fail (length > 0, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  lcms_profile = cmsOpenProfileFromMem (data, length);

  if (lcms_profile)
    {
      profile = g_object_new (GIMP_TYPE_COLOR_PROFILE, NULL);

      profile->priv->lcms_profile = lcms_profile;
      profile->priv->data         = g_memdup (data, length);
      profile->priv->length       = length;
   }
  else
    {
      g_set_error_literal (error, gimp_color_profile_error_quark (), 0,
                           _("Data does not appear to be an ICC color profile"));
    }

  return profile;
}

/**
 * gimp_color_profile_new_from_lcms_profile:
 * @lcms_profile: an LCMS cmsHPROFILE pointer
 * @error:        return location for #GError
 *
 * This function creates a GimpColorProfile from a cmsHPROFILE. On
 * error, %NULL is returned and @error is set. The passed
 * @lcms_profile pointer is not retained by the created
 * #GimpColorProfile.
 *
 * Return value: the #GimpColorProfile, or %NULL.
 *
 * Since: 2.10
 **/
GimpColorProfile *
gimp_color_profile_new_from_lcms_profile (gpointer   lcms_profile,
                                          GError   **error)
{
  cmsUInt32Number size;

  g_return_val_if_fail (lcms_profile != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (cmsSaveProfileToMem (lcms_profile, NULL, &size))
    {
      guint8 *data = g_malloc (size);

      if (cmsSaveProfileToMem (lcms_profile, data, &size))
        {
          gsize length = size;

          lcms_profile = cmsOpenProfileFromMem (data, length);

          if (lcms_profile)
            {
              GimpColorProfile *profile;

              profile = g_object_new (GIMP_TYPE_COLOR_PROFILE, NULL);

              profile->priv->lcms_profile = lcms_profile;
              profile->priv->data         = data;
              profile->priv->length       = length;

              return profile;
            }
        }

      g_free (data);
    }

  g_set_error_literal (error, gimp_color_profile_error_quark (), 0,
                       _("Could not save color profile to memory"));

  return NULL;
}

/**
 * gimp_color_profile_get_icc_profile:
 * @profile: a #GimpColorProfile
 * @length:  return location for the number of bytes
 * @error:   return location for #GError
 *
 * This function returns @profile as ICC profile data. The returned
 * memory belongs to @profile and must not be modified or freed.
 *
 * Return value: a pointer to the IIC profile data.
 *
 * Since: 2.10
 **/
const guint8 *
gimp_color_profile_get_icc_profile (GimpColorProfile  *profile,
                                    gsize             *length)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);
  g_return_val_if_fail (length != NULL, NULL);

  *length = profile->priv->length;

  return profile->priv->data;
}

/**
 * gimp_color_profile_get_lcms_profile:
 * @profile: a #GimpColorProfile
 *
 * This function returns @profile's cmsHPROFILE. The returned
 * value belongs to @profile and must not be modified or freed.
 *
 * Return value: a pointer to the cmsHPROFILE.
 *
 * Since: 2.10
 **/
gpointer
gimp_color_profile_get_lcms_profile (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  return profile->priv->lcms_profile;
}

static gchar *
gimp_color_profile_get_info (GimpColorProfile *profile,
                             cmsInfoType       info)
{
  cmsUInt32Number  size;
  gchar           *text = NULL;

  size = cmsGetProfileInfoASCII (profile->priv->lcms_profile, info,
                                 "en", "US", NULL, 0);
  if (size > 0)
    {
      gchar *data = g_new (gchar, size + 1);

      size = cmsGetProfileInfoASCII (profile->priv->lcms_profile, info,
                                     "en", "US", data, size);
      if (size > 0)
        text = gimp_any_to_utf8 (data, -1, NULL);

      g_free (data);
    }

  return text;
}

/**
 * gimp_color_profile_get_description:
 * @profile: a #GimpColorProfile
 *
 * Return value: a string containing @profile's description. The
 *               returned value belongs to @profile and must not be
 *               modified or freed.
 *
 * Since: 2.10
 **/
const gchar *
gimp_color_profile_get_description (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  if (! profile->priv->description)
    profile->priv->description =
      gimp_color_profile_get_info (profile, cmsInfoDescription);

  return profile->priv->description;
}

/**
 * gimp_color_profile_get_manufacturer:
 * @profile: a #GimpColorProfile
 *
 * Return value: a string containing @profile's manufacturer. The
 *               returned value belongs to @profile and must not be
 *               modified or freed.
 *
 * Since: 2.10
 **/
const gchar *
gimp_color_profile_get_manufacturer (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  if (! profile->priv->manufacturer)
    profile->priv->manufacturer =
      gimp_color_profile_get_info (profile, cmsInfoManufacturer);

  return profile->priv->manufacturer;
}

/**
 * gimp_color_profile_get_model:
 * @profile: a #GimpColorProfile
 *
 * Return value: a string containing @profile's model. The returned
 *               value belongs to @profile and must not be modified or
 *               freed.
 *
 * Since: 2.10
 **/
const gchar *
gimp_color_profile_get_model (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  if (! profile->priv->model)
    profile->priv->model =
      gimp_color_profile_get_info (profile, cmsInfoModel);

  return profile->priv->model;
}

/**
 * gimp_color_profile_get_copyright:
 * @profile: a #GimpColorProfile
 *
 * Return value: a string containing @profile's copyright. The
 *               returned value belongs to @profile and must not be
 *               modified or freed.
 *
 * Since: 2.10
 **/
const gchar *
gimp_color_profile_get_copyright (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  if (! profile->priv->copyright)
    profile->priv->copyright =
      gimp_color_profile_get_info (profile, cmsInfoCopyright);

  return profile->priv->copyright;
}

/**
 * gimp_color_profile_get_label:
 * @profile: a #GimpColorProfile
 *
 * This function returns a string containing @profile's "title", a
 * string that can be used to label the profile in a user interface.
 *
 * Unlike gimp_color_profile_get_description(), this function always
 * returns a string (as a fallback, it returns "(unnamed profile)".
 *
 * Return value: the @profile's label. The returned value belongs to
 *               @profile and must not be modified or freed.
 *
 * Since: 2.10
 **/
const gchar *
gimp_color_profile_get_label (GimpColorProfile *profile)
{

  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  if (! profile->priv->label)
    {
      const gchar *label = gimp_color_profile_get_description (profile);

      if (! label || ! strlen (label))
        label = _("(unnamed profile)");

      profile->priv->label = g_strdup (label);
    }

  return profile->priv->label;
}

/**
 * gimp_color_profile_get_summary:
 * @profile: a #GimpColorProfile
 *
 * This function return a string containing a multi-line summary of
 * @profile's description, model, manufacturer and copyright, to be
 * used as detailled information about the profile in a user
 * interface.
 *
 * Return value: the @profile's summary. The returned value belongs to
 *               @profile and must not be modified or freed.
 *
 * Since: 2.10
 **/
const gchar *
gimp_color_profile_get_summary (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  if (! profile->priv->summary)
    {
      GString     *string = g_string_new (NULL);
      const gchar *text;

      text = gimp_color_profile_get_description (profile);
      if (text)
        g_string_append (string, text);

      text = gimp_color_profile_get_model (profile);
      if (text)
        {
          if (string->len > 0)
            g_string_append (string, "\n");

          g_string_append_printf (string, _("Model: %s"), text);
        }

      text = gimp_color_profile_get_manufacturer (profile);
      if (text)
        {
          if (string->len > 0)
            g_string_append (string, "\n");

          g_string_append_printf (string, _("Manufacturer: %s"), text);
        }

      text = gimp_color_profile_get_copyright (profile);
      if (text)
        {
          if (string->len > 0)
            g_string_append (string, "\n");

          g_string_append_printf (string, _("Copyright: %s"), text);
        }

      profile->priv->summary = g_string_free (string, FALSE);
    }

  return profile->priv->summary;
}

/**
 * gimp_color_profile_is_equal:
 * @profile1: a #GimpColorProfile
 * @profile2: a #GimpColorProfile
 *
 * Compares two profiles.
 *
 * Return value: %TRUE if the profiles are equal, %FALSE otherwise.
 *
 * Since: 2.10
 **/
gboolean
gimp_color_profile_is_equal (GimpColorProfile *profile1,
                             GimpColorProfile *profile2)
{
  const gsize header_len = sizeof (cmsICCHeader);

  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile1), FALSE);
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile1), FALSE);

  return (profile1->priv->length == profile2->priv->length &&
          memcmp (profile1->priv->data + header_len,
                  profile2->priv->data + header_len,
                  profile1->priv->length - header_len) == 0);
}

/**
 * gimp_color_profile_is_rgb:
 * @profile: a #GimpColorProfile
 *
 * Return value: %TRUE if the profile's color space is RGB, %FALSE
 * otherwise.
 *
 * Since: 2.10
 **/
gboolean
gimp_color_profile_is_rgb (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), FALSE);

  return (cmsGetColorSpace (profile->priv->lcms_profile) == cmsSigRgbData);
}

/**
 * gimp_color_profile_is_cmyk:
 * @profile: a #GimpColorProfile
 *
 * Return value: %TRUE if the profile's color space is CMYK, %FALSE
 * otherwise.
 *
 * Since: 2.10
 **/
gboolean
gimp_color_profile_is_cmyk (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), FALSE);

  return (cmsGetColorSpace (profile->priv->lcms_profile) == cmsSigCmykData);
}

static void
gimp_color_profile_set_tag (cmsHPROFILE      profile,
                            cmsTagSignature  sig,
                            const gchar     *tag)
{
  cmsMLU *mlu;

  mlu = cmsMLUalloc (NULL, 1);
  cmsMLUsetASCII (mlu, "en", "US", tag);
  cmsWriteTag (profile, sig, mlu);
  cmsMLUfree (mlu);
}

static gboolean
gimp_color_profile_get_rgb_matrix_colorants (GimpColorProfile *profile,
                                             GimpMatrix3      *matrix)
{
  cmsHPROFILE  lcms_profile;
  cmsCIEXYZ   *red;
  cmsCIEXYZ   *green;
  cmsCIEXYZ   *blue;

  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), FALSE);

  lcms_profile = profile->priv->lcms_profile;

  red   = cmsReadTag (lcms_profile, cmsSigRedColorantTag);
  green = cmsReadTag (lcms_profile, cmsSigGreenColorantTag);
  blue  = cmsReadTag (lcms_profile, cmsSigBlueColorantTag);

  if (red && green && blue)
    {
      if (matrix)
        {
          matrix->coeff[0][0] = red->X;
          matrix->coeff[0][1] = red->Y;
          matrix->coeff[0][2] = red->Z;

          matrix->coeff[1][0] = green->X;
          matrix->coeff[1][1] = green->Y;
          matrix->coeff[1][2] = green->Z;

          matrix->coeff[2][0] = blue->X;
          matrix->coeff[2][1] = blue->Y;
          matrix->coeff[2][2] = blue->Z;
        }

      return TRUE;
    }

  return FALSE;
}

static GimpColorProfile *
gimp_color_profile_new_from_color_profile (GimpColorProfile *profile,
                                           gboolean          linear)
{
  GimpColorProfile *new_profile;
  cmsHPROFILE       target_profile;
  GimpMatrix3       matrix;
  cmsCIEXYZ         red;
  cmsCIEXYZ         green;
  cmsCIEXYZ         blue;
  cmsCIEXYZ        *whitepoint;
  cmsToneCurve     *curve;
  const gchar      *model;
  gchar            *new_model;

  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  if (! gimp_color_profile_get_rgb_matrix_colorants (profile, &matrix))
    return NULL;

  whitepoint = cmsReadTag (profile->priv->lcms_profile,
                           cmsSigMediaWhitePointTag);

  red.X = matrix.coeff[0][0];
  red.Y = matrix.coeff[0][1];
  red.Z = matrix.coeff[0][2];

  green.X = matrix.coeff[1][0];
  green.Y = matrix.coeff[1][1];
  green.Z = matrix.coeff[1][2];

  blue.X = matrix.coeff[2][0];
  blue.Y = matrix.coeff[2][1];
  blue.Z = matrix.coeff[2][2];

  target_profile = cmsCreateProfilePlaceholder (0);

  cmsSetProfileVersion (target_profile, 4.3);
  cmsSetDeviceClass (target_profile, cmsSigDisplayClass);
  cmsSetColorSpace (target_profile, cmsSigRgbData);
  cmsSetPCS (target_profile, cmsSigXYZData);

  cmsWriteTag (target_profile, cmsSigMediaWhitePointTag, whitepoint);

  cmsWriteTag (target_profile, cmsSigRedColorantTag,   &red);
  cmsWriteTag (target_profile, cmsSigGreenColorantTag, &green);
  cmsWriteTag (target_profile, cmsSigBlueColorantTag,  &blue);

  if (linear)
    {
      /* linear light */
      curve = cmsBuildGamma (NULL, 1.00);

      gimp_color_profile_set_tag (target_profile, cmsSigProfileDescriptionTag,
                                  "linear variant generated by GIMP");
    }
  else
    {
      cmsFloat64Number srgb_parameters[5] =
        { 2.4, 1.0 / 1.055,  0.055 / 1.055, 1.0 / 12.92, 0.04045 };

      /* sRGB curve */
      curve = cmsBuildParametricToneCurve (NULL, 4, srgb_parameters);

      gimp_color_profile_set_tag (target_profile, cmsSigProfileDescriptionTag,
                                  "sRGB gamma variant generated by GIMP");
    }

  cmsWriteTag (target_profile, cmsSigRedTRCTag,   curve);
  cmsWriteTag (target_profile, cmsSigGreenTRCTag, curve);
  cmsWriteTag (target_profile, cmsSigBlueTRCTag,  curve);

  cmsFreeToneCurve (curve);

  model = gimp_color_profile_get_model (profile);

  if (model && g_str_has_prefix (model, "Generated from '"))
    {
      /* don't add multiple "Generated from 'foo'" */
      new_model = g_strdup (model);
    }
  else
    {
      new_model = g_strdup_printf ("Generated from '%s'",
                                   gimp_color_profile_get_description (profile));
    }

  gimp_color_profile_set_tag (target_profile, cmsSigDeviceMfgDescTag,
                              "GIMP");
  gimp_color_profile_set_tag (target_profile, cmsSigDeviceModelDescTag,
                              new_model);
  gimp_color_profile_set_tag (target_profile, cmsSigCopyrightTag,
                              "Public Domain");

  g_free (new_model);

  new_profile = gimp_color_profile_new_from_lcms_profile (target_profile, NULL);

  cmsCloseProfile (target_profile);

  return new_profile;
}

/**
 * gimp_color_profile_new_srgb_gamma_from_color_profile:
 * @profile: a #GimpColorProfile
 *
 * This function creates a new RGB #GimpColorProfile with a sRGB gamma
 * TRC and @profile's RGB chromacities and whitepoint.
 *
 * Return value: the new #GimpColorProfile, or %NULL if @profile is not
 *               an RGB profile or not matrix-based.
 *
 * Since: 2.10
 **/
GimpColorProfile *
gimp_color_profile_new_srgb_gamma_from_color_profile (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  return gimp_color_profile_new_from_color_profile (profile, FALSE);
}

/**
 * gimp_color_profile_new_linear_rgb_from_color_profile:
 * @profile: a #GimpColorProfile
 *
 * This function creates a new RGB #GimpColorProfile with a linear TRC
 * and @profile's RGB chromacities and whitepoint.
 *
 * Return value: the new #GimpColorProfile, or %NULL if @profile is not
 *               an RGB profile or not matrix-based.
 *
 * Since: 2.10
 **/
GimpColorProfile *
gimp_color_profile_new_linear_rgb_from_color_profile (GimpColorProfile *profile)
{
  g_return_val_if_fail (GIMP_IS_COLOR_PROFILE (profile), NULL);

  return gimp_color_profile_new_from_color_profile (profile, TRUE);
}

static cmsHPROFILE *
gimp_color_profile_new_srgb_internal (void)
{
  cmsHPROFILE profile;

  /* white point is D65 from the sRGB specs */
  cmsCIExyY whitepoint = { 0.3127, 0.3290, 1.0 };

  /* primaries are ITU‐R BT.709‐5 (xYY), which are also the primaries
   * from the sRGB specs, modified to properly account for hexadecimal
   * quantization during the profile making process.
   */
  cmsCIExyYTRIPLE primaries =
    {
      /* R { 0.6400, 0.3300, 1.0 }, */
      /* G { 0.3000, 0.6000, 1.0 }, */
      /* B { 0.1500, 0.0600, 1.0 }  */
      /* R */ { 0.639998686, 0.330010138, 1.0 },
      /* G */ { 0.300003784, 0.600003357, 1.0 },
      /* B */ { 0.150002046, 0.059997204, 1.0 }
    };

  cmsFloat64Number srgb_parameters[5] =
    { 2.4, 1.0 / 1.055,  0.055 / 1.055, 1.0 / 12.92, 0.04045 };

  cmsToneCurve *curve[3];

  /* sRGB curve */
  curve[0] = curve[1] = curve[2] = cmsBuildParametricToneCurve (NULL, 4,
                                                                srgb_parameters);

  profile = cmsCreateRGBProfile (&whitepoint, &primaries, curve);

  cmsFreeToneCurve (curve[0]);

  gimp_color_profile_set_tag (profile, cmsSigProfileDescriptionTag,
                              "GIMP built-in sRGB");
  gimp_color_profile_set_tag (profile, cmsSigDeviceMfgDescTag,
                              "GIMP");
  gimp_color_profile_set_tag (profile, cmsSigDeviceModelDescTag,
                              "sRGB");
  gimp_color_profile_set_tag (profile, cmsSigCopyrightTag,
                              "Public Domain");

  /* The following line produces a V2 profile with a point curve TRC.
   * Profiles with point curve TRCs can't be used in LCMS2 unbounded
   * mode ICC profile conversions. A V2 profile might be appropriate
   * for embedding in sRGB images saved to disk, if the image is to be
   * opened by an image editing application that doesn't understand V4
   * profiles.
   *
   * cmsSetProfileVersion (srgb_profile, 2.1);
   */

  return profile;
}

/**
 * gimp_color_profile_new_srgb:
 *
 * This function is a replacement for cmsCreate_sRGBProfile() and
 * returns an sRGB profile that is functionally the same as the
 * ArgyllCMS sRGB.icm profile. "Functionally the same" means it has
 * the same red, green, and blue colorants and the V4 "chad"
 * equivalent of the ArgyllCMS V2 white point. The profile TRC is also
 * functionally equivalent to the ArgyllCMS sRGB.icm TRC and is the
 * same as the LCMS sRGB built-in profile TRC.
 *
 * The actual primaries in the sRGB specification are
 * red xy:   {0.6400, 0.3300, 1.0}
 * green xy: {0.3000, 0.6000, 1.0}
 * blue xy:  {0.1500, 0.0600, 1.0}
 *
 * The sRGB primaries given below are "pre-quantized" to compensate
 * for hexadecimal quantization during the profile-making process.
 * Unless the profile-making code compensates for this quantization,
 * the resulting profile's red, green, and blue colorants will deviate
 * slightly from the correct XYZ values.
 *
 * LCMS2 doesn't compensate for hexadecimal quantization. The
 * "pre-quantized" primaries below were back-calculated from the
 * ArgyllCMS sRGB.icm profile. The resulting sRGB profile's colorants
 * exactly matches the ArgyllCMS sRGB.icm profile colorants.
 *
 * Return value: the sRGB #GimpColorProfile.
 *
 * Since: 2.10
 **/
GimpColorProfile *
gimp_color_profile_new_srgb (void)
{
  static GimpColorProfile *profile = NULL;

  const guint8 *data;
  gsize         length;

  if (G_UNLIKELY (profile == NULL))
    {
      cmsHPROFILE lcms_profile = gimp_color_profile_new_srgb_internal ();

      profile = gimp_color_profile_new_from_lcms_profile (lcms_profile, NULL);

      cmsCloseProfile (lcms_profile);

#if 0
      /* for testing the code to get the colorants and make a new profile */
      {
        GimpMatrix3 matrix;

        if (gimp_color_profile_get_rgb_matrix_colorants (profile, &matrix))
          {
            GimpColorProfile *test;

            g_printerr ("Profile Red colorant XYZ: %1.8f, %1.8f, %1.8f \n",
                        matrix.coeff[0][0],
                        matrix.coeff[0][1],
                        matrix.coeff[0][2]);
            g_printerr ("Profile Green colorant XYZ: %1.8f, %1.8f, %1.8f \n",
                        matrix.coeff[1][0],
                        matrix.coeff[1][1],
                        matrix.coeff[1][2]);
            g_printerr ("Profile Blue colorant XYZ: %1.8f, %1.8f, %1.8f \n",
                        matrix.coeff[2][0],
                        matrix.coeff[2][1],
                        matrix.coeff[2][2]);

            test = gimp_color_profile_new_foobar (profile);
            g_object_unref (test);
          }
      }
#endif
    }

  data = gimp_color_profile_get_icc_profile (profile, &length);

  return gimp_color_profile_new_from_icc_profile (data, length, NULL);
}

static cmsHPROFILE
gimp_color_profile_new_linear_rgb_internal (void)
{
  cmsHPROFILE profile;

  /* white point is D65 from the sRGB specs */
  cmsCIExyY whitepoint = { 0.3127, 0.3290, 1.0 };

  /* primaries are ITU‐R BT.709‐5 (xYY), which are also the primaries
   * from the sRGB specs, modified to properly account for hexadecimal
   * quantization during the profile making process.
   */
  cmsCIExyYTRIPLE primaries =
    {
      /* R { 0.6400, 0.3300, 1.0 }, */
      /* G { 0.3000, 0.6000, 1.0 }, */
      /* B { 0.1500, 0.0600, 1.0 }  */
      /* R */ { 0.639998686, 0.330010138, 1.0 },
      /* G */ { 0.300003784, 0.600003357, 1.0 },
      /* B */ { 0.150002046, 0.059997204, 1.0 }
    };

  cmsToneCurve *curve[3];

  /* linear light */
  curve[0] = curve[1] = curve[2] = cmsBuildGamma (NULL, 1.0);

  profile = cmsCreateRGBProfile (&whitepoint, &primaries, curve);

  cmsFreeToneCurve (curve[0]);

  gimp_color_profile_set_tag (profile, cmsSigProfileDescriptionTag,
                              "GIMP built-in Linear RGB");
  gimp_color_profile_set_tag (profile, cmsSigDeviceMfgDescTag,
                              "GIMP");
  gimp_color_profile_set_tag (profile, cmsSigDeviceModelDescTag,
                              "Linear RGB");
  gimp_color_profile_set_tag (profile, cmsSigCopyrightTag,
                              "Public Domain");

  return profile;
}

/**
 * gimp_color_profile_new_linear_rgb:
 *
 * This function creates a profile for babl_model("RGB"). Please
 * somebody write someting smarter here.
 *
 * Return value: the linear RGB #GimpColorProfile.
 *
 * Since: 2.10
 **/
GimpColorProfile *
gimp_color_profile_new_linear_rgb (void)
{
  static GimpColorProfile *profile = NULL;

  const guint8 *data;
  gsize         length;

  if (G_UNLIKELY (profile == NULL))
    {
      cmsHPROFILE lcms_profile = gimp_color_profile_new_linear_rgb_internal ();

      profile = gimp_color_profile_new_from_lcms_profile (lcms_profile, NULL);

      cmsCloseProfile (lcms_profile);
    }

  data = gimp_color_profile_get_icc_profile (profile, &length);

  return gimp_color_profile_new_from_icc_profile (data, length, NULL);
}

static cmsHPROFILE *
gimp_color_profile_new_adobe_rgb_internal (void)
{
  cmsHPROFILE profile;

  /* white point is D65 from the sRGB specs */
  cmsCIExyY whitepoint = { 0.3127, 0.3290, 1.0 };

  /* AdobeRGB1998 and sRGB have the same white point.
   *
   * The primaries below are technically correct, but because of
   * hexadecimal rounding these primaries don't make a profile that
   * matches the original.
   *
   *  cmsCIExyYTRIPLE primaries = {
   *    { 0.6400, 0.3300, 1.0 },
   *    { 0.2100, 0.7100, 1.0 },
   *    { 0.1500, 0.0600, 1.0 }
   *  };
   */
  cmsCIExyYTRIPLE primaries =
    {
      { 0.639996511, 0.329996864, 1.0 },
      { 0.210005295, 0.710004866, 1.0 },
      { 0.149997606, 0.060003644, 1.0 }
    };

  cmsToneCurve *curve[3];

  /* gamma 2.2 */
  curve[0] = curve[1] = curve[2] = cmsBuildGamma (NULL, 2.19921875);

  profile = cmsCreateRGBProfile (&whitepoint, &primaries, curve);

  cmsFreeToneCurve (curve[0]);

  gimp_color_profile_set_tag (profile, cmsSigProfileDescriptionTag,
                              "Compatible with Adobe RGB (1998)");
  gimp_color_profile_set_tag (profile, cmsSigDeviceMfgDescTag,
                              "GIMP");
  gimp_color_profile_set_tag (profile, cmsSigDeviceModelDescTag,
                              "Compatible with Adobe RGB (1998)");
  gimp_color_profile_set_tag (profile, cmsSigCopyrightTag,
                              "Public Domain");

  return profile;
}

/**
 * gimp_color_profile_new_adobe_rgb:
 *
 * This function creates a profile compatible with AbobeRGB (1998).
 *
 * Return value: the AdobeRGB-compatible #GimpColorProfile.
 *
 * Since: 2.10
 **/
GimpColorProfile *
gimp_color_profile_new_adobe_rgb (void)
{
  static GimpColorProfile *profile = NULL;

  const guint8 *data;
  gsize         length;

  if (G_UNLIKELY (profile == NULL))
    {
      cmsHPROFILE lcms_profile = gimp_color_profile_new_adobe_rgb_internal ();

      profile = gimp_color_profile_new_from_lcms_profile (lcms_profile, NULL);

      cmsCloseProfile (lcms_profile);
    }

  data = gimp_color_profile_get_icc_profile (profile, &length);

  return gimp_color_profile_new_from_icc_profile (data, length, NULL);
}

/**
 * gimp_color_profile_get_format:
 * @format:      a #Babl format
 * @lcms_format: return location for an lcms format
 *
 * This function takes a #Babl format and returns the lcms format to
 * be used with that @format. It also returns a #Babl format to be
 * used instead of the passed @format, which usually is the same as
 * @format, unless lcms doesn't support @format.
 *
 * Note that this function currently only supports RGB, RGBA, R'G'B',
 * R'G'B'A and the cairo-RGB24 and cairo-ARGB32 formats.
 *
 * Return value: the #Babl format to be used instead of @format, or %NULL
 *               is the passed @format is not supported at all.
 *
 * Since: 2.10
 **/
const Babl *
gimp_color_profile_get_format (const Babl *format,
                               guint32    *lcms_format)
{
  const Babl *output_format = NULL;
  const Babl *type;
  const Babl *model;
  gboolean    has_alpha;
  gboolean    linear;

  g_return_val_if_fail (format != NULL, NULL);
  g_return_val_if_fail (lcms_format != NULL, NULL);

  has_alpha = babl_format_has_alpha (format);
  type      = babl_format_get_type (format, 0);
  model     = babl_format_get_model (format);

  if (format == babl_format ("cairo-RGB24"))
    {
      *lcms_format = TYPE_RGB_8;

      return babl_format ("R'G'B' u8");
    }
  else if (format == babl_format ("cairo-ARGB32"))
    {
      *lcms_format = TYPE_RGBA_8;

      return babl_format ("R'G'B'A u8");
    }
  else if (model == babl_model ("RGB") ||
           model == babl_model ("RGBA"))
    {
      linear = TRUE;
    }
  else if (model == babl_model ("R'G'B'") ||
           model == babl_model ("R'G'B'A"))
    {
      linear = FALSE;
    }
  else
    {
      g_printerr ("format: %s\n"
                  "has_alpha = %s\n"
                  "type = %s\n"
                  "model = %s\n",
                  babl_get_name (format),
                  has_alpha ? "TRUE" : "FALSE",
                  babl_get_name (type),
                  babl_get_name (model));
      g_return_val_if_reached (NULL);
    }

  *lcms_format = 0;

  if (type == babl_type ("u8"))
    {
      if (has_alpha)
        *lcms_format = TYPE_RGBA_8;
      else
        *lcms_format = TYPE_RGB_8;

      output_format = format;
    }
  else if (type == babl_type ("u16"))
    {
      if (has_alpha)
        *lcms_format = TYPE_RGBA_16;
      else
        *lcms_format = TYPE_RGB_16;

      output_format = format;
    }
  else if (type == babl_type ("half")) /* 16-bit floating point (half) */
    {
      if (has_alpha)
        *lcms_format = TYPE_RGBA_HALF_FLT;
      else
        *lcms_format = TYPE_RGB_HALF_FLT;

      output_format = format;
    }
  else if (type == babl_type ("float"))
    {
      if (has_alpha)
        *lcms_format = TYPE_RGBA_FLT;
      else
        *lcms_format = TYPE_RGB_FLT;

      output_format = format;
    }
  else if (type == babl_type ("double"))
    {
      if (has_alpha)
        {
#ifdef TYPE_RGBA_DBL
          /* RGBA double not implemented in lcms */
          *lcms_format = TYPE_RGBA_DBL;
          output_format = format;
#endif /* TYPE_RGBA_DBL */
        }
      else
        {
          *lcms_format = TYPE_RGB_DBL;
          output_format = format;
        }
    }

  if (*lcms_format == 0)
    {
      g_printerr ("%s: layer format %s not supported, "
                  "falling back to float\n",
                  G_STRFUNC, babl_get_name (format));

      if (has_alpha)
        {
          *lcms_format = TYPE_RGBA_FLT;

          if (linear)
            output_format = babl_format ("RGBA float");
          else
            output_format = babl_format ("R'G'B'A float");
        }
      else
        {
          *lcms_format = TYPE_RGB_FLT;

          if (linear)
            output_format = babl_format ("RGB float");
          else
            output_format = babl_format ("R'G'B' float");
        }
    }

  return output_format;
}

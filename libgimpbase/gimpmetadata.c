/* LIBGIMPBASE - The GIMP Basic Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpmetadata.c
 * Copyright (C) 2013 Hartmut Kuhse <hartmutkuhse@src.gnome.org>
 *                    Michael Natterer <mitch@gimp.org>
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

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>
#include <gexiv2/gexiv2.h>

#include "libgimpmath/gimpmath.h"

#include "gimpbasetypes.h"

#include "gimplimits.h"
#include "gimpmetadata.h"
#include "gimpunit.h"

#include "libgimp/libgimp-intl.h"


/**
 * SECTION: gimpmetadata
 * @title: gimpmetadata
 * @short_description: Basic functions for handling #GimpMetadata objects.
 * @see_also: gimp_image_metadata_load_prepare(),
 *            gimp_image_metadata_load_finish(),
 *            gimp_image_metadata_load_prepare(),
 *            gimp_image_metadata_load_finish().
 *
 * Basic functions for handling #GimpMetadata objects.
 **/


static GQuark   gimp_metadata_error_quark (void);
static void     gimp_metadata_add         (GimpMetadata *src,
                                           GimpMetadata *dest);


static const gchar *tiff_tags[] =
{
  "Xmp.tiff",
  "Exif.Image.ImageWidth",
  "Exif.Image.ImageLength",
  "Exif.Image.BitsPerSample",
  "Exif.Image.Compression",
  "Exif.Image.PhotometricInterpretation",
  "Exif.Image.FillOrder",
  "Exif.Image.SamplesPerPixel",
  "Exif.Image.StripOffsets",
  "Exif.Image.RowsPerStrip",
  "Exif.Image.StripByteCounts",
  "Exif.Image.PlanarConfiguration"
};

static const gchar *jpeg_tags[] =
{
  "Exif.Image.JPEGProc",
  "Exif.Image.JPEGInterchangeFormat",
  "Exif.Image.JPEGInterchangeFormatLength",
  "Exif.Image.JPEGRestartInterval",
  "Exif.Image.JPEGLosslessPredictors",
  "Exif.Image.JPEGPointTransforms",
  "Exif.Image.JPEGQTables",
  "Exif.Image.JPEGDCTables",
  "Exif.Image.JPEGACTables"
};

static const gchar *unsupported_tags[] =
{
  "Exif.Image.SubIFDs",
  "Exif.Image.ClipPath",
  "Exif.Image.XClipPathUnits",
  "Exif.Image.YClipPathUnits",
  "Xmp.xmpMM.History",
  "Exif.Image.XPTitle",
  "Exif.Image.XPComment",
  "Exif.Image.XPAuthor",
  "Exif.Image.XPKeywords",
  "Exif.Image.XPSubject",
  "Exif.Image.DNGVersion",
  "Exif.Image.DNGBackwardVersion",
  "Exif.Iop"
};

static const guint8 minimal_exif[] =
{
  0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x00, 0xff, 0xe1
};

static const guint8 wilber_jpg[] =
{
  0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x43,
  0x00, 0x50, 0x37, 0x3c, 0x46, 0x3c, 0x32, 0x50, 0x46, 0x41, 0x46, 0x5a,
  0x55, 0x50, 0x5f, 0x78, 0xc8, 0x82, 0x78, 0x6e, 0x6e, 0x78, 0xf5, 0xaf,
  0xb9, 0x91, 0xc8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdb, 0x00, 0x43, 0x01, 0x55, 0x5a,
  0x5a, 0x78, 0x69, 0x78, 0xeb, 0x82, 0x82, 0xeb, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xc0, 0x00, 0x11, 0x08, 0x00, 0x10, 0x00, 0x10, 0x03,
  0x01, 0x22, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xff, 0xc4, 0x00,
  0x16, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x02, 0xff, 0xc4, 0x00,
  0x1e, 0x10, 0x00, 0x01, 0x05, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x03, 0x11, 0x31,
  0x04, 0x12, 0x51, 0x61, 0x71, 0xff, 0xc4, 0x00, 0x14, 0x01, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xff, 0xc4, 0x00, 0x14, 0x11, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0xda, 0x00, 0x0c, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11,
  0x00, 0x3f, 0x00, 0x18, 0xa0, 0x0e, 0x6d, 0xbc, 0xf5, 0xca, 0xf7, 0x78,
  0xb6, 0xfe, 0x3b, 0x23, 0xb2, 0x1d, 0x64, 0x68, 0xf0, 0x8a, 0x39, 0x4b,
  0x74, 0x9c, 0xa5, 0x5f, 0x35, 0x8a, 0xb2, 0x7e, 0xa0, 0xff, 0xd9, 0x00
};

static const guint wilber_jpg_len = G_N_ELEMENTS (wilber_jpg);


/**
 * gimp_metadata_new:
 *
 * Creates a new #GimpMetadata instance.
 *
 * Return value: The new #GimpMetadata.
 *
 * Since: 2.10
 */
GimpMetadata *
gimp_metadata_new (void)
{
  GExiv2Metadata *metadata = NULL;

  if (gexiv2_initialize ())
    {
      metadata = gexiv2_metadata_new ();

      if (! gexiv2_metadata_open_buf (metadata, wilber_jpg, wilber_jpg_len,
                                      NULL))
        {
          g_object_unref (metadata);

          return NULL;
        }
    }

  return metadata;
}

/**
 * gimp_metadata_duplicate:
 * @metadata: The object to duplicate, or %NULL.
 *
 * Duplicates a #GimpMetadata instance.
 *
 * Return value: The new #GimpMetadata, or %NULL if @metadata is %NULL.
 *
 * Since: 2.10
 */
GimpMetadata *
gimp_metadata_duplicate (GimpMetadata *metadata)
{
  GimpMetadata *new_metadata = NULL;

  g_return_val_if_fail (metadata == NULL || GEXIV2_IS_METADATA (metadata), NULL);

  if (metadata)
    {
      gchar *xml;

      xml = gimp_metadata_serialize (metadata);
      new_metadata = gimp_metadata_deserialize (xml);
      g_free (xml);
    }

  return new_metadata;
}

typedef struct
{
  gchar         name[1024];
  gboolean      base64;
  GimpMetadata *metadata;
} GimpMetadataParseData;

static const gchar*
gimp_metadata_attribute_name_to_value (const gchar **attribute_names,
                                       const gchar **attribute_values,
                                       const gchar  *name)
{
  while (*attribute_names)
    {
      if (! strcmp (*attribute_names, name))
        {
          return *attribute_values;
        }

      attribute_names++;
      attribute_values++;
    }

  return NULL;
}

static void
gimp_metadata_deserialize_start_element (GMarkupParseContext *context,
                                         const gchar         *element_name,
                                         const gchar        **attribute_names,
                                         const gchar        **attribute_values,
                                         gpointer             user_data,
                                         GError             **error)
{
  GimpMetadataParseData *parse_data = user_data;

  if (! strcmp (element_name, "tag"))
    {
      const gchar *name;
      const gchar *encoding;

      name = gimp_metadata_attribute_name_to_value (attribute_names,
                                                    attribute_values,
                                                    "name");
      encoding = gimp_metadata_attribute_name_to_value (attribute_names,
                                                        attribute_values,
                                                        "encoding");

      if (! name)
        {
          g_set_error (error, gimp_metadata_error_quark (), 1001,
                       "Element 'tag' does not contain required attribute 'name'.");
          return;
        }

      strncpy (parse_data->name, name, sizeof (parse_data->name));
      parse_data->name[sizeof (parse_data->name) - 1] = 0;

      parse_data->base64 = (encoding && ! strcmp (encoding, "base64"));
    }
}

static void
gimp_metadata_deserialize_end_element (GMarkupParseContext *context,
                                       const gchar         *element_name,
                                       gpointer             user_data,
                                       GError             **error)
{
}

static void
gimp_metadata_deserialize_text (GMarkupParseContext  *context,
                                const gchar          *text,
                                gsize                 text_len,
                                gpointer              user_data,
                                GError              **error)
{
  GimpMetadataParseData *parse_data = user_data;
  const gchar           *current_element;

  current_element = g_markup_parse_context_get_element (context);

  if (! g_strcmp0 (current_element, "tag"))
    {
      gchar *value = g_strndup (text, text_len);

      if (parse_data->base64)
        {
          guchar *decoded;
          gsize   len;

          decoded = g_base64_decode (value, &len);

          if (decoded[len - 1] == '\0')
            gexiv2_metadata_set_tag_string (parse_data->metadata,
                                            parse_data->name,
                                            (const gchar *) decoded);

          g_free (decoded);
        }
      else
        {
          gexiv2_metadata_set_tag_string (parse_data->metadata,
                                          parse_data->name,
                                          value);
        }

      g_free (value);
    }
}

static  void
gimp_metadata_deserialize_error (GMarkupParseContext *context,
                                 GError              *error,
                                 gpointer             user_data)
{
  g_printerr ("Metadata parse error: %s\n", error->message);
}

/**
 * gimp_metadata_deserialize:
 * @metadata_xml: A string of serialized metadata XML.
 *
 * Deserializes a string of XML that has been created by
 * gimp_metadata_serialize().
 *
 * Return value: The new #GimpMetadata.
 *
 * Since: 2.10
 */
GimpMetadata *
gimp_metadata_deserialize (const gchar *metadata_xml)
{
  GimpMetadata          *metadata;
  GMarkupParser          markup_parser;
  GimpMetadataParseData  parse_data;
  GMarkupParseContext   *context;

  g_return_val_if_fail (metadata_xml != NULL, NULL);

  metadata = gimp_metadata_new ();

  parse_data.metadata = metadata;

  markup_parser.start_element = gimp_metadata_deserialize_start_element;
  markup_parser.end_element   = gimp_metadata_deserialize_end_element;
  markup_parser.text          = gimp_metadata_deserialize_text;
  markup_parser.passthrough   = NULL;
  markup_parser.error         = gimp_metadata_deserialize_error;

  context = g_markup_parse_context_new (&markup_parser, 0, &parse_data, NULL);

  g_markup_parse_context_parse (context,
                                metadata_xml, strlen (metadata_xml),
                                NULL);

  g_markup_parse_context_unref (context);

  return metadata;
}

static gchar *
gimp_metadata_escape (const gchar *name,
                      const gchar *value,
                      gboolean    *base64)
{
  if (! g_utf8_validate (value, -1, NULL))
    {
      gchar *encoded;

      encoded = g_base64_encode ((const guchar *) value, strlen (value) + 1);

      g_printerr ("Invalid UTF-8 in metadata value %s, encoding as base64: %s\n",
                  name, encoded);

      *base64 = TRUE;

      return encoded;
    }

  *base64 = FALSE;

  return g_markup_escape_text (value, -1);
}

static void
gimp_metadata_append_tag (GString     *string,
                          const gchar *name,
                          gchar       *value,
                          gboolean     base64)
{
  if (value)
    {
      if (base64)
        {
          g_string_append_printf (string, "  <tag name=\"%s\" encoding=\"base64\">%s</tag>\n",
                                  name, value);
        }
      else
        {
          g_string_append_printf (string, "  <tag name=\"%s\">%s</tag>\n",
                                  name, value);
        }

      g_free (value);
    }
}

/**
 * gimp_metadata_serialize:
 * @metadata: A #GimpMetadata instance.
 *
 * Serializes @metadata into an XML string that can later be deserialized
 * using gimp_metadata_deserialize().
 *
 * Return value: The serialized XML string.
 *
 * Since: 2.10
 */
gchar *
gimp_metadata_serialize (GimpMetadata *metadata)
{
  GString  *string;
  gchar   **exif_data = NULL;
  gchar   **iptc_data = NULL;
  gchar   **xmp_data  = NULL;
  gchar    *value;
  gchar    *escaped;
  gboolean  base64;
  gint      i;

  g_return_val_if_fail (GEXIV2_IS_METADATA (metadata), NULL);

  string = g_string_new (NULL);

  g_string_append (string, "<?xml version='1.0' encoding='UTF-8'?>\n");
  g_string_append (string, "<metadata>\n");

  exif_data = gexiv2_metadata_get_exif_tags (metadata);

  if (exif_data)
    {
      for (i = 0; exif_data[i] != NULL; i++)
        {
          value = gexiv2_metadata_get_tag_string (metadata, exif_data[i]);
          escaped = gimp_metadata_escape (exif_data[i], value, &base64);
          g_free (value);

          gimp_metadata_append_tag (string, exif_data[i], escaped, base64);
        }

      g_strfreev (exif_data);
    }

  xmp_data = gexiv2_metadata_get_xmp_tags (metadata);

  if (xmp_data)
    {
      for (i = 0; xmp_data[i] != NULL; i++)
        {
          value = gexiv2_metadata_get_tag_string (metadata, xmp_data[i]);
          escaped = gimp_metadata_escape (xmp_data[i], value, &base64);
          g_free (value);

          gimp_metadata_append_tag (string, xmp_data[i], escaped, base64);
        }

      g_strfreev (xmp_data);
    }

  iptc_data = gexiv2_metadata_get_iptc_tags (metadata);

  if (iptc_data)
    {
      for (i = 0; iptc_data[i] != NULL; i++)
        {
          value = gexiv2_metadata_get_tag_string (metadata, iptc_data[i]);
          escaped = gimp_metadata_escape (iptc_data[i], value, &base64);
          g_free (value);

          gimp_metadata_append_tag (string, iptc_data[i], escaped, base64);
        }

      g_strfreev (iptc_data);
    }

  g_string_append (string, "</metadata>\n");

  return g_string_free (string, FALSE);
}

/**
 * gimp_metadata_load_from_file:
 * @file:  The #GFile to load the metadata from
 * @error: Return location for error message
 *
 * Loads #GimpMetadata from @file.
 *
 * Return value: The loaded #GimpMetadata.
 *
 * Since: 2.10
 */
GimpMetadata  *
gimp_metadata_load_from_file (GFile   *file,
                              GError **error)
{
  GExiv2Metadata *meta = NULL;
  gchar          *path;
  gchar          *filename;

  g_return_val_if_fail (G_IS_FILE (file), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  path = g_file_get_path (file);

  if (! path)
    {
      g_set_error (error, gimp_metadata_error_quark (), 0,
                   _("Can load metadata only from local files"));
      return NULL;
    }

#ifdef G_OS_WIN32
  filename = g_win32_locale_filename_from_utf8 (path);
#else
  filename = g_strdup (path);
#endif

  g_free (path);

  if (gexiv2_initialize ())
    {
      meta = gexiv2_metadata_new ();

      if (! gexiv2_metadata_open_path (meta, filename, error))
        {
          g_object_unref (meta);
          g_free (filename);

          return NULL;
        }
    }

  g_free (filename);

  return meta;
}

/**
 * gimp_metadata_save_to_file:
 * @metadata: A #GimpMetadata instance.
 * @file:     The file to save the metadata to
 * @error:    Return location for error message
 *
 * Saves @metadata to @file.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
gimp_metadata_save_to_file (GimpMetadata  *metadata,
                            GFile         *file,
                            GError       **error)
{
  gchar    *path;
  gchar    *filename;
  gboolean  success;

  g_return_val_if_fail (GEXIV2_IS_METADATA (metadata), FALSE);
  g_return_val_if_fail (G_IS_FILE (file), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  path = g_file_get_path (file);

  if (! path)
    {
      g_set_error (error, gimp_metadata_error_quark (), 0,
                   _("Can save metadata only to local files"));
      return FALSE;
    }

#ifdef G_OS_WIN32
  filename = g_win32_locale_filename_from_utf8 (path);
#else
  filename = g_strdup (path);
#endif

  g_free (path);

  success = gexiv2_metadata_save_file (metadata, filename, error);

  g_free (filename);

  return success;
}

/**
 * gimp_metadata_set_from_exif:
 * @metadata:         A #GimpMetadata instance.
 * @exif_data:        The blob of Exif data to set
 * @exif_data_length: Length of @exif_data, in bytes
 * @error:            Return location for error message
 *
 * Sets the tags from a piece of Exif data on @metadata.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
gimp_metadata_set_from_exif (GimpMetadata  *metadata,
                             const guchar  *exif_data,
                             gint           exif_data_length,
                             GError       **error)
{

  GByteArray   *exif_bytes;
  GimpMetadata *exif_metadata;
  guint8        data_size[2] = { 0, };

  g_return_val_if_fail (GEXIV2_IS_METADATA (metadata), FALSE);
  g_return_val_if_fail (exif_data != NULL, FALSE);
  g_return_val_if_fail (exif_data_length > 0, FALSE);
  g_return_val_if_fail (exif_data_length < 65536, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  data_size[0] = (exif_data_length & 0xFF00) >> 8;
  data_size[1] = (exif_data_length & 0x00FF);

  exif_bytes = g_byte_array_new ();
  exif_bytes = g_byte_array_append (exif_bytes,
                                    minimal_exif, G_N_ELEMENTS (minimal_exif));
  exif_bytes = g_byte_array_append (exif_bytes,
                                    data_size, 2);
  exif_bytes = g_byte_array_append (exif_bytes,
                                    (guint8 *) exif_data, exif_data_length);

  exif_metadata = gimp_metadata_new ();

  if (! gexiv2_metadata_open_buf (exif_metadata,
                                  exif_bytes->data, exif_bytes->len, error))
    {
      g_object_unref (exif_metadata);
      g_byte_array_free (exif_bytes, TRUE);
      return FALSE;
    }

  if (! gexiv2_metadata_has_exif (exif_metadata))
    {
      g_set_error (error, gimp_metadata_error_quark (), 0,
                   _("Parsing Exif data failed."));
      g_object_unref (exif_metadata);
      g_byte_array_free (exif_bytes, TRUE);
      return FALSE;
    }

  gimp_metadata_add (exif_metadata, metadata);
  g_object_unref (exif_metadata);
  g_byte_array_free (exif_bytes, TRUE);

  return TRUE;
}

/**
 * gimp_metadata_set_from_xmp:
 * @metadata:        A #GimpMetadata instance.
 * @xmp_data:        The blob of Exif data to set
 * @xmp_data_length: Length of @exif_data, in bytes
 * @error:           Return location for error message
 *
 * Sets the tags from a piece of XMP data on @metadata.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
gimp_metadata_set_from_xmp (GimpMetadata  *metadata,
                            const guchar  *xmp_data,
                            gint           xmp_data_length,
                            GError       **error)
{
  GimpMetadata *xmp_metadata;

  g_return_val_if_fail (GEXIV2_IS_METADATA (metadata), FALSE);
  g_return_val_if_fail (xmp_data != NULL, FALSE);
  g_return_val_if_fail (xmp_data_length > 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  xmp_data        += 10;
  xmp_data_length -= 10;

  xmp_metadata = gimp_metadata_new ();

  if (! gexiv2_metadata_open_buf (xmp_metadata,
                                  xmp_data, xmp_data_length, error))
    {
      g_object_unref (xmp_metadata);
      return FALSE;
    }

  if (! gexiv2_metadata_has_xmp (xmp_metadata))
    {
      g_set_error (error, gimp_metadata_error_quark (), 0,
                   _("Parsing XMP data failed."));
      g_object_unref (xmp_metadata);
      return FALSE;
    }

  gimp_metadata_add (xmp_metadata, metadata);
  g_object_unref (xmp_metadata);

  return TRUE;
}

/**
 * gimp_metadata_set_pixel_size:
 * @metadata: A #GimpMetadata instance.
 * @width:    Width in pixels
 * @height:   Height in pixels
 *
 * Sets Exif.Image.ImageWidth and Exif.Image.ImageLength on @metadata.
 *
 * Since: 2.10
 */
void
gimp_metadata_set_pixel_size (GimpMetadata *metadata,
                              gint          width,
                              gint          height)
{
  gchar buffer[32];

  g_return_if_fail (GEXIV2_IS_METADATA (metadata));

  g_snprintf (buffer, sizeof (buffer), "%d", width);
  gexiv2_metadata_set_tag_string (metadata, "Exif.Image.ImageWidth", buffer);

  g_snprintf (buffer, sizeof (buffer), "%d", height);
  gexiv2_metadata_set_tag_string (metadata, "Exif.Image.ImageLength", buffer);
}

/**
 * gimp_metadata_set_bits_per_sample:
 * @metadata:        A #GimpMetadata instance.
 * @bits_per_sample: Bits per pixel, per component
 *
 * Sets Exif.Image.BitsPerSample on @metadata.
 *
 * Since: 2.10
 */
void
gimp_metadata_set_bits_per_sample (GimpMetadata *metadata,
                                   gint          bits_per_sample)
{
  gchar buffer[32];

  g_return_if_fail (GEXIV2_IS_METADATA (metadata));

  g_snprintf (buffer, sizeof (buffer), "%d %d %d",
              bits_per_sample, bits_per_sample, bits_per_sample);
  gexiv2_metadata_set_tag_string (metadata, "Exif.Image.BitsPerSample", buffer);
}

/**
 * gimp_metadata_get_resolution:
 * @metadata: A #GimpMetadata instance.
 * @xres:     Return location for the X Resolution, in ppi
 * @yres:     Return location for the Y Resolution, in ppi
 * @unit:     Return location for the unit unit
 *
 * Returns values based on Exif.Image.XResolution,
 * Exif.Image.YResolution and Exif.Image.ResolutionUnit of @metadata.
 *
 * Return value: %TRUE on success, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
gimp_metadata_get_resolution (GimpMetadata *metadata,
                              gdouble      *xres,
                              gdouble      *yres,
                              GimpUnit     *unit)
{
  gint xnom, xdenom;
  gint ynom, ydenom;

  g_return_val_if_fail (GEXIV2_IS_METADATA (metadata), FALSE);

  if (gexiv2_metadata_get_exif_tag_rational (metadata,
                                             "Exif.Image.XResolution",
                                             &xnom, &xdenom) &&
      gexiv2_metadata_get_exif_tag_rational (metadata,
                                             "Exif.Image.YResolution",
                                             &ynom, &ydenom))
    {
      gchar *un;
      gint   exif_unit = 2;

      un = gexiv2_metadata_get_tag_string (metadata,
                                           "Exif.Image.ResolutionUnit");
      if (un)
        {
          exif_unit = atoi (un);
          g_free (un);
        }

      if (xnom != 0 && xdenom != 0 &&
          ynom != 0 && ydenom != 0)
        {
          gdouble xresolution = (gdouble) xnom / (gdouble) xdenom;
          gdouble yresolution = (gdouble) ynom / (gdouble) ydenom;

          if (exif_unit == 3)
            {
              xresolution *= 2.54;
              yresolution *= 2.54;
            }

         if (xresolution >= GIMP_MIN_RESOLUTION &&
             xresolution <= GIMP_MAX_RESOLUTION &&
             yresolution >= GIMP_MIN_RESOLUTION &&
             yresolution <= GIMP_MAX_RESOLUTION)
           {
             if (xres)
               *xres = xresolution;

             if (yres)
               *yres = yresolution;

             if (unit)
               {
                 if (exif_unit == 3)
                   *unit = GIMP_UNIT_MM;
                 else
                   *unit = GIMP_UNIT_INCH;
               }

             return TRUE;
           }
        }
    }

  return FALSE;
}

/**
 * gimp_metadata_set_resolution:
 * @metadata: A #GimpMetadata instance.
 * @xres:     The image's X Resolution, in ppi
 * @yres:     The image's Y Resolution, in ppi
 * @unit:     The image's unit
 *
 * Sets Exif.Image.XResolution, Exif.Image.YResolution and
 * Exif.Image.ResolutionUnit of @metadata.
 *
 * Since: 2.10
 */
void
gimp_metadata_set_resolution (GimpMetadata *metadata,
                              gdouble       xres,
                              gdouble       yres,
                              GimpUnit      unit)
{
  gchar buffer[32];
  gint  exif_unit;
  gint  factor;

  g_return_if_fail (GEXIV2_IS_METADATA (metadata));

  if (gimp_unit_is_metric (unit))
    {
      xres /= 2.54;
      yres /= 2.54;

      exif_unit = 3;
    }
  else
    {
      exif_unit = 2;
    }

  for (factor = 1; factor <= 100 /* arbitrary */; factor++)
    {
      if (fabs (xres * factor - ROUND (xres * factor)) < 0.01 &&
          fabs (yres * factor - ROUND (yres * factor)) < 0.01)
        break;
    }

  gexiv2_metadata_set_exif_tag_rational (metadata,
                                         "Exif.Image.XResolution",
                                         ROUND (xres * factor), factor);

  gexiv2_metadata_set_exif_tag_rational (metadata,
                                         "Exif.Image.YResolution",
                                         ROUND (yres * factor), factor);

  g_snprintf (buffer, sizeof (buffer), "%d", exif_unit);
  gexiv2_metadata_set_tag_string (metadata, "Exif.Image.ResolutionUnit", buffer);
}

/**
 * gimp_metadata_get_colorspace:
 * @metadata: A #GimpMetadata instance.
 *
 * Returns values based on Exif.Photo.ColorSpace, Xmp.exif.ColorSpace,
 * Exif.Iop.InteroperabilityIndex, Exif.Nikon3.ColorSpace,
 * Exif.Canon.ColorSpace of @metadata.
 *
 * Return value: The colorspace specified by above tags.
 *
 * Since: 2.10
 */
GimpMetadataColorspace
gimp_metadata_get_colorspace (GimpMetadata *metadata)
{
  glong exif_cs = -1;

  g_return_val_if_fail (GEXIV2_IS_METADATA (metadata),
                        GIMP_METADATA_COLORSPACE_UNSPECIFIED);

  /*  the logic here was mostly taken from darktable and libkexiv2  */

  if (gexiv2_metadata_has_tag (metadata, "Exif.Photo.ColorSpace"))
    {
      exif_cs = gexiv2_metadata_get_tag_long (metadata,
                                              "Exif.Photo.ColorSpace");
    }
  else if (gexiv2_metadata_has_tag (metadata, "Xmp.exif.ColorSpace"))
    {
      exif_cs = gexiv2_metadata_get_tag_long (metadata,
                                              "Xmp.exif.ColorSpace");
    }

  if (exif_cs == 0x01)
    {
      return GIMP_METADATA_COLORSPACE_SRGB;
    }
  else if (exif_cs == 0x02)
    {
      return GIMP_METADATA_COLORSPACE_ADOBERGB;
    }
  else
    {
      if (exif_cs == 0xffff)
        {
          gchar *iop_index;

          iop_index = gexiv2_metadata_get_tag_string (metadata,
                                                      "Exif.Iop.InteroperabilityIndex");

          if (! g_strcmp0 (iop_index, "R03"))
            {
              g_free (iop_index);

              return GIMP_METADATA_COLORSPACE_ADOBERGB;
            }
          else if (! g_strcmp0 (iop_index, "R98"))
            {
              g_free (iop_index);

              return GIMP_METADATA_COLORSPACE_SRGB;
            }

          g_free (iop_index);
        }

      if (gexiv2_metadata_has_tag (metadata, "Exif.Nikon3.ColorSpace"))
        {
          glong nikon_cs;

          nikon_cs = gexiv2_metadata_get_tag_long (metadata,
                                                   "Exif.Nikon3.ColorSpace");

          if (nikon_cs == 0x01)
            {
              return GIMP_METADATA_COLORSPACE_SRGB;
            }
          else if (nikon_cs == 0x02)
            {
              return GIMP_METADATA_COLORSPACE_ADOBERGB;
            }
        }

      if (gexiv2_metadata_has_tag (metadata, "Exif.Canon.ColorSpace"))
        {
          glong canon_cs;

          canon_cs = gexiv2_metadata_get_tag_long (metadata,
                                                   "Exif.Canon.ColorSpace");

          if (canon_cs == 0x01)
            {
              return GIMP_METADATA_COLORSPACE_SRGB;
            }
          else if (canon_cs == 0x02)
            {
              return GIMP_METADATA_COLORSPACE_ADOBERGB;
            }
        }

      if (exif_cs == 0xffff)
        return GIMP_METADATA_COLORSPACE_UNCALIBRATED;
    }

  return GIMP_METADATA_COLORSPACE_UNSPECIFIED;
}

/**
 * gimp_metadata_set_colorspace:
 * @metadata:   A #GimpMetadata instance.
 * @colorspace: The color space.
 *
 * Sets Exif.Photo.ColorSpace, Xmp.exif.ColorSpace,
 * Exif.Iop.InteroperabilityIndex, Exif.Nikon3.ColorSpace,
 * Exif.Canon.ColorSpace of @metadata.
 *
 * Since: 2.10
 */
void
gimp_metadata_set_colorspace (GimpMetadata           *metadata,
                              GimpMetadataColorspace  colorspace)
{
  g_return_if_fail (GEXIV2_IS_METADATA (metadata));

  switch (colorspace)
    {
    case GIMP_METADATA_COLORSPACE_UNSPECIFIED:
      gexiv2_metadata_clear_tag (metadata, "Exif.Photo.ColorSpace");
      gexiv2_metadata_clear_tag (metadata, "Xmp.exif.ColorSpace");
      gexiv2_metadata_clear_tag (metadata, "Exif.Iop.InteroperabilityIndex");
      gexiv2_metadata_clear_tag (metadata, "Exif.Nikon3.ColorSpace");
      gexiv2_metadata_clear_tag (metadata, "Exif.Canon.ColorSpace");
      break;

    case GIMP_METADATA_COLORSPACE_UNCALIBRATED:
      gexiv2_metadata_set_tag_long (metadata, "Exif.Photo.ColorSpace", 0xffff);
      if (gexiv2_metadata_has_tag (metadata, "Xmp.exif.ColorSpace"))
        gexiv2_metadata_set_tag_long (metadata, "Xmp.exif.ColorSpace", 0xffff);
      gexiv2_metadata_clear_tag (metadata, "Exif.Iop.InteroperabilityIndex");
      gexiv2_metadata_clear_tag (metadata, "Exif.Nikon3.ColorSpace");
      gexiv2_metadata_clear_tag (metadata, "Exif.Canon.ColorSpace");
      break;

    case GIMP_METADATA_COLORSPACE_SRGB:
      gexiv2_metadata_set_tag_long (metadata, "Exif.Photo.ColorSpace", 0x01);

      if (gexiv2_metadata_has_tag (metadata, "Xmp.exif.ColorSpace"))
        gexiv2_metadata_set_tag_long (metadata, "Xmp.exif.ColorSpace", 0x01);

      if (gexiv2_metadata_has_tag (metadata, "Exif.Iop.InteroperabilityIndex"))
        gexiv2_metadata_set_tag_string (metadata,
                                        "Exif.Iop.InteroperabilityIndex", "R98");

      if (gexiv2_metadata_has_tag (metadata, "Exif.Nikon3.ColorSpace"))
        gexiv2_metadata_set_tag_long (metadata, "Exif.Nikon3.ColorSpace", 0x01);

      if (gexiv2_metadata_has_tag (metadata, "Exif.Canon.ColorSpace"))
        gexiv2_metadata_set_tag_long (metadata, "Exif.Canon.ColorSpace", 0x01);
      break;

    case GIMP_METADATA_COLORSPACE_ADOBERGB:
      gexiv2_metadata_set_tag_long (metadata, "Exif.Photo.ColorSpace", 0x02);

      if (gexiv2_metadata_has_tag (metadata, "Xmp.exif.ColorSpace"))
        gexiv2_metadata_set_tag_long (metadata, "Xmp.exif.ColorSpace", 0x02);

      if (gexiv2_metadata_has_tag (metadata, "Exif.Iop.InteroperabilityIndex"))
        gexiv2_metadata_set_tag_string (metadata,
                                        "Exif.Iop.InteroperabilityIndex", "R03");

      if (gexiv2_metadata_has_tag (metadata, "Exif.Nikon3.ColorSpace"))
        gexiv2_metadata_set_tag_long (metadata, "Exif.Nikon3.ColorSpace", 0x02);

      if (gexiv2_metadata_has_tag (metadata, "Exif.Canon.ColorSpace"))
        gexiv2_metadata_set_tag_long (metadata, "Exif.Canon.ColorSpace", 0x02);
      break;
    }
}

/**
 * gimp_metadata_is_tag_supported:
 * @tag:       A metadata tag name
 * @mime_type: A mime type
 *
 * Returns whether @tag is supported in a file of type @mime_type.
 *
 * Return value: %TRUE if the @tag supported with @mime_type, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
gimp_metadata_is_tag_supported (const gchar *tag,
                                const gchar *mime_type)
{
  gint j;

  g_return_val_if_fail (tag != NULL, FALSE);
  g_return_val_if_fail (mime_type != NULL, FALSE);

  for (j = 0; j < G_N_ELEMENTS (unsupported_tags); j++)
    {
      if (g_str_has_prefix (tag, unsupported_tags[j]))
        {
          return FALSE;
        }
    }

  if (! strcmp (mime_type, "image/jpeg"))
    {
      for (j = 0; j < G_N_ELEMENTS (tiff_tags); j++)
        {
          if (g_str_has_prefix (tag, tiff_tags[j]))
            {
              return FALSE;
            }
        }
    }
  else if (! strcmp (mime_type, "image/tiff"))
    {
      for (j = 0; j < G_N_ELEMENTS (jpeg_tags); j++)
        {
          if (g_str_has_prefix (tag, jpeg_tags[j]))
            {
              return FALSE;
            }
        }
    }

  return TRUE;
}


/* private functions */

static GQuark
gimp_metadata_error_quark (void)
{
  static GQuark quark = 0;

  if (G_UNLIKELY (quark == 0))
    quark = g_quark_from_static_string ("gimp-metadata-error-quark");

  return quark;
}

static void
gimp_metadata_add (GimpMetadata *src,
                   GimpMetadata *dest)
{
  gchar *value;
  gint   i;

  if (gexiv2_metadata_get_supports_exif (src) &&
      gexiv2_metadata_get_supports_exif (dest))
    {
      gchar **exif_data = gexiv2_metadata_get_exif_tags (src);

      if (exif_data)
        {
          for (i = 0; exif_data[i] != NULL; i++)
            {
              value = gexiv2_metadata_get_tag_string (src, exif_data[i]);
              gexiv2_metadata_set_tag_string (dest, exif_data[i], value);
              g_free (value);
            }

          g_strfreev (exif_data);
        }
    }

  if (gexiv2_metadata_get_supports_xmp (src) &&
      gexiv2_metadata_get_supports_xmp (dest))
    {
      gchar **xmp_data = gexiv2_metadata_get_xmp_tags (src);

      if (xmp_data)
        {
          for (i = 0; xmp_data[i] != NULL; i++)
            {
              value = gexiv2_metadata_get_tag_string (src, xmp_data[i]);
              gexiv2_metadata_set_tag_string (dest, xmp_data[i], value);
              g_free (value);
            }

          g_strfreev (xmp_data);
        }
    }

  if (gexiv2_metadata_get_supports_iptc (src) &&
      gexiv2_metadata_get_supports_iptc (dest))
    {
      gchar **iptc_data = gexiv2_metadata_get_iptc_tags (src);

      if (iptc_data)
        {
          for (i = 0; iptc_data[i] != NULL; i++)
            {
              value = gexiv2_metadata_get_tag_string (src, iptc_data[i]);
              gexiv2_metadata_set_tag_string (dest, iptc_data[i], value);
              g_free (value);
            }

          g_strfreev (iptc_data);
        }
    }
}

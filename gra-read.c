/* graread.c    reads any bitmap I could get for testing */
/* Alexander.Schulz@stud.uni-karlsruhe.de                */

/*
 * GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 */

#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include <glib/gstdio.h>

#include <libgimp/gimp.h>

#include "gra.h"

#include "compression.h"

#define DCF_COMPRESSED  0x01
#define DCF_PALETTE     0x02 //TODO: Implement this

gint32
ReadGRA (const gchar  *name,
         GError      **error)
{
    // My files
    FILE            *fd;
    gint            width, width_internal, height, flags;
    guchar          *body;
    long            original, body_size;
    GimpPixelRgn    pixel_rgn;
    GimpDrawable    *drawable;

  // My code
  filename = name;
  fd = g_fopen (filename, "rb");

  if (!fd)
    {
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Could not open '%s' for reading: %s",
                   gimp_filename_to_utf8 (filename), g_strerror (errno));
      goto out;
    }

  gimp_progress_init_printf ("Opening '%s'",
                             gimp_filename_to_utf8 (name));

  //TODO: If this is a GRA.Z file, do a round of decompression first

  if (!ReadOK(fd, &width, 4)){
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Error reading width",
                   gimp_filename_to_utf8 (filename), g_strerror (errno));
      goto out;
  }

  if (!ReadOK(fd, &width_internal, 4)){
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Error reading width_internal",
                   gimp_filename_to_utf8 (filename), g_strerror (errno));
      goto out;
  }

  if (!ReadOK(fd, &height, 4)){
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Error reading height",
                   gimp_filename_to_utf8 (filename), g_strerror (errno));
      goto out;
  }

  if (!ReadOK(fd, &flags, 4)){
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Error reading flags",
                   gimp_filename_to_utf8 (filename), g_strerror (errno));
      goto out;
  }

  // Get size of body
    original=ftell(fd);
    fseek(fd,0,SEEK_END);
    body_size = ftell(fd) - original;
    fseek(fd,original,SEEK_SET);

    printf("body_size is %d\n", body_size);

    // Allocate memory for the file
    body = (guchar*) malloc (sizeof(guchar)*body_size);
    if (!ReadOK(fd, body, body_size)){
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                   "Error reading body bytes",
                   gimp_filename_to_utf8 (filename), g_strerror (errno));
      goto out;
    }

    //TODO: Check flags to see if data is compressed before we compress here
    // Uncompress body data

    
    //int i;
    //for (i = 0; i < 20; i++){
        //printf ("%x ", body[i]);
    //}
    
    if (flags & DCF_COMPRESSED){
        guchar * decompressed_body;
        uncompress(body, body_size, &decompressed_body);
        free(body);
        body = decompressed_body;
    }

    // Create color map
    guchar color_map[3*16]; //TODO: Add transparency here?

    int col_index = 0;
    // BLACK
    color_map[col_index++] =  0x00;
    color_map[col_index++] =  0x00;
    color_map[col_index++] =  0x00;
    // BLUE
    color_map[col_index++] =  0x00;
    color_map[col_index++] =  0x00;
    color_map[col_index++] =  0xAA;
    // GREEN
    color_map[col_index++] =  0x00;
    color_map[col_index++] =  0xAA;
    color_map[col_index++] =  0x00;
    // CYAN
    color_map[col_index++] =  0x00;
    color_map[col_index++] = 0xAA;
    color_map[col_index++] = 0xAA;
    // RED
    color_map[col_index++] = 0xAA;
    color_map[col_index++] = 0x00;
    color_map[col_index++] = 0x00;
    // PURPLE
    color_map[col_index++] = 0xAA;
    color_map[col_index++] = 0x00;
    color_map[col_index++] = 0xAA;
    // BROWN
    color_map[col_index++] = 0xAA;
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0x00;
    // LTGRAY
    color_map[col_index++] = 0xAA;
    color_map[col_index++] = 0xAA;
    color_map[col_index++] = 0xAA;
    // DKGRAY
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0x55;
    // LTBLUE
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0xFF;
    // LTGREEN
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0xFF;
    color_map[col_index++] = 0x55;
    // LTCYAN
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0xFF;
    color_map[col_index++] = 0xFF;
    // LTRED
    color_map[col_index++] = 0xFF;
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0x55;
    // LTPURPLE
    color_map[col_index++] = 0xFF;
    color_map[col_index++] = 0x55;
    color_map[col_index++] = 0xFF;
    // YELLOW
    color_map[col_index++] = 0xFF;
    color_map[col_index++] = 0xFF;
    color_map[col_index++] = 0x55;
    // WHITE
    color_map[col_index++] = 0xFF;
    color_map[col_index++] = 0xFF;
    color_map[col_index++] = 0xFF;

  gint32 image = gimp_image_new (width, height, GIMP_INDEXED); // Assuming base_type is indexed

  gint32 layer = gimp_layer_new (image, "Background",
                          width, height,
                          GIMP_INDEXED_IMAGE, 100, GIMP_NORMAL_MODE); // Assuming image type is indexed

  gimp_image_set_filename (image, filename);

  gimp_image_insert_layer (image, layer, -1, 0);
  drawable = gimp_drawable_get (layer);

  //gimp_progress_update (1.0); // TODO: Add progress updates throughout process

  gimp_pixel_rgn_init (&pixel_rgn, drawable,
                       0, 0, drawable->width, drawable->height, TRUE, FALSE);
  
  gimp_pixel_rgn_set_rect (&pixel_rgn, body,
                           0, 0, drawable->width, drawable->height);

  gimp_image_set_colormap (image, color_map, 16); // no. of cols = 16

  gimp_drawable_flush(drawable);
  gimp_drawable_detach(drawable);
  g_free(body);

out:
  if (fd)
      fclose (fd);

  // Set the resolution
  return image;
}

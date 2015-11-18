/*
 * gra-read.c    reads .GRA files 
 * Uses bmp-read.c for inspiration, originally made by:  
 * Alexander.Schulz@stud.uni-karlsruhe.de               
 *
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

gint32 ReadGRA (const gchar *name, GError **error)
{
    // My files
    FILE            *fd;
    gint            width, width_internal, height, flags;
    guchar          *body;
    long            original, body_size;
    GimpPixelRgn    pixel_rgn;
    GimpDrawable    *drawable;
    guchar          color_map[3*16];
    gint32          image;
    gint32          layer;
    guchar          *alpha_body;
    int             i;

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

    if (!ReadOK(fd, &width, 4)){
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Error reading width");
        goto out;
    }

    if (!ReadOK(fd, &width_internal, 4)){
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Error reading width_internal");
        goto out;
    }

    if (!ReadOK(fd, &height, 4)){
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Error reading height");
        goto out;
    }

    if (!ReadOK(fd, &flags, 4)){
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Error reading flags");
        goto out;
    }

    // Get size of body
    original=ftell(fd);
    fseek(fd,0,SEEK_END);
    body_size = ftell(fd) - original;
    fseek(fd,original,SEEK_SET);

    // Allocate memory for the file
    body = (guchar*) malloc (body_size);
    if (!ReadOK(fd, body, body_size)){
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Error reading body bytes");
        goto out;
    }

    // Uncompress body data
    if (flags & DCF_COMPRESSED){
        guchar * decompressed_body;
        decompress(body, body_size, &decompressed_body);
        free(body);
        body = decompressed_body;
    }

    alpha_body = (guchar *) malloc(width * height * 2);
    guchar alpha_value;
    for (i = 0; i < width * height; i++){
        // Set first byte to the colour
        alpha_body[2*i] = body[i] & 0x0F;

        // Get the alpha value, scale it to a fraction out of 256, set this as the second byte
        alpha_value = 0xFF - (body[i] & 0xF0); // GIMP's alpha scale is the opposite of TempleOS'
        alpha_body[2*i + 1] = alpha_value | (alpha_value >> 1);
    }
    free(body);
    body = alpha_body;

    get_color_map(&color_map);

    image = gimp_image_new (width, height, GIMP_INDEXED); // Assuming base_type is indexed

    layer = gimp_layer_new (image, "Background",
            width, height,
            GIMP_INDEXEDA_IMAGE, 100, GIMP_NORMAL_MODE); // Assuming image type is indexed

    gimp_image_set_filename (image, filename);

    gimp_image_insert_layer (image, layer, -1, 0);
    drawable = gimp_drawable_get (layer);

    gimp_pixel_rgn_init (&pixel_rgn, drawable,
            0, 0, drawable->width, drawable->height, TRUE, FALSE);

    gimp_pixel_rgn_set_rect (&pixel_rgn, body,
            0, 0, drawable->width, drawable->height);

    if (!gimp_context_set_palette(PALETTE_NAME)){
        // Not a breaking error but something's wrong with the plugin
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Couldn't find palette. Re-install plugin.");
    }
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

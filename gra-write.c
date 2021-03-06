/* gra-write.c   Writes TempleOS GRA files.                      */
/* Created by Michael Barlow (michaelbarlow7@gmail.com)         */
/* Heavily inspired by BMP writing code by Alexander Schulz     */

/* Alexander.Schulz@stud.uni-karlsruhe.de                        */

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

#include <glib/gstdio.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "gra.h"

static gint    cur_progress = 0;
static gint    max_progress = 0;

static gboolean save_dialog ();

gboolean
check_color_mapping(int image){
    guchar      gra_color_map[3*16];
    guchar      *image_color_map;
    gint        colors; 
    int         i;

    image_color_map = gimp_image_get_colormap (image, &colors);
    if (colors != 16){
        return FALSE;
    }
    get_color_map(&gra_color_map); // Get our color map
    // Compare colors
    for (i = 0; i < 3*16; i++){
        if (gra_color_map[i] != image_color_map[i]){
            return FALSE;
        }
    }
    return TRUE;
}

GimpPDBStatusType
WriteGRA (const gchar  *filename,
        gint32        image,
        gint32        drawable_ID,
        GError      **error)
{
    FILE          *outfile;
    GimpDrawable  *drawable;
    GimpImageType  drawable_type;
    GimpPixelRgn   pixel_rgn;
    guchar        *pixels;
    gint          channels;

    if (!gimp_drawable_is_indexed(drawable_ID)) {
        if (!save_dialog()){
            g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Can only save indexed images as .GRA");
            return GIMP_PDB_EXECUTION_ERROR;
        }

        // Convert to indexed
        if (!gimp_image_convert_indexed(image,
                    GIMP_NO_DITHER,
                    GIMP_CUSTOM_PALETTE,
                    16,         // Ignored
                    FALSE,      // No dither
                    FALSE,      // Do NOT remove unused colours
                    PALETTE_NAME)){
            g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Error converting image to correct format (couldn't convert to indexed).");
            return GIMP_PDB_EXECUTION_ERROR;
        }
    }

    //TODO: Handle custom color maps. For now we'll assume it's the same as the default
    // Need to check the color map used is valid
    if (!check_color_mapping(image)){
        if (!save_dialog()){
            g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                    "Color mapping is incorrect. Please ensure you used the TempleOS GRA Color palette");
            return GIMP_PDB_EXECUTION_ERROR;
        }

        // Convert to RGB and then back to indexed with proper colourmap.
        // A little bit hacky, but it ensures the colours are the most accurate
        if (!gimp_image_convert_rgb(image)){
            g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Error converting image to correct format (couldn't convert to RGB).");
            return GIMP_PDB_EXECUTION_ERROR;
        }

        if (!gimp_image_convert_indexed(image,
                    GIMP_NO_DITHER,
                    GIMP_CUSTOM_PALETTE,
                    16,         // Ignored
                    FALSE,      // No dither
                    FALSE,      // Do NOT remove unused colours
                    PALETTE_NAME)){
            g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Error converting image to correct format (couldn't convert to indexed).");
            return GIMP_PDB_EXECUTION_ERROR;
        }
    }

    drawable = gimp_drawable_get (drawable_ID);

    drawable_type = gimp_drawable_type (drawable_ID);

    gimp_pixel_rgn_init (&pixel_rgn, drawable,
            0, 0, drawable->width, drawable->height, FALSE, FALSE);

    // Type is either GIMP_INDEXED_IMAGE or GIMP_INDEXEDA_IMAGE
    channels = drawable_type == GIMP_INDEXED_IMAGE ? 1 : 2;


    // Get the file
    outfile = g_fopen(filename, "wb");
    if (!outfile)
    {
        g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
                "Could not open '%s' for writing: %s",
                gimp_filename_to_utf8 (filename), g_strerror (errno));
        return GIMP_PDB_EXECUTION_ERROR;
    }

    // Get the image
    pixels = g_new (guchar, drawable->width * drawable->height * channels );
    gimp_pixel_rgn_get_rect (&pixel_rgn, pixels,
            0, 0, drawable->width, drawable->height);
    // Set transparency here
    if (drawable_type == GIMP_INDEXEDA_IMAGE){
        guchar * alpha_pixels = (guchar *)malloc(drawable->width * drawable->height);
        int i;
        guchar alpha_value;
        for (i=0; i < drawable->width * drawable->height; i++){
            // Set colour value
            alpha_pixels[i] |= (pixels[i*2] & 0x0F); // Shouldn't need the & but just in case
            // Set alpha value
            alpha_value = 0xFF - pixels[i*2 + 1];
            alpha_pixels[i] |= (alpha_value & 0xF0); 
        }

        g_free(pixels);
        pixels = alpha_pixels;
    }

    // Begin the process
    gimp_progress_init_printf ("Saving '%s'",
            gimp_filename_to_utf8 (filename));
    cur_progress = 0;
    max_progress = drawable->height;

    // Write the width
    if (!fwrite(&drawable->width, 1, 4, outfile)){
        //TODO: Replace with proper errors
        fprintf(stderr, "Error writing width to file\n");
        return GIMP_PDB_EXECUTION_ERROR;
    }

    gint width_internal = drawable->width;

    // Calculate width_internal (round up to nearest multiple of 8)
    while (width_internal % 8){
        width_internal++;
        // I guess you could also do:
        // width_internal++ >> 3;
        // width_internal << 3;
    };

    // Write width_internal
    if (!fwrite(&width_internal, 1, 4, outfile)){
        fprintf(stderr, "Error writing width_internal to file\n");
        return GIMP_PDB_EXECUTION_ERROR;
    }

    // Write height
    if (!fwrite(&drawable->height, 1, 4, outfile)){
        //TODO: Replace with proper errors
        fprintf(stderr, "Error writing height to file\n");
        return GIMP_PDB_EXECUTION_ERROR;
    }

    // TODO: Add option for compression/no compression
    // Write compression flag
    gint flags = 0x00000001;
    if (!fwrite(&flags, 1, 4, outfile)){
        //TODO: Replace with proper errors
        fprintf(stderr, "Error writing height to file\n");
        return GIMP_PDB_EXECUTION_ERROR;
    }

    // Need to compress the image data
    guchar * compressed_pixels;
    long compressed_size = compress(&compressed_pixels, pixels, drawable->width*drawable->height);
    if (!fwrite(compressed_pixels, compressed_size, 1, outfile)){
        fprintf(stderr, "Error writing image data to file\n");
        return GIMP_PDB_EXECUTION_ERROR;
    }

    /* If we didn't have compression, we'd do the following and then we'd be done*/
    /*
       if (!fwrite(pixels, drawable->width*drawable->height, 1, outfile)){
       fprintf(stderr, "Error writing height to file\n");
       return GIMP_PDB_EXECUTION_ERROR;
       }
       */

    /*
       int i;
       for (i =0; i < 112; i++){
       printf("%02x", pixels[i]);
       if (i % 2){
       printf(" ");
       }
       if ((i + 1) % 16 == 0) {
       printf("\n");
       }
       }
       printf("\n");
       */

    fclose(outfile);
    gimp_drawable_detach (drawable);
    g_free(pixels);
    g_free(compressed_pixels);
    return GIMP_PDB_SUCCESS;
}

// Prompts the user to convert the image to indexed mode
// with the proper palette.
// Mostly taken from bmp-write.c
static gboolean save_dialog(gint channels){
    GtkWidget   *dialog;
    GtkWidget   *vbox;
    GtkWidget   *frame;
    GtkWidget   *label;
    gboolean    run;

    dialog = gimp_export_dialog_new ("GRA", PLUG_IN_BINARY, SAVE_PROC);
    
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
    gtk_box_pack_start (GTK_BOX (gimp_export_dialog_get_content_area (dialog)),
            vbox, TRUE, TRUE, 0);
    gtk_widget_show (vbox);

    //frame = gtk_frame_new("Normal Label");
    label = gtk_label_new("This image is not currently in the correct format to export as a .GRA file.\nClicking export will automatically convert the image to a 16-color indexed version.");
    gtk_container_add(GTK_CONTAINER(vbox), label);
    //gtk_box_pack_start (GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    //gtk_widget_show(frame);

    gtk_widget_show(label);

    gtk_widget_show(dialog);

    run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

    gtk_widget_destroy(dialog);

    return run;
}


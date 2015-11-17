/* grawrite.c   Writes Bitmap files. Even RLE encoded ones.      */
/*              (Windows (TM) doesn't read all of those, but who */
/*              cares? ;-)                                       */
/*              I changed a few things over the time, so perhaps */
/*              it dos now, but now there's no Windows left on   */
/*              my computer...                                   */

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

typedef enum
{
  RGB_565,
  RGBA_5551,
  RGB_555,
  RGB_888,
  RGBA_8888,
  RGBX_8888
} RGBMode;

static struct
{
  RGBMode rgb_format;
  gint    use_run_length_encoding;

  /* Weather or not to write BITMAPV5HEADER color space data */
  gint    dont_write_color_space_data;
} GRASaveData;

static gint    cur_progress = 0;
static gint    max_progress = 0;


static  void      write_image     (FILE   *f,
                                   guchar *src,
                                   gint    width,
                                   gint    height,
                                   gint    use_run_length_encoding,
                                   gint    channels,
                                   gint    bpp,
                                   gint    spzeile,
                                   gint    MapSize,
                                   RGBMode rgb_format);

static  gboolean  save_dialog     (gint    channels);


static void
FromL (gint32  wert,
       guchar *bopuffer)
{
  bopuffer[0] = (wert & 0x000000ff)>>0x00;
  bopuffer[1] = (wert & 0x0000ff00)>>0x08;
  bopuffer[2] = (wert & 0x00ff0000)>>0x10;
  bopuffer[3] = (wert & 0xff000000)>>0x18;
}

static void
FromS (gint16  wert,
       guchar *bopuffer)
{
  bopuffer[0] = (wert & 0x00ff)>>0x00;
  bopuffer[1] = (wert & 0xff00)>>0x08;
}

static void
write_color_map (FILE *f,
                 gint  red[MAXCOLORS],
                 gint  green[MAXCOLORS],
                 gint  blue[MAXCOLORS],
                 gint  size)
{
  gchar trgb[4];
  gint  i;

  size /= 4;
  trgb[3] = 0;
  for (i = 0; i < size; i++)
    {
      trgb[0] = (guchar) blue[i];
      trgb[1] = (guchar) green[i];
      trgb[2] = (guchar) red[i];
      Write (f, trgb, 4);
    }
}

static gboolean
warning_dialog (const gchar *primary,
                const gchar *secondary)
{
  GtkWidget *dialog;
  gboolean   ok;

  dialog = gtk_message_dialog_new (NULL, 0,
                                   GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
                                   "%s", primary);

  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            "%s", secondary);

  gimp_window_set_transient (GTK_WINDOW (dialog));

  ok = (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return ok;
}

int
check_color_mapping(int image){
    guchar      gra_color_map[3*16];
    guchar      *image_color_map;
    gint        colors; 
    int         i;

    image_color_map = gimp_image_get_colormap (image, &colors);
    if (colors != 16){
        return 0;
    }
    get_color_map(&gra_color_map); // Get our color map
    // Compare colors
    for (i = 0; i < 3*16; i++){
        if (gra_color_map[i] != image_color_map[i]){
            return 0;
        }
    }
    return 1;
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

  drawable = gimp_drawable_get (drawable_ID);
  drawable_type = gimp_drawable_type (drawable_ID);

  gimp_pixel_rgn_init (&pixel_rgn, drawable,
                       0, 0, drawable->width, drawable->height, FALSE, FALSE);

  if (drawable_type != GIMP_INDEXED_IMAGE
          && drawable_type != GIMP_INDEXEDA_IMAGE) {
      // Show error dialog
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
              "Can only save indexed images as .GRA");
      return GIMP_PDB_EXECUTION_ERROR;
  }

  //TODO: Handle custom color maps. For now we'll assume it's the same as the default
  // Need to check the color map used is valid
  if (check_color_mapping(image) == 0){
      g_set_error (error, G_FILE_ERROR, g_file_error_from_errno (errno),
              "Color mapping is incorrect. Please ensure you used the TempleOS GRA Color palette");
      return GIMP_PDB_EXECUTION_ERROR;
  }

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

static inline void Make565(guchar r, guchar g, guchar b, guchar *buf)
{
    gint p;
    p = (((gint)(r / 255.0 * 31.0 + 0.5))<<11) |
        (((gint)(g / 255.0 * 63.0 + 0.5))<<5)  |
         ((gint)(b / 255.0 * 31.0 + 0.5));
    buf[0] = (guchar)(p & 0xff);
    buf[1] = (guchar)(p>>8);
}

static inline void Make5551(guchar r, guchar g, guchar b, guchar a, guchar *buf)
{
    gint p;
    p = (((gint)(r / 255.0 * 31.0 + 0.5))<<10) |
        (((gint)(g / 255.0 * 31.0 + 0.5))<<5)  |
         ((gint)(b / 255.0 * 31.0 + 0.5))      |
         ((gint)(a / 255.0 + 0.5)<<15);
    buf[0] = (guchar)(p & 0xff);
    buf[1] = (guchar)(p>>8);
}

static void
write_image (FILE   *f,
             guchar *src,
             gint    width,
             gint    height,
             gint    use_run_length_encoding,
             gint    channels,
             gint    bpp,
             gint    spzeile,
             gint    MapSize,
             RGBMode rgb_format)
{
  guchar  buf[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 };
  guchar  puffer[8];
  guchar *temp, v;
  guchar *row, *ketten;
  gint    xpos, ypos, i, j, rowstride, length, thiswidth;
  gint    breite, k;
  guchar  n, r, g, b, a;

  xpos = 0;
  rowstride = width * channels;

  /* We'll begin with the 16/24/32 bit Bitmaps, they are easy :-) */

  if (bpp > 8)
    {
      for (ypos = height - 1; ypos >= 0; ypos--)  /* for each row   */
        {
          for (i = 0; i < width; i++)  /* for each pixel */
            {
              temp = src + (ypos * rowstride) + (xpos * channels);
              switch (rgb_format)
                {
                default:
                case RGB_888:
                  buf[2] = *temp++;
                  buf[1] = *temp++;
                  buf[0] = *temp++;
                  xpos++;
                  if (channels > 3 && (guchar) *temp == 0)
                    buf[0] = buf[1] = buf[2] = 0xff;
                  Write (f, buf, 3);
                  break;
                case RGBX_8888:
                  buf[3] = *temp++;
                  buf[2] = *temp++;
                  buf[1] = *temp++;
                  buf[0] = 0;
                  xpos++;
                  if (channels > 3 && (guchar) *temp == 0)
                    buf[0] = buf[1] = buf[2] = 0xff;
                  Write (f, buf, 4);
                  break;
                case RGBA_8888:
                  buf[3] = *temp++;
                  buf[2] = *temp++;
                  buf[1] = *temp++;
                  buf[0] = *temp;
                  xpos++;
                  Write (f, buf, 4);
                  break;
                case RGB_565:
                  r = *temp++;
                  g = *temp++;
                  b = *temp++;
                  if (channels > 3 && (guchar) *temp == 0)
                    r = g = b = 0xff;
                  Make565 (r, g, b, buf);
                  xpos++;
                  Write (f, buf, 2);
                  break;
                case RGB_555:
                  r = *temp++;
                  g = *temp++;
                  b = *temp++;
                  if (channels > 3 && (guchar) *temp == 0)
                    r = g = b = 0xff;
                  Make5551 (r, g, b, 0x0, buf);
                  xpos++;
                  Write (f, buf, 2);
                  break;
                case RGBA_5551:
                  r = *temp++;
                  g = *temp++;
                  b = *temp++;
                  a = *temp;
                  Make5551 (r, g, b, a, buf);
                  xpos++;
                  Write (f, buf, 2);
                  break;
                }
            }

          Write (f, &buf[4], spzeile - (width * (bpp/8)));

          cur_progress++;
          if ((cur_progress % 5) == 0)
            gimp_progress_update ((gdouble) cur_progress /
                                  (gdouble) max_progress);

          xpos = 0;
        }
    }
  else
    {
      switch (use_run_length_encoding)  /* now it gets more difficult */
        {               /* uncompressed 1,4 and 8 bit */
        case 0:
          {
            thiswidth = (width / (8 / bpp));
            if (width % (8 / bpp))
              thiswidth++;

            for (ypos = height - 1; ypos >= 0; ypos--) /* for each row */
              {
                for (xpos = 0; xpos < width;)  /* for each _byte_ */
                  {
                    v = 0;
                    for (i = 1;
                         (i <= (8 / bpp)) && (xpos < width);
                         i++, xpos++)  /* for each pixel */
                      {
                        temp = src + (ypos * rowstride) + (xpos * channels);
                        if (channels > 1 && *(temp+1) == 0) *temp = 0x0;
                        v=v | ((guchar) *temp << (8 - (i * bpp)));
                      }
                    Write (f, &v, 1);
                  }
                Write (f, &buf[3], spzeile - thiswidth);
                xpos = 0;

                cur_progress++;
                if ((cur_progress % 5) == 0)
                  gimp_progress_update ((gdouble) cur_progress /
                                        (gdouble) max_progress);
              }
            break;
          }
        default:
          {              /* Save RLE encoded file, quite difficult */
            length = 0;
            buf[12] = 0;
            buf[13] = 1;
            buf[14] = 0;
            buf[15] = 0;
            row = g_new (guchar, width / (8 / bpp) + 10);
            ketten = g_new (guchar, width / (8 / bpp) + 10);
            for (ypos = height - 1; ypos >= 0; ypos--)
              { /* each row separately */
                j = 0;
                /* first copy the pixels to a buffer,
                 * making one byte from two 4bit pixels
                 */
                for (xpos = 0; xpos < width;)
                  {
                    v = 0;
                    for (i = 1;
                         (i <= (8 / bpp)) && (xpos < width);
                         i++, xpos++)
                      { /* for each pixel */
                        temp = src + (ypos * rowstride) + (xpos * channels);
                        if (channels > 1 && *(temp+1) == 0) *temp = 0x0;
                        v = v | ((guchar) * temp << (8 - (i * bpp)));
                      }
                    row[j++] = v;
                  }
                breite = width / (8 / bpp);
                if (width % (8 / bpp))
                  breite++;
                /* then check for strings of equal bytes */
                for (i = 0; i < breite; i += j)
                  {
                    j = 0;
                    while ((i + j < breite) &&
                           (j < (255 / (8 / bpp))) &&
                           (row[i + j] == row[i]))
                      j++;

                    ketten[i] = j;
                  }

                /* then write the strings and the other pixels to the file */
                for (i = 0; i < breite;)
                  {
                    if (ketten[i] < 3)
                      /* strings of different pixels ... */
                      {
                        j = 0;
                        while ((i + j < breite) &&
                               (j < (255 / (8 / bpp))) &&
                               (ketten[i + j] < 3))
                          j += ketten[i + j];

                        /* this can only happen if j jumps over
                         * the end with a 2 in ketten[i+j]
                         */
                        if (j > (255 / (8 / bpp)))
                          j -= 2;
                        /* 00 01 and 00 02 are reserved */
                        if (j > 2)
                          {
                            Write (f, &buf[12], 1);
                            n = j * (8 / bpp);
                            if (n + i * (8 / bpp) > width)
                              n--;
                            Write (f, &n, 1);
                            length += 2;
                            Write (f, &row[i], j);
                            length += j;
                            if ((j) % 2)
                              {
                                Write (f, &buf[12], 1);
                                length++;
                              }
                          }
                        else
                          {
                            for (k = i; k < i + j; k++)
                              {
                                n = (8 / bpp);
                                if (n + i * (8 / bpp) > width)
                                  n--;
                                Write (f, &n, 1);
                                Write (f, &row[k], 1);
                                /*printf("%i.#|",n); */
                                length += 2;
                              }
                          }
                        i += j;
                      }
                    else
                      /* strings of equal pixels */
                      {
                        n = ketten[i] * (8 / bpp);
                        if (n + i * (8 / bpp) > width)
                          n--;
                        Write (f, &n, 1);
                        Write (f, &row[i], 1);
                        i += ketten[i];
                        length += 2;
                      }
                  }
                Write (f, &buf[14], 2);          /* End of row */
                length += 2;

                cur_progress++;
                if ((cur_progress % 5) == 0)
                  gimp_progress_update ((gdouble) cur_progress /
                                        (gdouble) max_progress);
              }
            fseek (f, -2, SEEK_CUR);     /* Overwrite last End of row ... */
            Write (f, &buf[12], 2);      /* ... with End of file */

            fseek (f, 0x22, SEEK_SET);            /* Write length of image */
            FromL (length, puffer);
            Write (f, puffer, 4);
            fseek (f, 0x02, SEEK_SET);            /* Write length of file */
            length += (0x36 + MapSize);
            FromL (length, puffer);
            Write (f, puffer, 4);
            g_free (ketten);
            g_free (row);
            break;
          }
        }
    }

  gimp_progress_update (1.0);
}

static void
format_callback (GtkToggleButton *toggle,
                 gpointer         data)
{
  if (gtk_toggle_button_get_active (toggle))
    GRASaveData.rgb_format = GPOINTER_TO_INT (data);
}

static gboolean
save_dialog (gint channels)
{
  GtkWidget *dialog;
  GtkWidget *toggle;
  GtkWidget *vbox_main;
  GtkWidget *vbox;
  GtkWidget *vbox2;
  GtkWidget *expander;
  GtkWidget *frame;
  GSList    *group;
  gboolean   run;

  /* Dialog init */
  dialog = gimp_export_dialog_new ("GRA", PLUG_IN_BINARY, SAVE_PROC);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  vbox_main = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 12);
  gtk_box_pack_start (GTK_BOX (gimp_export_dialog_get_content_area (dialog)),
                      vbox_main, TRUE, TRUE, 0);
  gtk_widget_show (vbox_main);

  /* Run-Length Encoded */
  toggle = gtk_check_button_new_with_mnemonic ("_Run-Length Encoded");
  gtk_box_pack_start (GTK_BOX (vbox_main), toggle, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                GRASaveData.use_run_length_encoding);
  gtk_widget_show (toggle);
  if (channels > 1)
    gtk_widget_set_sensitive (toggle, FALSE);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (gimp_toggle_button_update),
                    &GRASaveData.use_run_length_encoding);

  /* Compatibility Options */
  expander = gtk_expander_new_with_mnemonic ("Co_mpatibility Options");

  gtk_box_pack_start (GTK_BOX (vbox_main), expander, TRUE, TRUE, 0);
  gtk_widget_show (expander);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 12);
  gtk_container_add (GTK_CONTAINER (expander), vbox2);
  gtk_widget_show (vbox2);

  toggle = gtk_check_button_new_with_mnemonic ("_Do not write color space information");
  gimp_help_set_help_data (toggle,
                           "Some applications can not read GRA images that "
                             "include color space information. GIMP writes "
                             "color space information by default. Enabling "
                             "this option will cause GIMP to not write color "
                             "space information to the file.",
                           NULL);
  gtk_box_pack_start (GTK_BOX (vbox2), toggle, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
                                GRASaveData.dont_write_color_space_data);
  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (gimp_toggle_button_update),
                    &GRASaveData.dont_write_color_space_data);

  /* Advanced Options */
  expander = gtk_expander_new_with_mnemonic ("_Advanced Options");

  gtk_box_pack_start (GTK_BOX (vbox_main), expander, TRUE, TRUE, 0);
  gtk_widget_show (expander);

  if (channels < 3)
    gtk_widget_set_sensitive (expander, FALSE);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 12);
  gtk_container_add (GTK_CONTAINER (expander), vbox2);
  gtk_widget_show (vbox2);

  group = NULL;

  frame = gimp_frame_new ("16 bits");
  gtk_box_pack_start (GTK_BOX (vbox2), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  toggle = gtk_radio_button_new_with_label (group, "R5 G6 B5");
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);
  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (format_callback),
                    GINT_TO_POINTER (RGB_565));

  toggle = gtk_radio_button_new_with_label (group, "A1 R5 G5 B5");
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);

  if (channels < 4)
    gtk_widget_set_sensitive (toggle, FALSE);

  gtk_widget_show (toggle);

  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (format_callback),
                    GINT_TO_POINTER (RGBA_5551));
  toggle = gtk_radio_button_new_with_label (group, "X1 R5 G5 B5");
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);
  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (format_callback),
                    GINT_TO_POINTER (RGB_555));

  frame = gimp_frame_new ("24 bits");
  gtk_box_pack_start (GTK_BOX (vbox2), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  toggle = gtk_radio_button_new_with_label (group, "R8 G8 B8");
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON(toggle));
  gtk_container_add (GTK_CONTAINER (frame), toggle);
  gtk_widget_show (toggle);
  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (format_callback),
                    GINT_TO_POINTER (RGB_888));
  if (channels < 4)
    {
      GRASaveData.rgb_format = RGB_888;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), TRUE);
    }

  frame = gimp_frame_new ("32 bits");
  gtk_box_pack_start (GTK_BOX (vbox2), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  toggle = gtk_radio_button_new_with_label (group, "A8 R8 G8 B8");
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);
  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (format_callback),
                    GINT_TO_POINTER (RGBA_8888));


  if (channels < 4)
    {
      gtk_widget_set_sensitive (toggle, FALSE);
    }
  else
    {
      GRASaveData.rgb_format = RGBA_8888;
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle), TRUE);
    }

  toggle = gtk_radio_button_new_with_label (group, "X8 R8 G8 B8");
  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle));
  gtk_box_pack_start (GTK_BOX (vbox), toggle, FALSE, FALSE, 0);
  gtk_widget_show (toggle);
  g_signal_connect (toggle, "toggled",
                    G_CALLBACK (format_callback),
                    GINT_TO_POINTER (RGBX_8888));

  /* Dialog show */
  gtk_widget_show (dialog);

  run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

  gtk_widget_destroy (dialog);

  return run;
}

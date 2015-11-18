/* gra.c                                          */
/* Version 0.52                                   */
/* This is a File input and output filter for the */
/* Gimp. It loads and saves images in TempleOS    */
/* GRA format.                                    */
/* This was based off Alexander Schulz's BMP      */
/* filter plugin, and also uses code from Terry   */
/* Davis' TempleOS                                */
/* michaelbarlow7@gmail.com                       */

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

#include <stdlib.h>
#include <string.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "gra.h"

const gchar *filename    = NULL;
gboolean     interactive = FALSE;
gboolean     lastvals    = FALSE;


/* Declare some local functions.
 */
static void   query (void);
static void   run   (const gchar      *name,
                     gint              nparams,
                     const GimpParam  *param,
                     gint             *nreturn_vals,
                     GimpParam       **return_vals);

const GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

MAIN ()

static void
query (void)
{
  static const GimpParamDef load_args[] =
  {
    { GIMP_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { GIMP_PDB_STRING,   "filename",     "The name of the file to load" },
    { GIMP_PDB_STRING,   "raw-filename", "The name entered" },
  };
  static const GimpParamDef load_return_vals[] =
  {
    { GIMP_PDB_IMAGE, "image", "Output image" },
  };

  static const GimpParamDef save_args[] =
  {
    { GIMP_PDB_INT32,    "run-mode",     "The run mode { RUN-INTERACTIVE (0), RUN-NONINTERACTIVE (1) }" },
    { GIMP_PDB_IMAGE,    "image",        "Input image" },
    { GIMP_PDB_DRAWABLE, "drawable",     "Drawable to save" },
    { GIMP_PDB_STRING,   "filename",     "The name of the file to save the image in" },
    { GIMP_PDB_STRING,   "raw-filename", "The name entered" },
  };

  gimp_install_procedure (LOAD_PROC,
                          "Loads files of Windows GRA file format",
                          "Loads files of Windows GRA file format",
                          "Alexander Schulz",
                          "Alexander Schulz",
                          "1997",
                          "Windows GRA image",
                          NULL,
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals),
                          load_args, load_return_vals);

  gimp_register_file_handler_mime (LOAD_PROC, "image/gra");
  gimp_register_magic_load_handler (LOAD_PROC,
                                    "gra",
                                    "",
                                    "0,string,BM");

  gimp_install_procedure (SAVE_PROC,
                          "Saves files in Windows GRA file format",
                          "Saves files in Windows GRA file format",
                          "Alexander Schulz",
                          "Alexander Schulz",
                          "1997",
                          "Windows GRA image",
                          "INDEXED, GRAY, RGB*",
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);

  gimp_register_file_handler_mime (SAVE_PROC, "image/gra");
  gimp_register_save_handler (SAVE_PROC, "gra", "");
}

static void
run (const gchar      *name,
     gint             nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[2];
  GimpRunMode        run_mode;
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;
  gint32             image_ID;
  gint32             drawable_ID;
  GimpExportReturn   export = GIMP_EXPORT_CANCEL;
  GError            *error  = NULL;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;
  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;

  if (strcmp (name, LOAD_PROC) == 0)
    {
       switch (run_mode)
        {
        case GIMP_RUN_INTERACTIVE:
          interactive = TRUE;
          break;

        case GIMP_RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          if (nparams != 3)
            status = GIMP_PDB_CALLING_ERROR;
          break;

        default:
          break;
        }

       if (status == GIMP_PDB_SUCCESS)
         {
           image_ID = ReadGRA (param[1].data.d_string, &error);

           if (image_ID != -1)
             {
               *nreturn_vals = 2;
               values[1].type         = GIMP_PDB_IMAGE;
               values[1].data.d_image = image_ID;
             }
           else
             {
               status = GIMP_PDB_EXECUTION_ERROR;
             }
         }
    }
  else if (strcmp (name, SAVE_PROC) == 0)
    {
      image_ID    = param[1].data.d_int32;
      drawable_ID = param[2].data.d_int32;

      /*  eventually export the image */
      switch (run_mode)
        {
        case GIMP_RUN_INTERACTIVE:
          interactive = TRUE;
          /* fallthrough */

        case GIMP_RUN_WITH_LAST_VALS:
          if (run_mode == GIMP_RUN_WITH_LAST_VALS)
            lastvals = TRUE;

          gimp_ui_init (PLUG_IN_BINARY, FALSE);

          export = gimp_export_image (&image_ID, &drawable_ID, "GRA",
                                      GIMP_EXPORT_CAN_HANDLE_RGB   |
                                      GIMP_EXPORT_CAN_HANDLE_GRAY  |
                                      GIMP_EXPORT_CAN_HANDLE_ALPHA |
                                      GIMP_EXPORT_CAN_HANDLE_INDEXED);

          if (export == GIMP_EXPORT_CANCEL)
            {
              values[0].data.d_status = GIMP_PDB_CANCEL;
              return;
            }
          break;

        case GIMP_RUN_NONINTERACTIVE:
          /*  Make sure all the arguments are there!  */
          if (nparams != 5)
            status = GIMP_PDB_CALLING_ERROR;
          break;

        default:
          break;
        }

      if (status == GIMP_PDB_SUCCESS)
        status = WriteGRA (param[3].data.d_string, image_ID, drawable_ID,
                           &error);

      if (export == GIMP_EXPORT_EXPORT)
        gimp_image_delete (image_ID);
    }
  else
    {
      status = GIMP_PDB_CALLING_ERROR;
    }

  if (status != GIMP_PDB_SUCCESS && error)
    {
      *nreturn_vals = 2;
      values[1].type          = GIMP_PDB_STRING;
      values[1].data.d_string = error->message;
    }

  values[0].data.d_status = status;
}

void get_color_map(guchar * color_map){
    int             col_index = 0;

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
}


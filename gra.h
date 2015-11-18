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
 */

#ifndef __GRA_H__
#define __GRA_H__

#define LOAD_PROC      "file-gra-load"
#define SAVE_PROC      "file-gra-save"
#define PLUG_IN_BINARY "file-gra"
#define PLUG_IN_ROLE   "gimp-file-gra"

#define MAXCOLORS   256

#define BitSet(byte, bit)        (((byte) & (bit)) == (bit))

#define ReadOK(file,buffer,len)  (fread(buffer, len, 1, file) != 0)
#define Write(file,buffer,len)   fwrite(buffer, len, 1, file)
#define WriteOK(file,buffer,len) (Write(buffer, len, file) != 0)

#define PALETTE_NAME    "TempleOS GRA Colors"


gint32             ReadGRA   (const gchar  *filename,
                              GError      **error);
GimpPDBStatusType  WriteGRA  (const gchar  *filename,
                              gint32        image,
                              gint32        drawable_ID,
                              GError      **error);

extern       gboolean  interactive;
extern       gboolean  lastvals;
extern const gchar    *filename;
void get_color_map(guchar * color_map);

#endif /* __GRA_H__ */

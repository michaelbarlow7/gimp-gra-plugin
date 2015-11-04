/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpfloatingsel_pdb.h
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

#ifndef __GIMP_FLOATING_SEL_PDB_H__
#define __GIMP_FLOATING_SEL_PDB_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


gboolean gimp_floating_sel_remove   (gint32   floating_sel_ID);
gboolean gimp_floating_sel_anchor   (gint32   floating_sel_ID);
gboolean gimp_floating_sel_to_layer (gint32   floating_sel_ID);
gboolean gimp_floating_sel_attach   (gint32   layer_ID,
                                     gint32   drawable_ID);
GIMP_DEPRECATED
gboolean gimp_floating_sel_rigor    (gint32   floating_sel_ID,
                                     gboolean undo);
GIMP_DEPRECATED
gboolean gimp_floating_sel_relax    (gint32   floating_sel_ID,
                                     gboolean undo);


G_END_DECLS

#endif /* __GIMP_FLOATING_SEL_PDB_H__ */

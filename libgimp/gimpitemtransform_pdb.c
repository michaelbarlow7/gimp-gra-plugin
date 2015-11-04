/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpitemtransform_pdb.c
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

#include "gimp.h"


/**
 * SECTION: gimpitemtransform
 * @title: gimpitemtransform
 * @short_description: Functions to perform transformations on items.
 *
 * Functions to perform transformations on items.
 **/


/**
 * gimp_item_transform_flip_simple:
 * @item_ID: The affected item.
 * @flip_type: Type of flip.
 * @auto_center: Whether to automatically position the axis in the selection center.
 * @axis: coord. of flip axis.
 *
 * Flip the specified item either vertically or horizontally.
 *
 * This procedure flips the specified item. If a selection exists and
 * the item is a drawable, the portion of the drawable which lies under
 * the selection is cut from the drawable and made into a floating
 * selection which is then flipped. If auto_center is set to TRUE, the
 * flip is around the selection's center. Otherwise, the coordinate of
 * the axis needs to be specified. The return value is the ID of the
 * flipped item. If there was no selection or the item is not a
 * drawable, this will be equal to the item ID supplied as input.
 * Otherwise, this will be the newly created and flipped drawable.
 * This procedure is affected by the following context setters:
 * gimp_context_set_transform_resize().
 *
 * Returns: The flipped item.
 *
 * Since: 2.2
 **/
gint32
gimp_item_transform_flip_simple (gint32              item_ID,
                                 GimpOrientationType flip_type,
                                 gboolean            auto_center,
                                 gdouble             axis)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-flip-simple",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_INT32, flip_type,
                                    GIMP_PDB_INT32, auto_center,
                                    GIMP_PDB_FLOAT, axis,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

/**
 * gimp_item_transform_flip:
 * @item_ID: The affected item.
 * @x0: horz. coord. of one end of axis.
 * @y0: vert. coord. of one end of axis.
 * @x1: horz. coord. of other end of axis.
 * @y1: vert. coord. of other end of axis.
 *
 * Flip the specified item around a given line.
 *
 * This procedure flips the specified item. If a selection exists and
 * the item is a drawable , the portion of the drawable which lies
 * under the selection is cut from the drawable and made into a
 * floating selection which is then flipped. The axis to flip around is
 * specified by specifying two points from that line. The return value
 * is the ID of the flipped item. If there was no selection or the item
 * is not a drawable, this will be equal to the item ID supplied as
 * input. Otherwise, this will be the newly created and flipped
 * drawable.
 * This procedure is affected by the following context setters:
 * gimp_context_set_interpolation(),
 * gimp_context_set_transform_direction(),
 * gimp_context_set_transform_resize().
 *
 * Returns: The flipped item.
 *
 * Since: 2.8
 **/
gint32
gimp_item_transform_flip (gint32  item_ID,
                          gdouble x0,
                          gdouble y0,
                          gdouble x1,
                          gdouble y1)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-flip",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_FLOAT, x0,
                                    GIMP_PDB_FLOAT, y0,
                                    GIMP_PDB_FLOAT, x1,
                                    GIMP_PDB_FLOAT, y1,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

/**
 * gimp_item_transform_perspective:
 * @item_ID: The affected item.
 * @x0: The new x coordinate of upper-left corner of original bounding box.
 * @y0: The new y coordinate of upper-left corner of original bounding box.
 * @x1: The new x coordinate of upper-right corner of original bounding box.
 * @y1: The new y coordinate of upper-right corner of original bounding box.
 * @x2: The new x coordinate of lower-left corner of original bounding box.
 * @y2: The new y coordinate of lower-left corner of original bounding box.
 * @x3: The new x coordinate of lower-right corner of original bounding box.
 * @y3: The new y coordinate of lower-right corner of original bounding box.
 *
 * Perform a possibly non-affine transformation on the specified item.
 *
 * This procedure performs a possibly non-affine transformation on the
 * specified item by allowing the corners of the original bounding box
 * to be arbitrarily remapped to any values. The specified item is
 * remapped if no selection exists or it is not a drawable. However, if
 * a selection exists and the item is a drawable, the portion of the
 * drawable which lies under the selection is cut from the drawable and
 * made into a floating selection which is then remapped as specified.
 * The return value is the ID of the remapped item. If there was no
 * selection or the item is not a drawable, this will be equal to the
 * item ID supplied as input. Otherwise, this will be the newly created
 * and remapped drawable. The 4 coordinates specify the new locations
 * of each corner of the original bounding box. By specifying these
 * values, any affine transformation (rotation, scaling, translation)
 * can be affected. Additionally, these values can be specified such
 * that the resulting transformed item will appear to have been
 * projected via a perspective transform.
 * This procedure is affected by the following context setters:
 * gimp_context_set_interpolation(),
 * gimp_context_set_transform_direction(),
 * gimp_context_set_transform_resize().
 *
 * Returns: The newly mapped item.
 *
 * Since: 2.8
 **/
gint32
gimp_item_transform_perspective (gint32  item_ID,
                                 gdouble x0,
                                 gdouble y0,
                                 gdouble x1,
                                 gdouble y1,
                                 gdouble x2,
                                 gdouble y2,
                                 gdouble x3,
                                 gdouble y3)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-perspective",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_FLOAT, x0,
                                    GIMP_PDB_FLOAT, y0,
                                    GIMP_PDB_FLOAT, x1,
                                    GIMP_PDB_FLOAT, y1,
                                    GIMP_PDB_FLOAT, x2,
                                    GIMP_PDB_FLOAT, y2,
                                    GIMP_PDB_FLOAT, x3,
                                    GIMP_PDB_FLOAT, y3,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

/**
 * gimp_item_transform_rotate_simple:
 * @item_ID: The affected item.
 * @rotate_type: Type of rotation.
 * @auto_center: Whether to automatically rotate around the selection center.
 * @center_x: The hor. coordinate of the center of rotation.
 * @center_y: The vert. coordinate of the center of rotation.
 *
 * Rotate the specified item about given coordinates through the
 * specified angle.
 *
 * This function rotates the specified item. If a selection exists and
 * the item is a drawable, the portion of the drawable which lies under
 * the selection is cut from the drawable and made into a floating
 * selection which is then rotated by the specified amount. The return
 * value is the ID of the rotated item. If there was no selection or
 * the item is not a drawable, this will be equal to the item ID
 * supplied as input. Otherwise, this will be the newly created and
 * rotated drawable.
 * This procedure is affected by the following context setters:
 * gimp_context_set_transform_resize().
 *
 * Returns: The rotated item.
 *
 * Since: 2.8
 **/
gint32
gimp_item_transform_rotate_simple (gint32           item_ID,
                                   GimpRotationType rotate_type,
                                   gboolean         auto_center,
                                   gdouble          center_x,
                                   gdouble          center_y)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-rotate-simple",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_INT32, rotate_type,
                                    GIMP_PDB_INT32, auto_center,
                                    GIMP_PDB_FLOAT, center_x,
                                    GIMP_PDB_FLOAT, center_y,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

/**
 * gimp_item_transform_rotate:
 * @item_ID: The affected item.
 * @angle: The angle of rotation (radians).
 * @auto_center: Whether to automatically rotate around the selection center.
 * @center_x: The hor. coordinate of the center of rotation.
 * @center_y: The vert. coordinate of the center of rotation.
 *
 * Rotate the specified item about given coordinates through the
 * specified angle.
 *
 * This function rotates the specified item. If a selection exists and
 * the item is a drawable, the portion of the drawable which lies under
 * the selection is cut from the drawable and made into a floating
 * selection which is then rotated by the specified amount. The return
 * value is the ID of the rotated item. If there was no selection or
 * the item is not a drawable, this will be equal to the item ID
 * supplied as input. Otherwise, this will be the newly created and
 * rotated drawable.
 * This procedure is affected by the following context setters:
 * gimp_context_set_interpolation(),
 * gimp_context_set_transform_direction(),
 * gimp_context_set_transform_resize().
 *
 * Returns: The rotated item.
 *
 * Since: 2.8
 **/
gint32
gimp_item_transform_rotate (gint32   item_ID,
                            gdouble  angle,
                            gboolean auto_center,
                            gdouble  center_x,
                            gdouble  center_y)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-rotate",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_FLOAT, angle,
                                    GIMP_PDB_INT32, auto_center,
                                    GIMP_PDB_FLOAT, center_x,
                                    GIMP_PDB_FLOAT, center_y,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

/**
 * gimp_item_transform_scale:
 * @item_ID: The affected item.
 * @x0: The new x coordinate of the upper-left corner of the scaled region.
 * @y0: The new y coordinate of the upper-left corner of the scaled region.
 * @x1: The new x coordinate of the lower-right corner of the scaled region.
 * @y1: The new y coordinate of the lower-right corner of the scaled region.
 *
 * Scale the specified item.
 *
 * This procedure scales the specified item. If a selection exists and
 * the item is a drawable, the portion of the drawable which lies under
 * the selection is cut from the drawable and made into a floating
 * selection which is then scaled by the specified amount. The return
 * value is the ID of the scaled item. If there was no selection or the
 * item is not a drawable, this will be equal to the item ID supplied
 * as input. Otherwise, this will be the newly created and scaled
 * drawable.
 * This procedure is affected by the following context setters:
 * gimp_context_set_interpolation(),
 * gimp_context_set_transform_direction(),
 * gimp_context_set_transform_resize().
 *
 * Returns: The scaled item.
 *
 * Since: 2.8
 **/
gint32
gimp_item_transform_scale (gint32  item_ID,
                           gdouble x0,
                           gdouble y0,
                           gdouble x1,
                           gdouble y1)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-scale",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_FLOAT, x0,
                                    GIMP_PDB_FLOAT, y0,
                                    GIMP_PDB_FLOAT, x1,
                                    GIMP_PDB_FLOAT, y1,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

/**
 * gimp_item_transform_shear:
 * @item_ID: The affected item.
 * @shear_type: Type of shear.
 * @magnitude: The magnitude of the shear.
 *
 * Shear the specified item about its center by the specified
 * magnitude.
 *
 * This procedure shears the specified item. If a selection exists and
 * the item is a drawable, the portion of the drawable which lies under
 * the selection is cut from the drawable and made into a floating
 * selection which is then sheard by the specified amount. The return
 * value is the ID of the sheard item. If there was no selection or the
 * item is not a drawable, this will be equal to the item ID supplied
 * as input. Otherwise, this will be the newly created and sheard
 * drawable. The shear type parameter indicates whether the shear will
 * be applied horizontally or vertically. The magnitude can be either
 * positive or negative and indicates the extent (in pixels) to shear
 * by.
 * This procedure is affected by the following context setters:
 * gimp_context_set_interpolation(),
 * gimp_context_set_transform_direction(),
 * gimp_context_set_transform_resize().
 *
 * Returns: The sheared item.
 *
 * Since: 2.8
 **/
gint32
gimp_item_transform_shear (gint32              item_ID,
                           GimpOrientationType shear_type,
                           gdouble             magnitude)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-shear",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_INT32, shear_type,
                                    GIMP_PDB_FLOAT, magnitude,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

/**
 * gimp_item_transform_2d:
 * @item_ID: The affected item.
 * @source_x: X coordinate of the transformation center.
 * @source_y: Y coordinate of the transformation center.
 * @scale_x: Amount to scale in x direction.
 * @scale_y: Amount to scale in y direction.
 * @angle: The angle of rotation (radians).
 * @dest_x: X coordinate of where the center goes.
 * @dest_y: Y coordinate of where the center goes.
 *
 * Transform the specified item in 2d.
 *
 * This procedure transforms the specified item. If a selection exists
 * and the item is a drawable, the portion of the drawable which lies
 * under the selection is cut from the drawable and made into a
 * floating selection which is then transformed. The transformation is
 * done by scaling the image by the x and y scale factors about the
 * point (source_x, source_y), then rotating around the same point,
 * then translating that point to the new position (dest_x, dest_y).
 * The return value is the ID of the rotated drawable. If there was no
 * selection or the item is not a drawable, this will be equal to the
 * item ID supplied as input. Otherwise, this will be the newly created
 * and transformed drawable.
 * This procedure is affected by the following context setters:
 * gimp_context_set_interpolation(),
 * gimp_context_set_transform_direction(),
 * gimp_context_set_transform_resize().
 *
 * Returns: The transformed item.
 *
 * Since: 2.8
 **/
gint32
gimp_item_transform_2d (gint32  item_ID,
                        gdouble source_x,
                        gdouble source_y,
                        gdouble scale_x,
                        gdouble scale_y,
                        gdouble angle,
                        gdouble dest_x,
                        gdouble dest_y)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-2d",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_FLOAT, source_x,
                                    GIMP_PDB_FLOAT, source_y,
                                    GIMP_PDB_FLOAT, scale_x,
                                    GIMP_PDB_FLOAT, scale_y,
                                    GIMP_PDB_FLOAT, angle,
                                    GIMP_PDB_FLOAT, dest_x,
                                    GIMP_PDB_FLOAT, dest_y,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

/**
 * gimp_item_transform_matrix:
 * @item_ID: The affected item.
 * @coeff_0_0: coefficient (0,0) of the transformation matrix.
 * @coeff_0_1: coefficient (0,1) of the transformation matrix.
 * @coeff_0_2: coefficient (0,2) of the transformation matrix.
 * @coeff_1_0: coefficient (1,0) of the transformation matrix.
 * @coeff_1_1: coefficient (1,1) of the transformation matrix.
 * @coeff_1_2: coefficient (1,2) of the transformation matrix.
 * @coeff_2_0: coefficient (2,0) of the transformation matrix.
 * @coeff_2_1: coefficient (2,1) of the transformation matrix.
 * @coeff_2_2: coefficient (2,2) of the transformation matrix.
 *
 * Transform the specified item in 2d.
 *
 * This procedure transforms the specified item. If a selection exists
 * and the item is a drawable, the portion of the drawable which lies
 * under the selection is cut from the drawable and made into a
 * floating selection which is then transformed. The transformation is
 * done by assembling a 3x3 matrix from the coefficients passed. The
 * return value is the ID of the transformed item. If there was no
 * selection or the item is not a drawable, this will be equal to the
 * item ID supplied as input. Otherwise, this will be the newly created
 * and transformed drawable.
 * This procedure is affected by the following context setters:
 * gimp_context_set_interpolation(),
 * gimp_context_set_transform_direction(),
 * gimp_context_set_transform_resize().
 *
 * Returns: The transformed item.
 *
 * Since: 2.8
 **/
gint32
gimp_item_transform_matrix (gint32  item_ID,
                            gdouble coeff_0_0,
                            gdouble coeff_0_1,
                            gdouble coeff_0_2,
                            gdouble coeff_1_0,
                            gdouble coeff_1_1,
                            gdouble coeff_1_2,
                            gdouble coeff_2_0,
                            gdouble coeff_2_1,
                            gdouble coeff_2_2)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 ret_item_ID = -1;

  return_vals = gimp_run_procedure ("gimp-item-transform-matrix",
                                    &nreturn_vals,
                                    GIMP_PDB_ITEM, item_ID,
                                    GIMP_PDB_FLOAT, coeff_0_0,
                                    GIMP_PDB_FLOAT, coeff_0_1,
                                    GIMP_PDB_FLOAT, coeff_0_2,
                                    GIMP_PDB_FLOAT, coeff_1_0,
                                    GIMP_PDB_FLOAT, coeff_1_1,
                                    GIMP_PDB_FLOAT, coeff_1_2,
                                    GIMP_PDB_FLOAT, coeff_2_0,
                                    GIMP_PDB_FLOAT, coeff_2_1,
                                    GIMP_PDB_FLOAT, coeff_2_2,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    ret_item_ID = return_vals[1].data.d_item;

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_item_ID;
}

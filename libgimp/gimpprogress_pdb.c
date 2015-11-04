/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpprogress_pdb.c
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
 * SECTION: gimpprogress
 * @title: gimpprogress
 * @short_description: Functions for embedding the progress bar into a plug-in's GUI.
 *
 * Functions for embedding the progress bar into a plug-in's GUI.
 **/


/**
 * _gimp_progress_init:
 * @message: Message to use in the progress dialog.
 * @gdisplay_ID: GimpDisplay to update progressbar in, or -1 for a separate window.
 *
 * Initializes the progress bar for the current plug-in.
 *
 * Initializes the progress bar for the current plug-in. It is only
 * valid to call this procedure from a plug-in.
 *
 * Returns: TRUE on success.
 **/
gboolean
_gimp_progress_init (const gchar *message,
                     gint32       gdisplay_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-progress-init",
                                    &nreturn_vals,
                                    GIMP_PDB_STRING, message,
                                    GIMP_PDB_DISPLAY, gdisplay_ID,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * _gimp_progress_update:
 * @percentage: Percentage of progress completed which must be between 0.0 and 1.0.
 *
 * Updates the progress bar for the current plug-in.
 *
 * Updates the progress bar for the current plug-in. It is only valid
 * to call this procedure from a plug-in.
 *
 * Returns: TRUE on success.
 **/
gboolean
_gimp_progress_update (gdouble percentage)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-progress-update",
                                    &nreturn_vals,
                                    GIMP_PDB_FLOAT, percentage,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_progress_pulse:
 *
 * Pulses the progress bar for the current plug-in.
 *
 * Updates the progress bar for the current plug-in. It is only valid
 * to call this procedure from a plug-in. Use this function instead of
 * gimp_progress_update() if you cannot tell how much progress has been
 * made. This usually causes the the progress bar to enter \"activity
 * mode\", where a block bounces back and forth.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_progress_pulse (void)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-progress-pulse",
                                    &nreturn_vals,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_progress_set_text:
 * @message: Message to use in the progress dialog.
 *
 * Changes the text in the progress bar for the current plug-in.
 *
 * This function changes the text in the progress bar for the current
 * plug-in. Unlike gimp_progress_init() it does not change the
 * displayed value.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_progress_set_text (const gchar *message)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-progress-set-text",
                                    &nreturn_vals,
                                    GIMP_PDB_STRING, message,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_progress_end:
 *
 * Ends the progress bar for the current plug-in.
 *
 * Ends the progress display for the current plug-in. Most plug-ins
 * don't need to call this, they just exit when the work is done. It is
 * only valid to call this procedure from a plug-in.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.4
 **/
gboolean
gimp_progress_end (void)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-progress-end",
                                    &nreturn_vals,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_progress_get_window_handle:
 *
 * Returns the native window ID of the toplevel window this plug-in's
 * progress is displayed in.
 *
 * This function returns the native window ID of the toplevel window
 * this plug-in\'s progress is displayed in.
 *
 * Returns: The progress bar's toplevel window.
 *
 * Since: 2.2
 **/
gint
gimp_progress_get_window_handle (void)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint window = 0;

  return_vals = gimp_run_procedure ("gimp-progress-get-window-handle",
                                    &nreturn_vals,
                                    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    window = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return window;
}

/**
 * _gimp_progress_install:
 * @progress_callback: The callback PDB proc to call.
 *
 * Installs a progress callback for the current plug-in.
 *
 * This function installs a temporary PDB procedure which will handle
 * all progress calls made by this plug-in and any procedure it calls.
 * Calling this function multiple times simply replaces the old
 * progress callbacks.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.2
 **/
gboolean
_gimp_progress_install (const gchar *progress_callback)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-progress-install",
                                    &nreturn_vals,
                                    GIMP_PDB_STRING, progress_callback,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * _gimp_progress_uninstall:
 * @progress_callback: The name of the callback registered for this progress.
 *
 * Uninstalls the progress callback for the current plug-in.
 *
 * This function uninstalls any progress callback installed with
 * gimp_progress_install() before.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.2
 **/
gboolean
_gimp_progress_uninstall (const gchar *progress_callback)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-progress-uninstall",
                                    &nreturn_vals,
                                    GIMP_PDB_STRING, progress_callback,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_progress_cancel:
 * @progress_callback: The name of the callback registered for this progress.
 *
 * Cancels a running progress.
 *
 * This function cancels the currently running progress.
 *
 * Returns: TRUE on success.
 *
 * Since: 2.2
 **/
gboolean
gimp_progress_cancel (const gchar *progress_callback)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp-progress-cancel",
                                    &nreturn_vals,
                                    GIMP_PDB_STRING, progress_callback,
                                    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

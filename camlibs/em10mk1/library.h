/** \file library.h
 *
 * \author Copyright 2023 Bruno Palos
 *
 * \par
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * \par
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 */
#ifndef CAMLIBS_EM10MK1_LIBRARY_H
#define CAMLIBS_EM10MK1_LIBRARY_H

#define GP_MODULE "em10mk1"

#define CHECK(result)                                                                           \
	{                                                                                           \
		int res = (result);                                                                     \
                                                                                                \
		if (res < 0)                                                                            \
		{                                                                                       \
			gp_log(GP_LOG_DEBUG, "em10mk1", "Operation failed in %s (%i)!", __FUNCTION__, res); \
			return (res);                                                                       \
		}                                                                                       \
	}

int camera_exit(Camera *camera, GPContext *context);

/**
 * Get information on all available storages in the camera.
 *
 * This function is a CameraFilesystem method.
 */
int storage_info_func(CameraFilesystem *fs,
					  CameraStorageInformation **storageinformations,
					  int *nrofstorageinformations, void *data,
					  GPContext *context);

int camera_capture_preview(Camera *camera, CameraFile *file, GPContext *context);

int folder_list_func(CameraFilesystem *fs, const char *folder, CameraList *list,
					 void *data, GPContext *context);

/**
 * List available files in the specified folder.
 *
 * This function is a CameraFilesystem method.
 */
int file_list_func(CameraFilesystem *fs, const char *folder, CameraList *list,
				   void *data, GPContext *context);

/**
 * Capture an image and tell libgphoto2 where to find it by filling
 * out the path.
 *
 * This function is a method of the Camera object.
 */
int camera_capture(Camera *camera, CameraCaptureType type, CameraFilePath *path, GPContext *context);

/**
 * Fill out the summary with textual information about the current
 * state of the camera (like pictures taken, etc.).
 *
 * This function is a method of the Camera object.
 */
int camera_summary(Camera *camera, CameraText *summary, GPContext *context);

/**
 * Return the camera drivers manual.
 * If you would like to tell the user some information about how
 * to use the camera or the driver, this is the place to do.
 *
 * This function is a method of the Camera object.
 */
static int camera_manual(Camera *camera, CameraText *manual, GPContext *context);

/**
 * Return "About" content as textual description.
 * Will be translated.
 *
 * This function is a method of the Camera object.
 */
int camera_about(Camera *camera, CameraText *about, GPContext *context);

int camera_wait_for_event(Camera *camera, int timeout, CameraEventType *eventtype, void **eventdata, GPContext *context);

/*@}*/

/**********************************************************************/
/**
 * @name CameraFilesystem member functions
 *
 * @{
 */
/**********************************************************************/

/**
 * Get the file from the camera.
 *
 * This function is a CameraFilesystem method.
 */

int get_file_func(CameraFilesystem *fs, const char *folder, const char *filename, CameraFileType type, CameraFile *file, void *data, GPContext *context);

int camera_config_get(Camera *camera, CameraWidget **window, GPContext *context);

int camera_config_set(Camera *camera, CameraWidget *window, GPContext *context);

int camera_abilities(CameraAbilitiesList *list);

/**
 * Initialize a Camera object.
 *
 * Sets up all the proper object function pointers, initialize camlib
 * internal data structures, and probably establish a connection to
 * the camera.
 *
 * This is a camlib API function.
 */
int camera_init(Camera *camera, GPContext *context);

int camera_id(CameraText *id);

#endif /* !defined(CAMLIBS_EM10MK1_LIBRARY_H) */
/** \file em10.c
 *
 * \author Copyright 2019 Robert Hasson <robert_hasson@yahoo.com>
 * \author Copyright 2019 Marcus Meissner <marcus@jet.franken.de>
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
#include "config.h"


#include <string.h>
#include <errno.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlreader.h>


#ifdef WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif
#include <stdlib.h>


#include <gphoto2/gphoto2-library.h>
#include <gphoto2/gphoto2-result.h>

#include "libgphoto2/i18n.h"


#define GP_MODULE "em10"

#define CHECK(result)                                   \
{                                                       \
	int res = (result);                             \
	\
	if (res < 0) {                                  \
		gp_log (GP_LOG_DEBUG, "em10", "Operation failed in %s (%i)!", __FUNCTION__, res);     \
		return (res); 	\
	}                      	\
}


static int
camera_exit (Camera *camera, GPContext *context)
{
	return GP_OK;
}

static int
camera_capture_preview (Camera *camera, CameraFile *file, GPContext *context)
{
	return GP_OK;
}

static int camera_about (Camera *camera, CameraText *about, GPContext *context);


/**
 * Put a file onto the camera.
 *
 * This function is a CameraFilesystem method.
 */
int
put_file_func (CameraFilesystem *fs, const char *folder, const char *name,
	       CameraFileType type, CameraFile *file, void *data, GPContext *context);
int
put_file_func (CameraFilesystem *fs, const char *folder, const char *name,
	       CameraFileType type, CameraFile *file, void *data, GPContext *context)
{
	/*Camera *camera = data;*/

	/*
	 * Upload the file to the camera. Use gp_file_get_data_and_size, etc
	 */

	return GP_OK;
}


/**
 * Delete a file from the camera.
 *
 * This function is a CameraFilesystem method.
 */
int
delete_file_func (CameraFilesystem *fs, const char *folder,
		  const char *filename, void *data, GPContext *context);
int
delete_file_func (CameraFilesystem *fs, const char *folder,
		  const char *filename, void *data, GPContext *context)
{
	/*Camera *camera = data;*/

	/* Delete the file from the camera. */

	return GP_OK;
}


/**
 * Delete all files from the camera.
 *
 * This function is a CameraFilesystem method.
 */
int
delete_all_func (CameraFilesystem *fs, const char *folder, void *data,
		 GPContext *context);
int
delete_all_func (CameraFilesystem *fs, const char *folder, void *data,
		 GPContext *context)
{
	/*Camera *camera = data;*/

	/*
	 * Delete all files in the given folder. If your camera doesn't have
	 * such a functionality, just don't implement this function.
	 */

	return GP_OK;
}


/**
 * Get the file info here and write it to space provided by caller.
 *
 * \param info Space provided by caller in which file info is written.
 *
 * This function is a CameraFilesystem method.
 */
int
get_info_func (CameraFilesystem *fs, const char *folder, const char *filename,
	       CameraFileInfo *info, void *data, GPContext *context);
int
get_info_func (CameraFilesystem *fs, const char *folder, const char *filename,
	       CameraFileInfo *info, void *data, GPContext *context)
{
	/*Camera *camera = data;*/

	return GP_OK;
}

#if 0
static int
set_info_func (CameraFilesystem *fs, const char *folder, const char *file,
	       CameraFileInfo info, void *data, GPContext *context)
{
	/*Camera *camera = data;*/

	/* Set the file info here from <info> */

	return GP_OK;
}
#endif

static int
folder_list_func (CameraFilesystem *fs, const char *folder, CameraList *list,
		  void *data, GPContext *context)
{
	/*Camera *camera = data;*/
	/* currently no folders exposed */
	return GP_OK;
}


/**
 * List available files in the specified folder.
 *
 * This function is a CameraFilesystem method.
 */
static int
file_list_func (CameraFilesystem *fs, const char *folder, CameraList *list,
		void *data, GPContext *context)
{
	return GP_OK;
}

/**
 * Get information on all available storages in the camera.
 *
 * This function is a CameraFilesystem method.
 */
int
storage_info_func (CameraFilesystem *fs,
		CameraStorageInformation **storageinformations,
		int *nrofstorageinformations, void *data,
		GPContext *context);
int
storage_info_func (CameraFilesystem *fs,
		CameraStorageInformation **storageinformations,
		int *nrofstorageinformations, void *data,
		GPContext *context)
{
	/*Camera *camera = data;*/

	/* List your storages here */

	return GP_ERROR_NOT_SUPPORTED;
}

/*@}*/


/**********************************************************************/
/**
 * @name camlib API functions
 *
 * @{
 */
/**********************************************************************/


/**
* Capture an image and tell libgphoto2 where to find it by filling
* out the path.
*
* This function is a method of the Camera object.
*/
static int
camera_capture (Camera *camera, CameraCaptureType type, CameraFilePath *path, GPContext *context) {
	return GP_OK;
}

/**
* Fill out the summary with textual information about the current
* state of the camera (like pictures taken, etc.).
*
* This function is a method of the Camera object.
*/
static int
camera_summary (Camera *camera, CameraText *summary, GPContext *context)
{
	return GP_OK;
}


/**
* Return the camera drivers manual.
* If you would like to tell the user some information about how
* to use the camera or the driver, this is the place to do.
*
* This function is a method of the Camera object.
*/
static int camera_manual (Camera *camera, CameraText *manual, GPContext *context) {
	return GP_OK;
}


/**
* Return "About" content as textual description.
* Will be translated.
*
* This function is a method of the Camera object.
*/
int  camera_about (Camera *camera, CameraText *about, GPContext *context);
int camera_about (Camera *camera, CameraText *about, GPContext *context) {
	strcpy (about->text, _("Olympus E-M10 WiFi Library\n"
	"Bruno Palos <brunopalos@gmail.com>\n"
	"Connects to Olympus E-M10 Cameras over Wifi.\n"
	"using the HTTP GET commands."));
	return GP_OK;
}

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
static int
get_file_func (CameraFilesystem *fs, const char *folder, const char *filename, CameraFileType type, CameraFile *file, void *data, GPContext *context)
{
	return GP_OK;
}


int camera_abilities (CameraAbilitiesList *list) {
	CameraAbilities a;

	memset(&a, 0, sizeof(a));
	strcpy(a.model, "Olympus:E-M10");
	a.status	= GP_DRIVER_STATUS_EXPERIMENTAL;
	a.port		= GP_PORT_IP;
	a.operations	= GP_CAPTURE_IMAGE;
	a.file_operations = GP_FILE_OPERATION_PREVIEW;
	/* it should be possible to browse and DL images the files using the ReadImageFromCamera() function but for now lets keep it simple*/
	a.folder_operations = GP_FOLDER_OPERATION_NONE;
	return gp_abilities_list_append(list, a);
}


/**
* Initialize a Camera object.
*
* Sets up all the proper object function pointers, initialize camlib
* internal data structures, and probably establish a connection to
* the camera.
*
* This is a camlib API function.
*/
int
camera_init (Camera *camera, GPContext *context)
{
	GPPortInfo      info;
	int		ret;
	int		tries;
	char		*result;

	camera->pl = calloc(sizeof(CameraPrivateLibrary),1);

	/* First, set up all the function pointers */
	camera->functions->exit                 = camera_exit;
	camera->functions->capture              = camera_capture;
	camera->functions->capture_preview      = camera_capture_preview;
	camera->functions->summary              = camera_summary;
	camera->functions->manual               = camera_manual;
	camera->functions->about                = camera_about;

	LIBXML_TEST_VERSION

	curl_global_init(CURL_GLOBAL_ALL);

	ret = gp_port_get_info (camera->port, &info);
	if (ret != GP_OK) {
		GP_LOG_E ("Failed to get port info?");
		return ret;
	}
	gp_filesystem_set_funcs (camera->fs, &fsfuncs, camera);

	return GP_OK;
}


int
camera_id (CameraText *id)
{
	strcpy(id->text, "Olympus E-M10 Wifi");

	return GP_OK;
}

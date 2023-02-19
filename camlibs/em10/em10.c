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
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <stdlib.h>

#include <gphoto2/gphoto2-library.h>
#include <gphoto2/gphoto2-result.h>

#include "libgphoto2/i18n.h"

#define GP_MODULE "em10"

#define CHECK(result)                                                                        \
	{                                                                                        \
		int res = (result);                                                                  \
                                                                                             \
		if (res < 0)                                                                         \
		{                                                                                    \
			gp_log(GP_LOG_DEBUG, "em10", "Operation failed in %s (%i)!", __FUNCTION__, res); \
			return (res);                                                                    \
		}                                                                                    \
	}

typedef struct
{
	char *data;
	size_t size;
} Em10MemoryBuffer;

typedef struct
{
	bool is_raw;
	char *dir_path;
	char *pic_path;
} Em10Picture;

struct _CameraPrivateLibrary
{
	/* all private data */
	int numpics;
	int liveview;
	Em10Picture *pics;
};

static int
camera_exit(Camera *camera, GPContext *context)
{
	return GP_OK;
}

static int
camera_capture_preview(Camera *camera, CameraFile *file, GPContext *context)
{
	return GP_OK;
}

static int camera_about(Camera *camera, CameraText *about, GPContext *context);

/**
 * Put a file onto the camera.
 *
 * This function is a CameraFilesystem method.
 */
int put_file_func(CameraFilesystem *fs, const char *folder, const char *name,
				  CameraFileType type, CameraFile *file, void *data, GPContext *context);
int put_file_func(CameraFilesystem *fs, const char *folder, const char *name,
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
int delete_file_func(CameraFilesystem *fs, const char *folder,
					 const char *filename, void *data, GPContext *context);
int delete_file_func(CameraFilesystem *fs, const char *folder,
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
int delete_all_func(CameraFilesystem *fs, const char *folder, void *data,
					GPContext *context);
int delete_all_func(CameraFilesystem *fs, const char *folder, void *data,
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
int get_info_func(CameraFilesystem *fs, const char *folder, const char *filename,
				  CameraFileInfo *info, void *data, GPContext *context);
int get_info_func(CameraFilesystem *fs, const char *folder, const char *filename,
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

static bool
is_subdir(const char *dir, const char *subdir)
{
	int dir_len = strlen(dir);
	if (strncmp(dir, subdir, dir_len) != 0)
	{
		return false;
	}
	return subdir[dir_len] == '/' && strchr(subdir + dir_len + 1, '/') == NULL;
}

static int
folder_list_func(CameraFilesystem *fs, const char *folder, CameraList *list,
				 void *data, GPContext *context)
{
	Camera *camera = data;
	// Iterate through all pictures in the library
	for (int i = 0; i < camera->pl->numpics; i++)
	{
		Em10Picture pic = camera->pl->pics[i];

		int offset = 1;
		int len = strlen(folder);
		if (folder[len - 1] == '/')
		{
			offset = 0;
		}
		char *subdir = strdup(pic.pic_path + strlen(folder) + offset);
		char *subdir_end = strchr(subdir, '/');
		if (subdir_end)
		{
			subdir_end = subdir_end;
			*subdir_end = '\0';

			if (strlen(subdir) > 0)
			{
				// Check if the picture is in the current folder
				if (gp_list_find_by_name(list, NULL, subdir) == GP_ERROR)
				{
					// Add the folder name to the list
					gp_list_append(list, subdir, NULL);
				}
			}
		}
	}

	return GP_OK;
}

/**
 * List available files in the specified folder.
 *
 * This function is a CameraFilesystem method.
 */
static int
file_list_func(CameraFilesystem *fs, const char *folder, CameraList *list,
			   void *data, GPContext *context)
{
	Camera *camera = data;
	int i;

	for (i = 0; i < camera->pl->numpics; i++)
	{
		if (camera->pl->pics[i].pic_path)
		{
			char *s = strrchr(camera->pl->pics[i].pic_path, '/') + 1;
			gp_list_append(list, s, NULL);
			continue;
		}
	}
	return GP_OK;
}

/**
 * Get information on all available storages in the camera.
 *
 * This function is a CameraFilesystem method.
 */
int storage_info_func(CameraFilesystem *fs,
					  CameraStorageInformation **storageinformations,
					  int *nrofstorageinformations, void *data,
					  GPContext *context);
int storage_info_func(CameraFilesystem *fs,
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

static size_t
write_callback(char *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	size_t oldsize;
	Em10MemoryBuffer *buffer = userp;

	oldsize = buffer->size;
	/* 1 additionally byte for 0x00 */
	buffer->data = realloc(buffer->data, buffer->size + realsize + 1);
	buffer->size += realsize;
	buffer->data[buffer->size] = 0x00;

	GP_LOG_DATA(contents, realsize, "em10 read from url");

	memcpy(buffer->data + oldsize, contents, realsize);
	return realsize;
}

static int
http_get(Camera *camera, char *cmd, Em10MemoryBuffer *buffer)
{
	// TODO - handle errors
	CURL *curl;
	CURLcode res;
	char URL[100];
	GPPortInfo info;
	char *xpath;

	curl = curl_easy_init();
	gp_port_get_info(camera->port, &info);
	gp_port_info_get_path(info, &xpath); /* xpath now contains ip:192.168.1.1 */
	snprintf(URL, 100, "http://%s/%s", xpath + strlen("ip:"), cmd);
	GP_LOG_D("cam url is %s", URL);

	curl_easy_setopt(curl, CURLOPT_URL, URL);

	buffer->size = 0;
	buffer->data = malloc(0);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);

	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return GP_ERROR;
	}
	else
	{
		GP_LOG_D("result was get size: %d\n", buffer->size);
	}
	curl_easy_cleanup(curl);
	return GP_OK;
}

static int
http_post(Camera *camera, char *cmd, char *post_body)
{
	// TODO - handle errors
	CURL *curl;
	CURLcode res;
	char URL[100];
	GPPortInfo info;
	char *xpath;
	Em10MemoryBuffer lmb;

	curl = curl_easy_init();
	gp_port_get_info(camera->port, &info);
	gp_port_info_get_path(info, &xpath); /* xpath now contains ip:192.168.1.1 */
	snprintf(URL, 100, "http://%s/%s", xpath + strlen("ip:"), cmd);
	GP_LOG_D("cam url is %s", URL);

	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_body));
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body);

	GP_LOG_D("posting %s", post_body);

	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return GP_ERROR;
	}
	else
	{
		GP_LOG_D("post successful\n");
	}
	curl_easy_cleanup(curl);
	return GP_OK;
}

static int
http_command(Camera *camera, char *cmd)
{
	Em10MemoryBuffer *buffer = malloc(sizeof(Em10MemoryBuffer));
	int ret = http_get(camera, cmd, buffer);
	free(buffer);
	return ret;
}

static int
startCapture(Camera *camera)
{
	return http_command(camera, "exec_shutter.cgi?com=1st2ndpush");
}

static int
stopCapture(Camera *camera)
{
	return http_command(camera, "exec_shutter.cgi?com=2nd1strelease");
}

static int
switch_to_shutter_mode(Camera *camera)
{
	return http_command(camera, "switch_cammode.cgi?mode=shutter");
}

static int
switch_to_rec_mode(Camera *camera)
{
	return http_command(camera, "switch_cammode.cgi?mode=rec");
}

static int
switch_to_play_mode(Camera *camera)
{
	return http_command(camera, "switch_cammode.cgi?mode=play");
}

static int get_dcf_file_num(Camera *camera)
{
	switch_to_play_mode(camera);
	Em10MemoryBuffer *buffer = malloc(sizeof(Em10MemoryBuffer));
	// TODO - handle errors
	http_get(camera, "get_dcffilenum.cgi", buffer);
	char *temp = buffer->data;
	xmlDocPtr doc = xmlParseDoc((unsigned char *)temp);
	xmlNodePtr cur = NULL;

	// TODO - handle errors and unexpected responses

	cur = xmlDocGetRootElement(doc);
	xmlChar *str_file_num = xmlNodeGetContent(cur);

	free(buffer);
	return (int)strtol((char *)str_file_num, NULL, 10);
}

static void load_image_list(Camera *camera)
{
	// Set the number of files
	int num_pics;

	num_pics = get_dcf_file_num(camera);

	if (camera->pl->numpics < num_pics)
	{
		camera->pl->pics = realloc(camera->pl->pics, num_pics * sizeof(camera->pl->pics[0]));
		memset(camera->pl->pics + camera->pl->numpics, 0, (num_pics - camera->pl->numpics) * sizeof(camera->pl->pics[0]));
		camera->pl->numpics = num_pics;
	}

	// TODO - discover olympus directories & handle errors
	Em10MemoryBuffer *buffer = malloc(sizeof(Em10MemoryBuffer));
	http_get(camera, "get_imglist.cgi?DIR=/DCIM/100OLYMP", buffer);
	char *response = buffer->data;

	int token_pos = 0;
	int picture_pos = 0;
	char *dir;

	char *token = strtok(response, ", \n");
	token = strtok(NULL, ", \n"); // skip version token
	while (token != NULL)
	{
		if (token_pos == 6)
		{
			token_pos = 0;
			picture_pos++;
		}

		if (token_pos == 0)
			dir = token;
		else if (token_pos == 1)
		{
			char *pic_path = malloc(strlen(dir) + strlen(token) + 1);
			strcpy(pic_path, dir);
			strcat(pic_path, "/");
			strcat(pic_path, token);
			camera->pl->pics[picture_pos].pic_path = pic_path;
			camera->pl->pics[picture_pos].dir_path = dir;
		}

		token = strtok(NULL, ", \n");
		token_pos++;
	}
	free(buffer);
	return;
}

/**
 * Capture an image and tell libgphoto2 where to find it by filling
 * out the path.
 *
 * This function is a method of the Camera object.
 */
static int
camera_capture(Camera *camera, CameraCaptureType type, CameraFilePath *path, GPContext *context)
{
	printf("Capturing\n");

	int ret;
	char *s, *url;

	switch_to_shutter_mode(camera);

	ret = startCapture(camera);
	if (ret != GP_OK)
		return ret;

	stopCapture(camera);

	printf("Captured\n");

	if (ret < GP_OK)
		return ret;

	// s = strrchr(url,'/')+1;
	// strcpy(path->name, s);
	// strcpy(path->folder, "/");

	return GP_OK;
}

/**
 * Fill out the summary with textual information about the current
 * state of the camera (like pictures taken, etc.).
 *
 * This function is a method of the Camera object.
 */
static int
camera_summary(Camera *camera, CameraText *summary, GPContext *context)
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
static int camera_manual(Camera *camera, CameraText *manual, GPContext *context)
{
	return GP_OK;
}

/**
 * Return "About" content as textual description.
 * Will be translated.
 *
 * This function is a method of the Camera object.
 */
int camera_about(Camera *camera, CameraText *about, GPContext *context);
int camera_about(Camera *camera, CameraText *about, GPContext *context)
{
	strcpy(about->text, _("Olympus E-M10 WiFi Library\n"
						  "Bruno Palos <brunopalos@gmail.com>\n"
						  "Connects to Olympus E-M10 Cameras over Wifi.\n"
						  "using the HTTP commands."));
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
get_file_func(CameraFilesystem *fs, const char *folder, const char *filename, CameraFileType type, CameraFile *file, void *data, GPContext *context)
{
	Camera *camera = data;
	int i;
	CURLcode res;
	CURL *imageUrl;
	long http_response;
	int ret_val = 0;
	char url[100];

	switch (type)
	{
	case GP_FILE_TYPE_PREVIEW:
		strcat(url, "get_thumbnail.cgi?DIR=");
		break;
	case GP_FILE_TYPE_NORMAL:
		gp_file_set_mime_type(file, GP_MIME_JPEG);
		break;
	default:
		break;
	}

	for (i = 0; i < camera->pl->numpics; i++)
	{
		char *s;

		if (camera->pl->pics[i].pic_path)
		{
			s = strrchr(camera->pl->pics[i].pic_path, '/') + 1;
			if (!strcmp(s, filename))
			{
				strcat(url, camera->pl->pics[i].pic_path);
				break;
			}
		}
	}
	if (i == camera->pl->numpics) /* not found */
		return GP_ERROR;

	switch_to_play_mode(camera);

	Em10MemoryBuffer *buffer = malloc(sizeof(Em10MemoryBuffer));
	int ret = http_get(camera, url, buffer);
	int data_set_ret = gp_file_set_data_and_size(file, buffer->data, buffer->size);

	free(buffer);
	return data_set_ret;
}

static int
camera_config_get(Camera *camera, CameraWidget **window, GPContext *context)
{
	CameraWidget *widget, *section;

	switch_to_rec_mode(camera);

	gp_widget_new(GP_WIDGET_WINDOW, _("EM-10 Configuration"), window);
	gp_widget_set_name(*window, "config");

	gp_widget_new(GP_WIDGET_SECTION, _("Camera Settings"), &section);
	gp_widget_set_name(section, "settings");
	gp_widget_append(*window, section);

	Em10MemoryBuffer *buffer = malloc(sizeof(Em10MemoryBuffer));
	// TODO - handle errors
	http_get(camera, "get_camprop.cgi?com=desc&propname=desclist", buffer);
	char *temp = buffer->data;
	xmlDocPtr doc = xmlParseDoc((unsigned char *)temp);
	xmlNodePtr cur = NULL;

	// TODO - handle errors and unexpected responses

	cur = xmlDocGetRootElement(doc);
	xmlNodePtr next_property = xmlFirstElementChild(cur);

	while (next_property != NULL)
	{
		// propname element
		xmlNode *property = next_property->children;
		xmlChar *property_content = xmlNodeGetContent(property);
		char *property_name = (char *)property_content;

		// attribute element
		xmlNode *attribute = property->next;

		// value element
		xmlNode *value = attribute->next;
		xmlChar *value_content = xmlNodeGetContent(value);

		// enum element
		xmlNode *property_enum = value->next;
		if (property_enum != NULL)
		{
			gp_widget_new(GP_WIDGET_RADIO, _(property_name), &widget);

			xmlChar *property_enum_content = xmlNodeGetContent(property_enum);
			char *choice = strtok((char *)property_enum_content, " ");
			while (choice != NULL)
			{
				gp_widget_add_choice(widget, choice);
				choice = strtok(NULL, " ");
			}
		}
		else
		{
			gp_widget_new(GP_WIDGET_TEXT, _(property_name), &widget);
		}
		gp_widget_set_name(widget, property_name);
		gp_widget_set_value(widget, (char *)value_content);
		gp_widget_append(section, widget);

		next_property = next_property->next;
	}

	int valset = 2;
	gp_widget_new (GP_WIDGET_TOGGLE, _("Bulb"), &widget);
	gp_widget_set_name (widget, "bulb");
	gp_widget_set_value (widget, &valset);
	gp_widget_append (section, widget);

	free(buffer);

	return GP_OK;
}

static int
camera_config_set(Camera *camera, CameraWidget *window, GPContext *context)
{
	int ret;
	CameraWidget *settings;

	gp_widget_get_child(window, 0, &settings);

	int widgets_count = gp_widget_count_children(settings);

	for (int i = 0; i < widgets_count; i++)
	{
		char **widget_label;
		CameraWidget *widget;

		gp_widget_get_child(settings, i, &widget);
		gp_widget_get_label(widget, &widget_label);

		if ((GP_OK == gp_widget_get_child_by_label(window, widget_label, &widget)) && gp_widget_changed(widget))
		{
			char *value;
			char cmd[50];
			char http_body[150];

			if (GP_OK != (ret = gp_widget_get_value(widget, &value)))
				return ret;
			snprintf(cmd, 50, "set_camprop.cgi?com=set&propname=%s", widget_label);
			snprintf(http_body, 150, "<set><value>%s</value></set>", value);

			http_post(camera, cmd, http_body);
		}
	}

	return GP_OK;
}

int camera_abilities(CameraAbilitiesList *list)
{
	printf("camera_abilities");
	CameraAbilities a;

	memset(&a, 0, sizeof(a));
	strcpy(a.model, "Olympus:E-M10");
	a.status = GP_DRIVER_STATUS_EXPERIMENTAL;
	a.port = GP_PORT_IP;
	a.operations = GP_CAPTURE_IMAGE;
	a.file_operations = GP_FILE_OPERATION_PREVIEW;
	/* it should be possible to browse and DL images the files using the ReadImageFromCamera() function but for now lets keep it simple*/
	a.folder_operations = GP_FOLDER_OPERATION_NONE;
	return gp_abilities_list_append(list, a);
}

/**
 * All filesystem accessor functions.
 *
 * This should contain all filesystem accessor functions
 * available in the camera library. Non-present fields
 * are NULL.
 *
 */
CameraFilesystemFuncs fsfuncs = {
	.file_list_func = file_list_func,
	.folder_list_func = folder_list_func,
	//	.get_info_func = get_info_func,
	//	.set_info_func = set_info_func,
	.get_file_func = get_file_func,
	//	.del_file_func = delete_file_func,
	//	.put_file_func = put_file_func,
	//	.delete_all_func = delete_all_func,
	//	.storage_info_func = storage_info_func
};

/**
 * Initialize a Camera object.
 *
 * Sets up all the proper object function pointers, initialize camlib
 * internal data structures, and probably establish a connection to
 * the camera.
 *
 * This is a camlib API function.
 */
int camera_init(Camera *camera, GPContext *context)
{
	GPPortInfo info;
	int ret;
	int tries;
	char *result;
	int file_num;

	camera->pl = calloc(sizeof(CameraPrivateLibrary), 1);

	/* First, set up all the function pointers */
	camera->functions->exit = camera_exit;
	camera->functions->get_config = camera_config_get;
	camera->functions->set_config = camera_config_set;
	camera->functions->capture = camera_capture;
	camera->functions->capture_preview = camera_capture_preview;
	camera->functions->summary = camera_summary;
	camera->functions->manual = camera_manual;
	camera->functions->about = camera_about;

	LIBXML_TEST_VERSION

	curl_global_init(CURL_GLOBAL_ALL);

	ret = gp_port_get_info(camera->port, &info);
	if (ret != GP_OK)
	{
		GP_LOG_E("Failed to get port info?");
		return ret;
	}
	gp_filesystem_set_funcs(camera->fs, &fsfuncs, camera);

	file_num = get_dcf_file_num(camera);
	load_image_list(camera);
	return GP_OK;
}

int camera_id(CameraText *id)
{
	strcpy(id->text, "Olympus E-M10 Wifi");

	return GP_OK;
}

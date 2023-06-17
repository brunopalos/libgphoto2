/** \file library.c
 *
 * \author Copyright 2023 Bruno Palos
 * 
 * \par
 * This library was only tested for EM-10 Mark I.
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
#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlreader.h>
#include <sys/time.h>

#include <gphoto2/gphoto2-library.h>
#include <gphoto2/gphoto2-result.h>

#include "libgphoto2/i18n.h"
#include "liveview.h"
#include "common.h"
#include "library.h"
#include "protocol.h"

static void
print_to_file(char *fmt, ...)
{
	va_list argp;
	FILE *fptr;

	// use appropriate location if you are using MacOS or Linux
	fptr = fopen("/tmp/em10.log", "a");

	if (fptr == NULL)
	{
		printf("Error!");
		exit(1);
	}

	va_start(argp, fmt);

	vfprintf(fptr, fmt, argp);

	va_end(argp);
	fclose(fptr);
}

int
camera_exit(Camera *camera, GPContext *context)
{
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
					  GPContext *context)
{
	Camera *camera = data;
	CameraStorageInformation *sinfo;

	sinfo = malloc(sizeof(CameraStorageInformation));
	if (!sinfo)
		return GP_ERROR_NO_MEMORY;

	*storageinformations = sinfo;
	*nrofstorageinformations = 1;

	int unused_capacity = get_unused_capacity(camera);

	sinfo->fields  = GP_STORAGEINFO_BASE;
	strcpy(sinfo->basedir, "/");
	sinfo->fields |= GP_STORAGEINFO_ACCESS;
	sinfo->access  = GP_STORAGEINFO_AC_READONLY;
	sinfo->fields |= GP_STORAGEINFO_STORAGETYPE;
	sinfo->type    = GP_STORAGEINFO_ST_REMOVABLE_RAM;
	sinfo->fields |= GP_STORAGEINFO_FILESYSTEMTYPE;
	sinfo->fstype  = GP_STORAGEINFO_FST_DCF;
	sinfo->fields |= GP_STORAGEINFO_FREESPACEKBYTES;
	sinfo->freekbytes = le32toh(unused_capacity) / 1024;

	return GP_OK;
}

int
camera_capture_preview(Camera *camera, CameraFile *file, GPContext *context)
{
	bool has_picture = false;

	if (camera->pl->cam_mode != Rec)
	{
		if (camera->pl->liveview)
		{

		}
	}

	if (!camera->pl->liveview)
	{
		print_to_file("First time streaming\n");
		switch_to_rec_mode(camera);
		start_liveview(camera);

		if (start_streaming(camera) != GP_OK) {
			return GP_ERROR;
		}
	}
	else if (camera->pl->cam_mode != Rec)
	{
			print_to_file("Already streaming\n");
			switch_to_rec_mode(camera);
			start_liveview(camera);
	}

	Picture *picture = new_picture();

	while (!has_picture)
	{
		Frame *frame = malloc(sizeof(Frame));
		generate_frame(camera->pl->udpsocket, frame);
		if (frame->buffer == NULL)
		{
			break;
		}

		has_picture = add_data(picture, frame);
		if (has_picture)
		{
			if (picture->jpeg_len > 0)
			{
				gp_file_set_mime_type(file, GP_MIME_JPEG);
				int result = gp_file_append(file, picture->jpeg, picture->jpeg_len);

				free(frame);
				return result;
			} 
			else 
			{
				print_to_file("Empty frame\n");
			}
		}
		free(frame);
	}
	return GP_ERROR;
}

int
folder_list_func(CameraFilesystem *fs, const char *folder, CameraList *list,
				 void *data, GPContext *context)
{
	print_to_file("folder_list_func. Folder: %s\n", folder);
	Camera *camera = data;
	load_image_list(camera);

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
int
file_list_func(CameraFilesystem *fs, const char *folder, CameraList *list,
			   void *data, GPContext *context)
{
	print_to_file("file_list_func. Folder: %s\n", folder);
	Camera *camera = data;
	int i;

	load_image_list(camera);

	for (i = 0; i < camera->pl->numpics; i++)
	{
		char tmp[100];
		char *filename = strrchr(camera->pl->pics[i].pic_path, '/') + 1;
		strcpy(tmp, folder);
		strcat(tmp, "/");
		strcat(tmp, filename);
		if (!strcmp(tmp, camera->pl->pics[i].pic_path))
		{
			gp_list_append(list, filename, NULL);
			continue;
		}
	}
	return GP_OK;
}

/**
 * Capture an image and tell libgphoto2 where to find it by filling
 * out the path.
 *
 * This function is a method of the Camera object.
 */
int
camera_capture(Camera *camera, CameraCaptureType type, CameraFilePath *path, GPContext *context)
{
	print_to_file("camera_capture\n");

	int ret;
	char *s, *url;

	switch_to_shutter_mode(camera);

	ret = start_capture(camera);
	if (ret != GP_OK)
		return ret;

	stop_capture(camera);

	print_to_file("Captured\n");

	if (ret < GP_OK)
		return ret;

	int num_pics = get_dcf_file_num(camera);
	load_image_list(camera);
	strcpy(path->folder, camera->pl->pics[num_pics - 1].dir_path);
	strcpy(path->name, camera->pl->pics[num_pics - 1].name);

	return GP_OK;
}

/**
 * Fill out the summary with textual information about the current
 * state of the camera (like pictures taken, etc.).
 *
 * This function is a method of the Camera object.
 */
int
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
int camera_about(Camera *camera, CameraText *about, GPContext *context)
{
	strcpy(about->text, _("Olympus E-M10 WiFi Library\n"
						  "Bruno Palos <brunopalos@gmail.com>\n"
						  "Connects to Olympus E-M10 Cameras over Wifi.\n"
						  "using the HTTP commands."));
	return GP_OK;
}

static int
_timeout_passed(struct timeval *start, int timeout)
{
	struct timeval curtime;

	gettimeofday(&curtime, NULL);
	return ((curtime.tv_sec - start->tv_sec) * 1000) + ((curtime.tv_usec - start->tv_usec) / 1000) >= timeout;
}

int
camera_wait_for_event(Camera *camera, int timeout, CameraEventType *eventtype, void **eventdata, GPContext *context)
{
	print_to_file("camera_wait_for_event\n");
	struct timeval event_start;
	CameraFilePath *path;

	*eventtype = GP_EVENT_TIMEOUT;
	*eventdata = NULL;

	gettimeofday(&event_start, 0);
	while (1)
	{
		path = malloc(sizeof(CameraFilePath));
		int num_pics = get_dcf_file_num(camera);
		print_to_file("existing num_pics: %d\n", camera->pl->numpics);
		print_to_file("new num_pics: %d\n", num_pics);
		if (num_pics > camera->pl->numpics)
		{
			load_image_list(camera);
			strcpy(path->folder, camera->pl->pics[num_pics - 1].dir_path);
			strcpy(path->name, camera->pl->pics[num_pics - 1].name);
			print_to_file("file added: %s/%s\n", path->folder, path->name);
			gp_filesystem_reset(camera->fs);
			*eventtype = GP_EVENT_FILE_ADDED;
			*eventdata = path;
			return GP_OK;
		}

		free(path);

		if (_timeout_passed(&event_start, timeout))
			break;
		usleep(1000 * 1000); /* 1000 ms */
	}
	print_to_file("event type: %d\n", *eventtype);
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
int
get_file_func(CameraFilesystem *fs, const char *folder, const char *filename, CameraFileType type, CameraFile *file, void *data, GPContext *context)
{

	print_to_file("get_file_func\n");
	Camera *camera = data;
	int i;
	int ret_val = 0;
	char url[100];

	load_image_list(camera);

	switch (type)
	{
	case GP_FILE_TYPE_PREVIEW:
		strcpy(url, "get_thumbnail.cgi?DIR=");
		break;
	case GP_FILE_TYPE_NORMAL:
		gp_file_set_mime_type(file, GP_MIME_JPEG);
		strcpy(url, "");
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
				GP_LOG_D("cmd is %s", camera->pl->pics[i].pic_path);
				GP_LOG_D("url is %s", url);
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

int
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

	gp_widget_new(GP_WIDGET_TOGGLE, _("Bulb"), &widget);
	gp_widget_set_name(widget, "bulb");
	gp_widget_set_value(widget, FALSE);
	gp_widget_append(section, widget);

	free(buffer);

	return GP_OK;
}

int
camera_config_set(Camera *camera, CameraWidget *window, GPContext *context)
{
	print_to_file("camera_config_set\n");

	int ret;
	CameraWidget *settings;

	gp_widget_get_child(window, 0, &settings);

	int widgets_count = gp_widget_count_children(settings);

	for (int i = 0; i < widgets_count; i++)
	{
		const char *widget_label;
		CameraWidget *widget;

		gp_widget_get_child(settings, i, &widget);
		gp_widget_get_label(widget, &widget_label);
		print_to_file("camera_config_set: %s\n", widget_label);

		if (GP_OK == gp_widget_get_child_by_name(window, "bulb", &widget) && gp_widget_changed(widget))
		{
			int val;

			if (GP_OK != (ret = gp_widget_get_value(widget, &val)))
				return ret;

			gp_widget_set_changed(widget, 0);

			if (val)
			{
				switch_to_shutter_mode(camera);
				ret = start_capture(camera);
				if (ret != GP_OK)
					return ret;
			}
			else
			{
				stop_capture(camera);
			}
		}
		else if ((GP_OK == gp_widget_get_child_by_label(window, widget_label, &widget)) && gp_widget_changed(widget))
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
	print_to_file("camera_abilities\n");
	CameraAbilities a;

	memset(&a, 0, sizeof(a));
	strcpy(a.model, "Olympus:E-M10");
	a.status = GP_DRIVER_STATUS_EXPERIMENTAL;
	a.port = GP_PORT_IP;
	a.operations = GP_OPERATION_CAPTURE_IMAGE | GP_OPERATION_CAPTURE_VIDEO | GP_OPERATION_CONFIG | GP_OPERATION_TRIGGER_CAPTURE;
	a.file_operations = GP_FILE_OPERATION_NONE;
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
	.get_file_func = get_file_func,
	.storage_info_func = storage_info_func
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
	print_to_file("camera_init\n");
	int ret;
	int tries;
	char *result;
	int file_num;

	camera->pl = calloc(sizeof(CameraPrivateLibrary), 1);
	camera->pl->cam_mode = Shutter;

	/* First, set up all the function pointers */
	camera->functions->exit = camera_exit;
	camera->functions->get_config = camera_config_get;
	camera->functions->set_config = camera_config_set;
	camera->functions->capture = camera_capture;
	camera->functions->capture_preview = camera_capture_preview;
	camera->functions->summary = camera_summary;
	camera->functions->manual = camera_manual;
	camera->functions->about = camera_about;
	camera->functions->wait_for_event = camera_wait_for_event;

	init_protocol();

	gp_filesystem_set_funcs(camera->fs, &fsfuncs, camera);
	load_image_list(camera);
	return GP_OK;
}

int camera_id(CameraText *id)
{
	strcpy(id->text, "Olympus Wifi");

	return GP_OK;
}

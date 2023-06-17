/** \file protocol.c
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

#include <gphoto2/gphoto2-library.h>
#include <gphoto2/gphoto2-result.h>

#include "libgphoto2/i18n.h"
#include "liveview.h"
#include "common.h"
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

static int
http_command(Camera *camera, char *cmd)
{
	print_to_file("http_command\n");
	Em10MemoryBuffer *buffer = malloc(sizeof(Em10MemoryBuffer));
	int ret = http_get(camera, cmd, buffer);
	free(buffer);
	return ret;
}


size_t
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

int
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

int
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

/*@}*/

/**********************************************************************/
/**
 * @name camlib API functions
 *
 * @{
 */
/**********************************************************************/

int
start_capture(Camera *camera)
{
	print_to_file("start_capture\n");
	return http_command(camera, "exec_shutter.cgi?com=1st2ndpush");
}

int
stop_capture(Camera *camera)
{
	print_to_file("stop_capture\n");
	return http_command(camera, "exec_shutter.cgi?com=2nd1strelease");
}

int
start_liveview(Camera *camera)
{
	print_to_file("start_liveview\n");
	return http_command(camera, "exec_takemisc.cgi?com=startliveview&port=23333");
}

int
stop__liveview(Camera *camera)
{
	print_to_file("stop__liveview\n");
	return http_command(camera, "exec_takemisc.cgi?com=stopliveview&port=23333");
}

int
switch_to_shutter_mode(Camera *camera)
{
	print_to_file("switch_to_shutter_mode\n");
	camera->pl->cam_mode = Shutter;
	int ret = http_command(camera, "switch_cammode.cgi?mode=shutter");
	usleep(1000 * 1000); /* 1000 ms */
	return ret;
}

int
switch_to_rec_mode(Camera *camera)
{
	print_to_file("switch_to_rec_mode\n");
	camera->pl->cam_mode = Rec;
	int ret = http_command(camera, "switch_cammode.cgi?mode=rec&lvqty=0640x0480");
	usleep(1000 * 1000); /* 1000 ms */
	return ret;
}

int
switch_to_play_mode(Camera *camera)
{
	print_to_file("switch_to_play_mode\n");
	camera->pl->cam_mode = Play;
	print_to_file("switch_to_play_mode 2\n");
	int ret = http_command(camera, "switch_cammode.cgi?mode=play");
	print_to_file("switch_to_play_mode 3\n");
	usleep(1000 * 1000); /* 1000 ms */
	return ret;
}

int get_dcf_file_num(Camera *camera)
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

int get_unused_capacity(Camera *camera)
{
	switch_to_play_mode(camera);
	Em10MemoryBuffer *buffer = malloc(sizeof(Em10MemoryBuffer));
	// TODO - handle errors
	http_get(camera, "get_unusedcapacity.cgi", buffer);
	char *temp = buffer->data;
	xmlDocPtr doc = xmlParseDoc((unsigned char *)temp);
	xmlNodePtr cur = NULL;

	// TODO - handle errors and unexpected responses

	cur = xmlDocGetRootElement(doc);
	xmlChar *unused_capacity_bytes = xmlNodeGetContent(cur);

	free(buffer);
	return (int)strtol((char *)unused_capacity_bytes, NULL, 10);
}

void load_image_list(Camera *camera)
{
	print_to_file("load_image_list\n");
	// Set the number of files
	int num_pics;

	num_pics = get_dcf_file_num(camera);
	print_to_file("num_pics: %d\n", num_pics);

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
			camera->pl->pics[picture_pos].name = token;
		}

		token = strtok(NULL, ", \n");
		token_pos++;
	}
	free(buffer);
	return;
}

void
init_protocol()
{
	print_to_file("init_protocol\n");
	curl_global_init(CURL_GLOBAL_ALL);
	print_to_file("init_protocol 2\n");
	LIBXML_TEST_VERSION
	print_to_file("init_protocol_end\n");
}

int start_streaming(Camera *camera)
{
	struct sockaddr_in serv_addr;
	camera->pl->liveview = 1;
	if (camera->pl->udpsocket <= 0)
	{
		print_to_file("Creating socket\n");
		if ((camera->pl->udpsocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			GP_LOG_E("\n Socket creation error \n");
			return GP_ERROR;
		}

		memset(&serv_addr, 0, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(23333);
		serv_addr.sin_addr.s_addr = 0;

		if (bind(camera->pl->udpsocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			GP_LOG_E("bind Failed: %d", errno);
			return GP_ERROR;
		}
	}

	return GP_OK;
}

/** \file protocol.h
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
#ifndef CAMLIBS_EM10_PROTOCOL_H
#define CAMLIBS_EM10_PROTOCOL_H

size_t
write_callback(char *contents, size_t size, size_t nmemb, void *userp);

int
http_get(Camera *camera, char *cmd, Em10MemoryBuffer *buffer);

int
http_post(Camera *camera, char *cmd, char *post_body);


/*@}*/

/**********************************************************************/
/**
 * @name camlib API functions
 *
 * @{
 */
/**********************************************************************/

int start_capture(Camera *camera);

int stop_capture(Camera *camera);

int start_liveview(Camera *camera);

int stop__liveview(Camera *camera);

int switch_to_shutter_mode(Camera *camera);

int switch_to_rec_mode(Camera *camera);

int switch_to_play_mode(Camera *camera);

int get_dcf_file_num(Camera *camera);

int get_unused_capacity(Camera *camera);

void load_image_list(Camera *camera);

int start_streaming(Camera *camera);

void init_protocol();

#endif /* !defined(CAMLIBS_EM10_PROTOCOL_H) */
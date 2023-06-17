/** \file livevew.h
 * 
 * \author Copyright 2023 Bruno Palos
 * 
 * \par
 * The liveview for EM-10 Mark I was inspired from 
 * \link https://github.com/misaka4e21/olympus-liveview and
 * implemented following the Liveview specification  from the OPC
 * Communication Protocol, which can be downloaded from
 * \link https://dl-support.olympus-imaging.com/opc/en/.
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
#ifndef CAMLIBS_EM10_LIVEVIEW_H
#define CAMLIBS_EM10_LIVEVIEW_H

typedef enum
{
    First,
    Middle,
    End
} PacketType;

typedef struct
{
    PacketType packet_type;
    uint32_t stream_number;
    uint32_t frame_number;
    uint16_t chunk_number;
    uint8_t buffer[1500];
    uint16_t buffer_len;
} Frame;

typedef struct
{
    char *jpeg;
    size_t jpeg_len;
    uint16_t current_chunk_number;
    uint32_t stream_number;
    uint32_t frame_number;
} Picture;

Picture *new_picture();
int generate_frame(int sockfd, Frame *frame);
bool add_data(Picture *pic, Frame *frame);


#endif /* !defined(CAMLIBS_EM10_LIVEVIEW_H) */
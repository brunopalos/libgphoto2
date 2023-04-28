#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdarg.h>
#include "liveview.h"

uint16_t u8_to_u16(const uint8_t *buf)
{
    return (uint16_t)buf[0] * 0x100 + (uint16_t)buf[1];
}

uint32_t u8_to_u32(const uint8_t *buf)
{
    return (uint32_t)u8_to_u16(&buf[0]) * 0x10000 + (uint32_t)u8_to_u16(&buf[2]);
}

int generate_frame(int sockfd, Frame *frame)
{
    uint8_t buf[1500];
    struct sockaddr src_addr;
    socklen_t addrlen = sizeof(src_addr);
    ssize_t num_bytes = recvfrom(sockfd, buf, sizeof(buf), 0, &src_addr, &addrlen);
    if (num_bytes == -1)
    {
        return -1;
    }
    uint16_t packet_type_code = u8_to_u16(&buf[0]);

    PacketType packet_type;
    switch (packet_type_code)
    {
    case 0x9060:
        packet_type = First;
        break;
    case 0x8060:
        packet_type = Middle;
        break;
    case 0x80e0:
        packet_type = End;
        break;
    default:
        return 0;
    }

    memcpy(frame->buffer, buf, num_bytes);
    frame->buffer_len = num_bytes;
    frame->packet_type = packet_type;
    frame->chunk_number = u8_to_u16(&buf[2]);
    frame->frame_number = u8_to_u32(&buf[4]);
    frame->stream_number = u8_to_u32(&buf[8]);
    return 1;
}

uint8_t *find_jpeg(const uint8_t *buf, uint16_t buf_len, uint16_t* jpeg_len)
{
    uint8_t *result_buf;
    for (size_t i = 12; i < buf_len - 2; i++)
    {
        if (u8_to_u16(&buf[i]) == 0xffd8)
        {
            *jpeg_len = buf_len - i;
            result_buf = malloc(*jpeg_len);
            memcpy(result_buf, &buf[i], *jpeg_len);
            return result_buf;
        }
    }
    *jpeg_len = buf_len - 12;
    result_buf = malloc(*jpeg_len);
    memcpy(result_buf, &buf[12], *jpeg_len);
    return result_buf;
}

Picture *new_picture()
{
    Picture *pic = malloc(sizeof(Picture));
    pic->jpeg = NULL;
    pic->jpeg_len = 0;
    pic->current_chunk_number = 0;
    pic->stream_number = 0;
    pic->frame_number = 0;
    return pic;
}

void add_initial_data(Picture *pic, Frame *frame)
{
    pic->current_chunk_number = frame->chunk_number;
    pic->frame_number = frame->frame_number;
    pic->stream_number = frame->stream_number;
    uint16_t *jpeg_len = malloc(sizeof(uint16_t));
    uint8_t *jpeg = find_jpeg(frame->buffer, frame->buffer_len, jpeg_len);
    pic->jpeg = malloc(*jpeg_len);
    memcpy(pic->jpeg, jpeg, *jpeg_len);
    pic->jpeg_len = *jpeg_len;

    free(jpeg_len);
}

void add_appended_data(Picture *pic, Frame *frame)
{
    if (frame->frame_number == pic->frame_number)
    {
        size_t appended_jpeg_len = frame->buffer_len - 12;
        if (appended_jpeg_len > 0)
        {
            size_t old_jpeg_len = pic->jpeg_len;
            pic->jpeg_len += appended_jpeg_len;
            pic->jpeg = realloc(pic->jpeg, pic->jpeg_len);
            memcpy(pic->jpeg + old_jpeg_len, frame->buffer + 12, appended_jpeg_len);
        }
    }
}

bool add_data(Picture *pic, Frame *frame)
{
    bool is_end_packet = false;
    switch (frame->packet_type)
    {
    case First:
    {
        add_initial_data(pic, frame);
        break;
    }
    case End:
    {
        add_appended_data(pic, frame);
        is_end_packet = true;
        break;
    }
    default:
    {
        add_appended_data(pic, frame);
        break;
    }
    }
    return is_end_packet;
}

void free_picture(Picture *pic)
{
    free(pic->jpeg);
    pic->jpeg = NULL;
    pic->jpeg_len = 0;
    pic->current_chunk_number = 0;
    pic->stream_number = 0;
    pic->frame_number = 0;
}

int main()
{
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(23333);

    if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Failed to bind socket");
        return EXIT_FAILURE;
    }

    Picture *picture = new_picture();

    while (true)
    {
        Frame *frame = malloc(sizeof(Frame));
        generate_frame(socket_fd, frame);
        if (frame->buffer == NULL)
        {
            break;
        }

        bool is_end = add_data(picture, frame);
        if (is_end)
        {
            if (write(STDOUT_FILENO, picture->jpeg, picture->jpeg_len) < 0)
            {
                perror("Failed to write to stdout");
                return EXIT_FAILURE;
            }
        }
            free(frame);
    }

    free_picture(picture);

    return EXIT_SUCCESS;
}
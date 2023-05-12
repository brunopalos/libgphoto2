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
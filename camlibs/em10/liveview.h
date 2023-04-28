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
    uint8_t *jpeg;
    size_t jpeg_len;
    uint16_t current_chunk_number;
    uint32_t stream_number;
    uint32_t frame_number;
} Picture;

// TODO

#endif /* !defined(CAMLIBS_EM10_LIVEVIEW_H) */
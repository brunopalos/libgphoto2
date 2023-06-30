#include <stddef.h>

#ifndef CAMLIBS_EM10MK1_COMMON_H
#define CAMLIBS_EM10MK1_COMMON_H

typedef struct
{
	char *data;
	size_t size;
} Em10MemoryBuffer;

typedef struct
{
	bool is_raw;
	char *name;
	char *dir_path;
	char *pic_path;
} Em10Picture;

typedef enum
{
	Play,
	Rec,
	Shutter
} EM10CamMode;

struct _CameraPrivateLibrary
{
	/* all private data */
	EM10CamMode cam_mode;
	int numpics;
	int liveview;
	int udpsocket;
	Em10Picture *pics;
};

#endif /* !defined(CAMLIBS_EM10MK1_COMMON_H) */
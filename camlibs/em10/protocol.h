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
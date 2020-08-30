/* fujiptpip.c
 *
 * Copyright (C) 2020 Marcus Meissner <marcus@jet.franken.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
/*
 * This is a copy of ptpip.c...
 *
 * This is working, but unfinished!
 * - Event handling not finished.
 * - Some configure checking magic missing for the special header files
 *   and functions.
 * - Not everything implementation correctly cross checked.
 * - Coolpix P3 does not give transfer status (image 000x/000y), and reports an
 *   error when transfers finish correctly.
 *
 * Nikon WU-1* adapters might use 0011223344556677 as GUID always...
 */
#define _DEFAULT_SOURCE
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef WIN32
# include <winsock.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif

#include <gphoto2/gphoto2-library.h>
#include <gphoto2/gphoto2-port-log.h>
#include <gphoto2/gphoto2-setting.h>

#include "ptp.h"
#include "ptp-private.h"

#define PTPIP_VERSION_MAJOR 0x0001
#define PTPIP_VERSION_MINOR 0x0000

#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (GETTEXT_PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include "ptp.h"
#include "ptp-bugs.h"

#include "ptp-pack.c"

#define fujiptpip_len		0
#define fujiptpip_type		4	/* not always present */

#define fujiptpip_cmd_dataphase	4
#define fujiptpip_cmd_code	6
#define fujiptpip_cmd_transid	8
#define fujiptpip_cmd_param1	12
#define fujiptpip_cmd_param2	16
#define fujiptpip_cmd_param3	20
#define fujiptpip_cmd_param4	24
#define fujiptpip_cmd_param5	28

#define PTP_EVENT_CHECK			0x0000	/* waits for */
#define PTP_EVENT_CHECK_FAST		0x0001	/* checks */
static uint16_t ptp_fujiptpip_check_event (PTPParams* params);
static uint16_t ptp_fujiptpip_event (PTPParams* params, PTPContainer* event, int wait);

/* send / receive functions */
uint16_t
ptp_fujiptpip_sendreq (PTPParams* params, PTPContainer* req, int dataphase)
{
	int		ret;
	int		len = fujiptpip_cmd_param1 +req->Nparam*4;
	unsigned char 	*request = malloc(len);

	switch (req->Nparam) {
	default:
	case 0: GP_LOG_D ("Sending PTP_OC 0x%0x (%s) request...", req->Code, ptp_get_opcode_name(params, req->Code));
		break;
	case 1: GP_LOG_D ("Sending PTP_OC 0x%0x (%s) (0x%x) request...", req->Code, ptp_get_opcode_name(params, req->Code), req->Param1);
		break;
	case 2: GP_LOG_D ("Sending PTP_OC 0x%0x (%s) (0x%x,0x%x) request...", req->Code, ptp_get_opcode_name(params, req->Code), req->Param1, req->Param2);
		break;
	case 3: GP_LOG_D ("Sending PTP_OC 0x%0x (%s) (0x%x,0x%x,0x%x) request...", req->Code, ptp_get_opcode_name(params, req->Code), req->Param1, req->Param2, req->Param3);
		break;
	}

	ptp_fujiptpip_check_event (params);

	/*htod32a(&request[ptpip_type],PTPIP_CMD_REQUEST);*/
	htod32a(&request[fujiptpip_len],len);
	/* sending data = 2, receiving data or no data = 1 */
	if ((dataphase&PTP_DP_DATA_MASK) == PTP_DP_SENDDATA)
		htod16a(&request[fujiptpip_cmd_dataphase],2);
	else
		htod16a(&request[fujiptpip_cmd_dataphase],1);
	htod16a(&request[fujiptpip_cmd_code],req->Code);
	htod32a(&request[fujiptpip_cmd_transid],req->Transaction_ID);

	switch (req->Nparam) {
	case 5: htod32a(&request[fujiptpip_cmd_param5],req->Param5);/* fallthrough */
	case 4: htod32a(&request[fujiptpip_cmd_param4],req->Param4);/* fallthrough */
	case 3: htod32a(&request[fujiptpip_cmd_param3],req->Param3);/* fallthrough */
	case 2: htod32a(&request[fujiptpip_cmd_param2],req->Param2);/* fallthrough */
	case 1: htod32a(&request[fujiptpip_cmd_param1],req->Param1);/* fallthrough */
	case 0:
	default:
		break;
	}
	GP_LOG_DATA ( (char*)request, len, "ptpip/oprequest data:");
	ret = write (params->cmdfd, request, len);
	free (request);
	if (ret == -1)
		perror ("sendreq/write to cmdfd");
	if (ret != len) {
		GP_LOG_E ("ptp_fujiptpip_sendreq() len =%d but ret=%d", len, ret);
		return PTP_RC_OK;
	}
	return PTP_RC_OK;
}

static uint16_t
ptp_fujiptpip_generic_read (PTPParams *params, int fd, PTPIPHeader *hdr, unsigned char**data, int withtype) {
	int	ret, len, curread;
	unsigned char *xhdr;
	unsigned int hdrlen;

	xhdr = (unsigned char*)hdr; curread = 0;
	hdrlen = len = sizeof (PTPIPHeader);
	if (!withtype)
		hdrlen = len = sizeof(uint32_t);

	while (curread < len) {
		ret = read (fd, xhdr + curread, len - curread);
		if (ret == -1) {
			perror ("read PTPIPHeader");
			return PTP_RC_GeneralError;
		}
		GP_LOG_DATA ((char*)xhdr+curread, ret, "ptpip/generic_read header:");
		curread += ret;
		if (ret == 0) {
			GP_LOG_E ("End of stream after reading %d bytes of ptpipheader", ret);
			return PTP_RC_GeneralError;
		}
	}
	len = dtoh32 (hdr->length) - hdrlen;
	if (len < 0) {
		GP_LOG_E ("len < 0, %d?", len);
		return PTP_RC_GeneralError;
	}
	*data = malloc (len);
	if (!*data) {
		GP_LOG_E ("malloc failed.");
		return PTP_RC_GeneralError;
	}
	curread = 0;
	while (curread < len) {
		ret = read (fd, (*data)+curread, len-curread);
		if (ret == -1) {
			GP_LOG_E ("error %d in reading PTPIP data", errno);
			free (*data);*data = NULL;
			return PTP_RC_GeneralError;
		} else {
			GP_LOG_DATA ((char*)((*data)+curread), ret, "ptpip/generic_read data:");
		}
		if (ret == 0)
			break;
		curread += ret;
	}
	if (curread != len) {
		GP_LOG_E ("read PTPIP data, ret %d vs len %d", ret, len);
		free (*data);*data = NULL;
		return PTP_RC_GeneralError;
	}
	return PTP_RC_OK;
}

static uint16_t
ptp_fujiptpip_cmd_read (PTPParams* params, PTPIPHeader *hdr, unsigned char** data) {
	ptp_fujiptpip_check_event (params);
	return ptp_fujiptpip_generic_read (params, params->cmdfd, hdr, data, 0);
}

static uint16_t
ptp_fujiptpip_evt_read (PTPParams* params, PTPIPHeader *hdr, unsigned char** data) {
	return ptp_fujiptpip_generic_read (params, params->evtfd, hdr, data, 0);
}

static uint16_t
ptp_fujiptpip_check_event (PTPParams* params) {
	PTPContainer	event;
	uint16_t	ret;

	event.Code = 0;
	ret = ptp_fujiptpip_event (params, &event, PTP_EVENT_CHECK_FAST);
	if (ret != PTP_RC_OK)
		return ret;
	if (event.Code == 0)
		return ret;
	return ptp_add_event (params, &event);
}

#define fujiptpip_data_datatype		4
#define fujiptpip_data_code		6
#define fujiptpip_data_transid		8
#define fujiptpip_data_payload		12

#define WRITE_BLOCKSIZE 65536
uint16_t
ptp_fujiptpip_senddata (PTPParams* params, PTPContainer* ptp,
		uint64_t size, PTPDataHandler *handler
) {
	unsigned char	request[fujiptpip_data_payload];
	unsigned int	curwrite, towrite;
	int		ret;
	unsigned char*	xdata;

	GP_LOG_D ("Sending PTP_OC 0x%0x (%s) data...", ptp->Code, ptp_get_opcode_name(params, ptp->Code));
	//htod32a(&request[ptpip_type],PTPIP_START_DATA_PACKET);
	htod32a(&request[fujiptpip_len],sizeof(request));
	htod16a(&request[fujiptpip_data_datatype],2);
	htod16a(&request[fujiptpip_data_code],ptp->Code);
	htod32a(&request[fujiptpip_data_transid],ptp->Transaction_ID);
	GP_LOG_DATA ((char*)request, sizeof(request), "ptpip/senddata header:");
	ret = write (params->cmdfd, request, sizeof(request));
	if (ret == -1)
		perror ("sendreq/write to cmdfd");
	if (ret != sizeof(request)) {
		GP_LOG_E ("ptp_fujiptpip_senddata() len=%d but ret=%d", (int)sizeof(request), ret);
		return PTP_RC_GeneralError;
	}
	xdata = malloc(WRITE_BLOCKSIZE);
	if (!xdata) return PTP_RC_GeneralError;
	curwrite = 0;
	while (curwrite < size) {
		unsigned long written, towrite2, xtowrite;

		ptp_fujiptpip_check_event (params);

		towrite = size - curwrite;
		if (towrite > WRITE_BLOCKSIZE) {
			towrite	= WRITE_BLOCKSIZE;
		}
		ret = handler->getfunc (params, handler->priv, towrite, xdata, &xtowrite);
		if (ret == -1) {
			perror ("getfunc in senddata failed");
			free (xdata);
			return PTP_RC_GeneralError;
		}
		towrite2 = xtowrite;
		GP_LOG_DATA ((char*)xdata, towrite2, "ptpip/senddata data:");
		written = 0;
		while (written < towrite2) {
			ret = write (params->cmdfd, xdata+written, towrite2-written);
			if (ret == -1) {
				perror ("write in senddata failed");
				free (xdata);
				return PTP_RC_GeneralError;
			}
			written += ret;
		}
		curwrite += towrite;
	}
	free (xdata);
	return PTP_RC_OK;
}

uint16_t
ptp_fujiptpip_getdata (PTPParams* params, PTPContainer* ptp, PTPDataHandler *handler) {
	PTPIPHeader		hdr;
	unsigned char		*xdata = NULL;
	uint16_t 		ret;
	unsigned long		toread, curread;
	int			xret;

	GP_LOG_D ("Reading PTP_OC 0x%0x (%s) data...", ptp->Code, ptp_get_opcode_name(params, ptp->Code));
	ret = ptp_fujiptpip_cmd_read (params, &hdr, &xdata);
	if (ret != PTP_RC_OK)
		return ret;

#if 0
	if (dtoh32(hdr.type) == PTPIP_CMD_RESPONSE) { /* might happen if we have no data transfer due to error? */
		GP_LOG_E ("Unexpected ptp response, ptp code %x", dtoh16a(&xdata[0]));
		return dtoh16a(&xdata[0]);
	}
	if (dtoh32(hdr.type) != PTPIP_START_DATA_PACKET) {
		GP_LOG_E ("got reply type %d\n", dtoh32(hdr.type));
		return PTP_RC_GeneralError;
	}
#endif
	toread = dtoh32a(&xdata[fujiptpip_data_payload]);
	free (xdata); xdata = NULL;
	curread = 0;
	while (curread < toread) {
		ret = ptp_fujiptpip_cmd_read (params, &hdr, &xdata);
		if (ret != PTP_RC_OK)
			return ret;
		unsigned long datalen = dtoh32(hdr.length)-4-fujiptpip_data_payload;
		if (datalen > (toread-curread)) {
			GP_LOG_E ("returned data is too much, expected %ld, got %ld",
				  (toread-curread), datalen
			);
			break;
		}
		xret = handler->putfunc (params, handler->priv,
			datalen, xdata+fujiptpip_data_payload
		);
		if (xret != PTP_RC_OK) {
			GP_LOG_E ("failed to putfunc of returned data");
			break;
		}
		curread += datalen;
		free (xdata); xdata = NULL;
		continue;
	}
	if (curread < toread)
		return PTP_RC_GeneralError;
	return PTP_RC_OK;
}

#define fujiptpip_dataphase	0
#define fujiptpip_resp_code	2
#define fujiptpip_resp_transid	4
#define fujiptpip_resp_param1	8
#define fujiptpip_resp_param2	12
#define fujiptpip_resp_param3	16
#define fujiptpip_resp_param4	20
#define fujiptpip_resp_param5	24

uint16_t
ptp_fujiptpip_getresp (PTPParams* params, PTPContainer* resp)
{
	PTPIPHeader	hdr;
	unsigned char	*data = NULL;
	uint16_t 	ret;
	int		n;

	GP_LOG_D ("Reading PTP_OC 0x%0x (%s) response...", resp->Code, ptp_get_opcode_name(params, resp->Code));
retry:
	ret = ptp_fujiptpip_cmd_read (params, &hdr, &data);
	if (ret != PTP_RC_OK)
		return ret;

	switch (dtoh16a(data)) {
	case 3:
		GP_LOG_D("PTPIP_CMD_RESPONSE");
		resp->Code		= dtoh16a(&data[fujiptpip_resp_code]);
		resp->Transaction_ID	= dtoh32a(&data[fujiptpip_resp_transid]);
		n = (dtoh32(hdr.length) - sizeof(uint32_t) - fujiptpip_resp_param1)/sizeof(uint32_t);
		switch (n) {
		case 5: resp->Param5 = dtoh32a(&data[fujiptpip_resp_param5]);/* fallthrough */
		case 4: resp->Param4 = dtoh32a(&data[fujiptpip_resp_param4]);/* fallthrough */
		case 3: resp->Param3 = dtoh32a(&data[fujiptpip_resp_param3]);/* fallthrough */
		case 2: resp->Param2 = dtoh32a(&data[fujiptpip_resp_param2]);/* fallthrough */
		case 1: resp->Param1 = dtoh32a(&data[fujiptpip_resp_param1]);/* fallthrough */
		case 0: break;
		default:
			GP_LOG_E ("response got %d parameters?", n);
			break;
		}
		break;
	default:
		GP_LOG_E ("response type %d packet?", dtoh16a(data));
		break;
	}
	free (data);
	return PTP_RC_OK;
}

#define fujiptpip_initcmd_protocolversion	8
#define fujiptpip_initcmd_guid			12
#define fujiptpip_initcmd_name			28

static uint16_t
ptp_fujiptpip_init_command_request (PTPParams* params)
{
	char		hostname[100];
	unsigned char*	cmdrequest;
	unsigned int	i;
	int 		len, ret;
	unsigned char	guid[16];

	ptp_nikon_getptpipguid(guid);
#if !defined (WIN32)
	if (gethostname (hostname, sizeof(hostname)))
		return PTP_RC_GeneralError;
#else
	strcpy (hostname, "gpwindows");
#endif
	len = fujiptpip_initcmd_name + (strlen(hostname)+1)*2;

	cmdrequest = malloc(len);
	htod32a(&cmdrequest[fujiptpip_type],PTPIP_INIT_COMMAND_REQUEST);
	htod32a(&cmdrequest[fujiptpip_len],len);

	htod32a(&cmdrequest[fujiptpip_initcmd_protocolversion], 0x8f53e4f2);	/* magic number */

	memcpy(&cmdrequest[fujiptpip_initcmd_guid], guid, 16);
	for (i=0;i<strlen(hostname)+1;i++) {
		/* -> ucs-2 in little endian */
		cmdrequest[fujiptpip_initcmd_name+i*2] = hostname[i];
		cmdrequest[fujiptpip_initcmd_name+i*2+1] = 0;
	}

	/*
	htod16a(&cmdrequest[fujiptpip_initcmd_name+(strlen(hostname)+1)*2],PTPIP_VERSION_MINOR);
	htod16a(&cmdrequest[fujiptpip_initcmd_name+(strlen(hostname)+1)*2+2],PTPIP_VERSION_MAJOR);
	*/


	GP_LOG_DATA ((char*)cmdrequest, len, "ptpip/init_cmd data:");
	ret = write (params->cmdfd, cmdrequest, len);
	free (cmdrequest);
	if (ret == -1) {
		perror("write init cmd request");
		return PTP_RC_GeneralError;
	}
	GP_LOG_E ("return %d / len %d", ret, len);
	if (ret != len) {
		GP_LOG_E ("return %d vs len %d", ret, len);
		return PTP_RC_GeneralError;
	}
	return PTP_RC_OK;
}

#define ptpip_cmdack_idx	0
#define ptpip_cmdack_guid	4
#define ptpip_cmdack_name	20

static uint16_t
ptp_fujiptpip_init_command_ack (PTPParams* params)
{
	PTPIPHeader	hdr;
	unsigned char	*data = NULL;
	uint16_t 	ret;
	int		i;
	unsigned short	*name;

	ret = ptp_fujiptpip_generic_read (params, params->cmdfd, &hdr, &data, 1);
	if (ret != PTP_RC_OK)
		return ret;
	if (hdr.type != dtoh32(PTPIP_INIT_COMMAND_ACK)) {
		GP_LOG_E ("bad type returned %d", htod32(hdr.type));
		free (data);
		if (hdr.type == PTPIP_INIT_FAIL) /* likely reason is permission denied */
			return PTP_RC_AccessDenied;
		return PTP_RC_GeneralError;
	}
	params->eventpipeid = dtoh32a(&data[ptpip_cmdack_idx]);
	memcpy (params->cameraguid, &data[ptpip_cmdack_guid], 16);
	name = (unsigned short*)&data[ptpip_cmdack_name];
	for (i=0;name[i];i++) /* EMPTY */;
	params->cameraname = malloc((i+1)*sizeof(uint16_t));
	for (i=0;name[i];i++)
		params->cameraname[i] = name[i];
	free (data);
	return PTP_RC_OK;
}

#define fujiptpip_eventinit_idx	8
#define fujiptpip_eventinit_size	12

static uint16_t
ptp_fujiptpip_init_event_request (PTPParams* params)
{
	unsigned char	evtrequest[fujiptpip_eventinit_size];
	int 		ret;

	htod32a(&evtrequest[fujiptpip_type],PTPIP_INIT_EVENT_REQUEST);
	htod32a(&evtrequest[fujiptpip_len],fujiptpip_eventinit_size);
	htod32a(&evtrequest[fujiptpip_eventinit_idx],params->eventpipeid);

	GP_LOG_DATA ((char*)evtrequest, fujiptpip_eventinit_size, "ptpip/init_event data:");
	ret = write (params->evtfd, evtrequest, fujiptpip_eventinit_size);
	if (ret == -1) {
		perror("write init evt request");
		return PTP_RC_GeneralError;
	}
	if (ret != fujiptpip_eventinit_size) {
		GP_LOG_E ("unexpected retsize %d, expected %d", ret, fujiptpip_eventinit_size);
		return PTP_RC_GeneralError;
	}
	return PTP_RC_OK;
}

static uint16_t
ptp_fujiptpip_init_event_ack (PTPParams* params)
{
	PTPIPHeader	hdr;
	unsigned char	*data = NULL;
	uint16_t	ret;

	ret = ptp_fujiptpip_evt_read (params, &hdr, &data);
	if (ret != PTP_RC_OK)
		return ret;
	free (data);
	if (hdr.type != dtoh32(PTPIP_INIT_EVENT_ACK)) {
		GP_LOG_E ("bad type returned %d\n", htod32(hdr.type));
		return PTP_RC_GeneralError;
	}
	return PTP_RC_OK;
}


/* Event handling functions */

/* PTP Events wait for or check mode */

#define ptpip_event_code    0
#define ptpip_event_transid	2
#define ptpip_event_param1	6
#define ptpip_event_param2	10
#define ptpip_event_param3	14

static uint16_t
ptp_fujiptpip_event (PTPParams* params, PTPContainer* event, int wait)
{
#ifndef WIN32
	fd_set		infds;
	struct timeval	timeout;
	int ret;
	unsigned char*	data = NULL;
	PTPIPHeader	hdr;
	int n;

	while (1) {
		FD_ZERO(&infds);
		FD_SET(params->evtfd, &infds);
		timeout.tv_sec = 0;
		if (wait == PTP_EVENT_CHECK_FAST)
			timeout.tv_usec = 1;
		else
			timeout.tv_usec = 1000; /* 1/1000 second  .. perhaps wait longer? */

		ret = select (params->evtfd+1, &infds, NULL, NULL, &timeout);
		if (1 != ret) {
			if (-1 == ret) {
				GP_LOG_D ("select returned error, errno is %d", errno);
				return PTP_ERROR_IO;
			}
			return PTP_ERROR_TIMEOUT;
		}

		ret = ptp_fujiptpip_evt_read (params, &hdr, &data);
		if (ret != PTP_RC_OK)
			return ret;
		GP_LOG_D ("hdr type %d, length %d", hdr.type, hdr.length);

		if (dtoh32(hdr.type) == PTPIP_EVENT) {
			break;
		}

		/* TODO: Handle cancel transaction and ping/pong
		 * If not PTPIP_EVENT, process it and wait for next PTPIP_EVENT
		 */
		GP_LOG_E ("unknown/unhandled event type %d", dtoh32(hdr.type));
	}

	event->Code		= dtoh16a(&data[ptpip_event_code]);
	event->Transaction_ID	= dtoh32a(&data[ptpip_event_transid]);
	n = (dtoh32(hdr.length) - sizeof(hdr) - ptpip_event_param1)/sizeof(uint32_t);
	switch (n) {
	case 3: event->Param3 = dtoh32a(&data[ptpip_event_param3]);/* fallthrough */
	case 2: event->Param2 = dtoh32a(&data[ptpip_event_param2]);/* fallthrough */
	case 1: event->Param1 = dtoh32a(&data[ptpip_event_param1]);/* fallthrough */
	case 0: break;
	default:
		GP_LOG_E ("response got %d parameters?", n);
		break;
	}
	free (data);
	return PTP_RC_OK;
#else
	GP_LOG_E ("not supported currently on Windows");
	return PTP_RC_OK;
#endif
}

uint16_t
ptp_fujiptpip_event_check_queue (PTPParams* params, PTPContainer* event) {
	/* the fast check just takes 1ms, so lets keep it */
	return ptp_fujiptpip_event (params, event, PTP_EVENT_CHECK_FAST);
}

uint16_t
ptp_fujiptpip_event_check (PTPParams* params, PTPContainer* event) {
	return ptp_fujiptpip_event (params, event, PTP_EVENT_CHECK_FAST);
}

uint16_t
ptp_fujiptpip_event_wait (PTPParams* params, PTPContainer* event) {
	return ptp_fujiptpip_event (params, event, PTP_EVENT_CHECK);
}

int
ptp_fujiptpip_connect (PTPParams* params, const char *address) {
	char 		*addr, *s, *p;
	int		port, eventport, tries;
	struct sockaddr_in	saddr;
	uint16_t	ret;

	GP_LOG_D ("connecting to %s.", address);
	if (NULL == strchr (address,':'))
		return GP_ERROR_BAD_PARAMETERS;

#ifdef HAVE_INET_ATON
	addr = strdup (address);
	if (!addr)
		return GP_ERROR_NO_MEMORY;
	s = strchr (addr,':');
	if (!s) {
		GP_LOG_E ("addr %s should contain a :", address);
		free (addr);
		return GP_ERROR_BAD_PARAMETERS;
	}
	*s = '\0';
	p = strchr (s+1,':');
	port = 55740;
	eventport = port+1;
	if (p) {
		*p = '\0';
		if (!sscanf (p+1,"%d",&port)) {
			fprintf(stderr,"failed to scan for port in %s\n", p+1);
			free (addr);
			return GP_ERROR_BAD_PARAMETERS;
		}
		/* different event port ? */
		p = strchr (p+1,':');
		if (p) {
			if (!sscanf (p+1,"%d",&eventport)) {
				fprintf(stderr,"failed to scan for eventport in %s\n", p+1);
				free (addr);
				return GP_ERROR_BAD_PARAMETERS;
			}
		}
	}
	if (!inet_aton (s+1,  &saddr.sin_addr)) {
		fprintf(stderr,"failed to scan for addr in %s\n", s+1);
		free (addr);
		return GP_ERROR_BAD_PARAMETERS;
	}
	saddr.sin_port		= htons(port);
	saddr.sin_family	= AF_INET;
	free (addr);
	params->cmdfd = socket (PF_INET, SOCK_STREAM, 0);
	if (params->cmdfd == -1) {
		perror ("socket cmd");
		return GP_ERROR_BAD_PARAMETERS;
	}
	params->evtfd = socket (PF_INET, SOCK_STREAM, 0);
	if (params->evtfd == -1) {
		perror ("socket evt");
		close (params->cmdfd);
		return GP_ERROR_BAD_PARAMETERS;
	}
	if (-1 == connect (params->cmdfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in))) {
		perror ("connect cmd");
		close (params->cmdfd);
		close (params->evtfd);
		return GP_ERROR_IO;
	}
	ret = ptp_fujiptpip_init_command_request (params);
	if (ret != PTP_RC_OK) {
		close (params->cmdfd);
		close (params->evtfd);
		return translate_ptp_result (ret);
	}
	ret = ptp_fujiptpip_init_command_ack (params);
	if (ret != PTP_RC_OK) {
		close (params->cmdfd);
		close (params->evtfd);
		return translate_ptp_result (ret);
	}
	GP_LOG_D ("fujiptpip connected!");
	return GP_OK;
#if 0
	/* seen on Ricoh Theta, camera is not immediately ready. try again two times. */
	tries = 2;
	saddr.sin_port		= htons(eventport);
	do {
		if (-1 != connect (params->evtfd, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in)))
			break;
		if ((errno == ECONNREFUSED) && (tries--)) {
			GP_LOG_D ("event connect failed, retrying after short wait");
			usleep(100*1000);
			continue;
		}
		GP_LOG_E ("could not connect event");
		close (params->cmdfd);
		close (params->evtfd);
		return GP_ERROR_IO;
	} while (1);
	ret = ptp_fujiptpip_init_event_request (params);
	if (ret != PTP_RC_OK)
		return translate_ptp_result (ret);
	ret = ptp_fujiptpip_init_event_ack (params);
	if (ret != PTP_RC_OK)
		return translate_ptp_result (ret);
	GP_LOG_D ("fujiptpip connected!");
	return GP_OK;
#endif
#else
	GP_LOG_E ("Windows currently not supported, neeeds a winsock port.");
	return GP_ERROR_NOT_SUPPORTED;
#endif
}

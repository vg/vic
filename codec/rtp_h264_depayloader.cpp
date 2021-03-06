/*
 * RTP H264 Protocol (RFC3984)
 * Copyright (c) 2006 Ryan Martell.
 * Modified by Socrates VaraKliotis and Piers O'Hanlon (c) 2008
 * - Created H264Depayloader class out of the original C calls
 * - Added custom handling for IOCOM H.264 
 *
 * This file was part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
* @file rtp_h264.c
 * @brief H.264 / RTP Code (RFC3984)
 * @author Ryan Martell <rdm4@martellventures.com>
 *
 * @note Notes:
 * Notes:
 * This currently supports packetization mode:
 * Single Nal Unit Mode (0), or
 * Non-Interleaved Mode (1).  It currently does not support
 * Interleaved Mode (2). (This requires implementing STAP-B, MTAP16, MTAP24, FU-B packet types)
 *
 * @note TODO:
 * 1) RTCP sender reports for udp streams are required..
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "config.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <assert.h>

#include "rtp_h264_depayloader.h"
#include "packetbuffer.h"
#include "rtp.h"

//using namespace std;

/**
 * Return TRUE if val is a prefix of str. If it returns TRUE, ptr is
 * set to the next character in 'str' after the prefix.
 *
 * @param str input string
 * @param val prefix to test
 * @param ptr updated after the prefix in str in there is a match
 * @return TRUE if there is a match
 */
int strstart(const char *str, const char *val, const char **ptr)
{
    const char *p, *q;
    p = str;
    q = val;
    while (*q != '\0') {
        if (*p != *q)
            return 0;
        p++;
        q++;
    }
    if (ptr)
        *ptr = p;
    return 1;
}


//helpers from libavformat/utils.c =============================================
/**
 * Default packet destructor.
 */
/*void av_destruct_packet(AVPacket *pkt)
{
    av_free(pkt->data);
    pkt->data = NULL; pkt->size = 0;
}
*/

/**
 * Allocate the payload of a packet and intialized its fields to default values.
 *
 * @param pkt packet
 * @param size wanted payload size
 * @return 0 if OK. AVERROR_xxx otherwise.
 */
/*
int av_new_packet(AVPacket *pkt, int size)
{
    uint8_t *data;
    if((unsigned)size > (unsigned)size + FF_INPUT_BUFFER_PADDING_SIZE)
        return AVERROR_NOMEM;
    data = (uint8_t *) av_malloc(size + FF_INPUT_BUFFER_PADDING_SIZE);
    if (!data)
        return AVERROR_NOMEM;
    memset(data + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

    av_init_packet(pkt);
    pkt->data = data;
    pkt->size = size;
    pkt->destruct = av_destruct_packet;
    return 0;
}
*/

//helpers from rtsp.c =============================================
static int redir_isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static void skip_spaces(const char **pp)
{
    const char *p;
    p = *pp;
    while (redir_isspace(*p))
        p++;
    *pp = p;
}

static void get_word_sep(char *buf, int buf_size, const char *sep,
                         const char **pp)
{
    const char *p;
    char *q;

    p = *pp;
    if (*p == '/')
        p++;
    skip_spaces(&p);
    q = buf;
    while (!strchr(sep, *p) && *p != '\0') {
        if ((q - buf) < buf_size - 1)
            *q++ = *p;
        p++;
    }
    if (buf_size > 0)
        *q = '\0';
    *pp = p;
}
//===================================================================

/** parse the attribute line from the fmtp a line of an sdp resonse.  This is broken out as a function
* because it is used in rtp_h264.c, which is forthcoming.
*/
int rtsp_next_attr_and_value(const char **p, char *attr, int attr_size, char *value, int value_size)
{
    skip_spaces(p);
    if(**p)
    {
        get_word_sep(attr, attr_size, "=", p);
        if (**p == '=')
            (*p)++;
        get_word_sep(value, value_size, ";", p);
        if (**p == ';')
            (*p)++;
        return 1;
    }
    return 0;
}





H264Depayloader::H264Depayloader()
{
    h264_extradata = (h264_rtp_extra_data *) h264_new_extradata();
    //fprintf(stderr, "H264_RTP: H264Depayloader Constructor done.\n");
    aggregate_pkt = 0;
}

H264Depayloader::~H264Depayloader()
{
    h264_free_extradata(h264_extradata);
    //fprintf(stderr, "H264_RTP: H264Depayloader Destructor done.\n");
}




/* ---------------- private code */
void H264Depayloader::sdp_parse_fmtp_config_h264(AVCodecContext *codec, /*AVStream * stream,*/
                                       h264_rtp_extra_data * h264_data,
                                       char *attr, char *value)
{
    //AVCodecContext *codec = stream->codec;
    assert(codec->codec_id == CODEC_ID_H264);
    assert(h264_data != NULL);

    if (!strcmp(attr, "packetization-mode")) {
        //fprintf(stderr, /*NULL, AV_LOG_DEBUG,*/ "H.264/RTP Packetization Mode: %d\n", atoi(value));
        h264_data->packetization_mode = atoi(value);
        /*
           Packetization Mode:
           0 or not present: Single NAL mode (Only nals from 1-23 are allowed)
           1: Non-interleaved Mode: 1-23, 24 (STAP-A), 28 (FU-A) are allowed.
           2: Interleaved Mode: 25 (STAP-B), 26 (MTAP16), 27 (MTAP24), 28 (FU-A), and 29 (FU-B) are allowed.
         */
        if (h264_data->packetization_mode > 1)
            fprintf(stderr, /*stream, AV_LOG_ERROR,*/
                   "H.264/RTP: Interleaved RTP mode is not supported yet.");
    } else if (!strcmp(attr, "profile-level-id")) {
        if (strlen(value) == 6) {
            char buffer[3];
            // 6 characters=3 bytes, in hex.
            uint8_t profile_idc;
            uint8_t profile_iop;
            uint8_t level_idc;

            buffer[0] = value[0]; buffer[1] = value[1]; buffer[2] = '\0';
            profile_idc = (uint8_t)strtol(buffer, NULL, 16);
            buffer[0] = value[2]; buffer[1] = value[3];
            profile_iop = (uint8_t)strtol(buffer, NULL, 16);
            buffer[0] = value[4]; buffer[1] = value[5];
            level_idc = (uint8_t)strtol(buffer, NULL, 16);

            // set the parameters...
            //fprintf(stderr, /* NULL, AV_LOG_DEBUG,*/ "H.264/RTP Profile IDC: %x Profile IOP: %x Level: %x\n", profile_idc, profile_iop, level_idc);
            h264_data->profile_idc = profile_idc;
            h264_data->profile_iop = profile_iop;
            h264_data->level_idc = level_idc;
        }
    } else  if (!strcmp(attr, "sprop-parameter-sets")) {
        uint8_t start_sequence[]= { 0, 0, 1 };
        codec->extradata_size= 0;
        codec->extradata= NULL;

        while (*value) {
            char base64packet[1024];
            uint8_t decoded_packet[1024];
            uint32_t packet_size;
            char *dst = base64packet;

            while (*value && *value != ','
                   && (dst - base64packet) < sizeof(base64packet) - 1) {
                *dst++ = *value++;
            }
            *dst++ = '\0';

            if (*value == ',')
                value++;

	    packet_size=av_base64_decode(decoded_packet, base64packet, sizeof(decoded_packet));
            if (packet_size) {
                uint8_t *dest= (uint8_t *) av_malloc(packet_size+sizeof(start_sequence)+codec->extradata_size);
                if(dest)
                {
                    if(codec->extradata_size)
                    {
                        // av_realloc?
                        memcpy(dest, codec->extradata, codec->extradata_size);
                        av_free(codec->extradata);
                    }

                    memcpy(dest+codec->extradata_size, start_sequence, sizeof(start_sequence));
                    memcpy(dest+codec->extradata_size+sizeof(start_sequence), decoded_packet, packet_size);

                    codec->extradata= dest;
                    codec->extradata_size+= sizeof(start_sequence)+packet_size;
                } else {
                    fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H.264/RTP Unable to allocate memory for extradata!");
                }
            }
        }
        //fprintf(stderr, /*NULL, AV_LOG_DEBUG,*/ "H.264/RTP Extradata set to %p (size: %d)!\n", codec->extradata, codec->extradata_size);
    }
}

// return 0 on packet, no more left, 1 on packet, 1 on partial packet...
int H264Depayloader::h264_handle_packet(h264_rtp_extra_data *data,
                              int pktIdx, PacketBuffer *pb,
                              const uint8_t * buf, int len)
{
    //h264_rtp_extra_data *data = s->dynamic_protocol_context;
    uint8_t nal = buf[0];
    uint8_t type = (nal & 0x1f);
    int result= 0;
    char start_sequence[]= {0, 0, 1};
    int sslen = sizeof(start_sequence);

    assert(data);
    assert(data->cookie == MAGIC_COOKIE);
    assert(buf);

    //fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H264_RTP: Single NAL type (%d, equiv. to >=1 or <=23), len=%4d\n", type, len);

    if (type >= 1 && type <= 23)
        type = 1;              
    // simplify the case. (these are all the nal types used 
    // internally by the h264 codec)
    
    switch (type) {
    case 0:                    // undefined;
        fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H.264/RTP: Undefined NAL type (%d)\n", type);
        result= -1;
        break;

    case 1:

        //av_new_packet(pkt, len+sizeof(start_sequence));
        //memcpy(pkt->data, start_sequence, sizeof(start_sequence));
        //memcpy(pkt->data+sizeof(start_sequence), buf, len);

	//SV: XXX
/*
	temp = (char *) malloc(sslen+len);
        assert(temp);
        memcpy(temp, start_sequence, sslen);
        memcpy(temp+sslen, buf, len);

	pb->write(pktIdx, sslen+len, temp); //SV: XXX
*/	

	  pb->writeAppend(pktIdx, sslen, start_sequence); //PO: XXX
	  pb->writeAppend(pktIdx, len, (char *)buf); //PO: XXX

#ifdef DEBUG
        data->packet_types_received[nal & 0x1f]++;
#endif
        break;

    case 24:                   // STAP-A (one packet, multiple nals)
        // consume the STAP-A NAL
	//fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H264_RTP: STAP-A NAL type (%d)\n", type);

        buf++;
        len--;
        // first we are going to figure out the total size....
        {
            int pass= 0;
            int total_length= 0;
            uint8_t *dst= NULL;

            for(pass= 0; pass<2; pass++) {
                const uint8_t *src= buf;
                int src_len= len;

                do {
                    uint16_t nal_size = BE_16(src); // this going to be a problem if unaligned (can it be?)

                    // consume the length of the aggregate...
                    src += 2;
                    src_len -= 2;

                    if (nal_size <= src_len) {
                        if(pass==0) {
                            // counting...
                            total_length+= sizeof(start_sequence)+nal_size;
                        } else {
                            // copying
                            assert(dst);
                            memcpy(dst, start_sequence, sizeof(start_sequence));
                            dst+= sizeof(start_sequence);
                            memcpy(dst, src, nal_size);
#ifdef DEBUG
                            data->packet_types_received[*src & 0x1f]++;
#endif
                            dst+= nal_size;
                        }
                    } else {
                        //fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H264_RTP: nal size exceeds length: %d %d\n", nal_size, src_len);
                    }

                    // eat what we handled...
                    src += nal_size;
                    src_len -= nal_size;

                    if (src_len < 0) {
			;
                        //fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H264_RTP: Consumed more bytes than we got! (%d)\n", src_len);
		   }
                } while (src_len > 2);      // because there could be rtp padding..

                if(pass==0) {
                    // now we know the total size of the packet (with the start sequences added)

                    //SV: XXX
                    //av_new_packet(pkt, total_length);
                    //dst= pkt->data;

                    //temp = (char *) malloc(total_length);
                    pb->setSize(pktIdx,total_length); //PO:
                    dst = (uint8_t *) pb->packets[pktIdx]->data;

                } else {
                    //SV: XXX
                    //assert(dst-pkt->data==total_length);
                    assert(dst-(uint8_t *)(pb->packets[pktIdx]->data) == total_length);
                }
            }
        }
        break;

    case 25:                   // STAP-B
    case 26:                   // MTAP-16
    case 27:                   // MTAP-24
    case 29:                   // FU-B
        //fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H.264/RTP: Unhandled type (%d) (See RFC for implementation details\n", type);
        result= -1;
        break;

    case 28:                   // FU-A (fragmented nal)

        buf++;
        len--;                  // skip the fu_indicator
        {
            // these are the same as above, we just redo them here for clarity...
            uint8_t fu_indicator = nal;
            uint8_t fu_header = *buf;   // read the fu_header.
            uint8_t start_bit = (fu_header & 0x80) >> 7;
            // non used uint8_t end_bit = (fu_header & 0x40) >> 6;
            uint8_t nal_type = (fu_header & 0x1f);
            uint8_t reconstructed_nal;

            // reconstruct this packet's true nal; only the data follows..
            reconstructed_nal = fu_indicator & (0xe0);  // the original nal forbidden bit and NRI are stored in this packet's nal;
            reconstructed_nal |= (nal_type & 0x1f);

            // skip the fu_header...
            buf++;
            len--;

#ifdef DEBUG
            if (start_bit)
                data->packet_types_received[nal_type & 0x1f]++;
#endif
            if(start_bit) {
                // copy in the start sequence, and the reconstructed nal....

                //SV: XXX
                //av_new_packet(pkt, sizeof(start_sequence)+sizeof(nal)+len);
                //memcpy(pkt->data, start_sequence, sizeof(start_sequence));
                //pkt->data[sizeof(start_sequence)]= reconstructed_nal;
                //memcpy(pkt->data+sizeof(start_sequence)+sizeof(nal), buf, len);
/*
		//SV: XXX
		temp = (char *) malloc(sizeof(start_sequence)+sizeof(nal)+len);
        	memcpy(temp, start_sequence, sizeof(start_sequence));
		temp[sizeof(start_sequence)] = reconstructed_nal;
        	memcpy(temp+sizeof(start_sequence)+sizeof(nal), buf, len);

		pb->write(pktIdx, len+sizeof(start_sequence)+sizeof(nal), temp); //SV: XXX
*/
	pb->writeAppend(pktIdx, sizeof(start_sequence), start_sequence); //PO: XXX
	pb->writeAppend(pktIdx, sizeof(nal), (char*)&reconstructed_nal); //PO: XXX
	pb->writeAppend(pktIdx, len, (char *)buf); //PO: XXX

            } else {
                //SV: XXX
                //av_new_packet(pkt, len);
                //memcpy(pkt->data, buf, len);

		//temp = (char *) malloc(len);
        	//memcpy(temp, buf, len);

		pb->write(pktIdx, len, (char*)buf); //PO: XXX
            }

	    //fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H264_RTP: FU-A NAL type (%d): start_bit=%d, end_bit=%d, NAL_Header=0x%02x, FU_Header=0x%02x\n", type, start_bit, end_bit, reconstructed_nal, fu_header);

        }
        break;


    case 30:                   // undefined
    case 31:                   // undefined
    default:
        fprintf(stderr, /*NULL, AV_LOG_ERROR,*/ "H.264/RTP: Undefined NAL type (%d)\n", type);
        result= -1;
        break;
    }

    return result;
}

/* ---------------- public code */
void *H264Depayloader::h264_new_extradata()
{
    h264_rtp_extra_data *data = (h264_rtp_extra_data *) av_mallocz(sizeof(h264_rtp_extra_data) +
                   FF_INPUT_BUFFER_PADDING_SIZE);

    if (data) {
        data->cookie = MAGIC_COOKIE;
    }

    return data;
}

void H264Depayloader::h264_free_extradata(void *d)
{
    h264_rtp_extra_data *data = (h264_rtp_extra_data *) d;
#ifdef DEBUG
    int ii;

    for (ii = 0; ii < 32; ii++) {
        if (data->packet_types_received[ii])
            debug_msg(/*NULL, AV_LOG_DEBUG,*/ "Received %d packets of type %d\n",
                   data->packet_types_received[ii], ii);
    }
#endif

    assert(data);
    assert(data->cookie == MAGIC_COOKIE);

    // avoid stale pointers (assert)
    data->cookie = DEAD_COOKIE;

    // and clear out this...
    av_free(data);
}

int H264Depayloader::parse_h264_sdp_line(AVCodecContext *codec, /*AVStream * stream,*/ void *data,
                               const char *line)
{
    //AVCodecContext *codec = stream->codec;
    h264_rtp_extra_data *h264_data = (h264_rtp_extra_data *) data;
    const char *p = line;

    assert(h264_data->cookie == MAGIC_COOKIE);

    if (strstart(p, "a=framesize:", &p)) {
        char buf1[50];
        char *dst = buf1;

        // remove the protocol identifier..
        while (*p && *p == ' ') p++; // strip spaces.
        while (*p && *p != ' ') p++; // eat protocol identifier
        while (*p && *p == ' ') p++; // strip trailing spaces.
        while (*p && *p != '-' && (buf1 - dst) < sizeof(buf1) - 1) {
            *dst++ = *p++;
        }
        *dst = '\0';

        // a='framesize:96 320-240'
        // set our parameters..
        codec->width = atoi(buf1);
        codec->height = atoi(p + 1); // skip the -
        codec->pix_fmt = PIX_FMT_YUV420P;
    } else if (strstart(p, "a=fmtp:", &p)) { //SV: wtf!!!
        char attr[256];
        char value[4096];

        // remove the protocol identifier..
        while (*p && *p == ' ') p++; // strip spaces.
        while (*p && *p != ' ') p++; // eat protocol identifier
        while (*p && *p == ' ') p++; // strip trailing spaces.

        /* loop on each attribute */
        while (rtsp_next_attr_and_value
               (&p, attr, sizeof(attr), value, sizeof(value))) {
            /* grab the codec extra_data from the config parameter of the fmtp line */
	    //fprintf(stderr, "==========> attr=%s, value=%s \n", attr, value);
            sdp_parse_fmtp_config_h264(codec, /* stream, */ h264_data, attr, value);
        }
    } else if (strstart(p, "cliprect:", &p)) {
        // could use this if we wanted.
    }

    //av_set_pts_info(stream, 33, 1, 90000);      // 33 should be right, because the pts is 64 bit? (done elsewhere; this is a one time thing)

    return 0;                   // keep processing it the normal way...
}

/**
This is the structure for expanding on the dynamic rtp protocols (makes everything static. yay!)
*/
/*
RTPDynamicProtocolHandler ff_h264_dynamic_handler = {
    "H264",
    CODEC_TYPE_VIDEO,
    CODEC_ID_H264,
    parse_h264_sdp_line,
    h264_new_extradata,
    h264_free_extradata,
    h264_handle_packet
};
*/

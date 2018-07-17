#ifndef __USER_LLRP_H__
#define __USER_LLRP_H__

#include "os_type.h"
#include "c_types.h"

#define SERVER_LOCAL_PORT         5084
#define PEN                       60000   // IANA Private Enterprise Number, we don't have the code yet!
#define DEVICE_MANUFACTURER_NAME  PEN
#define MODEL_NAME                126
#define FIRMWARE_VERSION          V1.1.0

typedef uint8_t                 llrp_u8_t;
typedef int8_t                  llrp_s8_t;
typedef uint16_t                llrp_u16_t;
typedef int16_t                 llrp_s16_t;
typedef uint32_t                llrp_u32_t;
typedef int32_t                 llrp_s32_t;
typedef uint64_t                llrp_u64_t;
typedef sint64_t                llrp_s64_t;
typedef uint8_t                 llrp_u1_t;
typedef uint8_t                 llrp_u2_t;
typedef uint8_t                 llrp_utf8_t;
typedef bool                    llrp_bool_t;
typedef uint8_t                 llrp_byte_t;

typedef struct LLRP_SFrameExtract
{
    enum {
        LLRP_FRAME_UNKNOWN,
        LLRP_FRAME_READY,
        LLRP_FRAME_ERROR,
        LLRP_FRAME_NEED_MORE
    }                           eStatus;

    llrp_u32_t                  MessageLength;
    llrp_u16_t                  MessageType;
    llrp_u8_t                   ProtocolVersion;
    llrp_u32_t                  MessageID;

    unsigned int                nBytesNeeded;
} LLRP_tSFrameExtract;



#endif

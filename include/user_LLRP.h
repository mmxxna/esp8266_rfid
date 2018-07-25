#ifndef __USER_LLRP_H__
#define __USER_LLRP_H__

#include "os_type.h"
#include "c_types.h"

#define SERVER_LOCAL_PORT         5084
#define PEN                       60000   // IANA Private Enterprise Number, we don't have the code yet!
#define DEVICE_MANUFACTURER_NAME  PEN
#define MODEL_NAME                126
#define FIRMWARE_VERSION          V1.1.0

#define LLRP_FRAME_LENGTH_MIN     (10u)
#define LLRP_FRAME_BUFFER_SIZE    (2048u)

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
    /*
     * LLRP_FrameExtract() status
    *
    * LLRP_FRAME_ERROR     Impossible situation, like message
    *                      length too small or the like.
    *                      Recovery in this situation is
    *                      unlikely and probably the app
    *                      should drop the connection.
    *
    * LLRP_FRAME_READY     Frame is complete. Details are
    *                      available for pre-decode decisions.
    *
    * LLRP_FRAME_NEED_MORE Need more input bytes to finish the frame.
    *                      The nBytesNeeded field is how many more.
    */
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

enum LLRP_ResultCode
{
    LLRP_RC_OK                          = 0,
    LLRP_RC_MiscError                   = 100,
    LLRP_RC_Botch,
    LLRP_RC_SendIOError,
    LLRP_RC_RecvIOError,
    LLRP_RC_RecvEOF,
    LLRP_RC_RecvTimeout,
    LLRP_RC_RecvFramingError,
    LLRP_RC_RecvBufferOverflow,
    LLRP_RC_BadVersion,
    LLRP_RC_MissingResponseType,
    LLRP_RC_UnknownMessageType,
    LLRP_RC_UnknownParameterType,
    LLRP_RC_ExcessiveLength,
    LLRP_RC_InvalidLength,
    LLRP_RC_FieldUnderrun,
    LLRP_RC_ReservedBitsUnderrun,
    LLRP_RC_FieldOverrun,
    LLRP_RC_ReservedBitsOverrun,
    LLRP_RC_UnalignedBitField,
    LLRP_RC_UnalignedReservedBits,
    LLRP_RC_MessageAllocationFailed,
    LLRP_RC_ParameterAllocationFailed,
    LLRP_RC_FieldAllocationFailed,
    LLRP_RC_ExtraBytes,
    LLRP_RC_MissingParameter,
    LLRP_RC_UnexpectedParameter,
    LLRP_RC_InvalidChoiceMember,
    LLRP_RC_EnrollBadTypeNumber,
    LLRP_RC_NotAllowedAtExtensionPoint,
    LLRP_RC_XMLInvalidNodeType,
    LLRP_RC_XMLMissingField,
    LLRP_RC_XMLExtraNode,
    LLRP_RC_XMLInvalidFieldCharacters,
    LLRP_RC_XMLOutOfRange,

}LLRP_eResultCode;

/** Receive state */
typedef struct LLRP_SRecvFrame
{
    /** The buffer. Contains incomming frame. */
    unsigned char *     pBuffer;

    /** Count of bytes currently in buffer */
    unsigned int        nBuffer;

    /** Valid boolean. TRUE means the buffer and frame summary
     ** variables are valid (usable). This is always
     ** FALSE mid receive */
    bool                 bFrameValid;

    /** Frame summary variables. Derived by LLRP_FrameExtract() */
    LLRP_tSFrameExtract FrameExtract;

    /** Details of last I/O or decoder error. */
    LLRP_eResultCode ErrorDetails;
}LLRP_tSRecvFrame;

void tcp_recv_cb(void *arg, char *pusrdata, unsigned short length);


#endif

#ifndef __USER_LLRP_H__
#define __USER_LLRP_H__

#include "os_type.h"
#include "c_types.h"

#define SERVER_LOCAL_PORT         5084
#define PEN                       60000   // IANA Private Enterprise Number, we don't have the code yet!
#define DEVICE_MANUFACTURER_NAME  PEN
#define MODEL_NAME                126
#define FIRMWARE_VERSION          V1.1.0

#define LLRP_PROTOCOL_VERSION1    1
#define LLRP_PROTOCOL_VERSION2    2
#define LLRP_PROTOCOL_MY_VERSION  LLRP_PROTOCOL_VERSION2


#define LLRP_FRAME_LENGTH_MIN     (10u)
#define LLRP_FRAME_BUFFER_SIZE    (2048u)

#define tcp_recvTaskPrio        1
#define tcp_recvTaskQueueLen    10
//os_event_t  tcp_recvTaskQueue[tcp_recvTaskQueueLen]; // dynamic malloc during tcp listen
#define TCP_SIG_CONNECTED       0
#define TCP_SIG_RX              1
#define TCP_SIG_TX              2
#define TCP_SIG_DISCONNECTED    3

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

typedef enum LLRP_ResultCode
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

typedef enum LLRP_ResultCode            LLRP_tResultCode;

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

typedef struct LLRP_SVendorDescriptor
{
    /* Short name for the vendor, e.g. "Acme" */
    char *                      pName;

    /* Vendor PEN of a custom message or parameter */
    llrp_u32_t                  VendorID;
} LLRP_tSVendorDescriptor;

/*
 * SEnumTableEntry
 *
 * Simple table of enumerations. Table is terminated
 * by an entry with pName==NULL.
 */
typedef struct LLRP_SEnumTableEntry
{
    /* String name, (e.g. "Immediate") */
    char *                      pName;
    int                         Value;
} LLRP_tSEnumTableEntry;

/*
 * SFieldDescriptor
 *
 * Describes a single field.
 */
typedef struct LLRP_SFieldDescriptor
{
    /* A code for the field type */
    LLRP_tEFieldType            eFieldType;
    /* A code for how the field should be formatted */
    LLRP_tEFieldFormat          eFieldFormat;
    /* String name of field (e.g. "ROSpecID") */
    char *                      pName;
    /* NULL or ptr to table base for enumerated fields */
    const LLRP_tSEnumTableEntry * pEnumTable;
} LLRP_tSFieldDescriptor;

typedef struct LLRP_SErrorDetails
{
    LLRP_tResultCode            eResultCode;
    const LLRP_tSTypeDescriptor *pRefType;
    const LLRP_tSFieldDescriptor *pRefField;
    const char *                pWhatStr;
    int                         OtherDetail;
} LLRP_tSErrorDetails;

/*
 * STypeDescriptor
 *
 * Describes a message or parameter type.
 */

typedef struct LLRP_STypeDescriptor
{
    /* TRUE for a message type, FALSE for a parameter type */
    llrp_bool_t                 bIsMessage;

    /* String name of parameter/message type (e.g. "ROSpec") */
    char *                      pName;

    /* NULL=>standard LLRP, !NULL=>Vendor (PEN) of custom
     * message or parameter */
    const LLRP_tSVendorDescriptor *   pVendorDescriptor;

//    /* Namespace of message or parameter, for XML */
//    const LLRP_tSNamespaceDescriptor *pNamespaceDescriptor;

    /* Type number or, for custom, subtype number */
    llrp_u32_t                  TypeNum;

    /* For messages (bIsMessage==TRUE), this is the type descriptor for
     * the corresponding response. NULL for a request or notification. */
    const LLRP_tSTypeDescriptor *   pResponseType;

    /* Table of pointers to the field descriptors */
    const LLRP_tSFieldDescriptor * const * const ppFieldDescriptorTable;

    /* Size of an instance of this element type */
    unsigned int                nSizeBytes;

} LLRP_tSTypeDescriptor;

enum LLRP_EFieldType {
    LLRP_FT_U8,  LLRP_FT_S8,  LLRP_FT_U8V,  LLRP_FT_S8V,
    LLRP_FT_U16, LLRP_FT_S16, LLRP_FT_U16V, LLRP_FT_S16V,
    LLRP_FT_U32, LLRP_FT_S32, LLRP_FT_U32V, LLRP_FT_S32V,
    LLRP_FT_U64, LLRP_FT_S64, LLRP_FT_U64V, LLRP_FT_S64V,

    LLRP_FT_U1,  LLRP_FT_U1V, LLRP_FT_U2,   LLRP_FT_U96,
    LLRP_FT_UTF8V,

    LLRP_FT_E1,  LLRP_FT_E2,  LLRP_FT_E8,   LLRP_FT_E16,   LLRP_FT_E32,
    LLRP_FT_E8V,

    LLRP_FT_BYTESTOEND,
};


enum LLRP_EFieldFormat {
    LLRP_FMT_NORMAL,
    LLRP_FMT_DEC,
    LLRP_FMT_HEX,
    LLRP_FMT_UTF8,
    LLRP_FMT_DATETIME,
};

typedef enum LLRP_EFieldType            LLRP_tEFieldType;
typedef enum LLRP_EFieldFormat          LLRP_tEFieldFormat;
/*
 * SFieldDescriptor
 *
 * Describes a single field.
 */
struct LLRP_SFieldDescriptor
{
    /* A code for the field type */
    LLRP_tEFieldType            eFieldType;
    /* A code for how the field should be formatted */
    LLRP_tEFieldFormat          eFieldFormat;
    /* String name of field (e.g. "ROSpecID") */
    char *                      pName;
    /* NULL or ptr to table base for enumerated fields */
    const LLRP_tSEnumTableEntry * pEnumTable;
};


/*
 * SElement
 *
 * This is the base class for all parameter and message types.
 *
 * During decode, all subparameters found are entered
 * on m_listAllSubParameters. Then the element's
 * assimilateSubParameters() member function is called
 * to iterate through the list and attach the parameters
 * to specific fields.
 *
 * The m_listAllSubParameters is a secondary reference to
 * all the subparameters. When the element is destructed
 * all parameters referenced by m_listAllSubParameters
 * are deleted. The C++ intrinsic destructors take care
 * of deleting the list itself.
 *
 * During destruct the specific fields are not processed.
 * The fields that are lists are automatically desctructed.
 * So are the fields that are array types (i.e. utf8v) are
 * also automatically destructed. The fields that are simple
 * pointers are simply ignored.
 *
 * This works because every parameter referenced by specific
 * fields is also referenced by m_listAllSubParameters.
 */

typedef struct LLRP_SElement
{
    /* The type descriptor desribing this element */
    const LLRP_tSTypeDescriptor * pType;

    /* Element that encloses this one, NULL if this is top-level element */
    LLRP_tSElement *            pParent;

    /* List of all sub elements */
    LLRP_tSParameter *          listAllSubParameters;
}LLRP_tSElement;

typedef struct LLRP_SMessage
{
    LLRP_tSElement              elementHdr;

    llrp_u32_t                  MessageID;

    LLRP_tSMessage *            pQueueNext;
}LLRP_tSMessage;

typedef struct LLRP_SParameter
{
    LLRP_tSElement              elementHdr;

    /* Next pointer for list of all sub elements */
    LLRP_tSParameter *          pNextAllSubParameters;

    /* Next pointer for element headed by specific member */
    LLRP_tSParameter *          pNextSubParameter;
} LLRP_tSParameter;

/*
 *
 * By way of example, this is how the CDecoder and CDecoderStream
 * classes work. This example is for decoding a binary frame.
 *
 *      +-------------------+               +---------------+
 *      |                   |               |               |
 *      |   CDecoder        --------------->| CTypeRegistry |
 *      |                   |               |               |
 *      +--|----------------+               +---------------+
 *         |    ^
 * pointer |    |
 * to next |    |   +-------------------+
 * byte    |    |   |                   |           pointer to msg end
 *         |    ^----  CDecoderStream   ----------------+
 *         |    |   |                   |               |
 *         |    |   +-------------------+               |
 *         |    |             ^                         |
 *         |    |             |                         |
 *         |    |   +-------------------+   ptr to      |
 *         |    |   |                   |   TLV end     |
 *         |    ^----  CDecoderStream   ------------+   |
 *         |    |   |                   |           |   |
 *         |    |   +-------------------+           |   |
 *         |    |             ^                     |   |
 *         |    |             |                     |   |
 *         |    |   +-------------------+           |   |
 *         |    |   |                   |           |   |
 *         |    ^----  CDecoderStream   --------+   |   |
 *         |        |                   |       |   |   |
 *         |        +-------------------+       |   |   |
 *         |                                    |   |   |
 *         +-------------------+                |   |   |
 *                             |                |   |   |
 *                             v                v   v   v
 *  +---------------------------------------------------------------+
 *  |                   Binary Frame Buffer                         |
 *  +---------------------------------------------------------------+
 *
 *                            \_________________/          Nestec TLVs
 *        \________________/\___________________________/  Nested TLVs
 *    \_________________________________________________/  Message
 *
 *
 * In the case of binary frame the references are to
 * bytes within the buffer. Lookups are by type number.
 *
 * In the case of an XML DOM tree, the references are
 * to nodes in the DOM tre. Lookups are by string name.
 */

typedef struct LLRP_SDecoder
{
    const LLRP_tSDecoderOps *   pDecoderOps;

    const LLRP_tSTypeRegistry * pRegistry;

    LLRP_tSElement *            pRootElement;

    LLRP_tSErrorDetails         ErrorDetails;
} LLRP_tSDecoder;

typedef struct LLRP_SDecoderOps
{
    void
    (*pfDestruct) (
      LLRP_tSDecoder *          pDecoder);

    LLRP_tSMessage *
    (*pfDecodeMessage) (
      LLRP_tSDecoder *          pDecoder);
} LLRP_tSDecoderOps;

typedef struct LLRP_SDecoderStream
{
    LLRP_tSDecoderStreamOps *   pDecoderStreamOps;
} LLRP_tSDecoderStream;

typedef struct LLRP_SFrameDecoder
{
    LLRP_tSDecoder              decoderHdr;

    unsigned char *             pBuffer;
    unsigned int                nBuffer;

    unsigned int                iNext;
    unsigned int                BitFieldBuffer;
    unsigned int                nBitFieldResid;
} LLRP_tSFrameDecoder;

typedef struct LLRP_SFrameDecoderStream
{
    LLRP_tSDecoderStream        decoderStreamHdr;

    LLRP_tSFrameDecoder *       pDecoder;
    LLRP_tSFrameDecoderStream * pEnclosingDecoderStream;
    const LLRP_tSTypeDescriptor *pRefType;
    unsigned int                iBegin;
    unsigned int                iLimit;
} LLRP_tSFrameDecoderStream;

void init_tcp_llrp_task();
void tcp_recvTask(os_event_t *events);
void tcp_recv_cb(void *arg, char *pusrdata, unsigned short length);
LLRP_tSFrameExtract LLRP_FrameExtract (
  const unsigned char *         pBuffer,
  unsigned int                  nBuffer);

#endif

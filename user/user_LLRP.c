#include "user_LLRP.h"
#include "os_type.h"
#include "osapi.h"
#include "ets_sys.h"
#include "ip_addr.h"
#include "espconn.h"
#include "c_types.h"
#include "user_interface.h"
#include "mem.h"
#include <time.h>

// because of the resource limit, the reader cannot accomplish the xml type register and descriptor instanced in LTKC.

LLRP_tSRecvFrame g_LLRP_RecvFrame;

#define DEBUG

void LOCAL debug(char * msg)
{
#ifdef DEBUG
    os_printf("%s\n", msg);
#else
#endif
}

void ICACHE_FLASH_ATTR init_tcp_llrp_task()
{
    os_event_t * tcp_recvTaskQueue = (os_event_t *)os_malloc(sizeof(os_event_t) * tcp_recvTaskQueueLen);
    system_os_task(tcp_recvTask, tcp_recvTaskPrio, tcp_recvTaskQueue, tcp_recvTaskQueueLen);
    g_LLRP_RecvFrame.pBuffer = (llrp_byte_t *) os_malloc(LLRP_FRAME_BUFFER_SIZE);
    g_LLRP_RecvFrame.nBuffer = 0;
    g_LLRP_RecvFrame.bFrameValid = false;
}

void ICACHE_FLASH_ATTR
tcp_recvTask(os_event_t *events)
{
    LLRP_tSFrameDecoder * pDecoder;
    if(events->sig == TCP_SIG_RX) {
        llrp_byte_t * message = (llrp_byte_t *) events->par;
        int message_len = (message[4] << 8) | message[5];
//        int i;
//        for (i = 0; i < message_len; i++) {
//            os_printf("%2x ", *(message + i));
//        }
        /*
         * Construct a new frame decoder. It needs the registry
         * to facilitate decoding.
         */
        pDecoder = (LLRP_tSFrameDecoder * )LLRP_FrameDecoder_construct(message, message_len);
        /*
         * Make sure we really got one. If not, weird problem.
         */
        if(pDecoder == NULL)
        {
            /* All we can do is discard the frame. */
            os_free(message);
            debug("decoder constructor failed");
            return;
        }



    } else if (events->sig == TCP_SIG_CONNECTED) {

    } else if (events->sig == TCP_SIG_DISCONNECTED) {
        debug("free g_LLRP_RecvFrame.pBuf");
        os_free(g_LLRP_RecvFrame.pBuffer);
    }
}

void ICACHE_FLASH_ATTR
tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
//    struct espconn *pespconn = arg;
//    int i;
//    for (i = 0; i < length; i++)
//    {
//        os_printf("%2x ", *(pusrdata + i));
//    }
 //   os_printf("tcp recv : %s \r\n", pusrdata);
//    char rtc_time[24];
//    os_sprintf(rtc_time, "rtc cal: %d\n", system_rtc_clock_cali_proc()>>12);
//    espconn_sent(pespconn, rtc_time, os_strlen(rtc_time));

    // the length that we have read from the pusrdata
    int read_len = 0;
    if (g_LLRP_RecvFrame.FrameExtract.eStatus == LLRP_FRAME_NEED_MORE) {
        llrp_byte_t * pBufPos = &g_LLRP_RecvFrame.pBuffer[g_LLRP_RecvFrame.nBuffer];
        if (length >= g_LLRP_RecvFrame.FrameExtract.nBytesNeeded) {
            os_memcpy(pBufPos, pusrdata, g_LLRP_RecvFrame.FrameExtract.nBytesNeeded);
            length -= g_LLRP_RecvFrame.FrameExtract.nBytesNeeded;
            read_len += g_LLRP_RecvFrame.FrameExtract.nBytesNeeded;
            // copy the frame to a new array and post to task
            llrp_byte_t * pBuffer = (llrp_byte_t *)os_malloc(g_LLRP_RecvFrame.FrameExtract.MessageLength);
            os_memcpy(pBuffer, g_LLRP_RecvFrame.pBuffer, g_LLRP_RecvFrame.FrameExtract.MessageLength);
            system_os_post(tcp_recvTaskPrio, TCP_SIG_RX, (ETSParam)pBuffer);
            g_LLRP_RecvFrame.nBuffer = 0;
            g_LLRP_RecvFrame.FrameExtract.eStatus = LLRP_FRAME_READY;
        } else {
            os_memcpy(pBufPos, pusrdata, length);
            g_LLRP_RecvFrame.nBuffer += length;
            g_LLRP_RecvFrame.FrameExtract.nBytesNeeded -= length;
            return;
        }
    }
    while (length >= LLRP_FRAME_LENGTH_MIN)  // it may have a vaild frame
    {
        g_LLRP_RecvFrame.bFrameValid = false;
        g_LLRP_RecvFrame.FrameExtract = LLRP_FrameExtract(pusrdata+read_len, length);
        /*
         * Framing error?
         */
        if(LLRP_FRAME_ERROR == g_LLRP_RecvFrame.FrameExtract.eStatus)
        {
            g_LLRP_RecvFrame.ErrorDetails = LLRP_RC_RecvFramingError;
            g_LLRP_RecvFrame.FrameExtract.eStatus = LLRP_FRAME_ERROR;
            debug("framing error in message stream");
            break;
        }
        else if (LLRP_FRAME_NEED_MORE == g_LLRP_RecvFrame.FrameExtract.eStatus)
        {
            unsigned int nNeed = g_LLRP_RecvFrame.FrameExtract.nBytesNeeded;
            unsigned char * pBufPos = &g_LLRP_RecvFrame.pBuffer[g_LLRP_RecvFrame.nBuffer];
            /*
             * The frame extractor needs more data, make sure the
             * frame size fits in the receive buffer.
             */
            if(g_LLRP_RecvFrame.nBuffer + nNeed > LLRP_FRAME_BUFFER_SIZE)
            {
                /* Buffer overflow */
                g_LLRP_RecvFrame.ErrorDetails = LLRP_RC_RecvBufferOverflow;
                g_LLRP_RecvFrame.FrameExtract.eStatus = LLRP_FRAME_ERROR;
                g_LLRP_RecvFrame.nBuffer = 0;
                debug("buffer overflow");
                break;
            }
            os_memcpy(pBufPos, pusrdata+read_len, length);
            g_LLRP_RecvFrame.nBuffer += length;
            length = 0;
        }
        else if (LLRP_FRAME_READY == g_LLRP_RecvFrame.FrameExtract.eStatus)
        {
            os_printf("llrp frame len: %d\n", g_LLRP_RecvFrame.FrameExtract.MessageLength);
            llrp_byte_t * pBuffer = (llrp_byte_t *)os_malloc(g_LLRP_RecvFrame.FrameExtract.MessageLength);
            os_memcpy(pBuffer, pusrdata + read_len, g_LLRP_RecvFrame.FrameExtract.MessageLength);
            read_len += g_LLRP_RecvFrame.FrameExtract.MessageLength;
            system_os_post(tcp_recvTaskPrio, TCP_SIG_RX, (ETSParam)pBuffer);
            g_LLRP_RecvFrame.nBuffer = 0;
            length -= g_LLRP_RecvFrame.FrameExtract.MessageLength;
        }
    }

}

/**
     * Check to see if we have a frame in the buffer.
     * If not, how many more bytes do we need?
*/
LLRP_tSFrameExtract ICACHE_FLASH_ATTR
LLRP_FrameExtract (
  const unsigned char *         pBuffer,
  unsigned int                  nBuffer)
{
    LLRP_tSFrameExtract         frameExtract;

    os_memset(&frameExtract, 0, sizeof frameExtract);

    llrp_u8_t rsvdVer = pBuffer[0];
    uint8_t version = (rsvdVer >> 2) & 0x07;
    if ((version > LLRP_PROTOCOL_MY_VERSION) || (version < LLRP_PROTOCOL_VERSION1)) {
        frameExtract.eStatus = LLRP_FRAME_ERROR;
        return frameExtract;
    }

    if(LLRP_FRAME_LENGTH_MIN > nBuffer)
    {
        frameExtract.MessageLength = 10u;
        frameExtract.nBytesNeeded = frameExtract.MessageLength - nBuffer;
        frameExtract.eStatus = LLRP_FRAME_NEED_MORE;
    }
    else
    {
        llrp_u16_t                  VersType;

        VersType = pBuffer[0];
        VersType <<= 8u;
        VersType |= pBuffer[1];

        frameExtract.MessageLength = pBuffer[2];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[3];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[4];
        frameExtract.MessageLength <<= 8u;
        frameExtract.MessageLength |= pBuffer[5];

        /*
         * Should we be picky about reserved bits?
         */

        frameExtract.MessageType = VersType & 0x3FFu;
        frameExtract.ProtocolVersion = (VersType >> 10u) & 0x7u;

        frameExtract.MessageID = pBuffer[6];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[7];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[8];
        frameExtract.MessageID <<= 8u;
        frameExtract.MessageID |= pBuffer[9];

        if(10u > frameExtract.MessageLength)
        {
            frameExtract.nBytesNeeded = frameExtract.MessageLength - nBuffer;
            frameExtract.eStatus = LLRP_FRAME_ERROR;
        }
        else if(nBuffer >= frameExtract.MessageLength)
        {
            frameExtract.nBytesNeeded = 0;
            frameExtract.eStatus = LLRP_FRAME_READY;
        }
        else
        {
            frameExtract.nBytesNeeded = frameExtract.MessageLength - nBuffer;
            frameExtract.eStatus = LLRP_FRAME_NEED_MORE;
        }
    }

    return frameExtract;
}


static LLRP_tSMessage * ICACHE_FLASH_ATTR decodeMessage (
  LLRP_tSFrameDecoderStream *   pDecoderStream)
{
    LLRP_tSFrameDecoder *       pDecoder  = pDecoderStream->pDecoder;
    LLRP_tSErrorDetails *       pError    = &pDecoder->decoderHdr.ErrorDetails;
    const LLRP_tSTypeRegistry * pRegistry = pDecoder->decoderHdr.pRegistry;
    LLRP_tSDecoderStream *      pBaseDecoderStream =
                                        &pDecoderStream->decoderStreamHdr;
    const LLRP_tSTypeDescriptor *pTypeDescriptor;
    llrp_u16_t                  Type;
    llrp_u16_t                  Vers;
    llrp_u32_t                  nLength;
    unsigned int                iLimit;
    llrp_u32_t                  MessageID;
    LLRP_tSElement *            pElement;
    LLRP_tSMessage *            pMessage;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    Type = get_u16(pBaseDecoderStream, &LLRP_g_fdMessageHeader_Type);
    Vers = (Type >> 10u) & 3;
    Type &= 0x3FF;

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    if(1u != Vers)
    {
        pError->eResultCode = LLRP_RC_BadVersion;
        pError->pWhatStr    = "unsupported version";
        pError->pRefType    = NULL;
        pError->pRefField   = &LLRP_g_fdMessageHeader_Type;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    nLength = get_u32(pBaseDecoderStream, &LLRP_g_fdMessageHeader_Length);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    if(10u > nLength)
    {
        pError->eResultCode = LLRP_RC_InvalidLength;
        pError->pWhatStr    = "message length too small";
        pError->pRefType    = NULL;
        pError->pRefField   = &LLRP_g_fdMessageHeader_Length;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    iLimit = pDecoderStream->iBegin + nLength;

    if(iLimit > pDecoderStream->iLimit)
    {
        pError->eResultCode = LLRP_RC_ExcessiveLength;
        pError->pWhatStr    = "message length exceeds enclosing length";
        pError->pRefType    = NULL;
        pError->pRefField   = &LLRP_g_fdMessageHeader_Length;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    pDecoderStream->iLimit = iLimit;

    MessageID = get_u32(pBaseDecoderStream, &LLRP_g_fdMessageHeader_MessageID);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        return NULL;
    }

    /* Custom? */
    if(1023u == Type)
    {
        llrp_u32_t              VendorPEN;
        llrp_u8_t               Subtype;

        VendorPEN = get_u32(pBaseDecoderStream,
                            &LLRP_g_fdMessageHeader_VendorPEN);
        Subtype   = get_u8(pBaseDecoderStream,
                            &LLRP_g_fdMessageHeader_Subtype);

        if(LLRP_RC_OK != pError->eResultCode)
        {
            return NULL;
        }

        pTypeDescriptor = LLRP_TypeRegistry_lookupCustomMessage(pRegistry,
            VendorPEN, Subtype);
        if(NULL == pTypeDescriptor)
        {
            /*
             * If we don't have a definition for a particular
             * CUSTOM message, just use the generic one.
             */
            pDecoder->iNext -= 5;       /* back up to VendorPEN and SubType */
            pTypeDescriptor = LLRP_TypeRegistry_lookupMessage(pRegistry, Type);
        }
    }
    else
    {
        pTypeDescriptor = LLRP_TypeRegistry_lookupMessage(pRegistry, Type);
    }

    if(NULL == pTypeDescriptor)
    {
        pError->eResultCode = LLRP_RC_UnknownMessageType;
        pError->pWhatStr    = "unknown message type";
        pError->pRefType    = NULL;
        pError->pRefField   = &LLRP_g_fdMessageHeader_Type;
        pError->OtherDetail = 0;
        return NULL;
    }

    pDecoderStream->pRefType = pTypeDescriptor;

    pElement = LLRP_Element_construct(pTypeDescriptor);

    if(NULL == pElement)
    {
        pError->eResultCode = LLRP_RC_MessageAllocationFailed;
        pError->pWhatStr    = "message allocation failed";
        pError->pRefType    = pTypeDescriptor;
        pError->pRefField   = NULL;
        pError->OtherDetail = pDecoder->iNext;
        return NULL;
    }

    pMessage = (LLRP_tSMessage *) pElement;
    pMessage->MessageID = MessageID;

    pTypeDescriptor->pfDecodeFields(pElement, pBaseDecoderStream);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        LLRP_Element_destruct(pElement);
        return NULL;
    }

    /*
     * Subparameters
     */
    while(0 < getRemainingByteCount(pDecoderStream) &&
          LLRP_RC_OK == pError->eResultCode)
    {
        LLRP_tSFrameDecoderStream       NestStream;
        LLRP_tSParameter *              pParameter;

        streamConstruct_nested(&NestStream, pDecoderStream);

        pParameter = decodeParameter(&NestStream);

        if(NULL == pParameter)
        {
            if(LLRP_RC_OK == pError->eResultCode)
            {
                pError->eResultCode = LLRP_RC_Botch;
                pError->pWhatStr    = "botch -- no param and no error";
                pError->pRefType    = pTypeDescriptor;
                pError->pRefField   = NULL;
                pError->OtherDetail = pDecoder->iNext;
            }
            break;
        }

        pParameter->elementHdr.pParent = pElement;
        LLRP_Element_addSubParameterToAllList(pElement, pParameter);
    }

    if(LLRP_RC_OK == pError->eResultCode)
    {
        if(pDecoder->iNext != pDecoderStream->iLimit)
        {
            pError->eResultCode = LLRP_RC_ExtraBytes;
            pError->pWhatStr    = "extra bytes at end of message";
            pError->pRefType    = pTypeDescriptor;
            pError->pRefField   = NULL;
            pError->OtherDetail = pDecoder->iNext;
        }
    }

    if(LLRP_RC_OK != pError->eResultCode)
    {
        LLRP_Element_destruct(pElement);
        return NULL;
    }

    pTypeDescriptor->pfAssimilateSubParameters(pElement, pError);

    if(LLRP_RC_OK != pError->eResultCode)
    {
        LLRP_Element_destruct(pElement);
        return NULL;
    }

    return pMessage;
}

LLRP_tSFrameDecoder * ICACHE_FLASH_ATTR
LLRP_FrameDecoder_construct (
  unsigned char *               pBuffer,
  unsigned int                  nBuffer)
{
    LLRP_tSFrameDecoder *       pDecoder;

    pDecoder = malloc(sizeof *pDecoder);
    if(NULL == pDecoder)
    {
        return pDecoder;
    }

    memset(pDecoder, 0, sizeof *pDecoder);

    pDecoder->pBuffer        = pBuffer;
    pDecoder->nBuffer        = nBuffer;

    pDecoder->iNext          = 0;
    pDecoder->BitFieldBuffer = 0;
    pDecoder->nBitFieldResid = 0;

    return pDecoder;
}



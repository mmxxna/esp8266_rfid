#include "user_LLRP.h"
#include "os_type.h"
#include "osapi.h"
#include "espconn.h"

// because of the resource limit, the reader cannot accomplish the xml type register and descriptor instanced in LTKC.

LLRP_tSRecvFrame g_LLRP_RecvFrame;

#define DEBUG

void LOCAL debug(char * msg)
{
#ifdef DEBUG
    os_printf(msg);
    os_printf("\n");
#else
#endif
}

void ICACHE_FLASH_ATTR
tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pespconn = arg;
 //   os_printf("tcp recv : %s \r\n", pusrdata);
//    char rtc_time[24];
//    os_sprintf(rtc_time, "rtc cal: %d\n", system_rtc_clock_cali_proc()>>12);
//    espconn_sent(pespconn, rtc_time, os_strlen(rtc_time));

    while (length > LLRP_FRAME_LENGTH_MIN)  // it may have a vaild frame
    {
        g_LLRP_RecvFrame.bFrameValid = false;
        g_LLRP_RecvFrame.FrameExtract = LLRP_FrameExtract(pusrdata, length);
        /*
         * Framing error?
         */
        if(LLRP_FRAME_ERROR == g_LLRP_RecvFrame.FrameExtract.eStatus)
        {
            g_LLRP_RecvFrame.ErrorDetails = LLRP_RC_RecvFramingError;
            debug("framing error in message stream");
            break;
        }
        else if (LLRP_FRAME_NEED_MORE == g_LLRP_RecvFrame.FrameExtract.eStatus)
        {
            unsigned int nRead = g_LLRP_RecvFrame.FrameExtract.nBytesNeeded;
            unsigned char * pBufPos = &g_LLRP_RecvFrame.pBuffer[g_LLRP_RecvFrame.nBuffer];
            /*
             * The frame extractor needs more data, make sure the
             * frame size fits in the receive buffer.
             */
            if(g_LLRP_RecvFrame.nBuffer + nRead > LLRP_FRAME_BUFFER_SIZE)
            {
                /* Buffer overflow */
                g_LLRP_RecvFrame.ErrorDetails = LLRP_RC_RecvBufferOverflow;
                debug("buffer overflow");
                break;
            }

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

/**
 *****************************************************************************
 **
 ** @brief  Internal routine to advance receiver
 **
 ** @param[in]  pConn           Pointer to the connection instance.
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 **
 ** @return     LLRP_RC_OK          Frame received
 **             LLRP_RC_RecvEOF     End-of-file condition on fd
 **             LLRP_RC_RecvIOError I/O error in poll() or read().
 **                                 Probably means fd is bad.
 **             LLRP_RC_RecvFramingError
 **                                 LLRP_FrameExtract() detected an
 **                                 impossible situation. Recovery unlikely.
 **             LLRP_RC_RecvTimeout Frame didn't complete within allowed time
 **             LLRP_RC_RecvBufferOverflow
 **                                 LLRP_FrameExtract() detected an inbound
 **                                 message that would overflow the receive
 **                                 buffer.
 **             LLRP_RC_...         Decoder error.
 **
 *****************************************************************************/
static LLRP_tResultCode
recvAdvance (
  LLRP_tSConnection *           pConn,
  int                           nMaxMS,
  time_t                        timeLimit)
{
    LLRP_tSErrorDetails *       pError = &pConn->Recv.ErrorDetails;

    /*
     * Clear the error details in the receiver state.
     */
    LLRP_Error_clear(pError);

    /*
     * Loop until victory or some sort of exception happens
     */
    for(;;)
    {
        int                     rc;

        /*
         * Note that the frame is in progress.
         * Existing buffer content, if any, is deemed
         * invalid or incomplete.
         */
        pConn->Recv.bFrameValid = FALSE;


        pConn->Recv.FrameExtract =
            LLRP_FrameExtract(pConn->Recv.pBuffer, pConn->Recv.nBuffer);

        /*
         * Framing error?
         */
        if(LLRP_FRAME_ERROR == pConn->Recv.FrameExtract.eStatus)
        {
            LLRP_Error_resultCodeAndWhatStr(pError,
                LLRP_RC_RecvFramingError, "framing error in message stream");
            break;
        }

        /*
         * Need more bytes? extractRc>0 means we do and extractRc is the
         * number of bytes immediately required.
         */
        if(LLRP_FRAME_NEED_MORE == pConn->Recv.FrameExtract.eStatus)
        {
            unsigned int        nRead = pConn->Recv.FrameExtract.nBytesNeeded;
            unsigned char *     pBufPos =
                                  &pConn->Recv.pBuffer[pConn->Recv.nBuffer];

            /*
             * Before we do anything that might block,
             * check to see if the time limit is exceeded.
             */
            if(0 != timeLimit)
            {
                if(time(NULL) > timeLimit)
                {
                    /* Timeout */
                    LLRP_Error_resultCodeAndWhatStr(pError,
                        LLRP_RC_RecvTimeout, "timeout");
                    break;
                }
            }

            /*
             * The frame extractor needs more data, make sure the
             * frame size fits in the receive buffer.
             */
            if(pConn->Recv.nBuffer + nRead > pConn->nBufferSize)
            {
                /* Buffer overflow */
                LLRP_Error_resultCodeAndWhatStr(pError,
                    LLRP_RC_RecvBufferOverflow, "buffer overflow");
                break;
            }

            /*
             * If this is not a block indefinitely request use poll()
             * to see if there is data in time.
             */
            if(nMaxMS >= 0)
            {
                struct pollfd           pfd;

                pfd.fd = pConn->fd;
                pfd.events = POLLIN;
                pfd.revents = 0;

                rc = poll(&pfd, 1, nMaxMS);
                if(0 > rc)
                {
                    /* Error */
                    LLRP_Error_resultCodeAndWhatStr(pError,
                        LLRP_RC_RecvIOError, "poll failed");
                    break;
                }
                if(0 == rc)
                {
                    /* Timeout */
                    LLRP_Error_resultCodeAndWhatStr(pError,
                        LLRP_RC_RecvTimeout, "timeout");
                    break;
                }
            }

            /*
             * Read some number of bytes from the socket.
             */
            rc = read(pConn->fd, pBufPos, nRead);
            if(0 > rc)
            {
                /*
                 * Error. Note this could be EWOULDBLOCK if the
                 * file descriptor is using non-blocking I/O.
                 * So we return the error but do not tear-up
                 * the receiver state.
                 */
                LLRP_Error_resultCodeAndWhatStr(pError,
                    LLRP_RC_RecvIOError, "recv IO error");
                break;
            }

            if(0 == rc)
            {
                /* EOF */
                LLRP_Error_resultCodeAndWhatStr(pError,
                    LLRP_RC_RecvEOF, "recv end-of-file");
                break;
            }

            /*
             * When we get here, rc>0 meaning some bytes were read.
             * Update the number of bytes present.
             * Then loop to the top and retry the FrameExtract().
             */
            pConn->Recv.nBuffer += rc;

            continue;
        }

        /*
         * Is the frame ready?
         * If a valid frame is present, decode and enqueue it.
         */
        if(LLRP_FRAME_READY == pConn->Recv.FrameExtract.eStatus)
        {
            /*
             * Frame appears complete. Time to try to decode it.
             */
            LLRP_tSFrameDecoder *   pDecoder;
            LLRP_tSMessage *        pMessage;
            LLRP_tSMessage **       ppMessageTail;

            /*
             * Construct a new frame decoder. It needs the registry
             * to facilitate decoding.
             */
            pDecoder = LLRP_FrameDecoder_construct(pConn->pTypeRegistry,
                    pConn->Recv.pBuffer, pConn->Recv.nBuffer);

            /*
             * Make sure we really got one. If not, weird problem.
             */
            if(pDecoder == NULL)
            {
                /* All we can do is discard the frame. */
                pConn->Recv.nBuffer = 0;
                pConn->Recv.bFrameValid = FALSE;
                LLRP_Error_resultCodeAndWhatStr(pError,
                    LLRP_RC_MiscError, "decoder constructor failed");
                break;
            }

            /*
             * Now ask the nice, brand new decoder to decode the frame.
             * It returns NULL for some kind of error.
             * The &...decoderHdr is in lieu of type casting since
             * the generic LLRP_Decoder_decodeMessage() takes the
             * generic LLRP_tSDecoder.
             */
            pMessage = LLRP_Decoder_decodeMessage(&pDecoder->decoderHdr);

            /*
             * Always capture the error details even when it works.
             * Whatever happened, we are done with the decoder.
             */
            pConn->Recv.ErrorDetails = pDecoder->decoderHdr.ErrorDetails;

            /*
             * Bye bye and thank you li'l decoder.
             */
            LLRP_Decoder_destruct(&pDecoder->decoderHdr);

            /*
             * If NULL there was an error. Clean up the
             * receive state. Return the error.
             */
            if(NULL == pMessage)
            {
                /*
                 * Make sure the return is not LLRP_RC_OK
                 */
                if(LLRP_RC_OK == pError->eResultCode)
                {
                    LLRP_Error_resultCodeAndWhatStr(pError,
                        LLRP_RC_MiscError, "NULL message but no error");
                }

                /*
                 * All we can do is discard the frame.
                 */
                pConn->Recv.nBuffer = 0;
                pConn->Recv.bFrameValid = FALSE;

                break;
            }

            /*
             * Yay! It worked. Enqueue the message.
             */
            ppMessageTail = &pConn->pInputQueue;
            while(NULL != *ppMessageTail)
            {
                ppMessageTail = &(*ppMessageTail)->pQueueNext;
            }

            pMessage->pQueueNext = NULL;
            *ppMessageTail = pMessage;

            /*
             * Note that the frame is valid. Consult
             * Recv.FrameExtract.MessageLength.
             * Clear the buffer count to be ready for next time.
             */
            pConn->Recv.bFrameValid = TRUE;
            pConn->Recv.nBuffer = 0;

            break;
        }

        /*
         * If we get here there was an FrameExtract status
         * we didn't expect.
         */

        /*NOTREACHED*/
        assert(0);
    }

    return pError->eResultCode;
}


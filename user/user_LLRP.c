#include "user_LLRP.h"
#include "os_type.h"
#include "osapi.h"

LLRP_tSFrameExtract ICACHE_FLASH_ATTR
LLRP_FrameExtract (
  const unsigned char *         pBuffer,
  unsigned int                  nBuffer)
{
    LLRP_tSFrameExtract         frameExtract;

    os_memset(&frameExtract, 0, sizeof frameExtract);

    if(10 > nBuffer)
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

/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * File: wroute.c
 *
 * Purpose: handle WMAC frame relay & filterring
 *
 * Author: Lyndon Chen
 *
 * Date: May 20, 2003
 *
 * Functions:
 *      ROUTEbRelay - Relay packet
 *
 * Revision History:
 *
 */


#if !defined(__MAC_H__)
#include "mac.h"
#endif
#if !defined(__TCRC_H__)
#include "tcrc.h"
#endif
#if !defined(__RXTX_H__)
#include "rxtx.h"
#endif
#if !defined(__WROUTE_H__)
#include "wroute.h"
#endif
#if !defined(__CARD_H__)
#include "card.h"
#endif
#if !defined(__BASEBAND_H__)
#include "baseband.h"
#endif
/*---------------------  Static Definitions -------------------------*/

/*---------------------  Static Classes  ----------------------------*/

/*---------------------  Static Variables  --------------------------*/
static int          msglevel                =MSG_LEVEL_INFO;
//static int          msglevel                =MSG_LEVEL_DEBUG;
/*---------------------  Static Functions  --------------------------*/

/*---------------------  Export Variables  --------------------------*/



/*
 * Description:
 *      Relay packet.  Return TRUE if packet is copy to DMA1
 *
 * Parameters:
 *  In:
 *      pDevice             -
 *      pbySkbData          - rx packet skb data
 *  Out:
 *      TURE, FALSE
 *
 * Return Value: TRUE if packet duplicate; otherwise FALSE
 *
 */
BOOL ROUTEbRelay (PSDevice pDevice, PBYTE pbySkbData, UINT uDataLen, UINT uNodeIndex)
{
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    PSTxDesc        pHeadTD, pLastTD;
    UINT            cbFrameBodySize;
    UINT            uMACfragNum;
    BYTE            byPktTyp;
    BOOL            bNeedEncryption = FALSE;
    SKeyItem        STempKey;
    PSKeyItem       pTransmitKey = NULL;
    UINT            cbHeaderSize;
    UINT            ii;
    PBYTE           pbyBSSID;




    if (AVAIL_TD(pDevice, TYPE_AC0DMA)<=0) {
        DEVICE_PRT(MSG_LEVEL_DEBUG, KERN_INFO "Relay can't allocate TD1..\n");
        return FALSE;
    }

    pHeadTD = pDevice->apCurrTD[TYPE_AC0DMA];

    pHeadTD->m_td1TD1.byTCR = (TCR_EDP|TCR_STP);

    memcpy(pDevice->sTxEthHeader.abyDstAddr, (PBYTE)pbySkbData, U_HEADER_LEN);

    cbFrameBodySize = uDataLen - U_HEADER_LEN;

    if (ntohs(pDevice->sTxEthHeader.wType) > MAX_DATA_LEN) {
        cbFrameBodySize += 8;
    }

    if (pDevice->bEncryptionEnable == TRUE) {
        bNeedEncryption = TRUE;

        // get group key
        pbyBSSID = pDevice->abyBroadcastAddr;
        if(KeybGetTransmitKey(&(pDevice->sKey), pbyBSSID, GROUP_KEY, &pTransmitKey) == FALSE) {
            pTransmitKey = NULL;
            DEVICE_PRT(MSG_LEVEL_DEBUG, KERN_DEBUG"KEY is NULL. [%d]\n", pDevice->pMgmt->eCurrMode);
        } else {
            DEVICE_PRT(MSG_LEVEL_DEBUG, KERN_DEBUG"Get GTK.\n");
        }
    }

    if (pDevice->bEnableHostWEP) {
        if (uNodeIndex >= 0) {
            pTransmitKey = &STempKey;
            pTransmitKey->byCipherSuite = pMgmt->sNodeDBTable[uNodeIndex].byCipherSuite;
            pTransmitKey->dwKeyIndex = pMgmt->sNodeDBTable[uNodeIndex].dwKeyIndex;
            pTransmitKey->uKeyLength = pMgmt->sNodeDBTable[uNodeIndex].uWepKeyLength;
            pTransmitKey->dwTSC47_16 = pMgmt->sNodeDBTable[uNodeIndex].dwTSC47_16;
            pTransmitKey->wTSC15_0 = pMgmt->sNodeDBTable[uNodeIndex].wTSC15_0;
            memcpy(pTransmitKey->abyKey,
                &pMgmt->sNodeDBTable[uNodeIndex].abyWepKey[0],
                pTransmitKey->uKeyLength
                );
        }
    }

    uMACfragNum = cbGetFragCount(pDevice, pTransmitKey, cbFrameBodySize, &pDevice->sTxEthHeader);

    if (uMACfragNum > AVAIL_TD(pDevice,TYPE_AC0DMA)) {
        return FALSE;
    }
    byPktTyp = (BYTE)pDevice->byPacketType;

    if (pDevice->bFixRate) {
        if (pDevice->eCurrentPHYType == PHY_TYPE_11B) {
            if (pDevice->uConnectionRate >= RATE_11M) {
                pDevice->wCurrentRate = RATE_11M;
            } else {
                pDevice->wCurrentRate = (WORD)pDevice->uConnectionRate;
            }
        } else {
            if ((pDevice->eCurrentPHYType == PHY_TYPE_11A) &&
                (pDevice->uConnectionRate <= RATE_6M)) {
                pDevice->wCurrentRate = RATE_6M;
            } else {
                if (pDevice->uConnectionRate >= RATE_54M)
                    pDevice->wCurrentRate = RATE_54M;
                else
                    pDevice->wCurrentRate = (WORD)pDevice->uConnectionRate;
            }
        }
    }
    else {
        pDevice->wCurrentRate = pDevice->pMgmt->sNodeDBTable[uNodeIndex].wTxDataRate;
    }

    if (pDevice->wCurrentRate <= RATE_11M)
        byPktTyp = PK_TYPE_11B;

    vGenerateFIFOHeader(pDevice, byPktTyp, pDevice->pbyTmpBuff, bNeedEncryption,
                        cbFrameBodySize, TYPE_AC0DMA, pHeadTD,
                        &pDevice->sTxEthHeader, pbySkbData, pTransmitKey, uNodeIndex,
                        &uMACfragNum,
                        &cbHeaderSize
                        );

    if (MACbIsRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_PS)) {
        // Disable PS
        MACbPSWakeup(pDevice->PortOffset);
    }

    pDevice->bPWBitOn = FALSE;

    pLastTD = pHeadTD;
    for (ii = 0; ii < uMACfragNum; ii++) {
        // Poll Transmit the adapter
        wmb();
        pHeadTD->m_td0TD0.f1Owner=OWNED_BY_NIC;
        wmb();
        if (ii == (uMACfragNum - 1))
            pLastTD = pHeadTD;
        pHeadTD = pHeadTD->next;
    }

    pLastTD->pTDInfo->skb = 0;
    pLastTD->pTDInfo->byFlags = 0;

    pDevice->apCurrTD[TYPE_AC0DMA] = pHeadTD;

    MACvTransmitAC0(pDevice->PortOffset);

    return TRUE;
}




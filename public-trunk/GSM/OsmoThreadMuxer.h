/*
* Copyright 2011 Harald Welte <laforge@gnumonks.org>
*
* This software is distributed under the terms of the GNU Affero Public License.
* See the COPYING file in the main directory for details.
*
* This use of this software may be subject to additional restrictions.
* See the LEGAL file in the main directory for details.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef OsmoThreadMuxer_H
#define OsmoThreadMuxer_H

#include "OsmoLogicalChannel.h"
#include <TRXManager.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "gsmL1prim.h"

#define SYS_WRITE 0
#define L1_WRITE 1
#define SYS_READ 2
#define L1_READ 3

/* Resolves multiple definition conflict between ortp and osmocom msgb's */
namespace Osmo {
	extern "C" {
		#include <osmocom/core/msgb.h>
		#include <osmocom/core/utils.h>
	}

	const struct value_string femtobts_l1prim_names[GsmL1_PrimId_NUM+1] = {
		{ GsmL1_PrimId_MphInitReq,	"MPH-INIT.req" },
		{ GsmL1_PrimId_MphCloseReq,	"MPH-CLOSE.req" },
		{ GsmL1_PrimId_MphConnectReq,	"MPH-CONNECT.req" },
		{ GsmL1_PrimId_MphDisconnectReq,"MPH-DISCONNECT.req" },
		{ GsmL1_PrimId_MphActivateReq,	"MPH-ACTIVATE.req" },
		{ GsmL1_PrimId_MphDeactivateReq,"MPH-DEACTIVATE.req" },
		{ GsmL1_PrimId_MphConfigReq,	"MPH-CONFIG.req" },
		{ GsmL1_PrimId_MphMeasureReq,	"MPH-MEASURE.req" },
		{ GsmL1_PrimId_MphInitCnf,	"MPH-INIT.conf" },
		{ GsmL1_PrimId_MphCloseCnf,	"MPH-CLOSE.conf" },
		{ GsmL1_PrimId_MphConnectCnf,	"MPH-CONNECT.conf" },
		{ GsmL1_PrimId_MphDisconnectCnf,"MPH-DISCONNECT.conf" },
		{ GsmL1_PrimId_MphActivateCnf,	"MPH-ACTIVATE.conf" },
		{ GsmL1_PrimId_MphDeactivateCnf,"MPH-DEACTIVATE.conf" },
		{ GsmL1_PrimId_MphConfigCnf,	"MPH-CONFIG.conf" },
		{ GsmL1_PrimId_MphMeasureCnf,	"MPH-MEASURE.conf" },
		{ GsmL1_PrimId_MphTimeInd,	"MPH-TIME.ind" },
		{ GsmL1_PrimId_MphSyncInd,	"MPH-SYNC.ind" },
		{ GsmL1_PrimId_PhEmptyFrameReq,	"PH-EMPTY_FRAME.req" },
		{ GsmL1_PrimId_PhDataReq,	"PH-DATA.req" },
		{ GsmL1_PrimId_PhConnectInd,	"PH-CONNECT.ind" },
		{ GsmL1_PrimId_PhReadyToSendInd,"PH-READY_TO_SEND.ind" },
		{ GsmL1_PrimId_PhDataInd,	"PH-DATA.ind" },
		{ GsmL1_PrimId_PhRaInd,		"PH-RA.ind" },
		{ 0, NULL }
	};

	const struct value_string femtobts_sysprim_names[FemtoBts_PrimId_NUM+1] = {
		{ FemtoBts_PrimId_SystemInfoReq,	"SYSTEM-INFO.req" },
		{ FemtoBts_PrimId_SystemInfoCnf,	"SYSTEM-INFO.conf" },
		{ FemtoBts_PrimId_SystemFailureInd,	"SYSTEM-FAILURE.ind" },
		{ FemtoBts_PrimId_ActivateRfReq,	"ACTIVATE-RF.req" },
		{ FemtoBts_PrimId_ActivateRfCnf,	"ACTIVATE-RF.conf" },
		{ FemtoBts_PrimId_DeactivateRfReq,	"DEACTIVATE-RF.req" },
		{ FemtoBts_PrimId_DeactivateRfCnf,	"DEACTIVATE-RF.conf" },
		{ FemtoBts_PrimId_SetTraceFlagsReq,	"SET-TRACE-FLAGS.req" },
		{ FemtoBts_PrimId_RfClockInfoReq,	"RF-CLOCK-INFO.req" },
		{ FemtoBts_PrimId_RfClockInfoCnf,	"RF-CLOCK-INFO.conf" },
		{ FemtoBts_PrimId_RfClockSetupReq,	"RF-CLOCK-SETUP.req" },
		{ FemtoBts_PrimId_RfClockSetupCnf,	"RF-CLOCK-SETUP.conf" },
		{ FemtoBts_PrimId_Layer1ResetReq,	"LAYER1-RESET.req" },
		{ FemtoBts_PrimId_Layer1ResetCnf,	"LAYER1-RESET.conf" },
		{ 0, NULL }
	};

	const struct value_string femtobts_l1status_names[GSML1_STATUS_NUM+1] = {
		{ GsmL1_Status_Success,		"Success" },
		{ GsmL1_Status_Generic,		"Generic error" },
		{ GsmL1_Status_NoMemory,	"Not enough memory" },
		{ GsmL1_Status_Timeout,		"Timeout" },
		{ GsmL1_Status_InvalidParam,	"Invalid parameter" },
		{ GsmL1_Status_Busy,		"Resource busy" },
		{ GsmL1_Status_NoRessource,	"No more resources" },
		{ GsmL1_Status_Uninitialized, "Trying to use uninitialized resource" },
		{ GsmL1_Status_NullInterface,	"Trying to call a NULL interface" },
		{ GsmL1_Status_NullFctnPtr,	"Trying to call a NULL function ptr" },
		{ GsmL1_Status_BadCrc,		"Bad CRC" },
		{ GsmL1_Status_BadUsf,		"Bad USF" },
		{ GsmL1_Status_InvalidCPS,	"Invalid CPS field" },
		{ GsmL1_Status_UnexpectedBurst,	"Unexpected burst" },
		{ GsmL1_Status_UnavailCodec,	"AMR codec is unavailable" },
		{ GsmL1_Status_CriticalError,	"Critical error" },
		{ GsmL1_Status_OverheatError,	"Overheat error" },
		{ GsmL1_Status_DeviceError,	"Device error" },
		{ GsmL1_Status_FacchError,	"FACCH / TCH order error" },
		{ GsmL1_Status_AlreadyDeactivated, "Lchan already deactivated" },
		{ GsmL1_Status_TxBurstFifoOvrn,	"FIFO overrun" },
		{ GsmL1_Status_TxBurstFifoUndr,	"FIFO underrun" },
		{ GsmL1_Status_NotSynchronized,	"Not synchronized" },
		{ GsmL1_Status_Unsupported,	"Unsupported feature" },
		{ 0, NULL }
	};
}

namespace GSM {

/* The idea of this monster is to provide an interface between the
 * heavily multi-threaded OpenBTS architecture and the single-threaded
 * osmo-bts architecture.
 *
 * In the Uplink Rx path, L1Decoder calls OsmoSAPMux:writeLowSide,
 * which directly passes the call through to * OsmoThreadMuxer:writeLowSide
 * 
 * At this point, the L2Frame needs to be converted from unpacked bits
 * to packed bits, and wrapped with some layer1 primitive header.  Next,
 * it is enqueued into a FIFO leading towards osmo-bts.  Whenever that
 * FIFO has data to be written, we signal this via the sock_fd.
 * (socketpair).  The other fd ends up in the select() loop of osmo-bts.
 *
 * In the Downling Tx path, the OsmoThreadMuxer thread FIXME
 */
class OsmoThreadMuxer {

protected:
	int mSockFd[4];
	OsmoTRX *mTRX[1];
	unsigned int mNumTRX;

public:
	OsmoThreadMuxer()
		:mNumTRX(0)
	{
		createSockets();

		if(mSockFd[0] < 0 || mSockFd[1] < 0 || mSockFd[2] < 0 || mSockFd[3] < 0)
		{
			LOG(ERROR) << "Error creating socket files. RESTART!";
		}
		else
		{
			LOG(INFO) << "All 4 socket files created.";
		}

		startThreads();
	}

	OsmoTRX &addTRX(TransceiverManager &trx_mgr, unsigned int trx_nr) {
		/* for now we only support a single TRX */
		assert(mNumTRX == 0);
		OsmoTRX *otrx = new OsmoTRX(trx_mgr, trx_nr, this);
		mTRX[mNumTRX++] = otrx;
		return *otrx;
	}

	/* receive frame synchronously from L1Decoder->OsmoSAPMux and
	 * enqueue it towards osmo-bts */
	virtual void writeLowSide(const L2Frame& frame,
				  OsmoLogicalChannel *lchan);

	/* L1 informs us about the next TDMA time for which it needs
	 * data */
	virtual void signalNextWtime(GSM::Time &time,
				     OsmoLogicalChannel &lchan);

private:
	/* Initialization functions */
	void createSockets();
	void startThreads();

	/* Functions for processing SYS type messages */
	void recvSysMsg();
	void handleSysMsg(const char *buffer);
	void sendSysMsg(struct Osmo::msgb *msg);

	/* Functions for processing L1 type messages */
	void recvL1Msg();
	void handleL1Msg(const char *buffer);
	void sendL1Msg(struct Osmo::msgb *msg);

	/* Functions to process SYS REQ messages from osmo-bts and 
	 * build corresponding SYS CNF messages to send back */
	void processSystemInfoReq();
	void processActivateRfReq();
	void processDeactivateRfReq(struct Osmo::msgb *recv_msg);
	void processLayer1ResetReq();

	/* Functions to process L1 REQ messages from osmo-bts and 
	 * build corresponding L1 CNF messages to send back */
	void processMphInitReq();
	void processMphConnectReq();
	void processMphActivateReq();

	/* Helper function for value parsing */
	const char* getPath(const int index);

	/* Service loop adapters for pthreads */
	friend void *RecvSysMsgLoopAdapter(OsmoThreadMuxer *TMux);
	friend void *RecvL1MsgLoopAdapter(OsmoThreadMuxer *TMux);
};

void *RecvSysMsgLoopAdapter(OsmoThreadMuxer *TMux);
void *RecvL1MsgLoopAdapter(OsmoThreadMuxer *TMux);

};		// GSM

#endif /* OsmoThreadMuxer_H */

/*
* Copyright 2008 Free Software Foundation, Inc.
* Copyright 2012 Thomas Cooper <tacooper@vt.edu>
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



#ifndef SAPMUX_H
#define SAPMUX_H

#include "GSMTransfer.h"
#include "GSML1FEC.h"

#include <Logger.h>

namespace GSM {

class L2DL;


/**
	A SAPMux is a multipexer the connects a single L1 to multiple L2s.
	A "service access point" in GSM/ISDN is analogous to port number in IP.
	GSM allows up to 4 SAPs, although only two are presently used.
	See GSM 04.05 5.2 for an introduction.
*/
class SAPMux  {

	protected:

	mutable Mutex mLock;
	L2DL * mUpstream[4];		///< one L2 for each SAP, GSM 04.05 5.3
	L1FEC * mDownstream;		///< a single L1

	public:

	SAPMux(){ 
		mUpstream[0] = NULL;
		mUpstream[1] = NULL;
		mUpstream[2] = NULL;
		mUpstream[3] = NULL;
		mDownstream  = NULL;
	}

	virtual ~SAPMux() {}

	virtual void writeHighSide(const L2Frame& frame);
	virtual void writeLowSide(const L2Frame& frame);
	/* Call writeLowSides below, not above, to handle in OsmoSAPMux instead */
	virtual void writeLowSide(const L2Frame& frame, const GSM::Time time, 
		const float RSSI, const int TA, const float FER) {};
	virtual void writeLowSideSACCH(const L2Frame& frame, const GSM::Time time, 
		const float RSSI, const int TA, const float FER, const int MSpower, 
		const int MStiming) {};
	virtual void writeLowSideTCH(const unsigned char* frame, 
		const GSM::Time time, const float RSSI, const int TA, const float FER) 
		{};
	virtual void signalNextWtime(GSM::Time &time) {};

	virtual void upstream( L2DL * wUpstream, unsigned wSAPI=0 )
		{ assert(mUpstream[wSAPI]==NULL); mUpstream[wSAPI]=wUpstream; }
	void downstream( L1FEC * wDownstream )
		{ assert(mDownstream==NULL); mDownstream=wDownstream; }

};




/**
	The LoopbackSAPMux is a test fixture.
	Writes to the high side are eachoed back to the high side.
	Writes to the low side are echoed back to the low side.
*/
class LoopbackSAPMux : public SAPMux {

	public:

	LoopbackSAPMux():SAPMux() {}

	void writeHighSide(const L2Frame& frame);
	void writeLowSide(const L2Frame& frame);

};


/**
	The L1TestPointSAPMux is a test fixture.
	Writes to the high side pass through to the low side.
	Writes to the low side are dumped to cout.
*/
class L1TestPointSAPMux : public SAPMux {

	public:

	L1TestPointSAPMux():SAPMux() {}
	~L1TestPointSAPMux() {}

	// These are defined in the .h so that we
	// don't have to link in all of L2 to
	// use them.

	void writeHighSide(const L2Frame& frame)
	{
		assert(mDownstream);
		mLock.lock();
		mDownstream->writeHighSide(frame);
		mLock.unlock();
	}

	void writeLowSide(const L2Frame& frame)
	{
		LOG(DEBUG) << "SAPMux::writeLowSide frame=" << frame;
	}

};




}	// namespace GSM


#endif
// vim: ts=4 sw=4

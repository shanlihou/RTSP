#pragma once
#include "RTP.h"
#include "RTCPPacket.h"
class RTCP :public UDP
{
public:
    RTCP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort);
	void processData(std::string data) override;
private:
	void sendRR();

    RTP rtp;
	UINT32 mSRRecvTime;
	SRPacket mLastSR;
	SDPacket mLastSD;
	UINT32 SSRC;
	std::unique_ptr<UINT8[]> mSDPtr;
	UINT32 mSDLen;
};
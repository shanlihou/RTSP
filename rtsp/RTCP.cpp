#include <ctime>
#include "RTCP.h"
#include "RTCPPacket.h"
static UINT8 RRHeader[] = {0x81, 0xc9, 0x00, 0x07};
RTCP::RTCP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort) :UDP(srcPort + 1, dstAddr, dstPort + 1), 
    rtp(srcPort, dstAddr, dstPort)
{
    SSRC = rand() << 16 + rand();
}

void RTCP::processData(std::string data)
{
	UINT32 index = 0;
	const char *cur = data.c_str();
	UINT32 len = data.length();
	while (len > 0)
	{
		RTCPPacket rPack;
		memcpy(&rPack, cur, RTCP_HEADER_SIZE);
		cur += 4;
		len -= 4;
		rPack.length = swap16(rPack.length);
		rPack.length = (rPack.length + 1) * 4;
		printf("rPack size:%d, pt:%d\n", rPack.length, rPack.pt);
		switch(rPack.pt)
		{
		case 200:
			mSRRecvTime = GetTickCount();
			memcpy(&mLastSR, cur, rPack.length - 4);
			mLastSR.senderPackCount = swap32(mLastSR.senderPackCount);
			break;
		case 202:
			mSDLen = rPack.length;
			mSDPtr.reset(new UINT8[mSDLen]);
			memcpy(mSDPtr.get(), cur - 4, mSDLen);
			memcpy(mSDPtr.get() + 4, &SSRC, 4);

			/*mLastSD.text.assign(cur + 6, mLastSD.length);
			printf("sd text:%s\n", mLastSD.text.c_str());*/
			break;
		default:
			break;
		}
		cur += rPack.length - 4;
		len -= rPack.length - 4;
	}
	sendRR();
}

void RTCP::sendRR()
{
	printf("will die?\n");
	RRPacket rrPack;
	memcpy(&rrPack, RRHeader, sizeof(RRHeader));
	rrPack.SSRC = SSRC;
	srand((unsigned int) time(NULL));
	rrPack.ID = mLastSR.SSRC;
	UINT32 count = rtp.getPackCount();
	rrPack.packLost = mLastSR.senderPackCount - count;
	if (mLastSR.senderPackCount <= count)
	{
		rrPack.fractionLost = 0;
	}
	else
	{
		rrPack.fractionLost = rrPack.packLost * 255 / mLastSR.senderPackCount;
	}
	rrPack.seqCycle = 1;
	rrPack.highSeq = rtp.getHighSeq();
	rrPack.jitter = rtp.getJitter();
	rrPack.SRTimeStamp = (mLastSR.timeStamp & 0xffffffff0000) >> 16;
	rrPack.DelaySR = (GetTickCount() - mSRRecvTime) * 65535 / 1000;
	printf("ID:%u, packLost:%d, fractionLost:%u, highSeq:%u jitter:%u DelaySR:%u\n", rrPack.ID, rrPack.packLost, rrPack.fractionLost, rrPack.highSeq, rrPack.jitter, rrPack.DelaySR);

	UINT32 sizeOfWholeRR = sizeof(rrPack) + mSDLen;
	printf("size:%d\n", sizeOfWholeRR);
	std::unique_ptr<UINT8[]> sendPtr(new UINT8[sizeOfWholeRR]);
	memcpy(sendPtr.get(), &rrPack, sizeof(rrPack));
	memcpy(sendPtr.get() + sizeof(rrPack), mSDPtr.get(), mSDLen);
	
	std::string sendMsg((char *)sendPtr.get(), sizeOfWholeRR);
    MyEvent::getInstance()->sendTo(sock, dstAddr.c_str(), dstPort, sendMsg);
}
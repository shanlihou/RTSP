#include "RTCP.h"
#include "RTCPPacket.h"
RTCP::RTCP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort) :UDP(srcPort + 1, dstAddr, dstPort + 1), 
    rtp(srcPort, dstAddr, dstPort)
{
    
}

void RTCP::processData(std::string data)
{
	UINT32 index = 0;
	while (index < data.length())
	{
		RTCPPacket rPack;
		memcpy(&rPack, data.c_str(), RTCP_HEADER_SIZE);
		rPack.length = swap(rPack.length);
		printf("rPack size:%d, pt:%d\n", rPack.length, rPack.pt);
		rPack.data = data.substr(index + RTCP_HEADER_SIZE);
		index += rPack.length;
	}
}
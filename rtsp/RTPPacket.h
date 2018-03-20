#pragma once
#include <Windows.h>
#define RTP_HEADER_SIZE 12
struct RTPPacket
{
	UINT8 ver:2;
	UINT8 pad:1;
	UINT8 ext:1;
	UINT8 csrcCount:4;
	UINT8 M:1;
	UINT8 PT:7;
	UINT16 seqNum;
	UINT32 timeStamp;
	UINT32 ssrc;
	std::string payload;
	RTPPacket(){};
	//RTPPacket(RTPPacket &&t){};
	friend bool operator > (const RTPPacket &r1, const RTPPacket &r2)
	{
		return r1.seqNum > r2.seqNum;
	};
};
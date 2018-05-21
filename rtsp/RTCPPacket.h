#pragma once
#include <Windows.h>
#define RTCP_HEADER_SIZE 4
#pragma pack(4)
struct SRPacket
{
	UINT32 SSRC;
	UINT64 timeStamp;
	UINT32 rtpTimeStamp;
	UINT32 senderPackCount;
	UINT32 senderOctetCount;
};

struct SDPacket
{
	UINT32 SSRC;
	UINT8 type;
	UINT8 length;
	std::string text;
};
struct RRPacket
{
	UINT8 ver:2;
	UINT8 pad:1;
	UINT8 rc:5;
	UINT8 pt;
	UINT16 length;
	UINT32 SSRC;
	UINT32 ID;
	UINT32 fractionLost:8;
	UINT32 packLost:24; 
	UINT16 seqCycle;
	UINT16 highSeq;
	UINT32 jitter;
	UINT32 SRTimeStamp;
	UINT32 DelaySR;
};

struct RTCPPacket
{
	UINT8 ver:2;
	UINT8 pad:1;
	UINT8 rc:5;
	UINT8 pt;
	UINT16 length;
	std::string data;
};
#pragma once
#include <Windows.h>
#define RTCP_HEADER_SIZE 4
struct RTCPPacket
{
	UINT8 ver:2;
	UINT8 pad:1;
	UINT8 rc:5;
	UINT8 pt;
	UINT16 length;
	std::string data;
};
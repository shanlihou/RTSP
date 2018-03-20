#pragma once
#include <queue>
#include <memory>
#include "MyEvent.h"
#include "RTPPacket.h"
class UDP
{
public:
    UDP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort);
    ~UDP();
    virtual void processData(std::string data);
    void *attachFunc();
protected:
    evutil_socket_t sock;
    std::string dstAddr;
    UINT32 dstPort;
    CallBack mFunc;
};


class RTP :public UDP
{
public:
    RTP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort);
	void processData(std::string data) override;
	void *createTimerFunc();
	void timerFunc();
private:
	std::unique_ptr<FILE, decltype(fclose)*> fw;
	std::priority_queue<RTPPacket, std::vector<RTPPacket>, std::greater<RTPPacket>> rtpPackets;
	CallBack mTimerFunc;
	std::unique_ptr<event, EventDeleter> mEvPtr;
};
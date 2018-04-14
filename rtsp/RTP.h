#pragma once
#include <queue>
#include <memory>
#include "MyEvent.h"
#include "RTPPacket.h"
inline UINT16 swap(UINT16 num);
class UDP
{
public:
	typedef std::function<void(std::string)> Proc;
	typedef std::vector<Proc> VecProc;
public:
    UDP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort);
    ~UDP();
    virtual void processData(std::string data);
    void *getCallBack();
	void attach(Proc);
protected:
	inline void notifyProc(std::string);
protected:
	VecProc mVProc;
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
	std::string parseNalU(std::string FU_A);

	std::unique_ptr<FILE, decltype(fclose)*> fw;
	std::priority_queue<RTPPacket, std::vector<RTPPacket>, std::greater<RTPPacket>> rtpPackets;
	CallBack mTimerFunc;
	std::unique_ptr<event, EventDeleter> mEvPtr;
};
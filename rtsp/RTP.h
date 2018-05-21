#pragma once
#include <queue>
#include <memory>
#include <list>
#include "MyEvent.h"
#include "RTPPacket.h"

inline UINT16 swap16(UINT16 num)
{
	return (num >> 8) | (num << 8);
}

inline UINT32 swap32(UINT32 num)
{
	return ((num & 0x000000ff) << 24) |
			((num & 0x0000ff00) << 8)|
			((num & 0x00ff0000) >> 8)|
			((num & 0xff000000) >> 24);
}

class UDP
{
public:
	typedef std::function<void(std::string &)> Proc;
	typedef std::vector<Proc> VecProc;
public:
    UDP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort);
    ~UDP();
    virtual void processData(std::string data);
    void *getCallBack();
	void attach(Proc);
protected:
	inline void notifyProc(std::string &);
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
	UINT32 getPackCount();
	UINT16 getHighSeq();
	UINT32 getJitter();
private:
	std::string parseNalU(std::string FU_A);

	std::unique_ptr<FILE, decltype(fclose)*> fw;
	std::priority_queue<RTPPacket, std::vector<RTPPacket>, std::greater<RTPPacket>> rtpPackets;
	CallBack mTimerFunc;
	std::unique_ptr<event, EventDeleter> mEvPtr;
	UINT32 count;
	UINT16 highSeq;
	UINT32 highTimeStamp;
	std::list<UINT32> mTimeList;
};
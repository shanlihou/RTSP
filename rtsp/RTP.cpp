#include "RTP.h"
UDP::UDP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort):dstAddr(dstAddr), dstPort(dstPort)
{
    sock = MyEvent::getInstance()->bindUdp(srcPort, attachFunc());
    char data[] = { 0xce, 0xfa, 0xed, 0xfe , 0};
    std::string sendMsg(data);
	printf("sendMsg len:%d\n", sendMsg.length());
    MyEvent::getInstance()->sendTo(sock, dstAddr, dstPort, sendMsg);
    MyEvent::getInstance()->sendTo(sock, dstAddr, dstPort, sendMsg);
}

void UDP::processData(std::string data)
{
    //printf("rtp data:%s\n", data.c_str());
}

void *UDP::attachFunc()
{
    mFunc = [=](std::string data) {processData(data); };
    return &mFunc;
}
UDP::~UDP()
{
    MyEvent::getInstance()->detachSock(sock);
}
    
RTP::RTP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort) 
	:UDP(srcPort, dstAddr, dstPort), fw(fopen("d:\\a.h264", "wb"), fclose)
{
	mEvPtr.reset(MyEvent::getInstance()->addTimer(2, createTimerFunc()));
};

void *RTP::createTimerFunc()
{
    mTimerFunc = [=](std::string data) {timerFunc(); };
    return &mTimerFunc;
}
void RTP::timerFunc()
{
	printf("im in time\n");
}

void RTP::processData(std::string data)
{
	//printf("packet:%d\n", sizeof(RTPPacket));
	//printf("data len:%d\n", data.length());
	RTPPacket rPack;
	memcpy((void *)&rPack, data.c_str(), RTP_HEADER_SIZE);
	rPack.payload = data.substr(RTP_HEADER_SIZE);
	rtpPackets.push(std::move(rPack));
	//fwrite(data.c_str() + sizeof(RTPPacket), data.length() - sizeof(RTPPacket), 1, fw.get());
    //printf("rtp data:%s\n", data.c_str());
}
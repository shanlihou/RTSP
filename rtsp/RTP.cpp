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
    static UINT32 packetSize = -1;
	printf("im in time ps:%d, packets:%d\n", packetSize, rtpPackets.size());
    if (packetSize != rtpPackets.size())
    {
        packetSize = rtpPackets.size();
    }
    else
    {
        RTPPacket tmp;
		UINT8 head[] = {0, 0, 0, 1};
		UINT32 seqNum = 0;
		FILE *fp = fopen("D:\\a.txt", "w");
        while (!rtpPackets.empty())
        {
            tmp = rtpPackets.top();
			if (seqNum + 1!= tmp.seqNum)
			{
				printf("%d, %d\n", seqNum, tmp.seqNum);
			}
			seqNum = tmp.seqNum;
			rtpPackets.pop();

			fwrite(head, sizeof(head), 1, fw.get());
            fwrite(tmp.payload.c_str(), tmp.payload.length(), 1, fw.get());
			fprintf(fp, "pt:%d, ssrc:%x, seq:%d, time:%u, pay len:%d\n", tmp.PT, tmp.ssrc, tmp.seqNum, tmp.timeStamp, tmp.payload.length());
			/*
			char buf[50];
			sprintf(buf, "D:\\RTPPacket\\%d.txt", tmp.seqNum);
			FILE *fPack = fopen(buf, "w");
			fwrite(tmp.data.c_str(), tmp.data.length(), 1, fPack);
			fclose(fPack);*/
        }
		fclose(fp);
        fw.reset();
        printf("write done\n");
		mEvPtr.reset();
    }
}
inline UINT16 swap(UINT16 num)
{
	return (num >> 8) | (num << 8);
}

void RTP::processData(std::string data)
{
	//printf("packet:%d\n", sizeof(RTPPacket));
	//printf("data len:%d\n", data.length());
	RTPPacket rPack;
	memcpy((void *)&rPack, data.c_str(), RTP_HEADER_SIZE);
	//rPack.data = data;
	rPack.payload = data.substr(RTP_HEADER_SIZE);
	rPack.seqNum = swap(rPack.seqNum);
	rtpPackets.push(std::move(rPack));
	//fwrite(data.c_str() + sizeof(RTPPacket), data.length() - sizeof(RTPPacket), 1, fw.get());
    //printf("rtp data:%s\n", data.c_str());
}
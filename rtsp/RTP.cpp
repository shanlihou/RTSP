#include "RTP.h"
#include "Decode.h"
#define NAL_TYPE_MASK 0x1f
#define F_NRI_MASK    0xe0
UINT8 g_HEAD_4[] = {0, 0, 0, 1};
UINT8 g_HEAD_3[] = {0, 0, 1};
UDP::UDP(UINT32 srcPort, const char *dstAddr, UINT32 dstPort):dstAddr(dstAddr), dstPort(dstPort)
{
    sock = MyEvent::getInstance()->bindUdp(srcPort, getCallBack());
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

void UDP::attach(Proc proc)
{
	mVProc.push_back(proc);
}

void UDP::notifyProc(std::string data)
{
	for (auto proc = mVProc.begin(); proc != mVProc.end(); proc++)
	{
		(*(proc))(data);
	}
}

void *UDP::getCallBack()
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
	//for test
	Decode *decode = Decode::getInstance();
	Decode::getInstance()->init();
	attach(std::bind(&Decode::onData, decode, std::placeholders::_1));
};

void *RTP::createTimerFunc()
{
    mTimerFunc = [=](std::string data) {timerFunc(); };
    return &mTimerFunc;
}
std::string RTP::parseNalU(std::string FU_A)
{
	std::string ret;
	auto start = FU_A.begin();
	UINT8 first;
	UINT8 nalType = (*start) & NAL_TYPE_MASK;
	if (nalType == 28)
	{
		if ((*(start + 1) & 0x80) == 0x80)
		{
			ret.assign((char *)g_HEAD_3, 3);
			first = ((*start) & F_NRI_MASK) | ((*(start + 1)) & NAL_TYPE_MASK);
			ret.push_back(first);
			ret.append(start + 2, FU_A.end());
		}
		else
		{
			ret.append(start + 2, FU_A.end());
		}
	}
	else if(nalType == 6)
	{
		//printf("get sei\n");
		ret.assign((char *)g_HEAD_3, 3);
		ret.append(start, FU_A.end());
	}
	else
	{
		ret.assign((char *)g_HEAD_4, 4);
		ret.append(start, FU_A.end());
	}
	return ret;
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
			std::string data = parseNalU(tmp.payload);
			fwrite(data.c_str(), data.length(), 1, fw.get());
			notifyProc(data);
			//fwrite(head, sizeof(head), 1, fw.get());
            //fwrite(tmp.payload.c_str(), tmp.payload.length(), 1, fw.get());
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
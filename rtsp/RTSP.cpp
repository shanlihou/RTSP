#ifndef WIN32
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#endif
#include <WinSock2.h>
#include <iostream>
#include <algorithm>
#include "RTSP.h"
const char *gRtspStep[] = {"OPTIONS", "DESCRIBE", "SETUP", "PLAY"};
Track::Track(Track &&t)
{
	name = std::move(t.name);
	trans = std::move(t.trans);
	pRtcp.reset(t.pRtcp.release());
	port = t.port;
}
void Track::createRTCP(const char *host)
{
	pRtcp.reset(new RTCP(port, host, dstPort));
}

#define ATTACH(x, y) key2FuncMap.insert(std::pair<std::string, std::function<void(std::string)>>(x, std::bind(&RTSP::y, this, std::placeholders::_1)));
RTSP::RTSP(const char *host, UINT32 port, const char *name, const char *pwd, const char *url)
    :host(host), port(port), name(name), pwd(pwd), trackIndex(0), session("")
{
	char buf[256];
	buf[sizeof(buf) - 1] = 0;
	_snprintf(buf, sizeof(buf) - 1, "rtsp://%s:%d/%s", host, port, url);
	this->url = buf;
	cseq = 0;
	mBev = NULL;
	step = 0;

	//key2FuncMap.insert(std::pair<std::string, DataFunc>("OPTIONS", std::bind(&RTSP::OPTIONS, this)));
	ATTACH("OPTIONS", OPTIONS)
	ATTACH("DESCRIBE", DESCRIBE)
	ATTACH("SETUP", SETUP)
	ATTACH("PLAY", PLAY)
};
RTSP::~RTSP()
{
	if (mBev)
	{
		bufferevent_free(mBev);
	}
}
void *RTSP::attachFunc()
{
	mFunc = [=](std::string data){processData(data);};
	return &mFunc;
}

int RTSP::getCode(std::string::const_iterator &iter)
{
	iter += strlen("RTSP/1.0 ");
	int code = 0;
	for (; *iter != ' '; iter++)
	{
		code = code * 10 + *iter - '0';
	}
	return code;
}

STR_LIST RTSP::split(const std::string &data, const char *sp)
{
	UINT32 start = 0;
	UINT32 end = 0;
	STR_LIST retList;
	while((end = data.find(sp, start)) != -1)
	{
		retList.push_back(data.substr(start, end - start));
		start = end + strlen(sp);
	}
	retList.push_back(data.substr(start));
	return retList;
}

std::string RTSP::getStr(const std::string &data, const char *start, const char *end)
{
	UINT32 s = data.find(start) + strlen(start);
	UINT32 e = data.find(end, s);
	return data.substr(s, e - s);
}

Track RTSP::getTrack(std::string data)
{
	UINT32 firstEnter = data.find("\r");
	STR_LIST firstList = split(data.substr(0, firstEnter), " ");
	printf("trans:%s\n", firstList[2].c_str());
	return Track(getStr(data, "a=control:", "\r").c_str(), firstList[2].c_str());
	//return ret;
}

void RTSP::processData(std::string data)
{
	printf("receive data:%s\n", data.c_str());
	std::string::const_iterator iter = data.begin();
	int code = getCode(iter);
	printf("code is :%d\n", code);
	if (code == 200)
	{
		step++;
		if (step < sizeof(gRtspStep) / sizeof(char *))
		{
			key2FuncMap[gRtspStep[step]](data);
		}
	}
}
	

void RTSP::connect()
{
	mBev = MyEvent::getInstance()->connect(host.c_str(), port, attachFunc());
}
std::string join(STR_LIST list, const char *add)
{
	std::string ret("");
	std::vector<std::string>::iterator itr;
	for (itr = list.begin(); itr != list.end(); itr++)
	{
		ret += *itr;
		ret += add;
	}
	ret += add;
	return ret;
}

void RTSP::post(const char *method, const char *urlExt, STR_LIST headers)
{
	STR_LIST postList;
	char buf[296];
	buf[sizeof(buf) - 1] = 0;
    if (*urlExt != 0)
	    _snprintf(buf, sizeof(buf) - 1, "%s %s/%s RTSP/1.0", method, url.c_str(), urlExt);
    else
        _snprintf(buf, sizeof(buf) - 1, "%s %s RTSP/1.0", method, url.c_str());
	postList.push_back(std::string(buf));
	_snprintf(buf, sizeof(buf) - 1, "CSeq: %d", cseq++);
	postList.push_back(std::string(buf));
	postList.push_back("User-Agent: LibVLC/2.2.4 (LIVE555 Streaming Media v2016.02.22)");
	if (!session.empty())
	{
		postList.push_back(session);
	}
	postList.insert(postList.end(), headers.begin(), headers.end());

	std::string sendMsg = join(postList, "\r\n");
	printf("send:%s\n", sendMsg.c_str());
	bufferevent_write(mBev, sendMsg.c_str(), sendMsg.length());
}

void RTSP::OPTIONS(std::string &data)
{
	STR_LIST nothing;
	post("OPTIONS", "", nothing);
}

void RTSP::DESCRIBE(std::string &data)
{
	STR_LIST headers;
	headers.push_back("Accept: application/sdp");
	post("DESCRIBE", "", headers);
}
	
void RTSP::SETUP(std::string &data)
{
	printf("im in setup\n");

	STR_LIST trackList = split(data, "m=");
	std::for_each(trackList.begin() + 1, trackList.end(), [=](std::string x){
		STR_LIST headers;
		char buf[296];
		buf[sizeof(buf) - 1] = 0;

		Track tmpTrack = getTrack(x);
		_snprintf(buf, sizeof(buf) - 1, "Transport: %s;unicast;client_port=%d-%d", tmpTrack.trans.c_str(), tmpTrack.port, tmpTrack.port + 1);
		headers.push_back(buf);
		post("SETUP", tmpTrack.name.c_str(), headers);
		trackInfo.push_back(std::move(tmpTrack));
	});
}

void RTSP::PLAY(std::string &data)
{
	std::string serverPort = getStr(data, "server_port=", "\r");
	UINT32 tmp;
	sscanf(serverPort.c_str(), "%d-%d", &trackInfo[trackIndex].dstPort, &tmp);
	session = std::string("Session:") + getStr(data, "Session:", ";");
	printf("port :%d, sess:%s\n", trackInfo[trackIndex].dstPort, session.c_str());
	trackInfo[trackIndex].createRTCP(host.c_str());
	
	//start play
	
	STR_LIST headers;
	headers.push_back("Range: npt=0.000-");
	post("PLAY", "", headers);
}
#pragma once
#include <stdio.h>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <memory>
#include "RTCP.h"

typedef std::function<void(std::string)> DataFunc;
typedef std::vector<std::string> STR_VEC;
class Track
{
public:
	Track(const char *name, const char *trans):name(name), trans(trans), port(9526){};
	Track(Track &&t);
	void createRTCP(const char *host);

	std::string name;
	std::string trans;
	UINT32 port;
	UINT32 dstPort;
    std::unique_ptr<RTCP> pRtcp;
};
class RTSP
{
public:
	RTSP(const char *host, UINT32 port, const char *name, const char *pwd, const char *url);
	void connect();
	void processData(std::string data);
	void *attachFunc();
	void post(const char *method, const char *urlExt, STR_VEC headers);
	int getCode(std::string::const_iterator &iter);
	Track getTrack(std::string data);
	std::string getStr(const std::string &data, const char *start, const char *end);
	void OPTIONS(std::string &data);
	void DESCRIBE(std::string &data);
	void SETUP(std::string &data);
	void PLAY(std::string &data);
	~RTSP();
private:
	STR_VEC split(const std::string &data, const char *sp);

	UINT32 cseq;
	UINT32 step;
	UINT32 trackIndex;
	std::string host;
	std::string session;
	unsigned int port;
	std::string name;
	std::string pwd;
	std::string url;
	bufferevent *mBev;
	CallBack mFunc;
	std::vector<Track> trackInfo;
	std::map<std::string, std::function<void(std::string &)>> key2FuncMap;
};

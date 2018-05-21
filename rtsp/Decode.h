#pragma once
#include <string>
#include <list>
#include <Windows.h>
#include "util.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
struct MyPicture
{
	MyPicture();
	MyPicture(MyPicture &&pict);
	MyPicture &operator =(MyPicture &pict);
	UINT32 width;
	UINT32 height;
	std::shared_ptr<UINT8> data;
};

typedef std::list<std::string> STR_LIST;
class Decode
{
	PATTERN_SINGLETON_DECLARE(Decode)
public:
	~Decode();
	void init();
	int parse(const UINT8 *in_buffer, UINT32 size);
	void onData(std::string &);
	int pop(MyPicture &pict);
private:
	Decode();
	void dealAllQueue();

	AVCodecContext *pCodecCtx; 
	AVCodec *pCodec;
	AVCodecID codec_id; 
	AVCodecParserContext *pCodecParserCtx; 
	AVPacket packet;
	AVFrame *pFrame;             //yuv  
	AVPicture picture;                  //¥¢¥Êrgb∏Ò ΩÕº∆¨  
	SwsContext *pSwsCtx;  
	bool is_first_time;
	STR_LIST mOriList;
	std::list<MyPicture> mPictList;
	HANDLE mMutex;
};
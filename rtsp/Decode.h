#pragma once
#include <string>
#include "util.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

class Decode
{
	PATTERN_SINGLETON_DECLARE(Decode)
public:
	~Decode();
	void init();
	int parse(const UINT8 *in_buffer, UINT32 size);
	void onData(std::string);
private:
	Decode();
	AVCodecContext *pCodecCtx; 
	AVCodec *pCodec;
	AVCodecID codec_id; 
	AVCodecParserContext *pCodecParserCtx; 
	AVPacket packet;
	AVFrame *pFrame;             //yuv  
	AVPicture picture;                  //¥¢¥Êrgb∏Ò ΩÕº∆¨  
	SwsContext *pSwsCtx;  
	bool is_first_time;
};
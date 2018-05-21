#include <stdio.h>
#include <memory>
#include <Windows.h>
#include "Decode.h"
#include "Display.h"
#define FF_INPUT_BUFFER_PADDING_SIZE 64
#define MIN_PICT_LENGTH 50

extern void testPict(int argc,char *argv[], int width, int height, void *bitmap);
PATTERN_SINGLETON_IMPLEMENT(Decode)
Decode::Decode():pCodecCtx(nullptr), pCodec(nullptr), codec_id(AV_CODEC_ID_H264), pCodecParserCtx(nullptr), pFrame(nullptr), pSwsCtx(nullptr), is_first_time(true)
{
	mMutex = CreateMutex(NULL, FALSE, NULL);
}
Decode::~Decode()
{
	av_free_packet(&packet);  
	av_frame_free(&pFrame);  
	avpicture_free(&picture);  
	sws_freeContext(pSwsCtx);  
	avcodec_free_context(&pCodecCtx);  
	av_parser_close(pCodecParserCtx); 
}
void Decode::init()
{
	av_register_all();  
	avcodec_register_all();
	
	/* 初始化AVCodec */  
	pCodec = avcodec_find_decoder(codec_id);  

	/* 初始化AVCodecContext,只是分配，还没打开 */  
	pCodecCtx = avcodec_alloc_context3(pCodec);  

	/* 初始化AVCodecParserContext */  
	pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264);  
	if (!pCodecParserCtx)  
	{  
		printf( "AVCodecParseContext error");  
		::exit(0);  
	}  

	/* we do not send complete frames,什么意思？ */  
	if (pCodec->capabilities & AV_CODEC_CAP_TRUNCATED)  
		pCodecCtx->flags |= AV_CODEC_FLAG_TRUNCATED;  

	/* 打开解码器 */  
	int ret = avcodec_open2(pCodecCtx, pCodec, NULL);  
	if (ret < 0)  
	{  
		printf("avocodec_open2 error");  
		::exit(0);  
	}  


	pFrame = av_frame_alloc();  
	av_init_packet(&packet);  
	packet.size = 0;  
	packet.data = nullptr;  
}


int Decode::parse(const UINT8 *in_buffer, UINT32 size)
{
	const UINT8 *cur_ptr = in_buffer; 
	//printf("cur_size:%d\n", size);
	int ret;
	int got;
	int retValue = 0;
	while (size > 0)  
	{ 
		/* 返回解析了的字节数 */  
		int len = av_parser_parse2(pCodecParserCtx, pCodecCtx,  
			&packet.data, &packet.size, cur_ptr, size,  
			AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);  
		cur_ptr += len;  
		size -= len;  
		if (packet.size == 0)  
			continue;  

		//switch (pCodecParserCtx->pict_type)  
		//{  
		//  case AV_PICTURE_TYPE_I: printf("Type: I\t"); break;  
		//  case AV_PICTURE_TYPE_P: printf("Type: P\t"); break;  
		//  case AV_PICTURE_TYPE_B: printf("Type: B\t"); break;  
		//  default: printf("Type: Other\t"); break;  
		//}  
		//printf("Output Number:%4d\t", pCodecParserCtx->output_picture_number);  
		//printf("Offset:%8ld\n", pCodecParserCtx->cur_offset);  
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got, &packet);  
		if (ret < 0)  
		{  
			printf("decodec error");  
			return -1; 
		}  

		if (got)  
		{  
			if (is_first_time)  //分配格式转换存储空间  
			{  

				pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,  
					pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);  

				avpicture_alloc(&picture, AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height);  

				is_first_time = false;  
			}  
			/* YUV转RGB */  
			sws_scale(pSwsCtx, pFrame->data, pFrame->linesize,   
				0, pCodecCtx->height,  
				picture.data, picture.linesize);
			MyPicture pict;
			UINT8 *imgPtr = new UINT8[pCodecCtx->width * pCodecCtx->height * 3];
			const UINT8 *ptr = picture.data[0];
			for (int i = 0; i < pCodecCtx->width * pCodecCtx->height; i++)
			{
				imgPtr[i * 3] = ptr[i * 4];
				imgPtr[i * 3 + 1] = ptr[i * 4 + 1];
				imgPtr[i * 3 + 2] = ptr[i * 4 + 2];
			}
			pict.data = std::shared_ptr<UINT8> (imgPtr, std::default_delete<UINT8[]>());
			//pict.data.reset(imgPtr);
			pict.width = pCodecCtx->width;
			pict.height = pCodecCtx->height; 
			//Display::getInstance()->pushBackPict(pict);
			mPictList.push_back(std::move(pict));

			/*std::unique_ptr<FILE, decltype(fclose)*> fp(fopen("D:\\picture.txt", "wb"), fclose);
			fwrite(imgPtr, pict.width * pict.height * 3, 1, fp.get());*/
			//return 1;
			//QImage img(picture.data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);  

			//emit this->signal_receive_one_image(img); 
			//testPict(0, NULL, pCodecCtx->width, pCodecCtx->height, pict);
			//getchar();
		}
	}  
	return retValue;
}

void Decode::onData(std::string &data)
{
	//parse((UINT8 *)data.c_str(), data.length());
	WaitForSingleObject(mMutex, INFINITE);
	mOriList.push_back(std::move(data));
	dealAllQueue();
	ReleaseMutex(mMutex);
}

void Decode::dealAllQueue()
{
	while(mPictList.size() < MIN_PICT_LENGTH && (!mOriList.empty()))
	{
		std::string &first = mOriList.front();
		parse((UINT8 *)first.c_str(), first.length());
		mOriList.pop_front();
	}
}
int Decode::pop(MyPicture &pict)
{
	WaitForSingleObject(mMutex, INFINITE);
	if (mPictList.empty())
	{
		ReleaseMutex(mMutex);
		return -1;
	}
	//pict = mPictList.front();
	pict = std::move(mPictList.front());
	mPictList.pop_front();
	dealAllQueue();
	ReleaseMutex(mMutex);
	return 0;
}

#define BUFF_SIZE 4096
int testffmpeg()
{
	//test();
	Display::getInstance()->init();
	Decode::getInstance()->init();
	std::unique_ptr<FILE, decltype(fclose)*> fp(fopen("test.264", "rb"), fclose);
	UINT8 in_buffer[BUFF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
	while(1)
	{
		int size = fread(in_buffer, BUFF_SIZE, 1, fp.get());
		if (size != 1)
			break;
		std::string tmpData((char *)in_buffer, size * BUFF_SIZE);
		Decode::getInstance()->onData(tmpData);
		//int ret = Decode::getInstance()->parse(in_buffer, size * BUFF_SIZE);
		/*if (ret == 1)
			break;*/
	}
	getchar();
	return 0;
}



MyPicture::MyPicture()
{
}
MyPicture::MyPicture(MyPicture &&pict)
{
	this->width = pict.width;
	this->height = pict.height;
	this->data = pict.data;
}

MyPicture &MyPicture::operator =(MyPicture &pict)
{
	this->width = pict.width;
	this->height = pict.height;
	this->data = std::move(pict.data);
	return *this;
}
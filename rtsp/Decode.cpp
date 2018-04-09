#include <stdio.h>
#include <memory>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}
#define FF_INPUT_BUFFER_PADDING_SIZE 4099
extern void testPict(int argc,char *argv[], int width, int height, void *bitmap);
void test()
{
	AVCodecContext *pCodecCtx = NULL;  
	AVCodecParserContext *pCodecParserCtx = NULL;  
	AVCodec *pCodec = NULL;  
	AVFrame *pFrame = NULL;             //yuv  
	AVPacket packet;                    //h264  
	AVPicture picture;                  //储存rgb格式图片  
	SwsContext *pSwsCtx = NULL;  
	AVCodecID codec_id = AV_CODEC_ID_H264;  

	int ret;  

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
	ret = avcodec_open2(pCodecCtx, pCodec, NULL);  
	if (ret < 0)  
	{  
		printf("avocodec_open2 error");  
		::exit(0);  
	}  


	pFrame = av_frame_alloc();  
	av_init_packet(&packet);  
	packet.size = 0;  
	packet.data = NULL;  

	const int in_buffer_size = 4096;  
	uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = { 0 };  
	uint8_t *cur_ptr;  
	int cur_size;  
	int got;  
	bool is_first_time = true;  
	int exitFlag = 1;
	while (exitFlag)  
	{  
		std::unique_ptr<FILE, decltype(fclose)*> fp(fopen("test.264", "rb"), fclose);
		cur_size = fread(in_buffer, in_buffer_size, 2, fp.get());  
		if (cur_size != 2)  
			break;  
		cur_size *= in_buffer_size;

		cur_ptr = in_buffer; 
		printf("cur_size:%d\n", cur_size);
		while (cur_size > 0)  
		{  
			/* 返回解析了的字节数 */  
			int len = av_parser_parse2(pCodecParserCtx, pCodecCtx,  
				&packet.data, &packet.size, cur_ptr, cur_size,  
				AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);  
			cur_ptr += len;  
			cur_size -= len;  
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
				::exit(0);  
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
				//std::unique_ptr<FILE, decltype(fclose)*> fp(fopen("D:\\picture.txt", "wb"), fclose);
				//fwrite(picture.data[0], );
				//QImage img(picture.data[0], pCodecCtx->width, pCodecCtx->height, QImage::Format_RGB32);  

				//emit this->signal_receive_one_image(img);  
				unsigned char *pict = new unsigned char[pCodecCtx->width * pCodecCtx->height * 3];
				unsigned char *ptr = picture.data[0];
				for (int i = 0; i < pCodecCtx->width * pCodecCtx->height; i++)
				{
					pict[i * 3] = ptr[i * 4];
					pict[i * 3 + 1] = ptr[i * 4 + 1];
					pict[i * 3 + 2] = ptr[i * 4 + 2];
				}
				testPict(0, NULL, pCodecCtx->width, pCodecCtx->height, pict);
				delete []pict;
				exitFlag = 0;
			}  
		}  
	}  

	av_free_packet(&packet);  
	av_frame_free(&pFrame);  
	avpicture_free(&picture);  
	sws_freeContext(pSwsCtx);  
	avcodec_free_context(&pCodecCtx);  
	av_parser_close(pCodecParserCtx); 
}

int testffmpeg()
{
	test();
	getchar();
	return 0;
}
#pragma once
#include <Windows.h>
#include <memory>
#include <list>
#include "util.h"
#include "Decode.h"
class Display
{
	PATTERN_SINGLETON_DECLARE(Display)
public:
	void pushBackPict(MyPicture &pict);
	void onTimer();
	~Display();
	void init();
	void showTexture();

private:
	UINT32 loadTexture(UINT8 *pixels, int width, int height);

	std::list<MyPicture> mPictList;
	HANDLE mMutex;
	HANDLE mThread;
	//void timerFunc(int val);
	UINT32 mTexID;
	Display();
};
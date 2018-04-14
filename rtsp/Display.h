#pragma once
#include <Windows.h>
#include <memory>
#include <list>
#include "util.h"
struct MyPicture
{
	MyPicture();
	MyPicture(MyPicture &&pict);
	UINT32 width;
	UINT32 height;
	std::unique_ptr<UINT8[]> data;
};
class Display
{
	PATTERN_SINGLETON_DECLARE(Display)
public:
	void pushBackPict(MyPicture &pict);
	void onTimer();
	~Display();
	void init();

private:
	std::list<MyPicture> mPictList;
	HANDLE mMutex;
	HANDLE mThread;
	//void timerFunc(int val);
	Display();
};
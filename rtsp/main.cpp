#include <stdio.h>
#include "MyEvent.h"
#include "RTSP.h"
#include "Display.h"
extern int testffmpeg();
extern void testDisplay();
//extern int testOpenGL(int argc,char *argv[]);
//extern void testPict(int argc,char* argv[]);
extern int testWENLI(int argc, char* argv[]);
int main()
{
	
	Display::getInstance()->init();
	MyEvent::getInstance();
	RTSP rtsp("127.0.0.1", 554, "", "", "test.264");
	rtsp.connect();
	std::string data;
	rtsp.OPTIONS(data);
	MyEvent::getInstance()->loop();
	/*
	while(1)
	{
		Sleep(1000);
		printf("tick:%u\n", GetTickCount());
	}*/

	//testffmpeg();
	//testDisplay();
	//testOpenGL(0, NULL);
	//testPict(0, NULL);
	//testWENLI(0, NULL);
	return 0;
}
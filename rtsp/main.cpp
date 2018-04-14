#include <stdio.h>
#include "MyEvent.h"
#include "RTSP.h"
extern int testffmpeg();
extern void testDisplay();
//extern int testOpenGL(int argc,char *argv[]);
//extern void testPict(int argc,char* argv[]);
int main()
{
	/*
	MyEvent::getInstance();
	RTSP rtsp("127.0.0.1", 554, "", "", "test.264");
	rtsp.connect();
	std::string data;
	rtsp.OPTIONS(data);
	MyEvent::getInstance()->loop();
	*/
	testffmpeg();
	//testDisplay();
	//testOpenGL(0, NULL);
	//testPict(0, NULL);
	return 0;
}
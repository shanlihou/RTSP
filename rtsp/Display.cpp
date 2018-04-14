#include <functional>
#include <process.h>
#include "Display.h"
#define GLUT_DISABLE_ATEXIT_HACK  
#include "glew/glew.h"  
#include "freeglut\freeglut.h"  


PATTERN_SINGLETON_IMPLEMENT(Display)
void OnReshape(int w,int h)  
{  
    GLfloat aspect =(GLfloat)w/(GLfloat)h;  
    GLfloat nRange=100.0f;  
  
    glViewport(0,0,w,h);  
  
    glMatrixMode(GL_PROJECTION); //将当前矩阵指定为投影模式  
    glLoadIdentity();  
  
    //设置三维投影区  
  
    if (w<=h)  
        glOrtho(-nRange,nRange,-nRange*aspect,nRange*aspect,-nRange,nRange);  
    else  
        glOrtho(-nRange,nRange,-nRange/aspect,nRange/aspect,-nRange,nRange);  
}  
void Display::pushBackPict(MyPicture &pict)
{
	if (mPictList.size() > 200)
	{
		return;
	}
	WaitForSingleObject(mMutex, INFINITE);
	mPictList.push_back(std::move(pict));
	ReleaseMutex(mMutex);
}

void Display::onTimer()
{
	WaitForSingleObject(mMutex, INFINITE);
	if (mPictList.empty())
	{
		return;
	}	
    //glClear(GL_COLOR_BUFFER_BIT);  
  
    glRasterPos2i(-50, -50); //指定当前光栅位置  
	MyPicture &pict = mPictList.front();
	glDrawPixels(pict.width, pict.height, GL_BGR_EXT, GL_UNSIGNED_BYTE, pict.data.get());

	mPictList.pop_front();
	printf("leng:%d %d\n", mPictList.size(), pict.width * pict.height * 3);
	ReleaseMutex(mMutex);
  
    glutSwapBuffers();  
	//glutPostRedisplay();
}

void timerFunc(int val)
{
	Display::getInstance()->onTimer();
	glutTimerFunc(30, timerFunc,1);
}

Display::Display()
{
	mMutex = CreateMutex(NULL, FALSE, NULL); 
}

void OnDisPlay(void)  
{  
    glClear(GL_COLOR_BUFFER_BIT);  
  
    glColor3f(1,0,0);  
  
    //glRasterPos2i(-3,-3); //指定当前光栅位置  
    //glBitmap(g_w, g_h, 0, 0, 0, 0, (GLubyte *)ptr);  
    glPointSize(5);  
    glBegin(GL_POINTS);  
    glColor3f(0,1,0);  
    glVertex2i(0,0);  
    glEnd();  
  
    glutSwapBuffers();  
}  

void glutMainFunc(void *arg)
{
	int argc = 0;
    glutInit(&argc, NULL);  
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  
    glutInitWindowSize(600,480);  
    glutCreateWindow("EXAM601");  
  

    glutReshapeFunc(OnReshape);  
    glutDisplayFunc(OnDisPlay);  
	glutTimerFunc(50, timerFunc,1);
    glClearColor(1,1,1,1);  
    glutMainLoop(); 
}

void Display::init()
{
    //Initialization();  
	
    //glutMainLoop();
	_beginthread(glutMainFunc, 0, NULL);
}

Display::~Display()
{
	printf("~Display\n");
	CloseHandle(mMutex);
}

MyPicture::MyPicture()
{
}
MyPicture::MyPicture(MyPicture &&pict)
{
	this->width = pict.width;
	this->height = pict.height;
	this->data.reset(pict.data.release());
}

void Initialization(void)  
{  
    glClearColor(1,1,1,1);  
}  
void testDisplay()
{
	Display::getInstance()->init();
	getchar();
	/*int argc = 0;
    glutInit(&argc,NULL);  
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  
    glutInitWindowSize(600,480);  
    glutCreateWindow("EXAM601");  
  
	
	glutTimerFunc(50, timerFunc,1);
    glutReshapeFunc(OnReshape);  
    glutDisplayFunc(OnDisPlay);  
  
    glutMainLoop();  */
}
#include "glew\glew.h"  
#include "freeglut\freeglut.h"  
#include <stdio.h>

void init(void)  
{  
    glClearColor(1.0, 1.0, 1.0, 0.0);  //窗口颜色(红:[0.0~1.0]R, 绿:[0.0~1.0]G，蓝:[0.0~1.0]B, 透明度:[0.0~1.0]A)
    /*使用正投影将世界坐标系二位矩形区域的内容映射到屏幕上，区域的x坐标值从0.0到200.0，y坐标值从0.0到150.0*/
    glMatrixMode(GL_PROGRAM);  
    gluOrtho2D(0.0, 200.0, 0.0, 150.0);  
}  
static int g_w = -1;
static int g_h = -1;
static void *ptr = NULL;
void lineSegment(void)
{
    glClear(GL_COLOR_BUFFER_BIT);//GL_COLOR_BUFFER_BIT是一个OpenGL符号常量，用来指定它的颜色缓存（刷新缓存）中的位值，该缓存将使用                                      glClearColor函数中指定的值来设定。
    glColor3f(1.0f, 0.0f, 0.0f);//设置线段颜色为红色
	glRasterPos2i(-3, -3);
	glBitmap(g_w, g_h, 0, 0, 0, 0, (GLubyte *)ptr);
    /*定义了一个从整数笛卡尔端点坐标（180，15）到(10, 145)的二维直线段*/
    //glBegin(GL_LINES);
    //glVertex2i(180, 15);
   // glVertex2i(10, 145);
    //glEnd();
	//glutSwapBuffers();
    glFlush(); //执行所有的OpenGL程序
} 
int testOpenGL(int argc,char *argv[], int width, int height, void *bitmap)
{
    glutInit(&argc, argv);//初始化GULT
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);//设置显示模式
    glutInitWindowPosition(50, 100);//设置左上角窗口显示位置
    glutInitWindowSize(500, 500);//设置窗口显示的宽与高
    glutCreateWindow("An Example OpenGL Program！");//创建一个窗口
    init();//执行初始化程序
	
	
	//glBitmap(width, height, 0, 0, 0, 0, (GLubyte *)bitmap);
	g_w = width;
	g_h = height;
	ptr = bitmap;
    glutDisplayFunc(lineSegment);//把图形显示在窗口
    glutMainLoop();//显示所有并进入等待状态
	return 0;
}
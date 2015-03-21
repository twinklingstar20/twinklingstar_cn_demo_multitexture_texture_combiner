/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/12/02
****************************************************************************/
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glew32.lib")

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <gl\glew.h>
#include <GL\glut.h>
#include <gl\glu.h>			// Header File For The GLu32 Library
#include "SrImageBmp.h"




void drawScene()
{
	glBegin(GL_QUADS);
		//分别指定三个纹理的纹理坐标
		glMultiTexCoord2f(GL_TEXTURE0,0.0f, 0.0f);glMultiTexCoord2f(GL_TEXTURE1,0.0f, 0.0f);glMultiTexCoord2f(GL_TEXTURE2,0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		glMultiTexCoord2f(GL_TEXTURE0,1.0f, 0.0f);glMultiTexCoord2f(GL_TEXTURE1,1.0f, 0.0f);glMultiTexCoord2f(GL_TEXTURE2,2.0f, 0.0f);
		glVertex3f( 1.0f, -1.0f,  1.0f);
		glMultiTexCoord2f(GL_TEXTURE0,1.0f, 1.0f);glMultiTexCoord2f(GL_TEXTURE1,1.0f, 1.0f);glMultiTexCoord2f(GL_TEXTURE2,2.0f, 2.0f);
		glVertex3f( 1.0f,  1.0f,  1.0f);
		glMultiTexCoord2f(GL_TEXTURE0,0.0f, 1.0f);glMultiTexCoord2f(GL_TEXTURE1,0.0f, 1.0f);glMultiTexCoord2f(GL_TEXTURE2,0.0f, 2.0f);
		glVertex3f(-1.0f,  1.0f,  1.0f);
	glEnd();
}




void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-5.0f);

	drawScene();

	glFlush();

}

void myKeys(unsigned char key, int x, int y) 
{

}

unsigned int genTextureFile(const char* fileName )
{
	unsigned char* data;
	unsigned int textureId = 0;


	int pixelCount,rgbType;
	SrImageBmp bmp(IMAGE_READ_ONLY);

	if( !bmp.readFile(fileName,data,pixelCount,rgbType) )
	{
		return 0;
	}
	int internalFormat,format;

	if( rgbType == IMAGE_RGB)
	{
		internalFormat = 3;
		format = GL_RGB;
	}
	else if( rgbType == IMAGE_RGBA )
	{
		internalFormat = 4;
		format = GL_RGBA;		
	}

	//生成纹理名字
	glGenTextures(1, &textureId);
	//绑定纹理
	glBindTexture(GL_TEXTURE_2D, textureId);
	//存储纹理数据到硬件内存中
	gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, bmp.getWidth(), bmp.getHeight(), format, GL_UNSIGNED_BYTE, data);
	//设置纹理放大、缩小时的过滤方法
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	return textureId;
}

unsigned int genTexture(unsigned char red,unsigned char green,unsigned char blue)
{
	unsigned char data[16];
	int i;
	for( i=0 ; i<4 ; i++ )
	{
		data[i*3]		= red;
		data[i*3 + 1]	= green;
		data[i*3 + 2]	= blue;
	}
	unsigned int textureId = 0;
	//生成纹理名字
	glGenTextures(1, &textureId);
	//绑定纹理
	glBindTexture(GL_TEXTURE_2D, textureId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//存储纹理数据到硬件内存中
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	//设置纹理放大、缩小时的过滤方法
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	return textureId;

}

void initTexture()
{

	unsigned int textureId[3];
	//创建3个纹理
	textureId[0] = genTexture(128,0,0);
	textureId[1] = genTexture(0,128,0);
	textureId[2] = genTextureFile("text.bmp");

	//激活第1个纹理
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textureId[0]);
	//用纹理textureId[0]替换环境纹理
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textureId[1]);
	//把纹理textureId[1]与前一阶段得到的纹理相加
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_ADD);

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textureId[2]);
	//把纹理textureId[2]与前一阶段得到的纹理相加
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_ADD);
}

void init()
{
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	initTexture();
}

void myReshape(GLsizei w,GLsizei h)
{
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(float)w/(float)h,1.0f,1000.0f);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc,char ** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(400,400);
	glutCreateWindow("multitexture-demo");

	glewInit();
	init();
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeys);

	glutMainLoop();
	return(0);
}
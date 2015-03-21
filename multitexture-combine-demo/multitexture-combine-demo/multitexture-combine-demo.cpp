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
		//�ֱ�ָ�������������������
		glMultiTexCoord2f(GL_TEXTURE0,0.0f, 0.0f);glMultiTexCoord2f(GL_TEXTURE1,0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		glMultiTexCoord2f(GL_TEXTURE0,1.0f, 0.0f);glMultiTexCoord2f(GL_TEXTURE1,1.0f, 0.0f);
		glVertex3f( 1.0f, -1.0f,  1.0f);
		glMultiTexCoord2f(GL_TEXTURE0,1.0f, 1.0f);glMultiTexCoord2f(GL_TEXTURE1,1.0f, 1.0f);
		glVertex3f( 1.0f,  1.0f,  1.0f);
		glMultiTexCoord2f(GL_TEXTURE0,0.0f, 1.0f);glMultiTexCoord2f(GL_TEXTURE1,0.0f, 1.0f);
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

	//������������
	glGenTextures(1, &textureId);
	//������
	glBindTexture(GL_TEXTURE_2D, textureId);
	//�洢�������ݵ�Ӳ���ڴ���
	gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, bmp.getWidth(), bmp.getHeight(), format, GL_UNSIGNED_BYTE, data);
	//��������Ŵ���Сʱ�Ĺ��˷���
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
	//������������
	glGenTextures(1, &textureId);
	//������
	glBindTexture(GL_TEXTURE_2D, textureId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//�洢�������ݵ�Ӳ���ڴ���
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	//��������Ŵ���Сʱ�Ĺ��˷���
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	return textureId;

}

void initTexture()
{
	unsigned int textureId[2];
	//����2������
	//����һ��2*2����ɫΪ(0,0,255)������ͼƬ
	textureId[0] = genTexture(0,0,255);
	//��ȡһ��BMPͼƬ����������
	textureId[1] = genTextureFile("text.bmp");

	//�����1������
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textureId[0]);
	//������textureId[0]�滻��������
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textureId[1]);
	//��������������㷨���ǡ�������,
	//textureId[1]������GL_SRC0_RGB��ǰһ���׶μ����������GL_SRC1_RGB��
	//�������㷨��GL_SRC0_RGB-GL_SRC1_RGB�������м��ǰ�ɫ��(255,255,255)����ȥ(0,0,255)������ʾ����ɫ��
	//������Χ�Ǻ�ɫ�ģ���ȥ(0,0,255)���Ǻ�ɫ�ġ�
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV,GL_COMBINE_RGB,GL_SUBTRACT);
	glTexEnvf(GL_TEXTURE_ENV,GL_SRC0_RGB,GL_TEXTURE);
	glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND0_RGB,GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV,GL_SRC1_RGB,GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV,GL_OPERAND1_RGB,GL_SRC_COLOR);
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
	glutCreateWindow("multitexture-combine-demo");

	glewInit();
	init();
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeys);

	glutMainLoop();
	return(0);
}
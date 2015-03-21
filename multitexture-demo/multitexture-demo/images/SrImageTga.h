/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#ifndef SR_IMAGES_IMAGE_TGA_H_
#define SR_IMAGES_IMAGE_TGA_H_

/** \addtogroup images
  @{
*/
#include "SrImage.h"
#include <stdio.h>

/*
\brief TGA�ļ��Ķ�д������

һ��TGA�����ֻ�ܽ���TGA�ļ��Ķ�ȡ����д��

��TGA�ļ�������TGA�ļ�������������������
		   ��1��Ҫ��ͷ�ļ��еĿ�͸߱�����������
		   ��2��λ��ȿ�����8��15��16��24��32��
		   ��3�����λ�����15��16��24��32����ʹ����ɫ��
		   ��4�����λ�����8�������������ɫ����ֻ�ܽ����ڰ�ͼ����ͼ��������TGA_UN_BLACK_WHITE����TGA_RLE_BLACK_WHITE��
		   ��5�����ʹ�õ���ɫ��ÿ��������ռ�Ĵ�С������15��16��24��32������һ��ֵ
		   ��6��tgaXOrigin��tgaYOrigin������0
		    (7��tgaImageDesc������0

дTGA�ļ���ֻʵ���˱���24bit�����ѹ����TGA��ʽ
*/

class SrImageTga:public SrImage
{
protected:
//������7���ļ����ͣ�TGA_IMAGE_NO_DATA�ļ����Ͳ�����
#define		TGA_IMAGE_NO_DATA		0x00
#define		TGA_UN_COLOR_MAP		0x01
#define		TGA_UN_TRUE_COLOR		0x02
#define		TGA_UN_BLACK_WHITE		0x03
#define		TGA_RLE_COLOR_MAP		0x09
#define		TGA_RLE_TRUE_COLOR		0x0A
#define		TGA_RLE_BLACK_WHITE		0x0B

#define  SR_IMAGE_TGA_CHECK_ORIGIN		0
#define  SR_IMAGE_TGA_CHECK_IMAGEDESC	0

#pragma pack(push)
#pragma pack(1)
	//TGA�ļ�ͷ
	typedef struct 
	{
		BYTE tgaIdLength;			//ͼ����Ϣ�ֶγ���
		BYTE tgaColorMapType;		//��ɫ������
		BYTE tgaImageType;			//ͼ������
		WORD tgaFirstEnIndex;		//��ɫ���׵�ַ
		WORD tgaColorMapLen;		//��ɫ����
		BYTE tgaColorMapEnSize;		//��ɫ������С
		WORD tgaXOrigin;			//ͼ��Xλ�õ���ʼλ��
		WORD tgaYOrigin;			//ͼ��Yλ�õ���ʼλ��
		WORD tgaWidth;				//ͼ����
		WORD tgaHeight;				//ͼ��߶�
		BYTE tgaPixelDepth;			//�������
		BYTE tgaImageDesc;			//ͼ��������
	}TGAFILEHEADER;

	//TGA�ļ�ע��
	typedef struct  
	{
		LONG tgaExtOffset;			//��չ����ƫ����
		LONG tgaDevOffset;			//����������ƫ����
		BYTE tgaSigniture[16];		//ǩ��
		BYTE tgaAscii;				//ASCII��
		BYTE tgaTerminator;			//��������0x00
	}TGAFILEFOOTER;

	//ͼ����ɫ������
	typedef struct  
	{
		BYTE*	tgaImageId;			//ͼ����Ϣ�ֶ�
		BYTE*	tgaColorMap;		//��ɫ������
		BYTE*	tgaImageData;		//ͼ������
	}TGAIMAGECOLORDATA;

	//�洢��TGA�ļ�ͷ��������ݣ���Ϊ�ļ��ṹ����
	typedef struct  
	{
		LONG				tgaFileSize;		//�ļ���С
		TGAIMAGECOLORDATA	tgaImageColorData;	//ͼ����ɫ������
		BYTE*				tgaPtrDevData;		//��������������
		BYTE*				tgaPtrExtData;		//��չ��������
	}TGASTRUCTUREDATA;
#pragma pack(pop)

public:
	SrImageTga(int isReadOnly);
	~SrImageTga();
	/*
	\brief	��TGA�ļ��ж�ȡ���ݣ����ҷ�������
	\param[in] chPtrFileName ��ȡ��TGA�ļ���
	\param[out] btPtrImageData ���inRGBType����IMAGE_RGB,�򷵻ص�RGB��ɫ���ݣ�ÿ����ɫռ�����ֽڣ�����Ϊ�졢�̡�����ÿ����ɫ������1���ֽڱ��棻
							   ���inRGBType����IMAGE_RGBA,�򷵻ص�RGBA��ɫ����,ÿ����ɫռ�ĸ��ֽڣ�����Ϊ�졢�̡�����Alpha��ÿ����ɫ������1���ֽڱ���;
							   ��������������ptrOutDatak�е��ڴ棬����BMP������й���
	\param[out] lgPixelCount ���ص�RGB��ɫ��
	\param[out] inRGBType	ֻ����IMAGE_RGBA��IMAGE_RGB�е�һ��ֵ����ʾ���ص����ݸ�ʽ��RGBA����RGB
	\return true�����ļ��ɹ���false�����ļ�ʧ�ܣ�ͨ��getErrorId()��������ô�����š�
	*/
	bool readFile(const char* chPtrFileName,BYTE*& btPtrImageData,int& lgPixelCount,int& inRGBType);
	/*
	\brief	��RGB��ɫ���ݱ����TGA�����У��Ա����д�ļ�������
	\param[in] ucPtrRgbData RGB��ɫ���ݣ�Ҫ�����TGA�����е����ݣ�����Ϊ�졢�̡�����ÿ����ɫ������1���ֽڱ���
	\param[in] inWidth ָ��д��TGA����Ŀ��������0
	\param[in] inHeight ָ��д��TGA����ĸߣ��������0
	\return true��װ���ļ��ɹ���false��װ���ļ�ʧ�ܣ�ͨ��getErrorId()��������ô�����š�
	*/
	bool loadImageData(unsigned char* ucPtrRgbData ,unsigned short inWidth,unsigned short inHeight);
	/*
	\brief ���洢��TGA�ļ������е����ݱ��浽TGA�ļ��У�ֻ����24bit�����ѹ����TGA��ʽ
	\param[in] chPtrImageFile д����ļ���
	*/
	bool writeFile(const char* chPtrImageFile)const;

	virtual bool	isValid()const;
	virtual int		getWidth()const;
	virtual int		getHeight()const;
	virtual unsigned char* getImageData()const;
	/*
	\brief	�ж�TGA�����д洢����RGB���ݻ���RGBA����
	*/
	bool			getIsRGB()const;
	/*
	\brief	�ж�TGA�����ǲ���ԭʼ��TGA�ļ���ʽ�������µ�TGA�ļ���ʽ
	*/
	bool			isNewTgaFormat()const;
	/*
	\brief	ÿ�����صı�����
	*/
	unsigned char	getPixelDepth()const;
	/*
	\brief	TGA�ļ����ͣ��ܹ�����6���ļ�����
	*/
	unsigned char	getImageType()const;
	/*
	\brief	�ж�TGA�Ƿ�ʹ������ɫ��
	*/
	bool			getUseMapType()const;
	/*
	\brief	��ɫ������λ����
	\return 0,��ʾ��ʹ����ɫ��������15,16,24,32�е�ĳ��ֵ
	*/
	int				getGetMapEntrySize()const;
	/*
	\brief	����ͼ����Ϣ�ֶ�
	\param[out] ptrImageInfo ����ͼ����Ϣ�ֶε����ݣ�ע�⣺�ÿ��ڴ治��TGA����ʹ�������Ҫ�ͷţ����������ڴ�й©
	\return ͼ����Ϣ�ֶεĳ���
	*/
	int				getImageInfo(unsigned char*& ptrImageInfo)const;

	

private:
	bool mallocMemory();
	void deallocMemory();

	bool readFileFooter(FILE* flPtrFile);
	bool readFileDevExt(FILE * flPtrFile);
	bool readUncompressed(FILE*flPtrFile ,BYTE* btPtrOutData);
	bool readCompressed(FILE*flPtrFile ,BYTE* btPtrOutData);
	bool readCompressedMap(FILE*flPtrFile ,BYTE* btPtrColorMap,BYTE* btPtrOutData);
	bool readUncompressedMap(FILE*flPtrFile ,BYTE* btPtrColorMap,BYTE* btPtrOutData);
	bool checkFileFormat()const;
	bool readFileImageData(FILE* flPtrFile, BYTE*& btPtrOutData, int& inRGBType );

private:
	TGAFILEHEADER*		m_ptrFileHeader;
	TGAFILEFOOTER*		m_ptrFileFooter;
	TGASTRUCTUREDATA*	m_ptrStruInfo;

	int					m_inIsReadOnly;
	BYTE*				m_ptrImageData;
};

/** @} */
#endif
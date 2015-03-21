/************************************************************************		
\link	www.twinklingstar.cn
\author Twinkling Star
\date	2013/11/21
****************************************************************************/
#include "SrImageTga.h"
#include <assert.h>

SrImageTga::SrImageTga(int isReadOnly):SrImage()
{
	m_ptrFileHeader		=	NULL;
	m_ptrFileFooter		=	NULL;
	m_ptrStruInfo		=	NULL;
	m_inError			=	NULL;
	m_ptrImageData		=	NULL;

	m_inIsReadOnly		=	isReadOnly;
}

SrImageTga::~SrImageTga()
{
	deallocMemory();
}

bool	SrImageTga::isValid()const
{
	return m_ptrImageData!=NULL;
}

bool	SrImageTga::isNewTgaFormat()const
{
	return m_ptrFileFooter!=NULL;
}

int		SrImageTga::getWidth()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaWidth;
}

int		SrImageTga::getHeight()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaHeight;
}

unsigned char* SrImageTga::getImageData()const
{
	return m_ptrImageData;
}

bool	SrImageTga::getIsRGB()const
{
	assert(m_ptrFileHeader);
	return !(m_ptrFileHeader->tgaPixelDepth ==32 || m_ptrFileHeader->tgaColorMapEnSize==32);
}

unsigned char	SrImageTga::getPixelDepth()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaPixelDepth;
}

unsigned char	SrImageTga::getImageType()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaImageType;
}

bool			SrImageTga::getUseMapType()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaColorMapType==1;
}

int				SrImageTga::getGetMapEntrySize()const
{
	assert(m_ptrFileHeader);
	return m_ptrFileHeader->tgaColorMapEnSize;
}

int				SrImageTga::getImageInfo(unsigned char*& ptrImageInfo)const
{
	assert(m_ptrFileHeader);
	assert(m_ptrStruInfo);
	if( m_ptrFileHeader->tgaIdLength )
	{
		ptrImageInfo = (unsigned char*)malloc(m_ptrFileHeader->tgaIdLength+1);
		int i;
		for( i=0 ; i<m_ptrFileHeader->tgaIdLength ; i++ )
			ptrImageInfo[i] = m_ptrStruInfo->tgaImageColorData.tgaImageId[i];
		ptrImageInfo[i] = '\0';
	}
	return 0;
}
bool SrImageTga::mallocMemory()
{
	m_ptrStruInfo = (TGASTRUCTUREDATA*)malloc(sizeof(TGASTRUCTUREDATA));
	if( !m_ptrStruInfo )
	{
		m_inError =IMAGE_NO_MEMORY;
		return false;
	}
	memset(m_ptrStruInfo,0,sizeof(TGASTRUCTUREDATA));
	m_ptrFileHeader = (TGAFILEHEADER*)malloc(sizeof(TGAFILEHEADER));
	if( !m_ptrFileHeader )
	{
		m_inError = IMAGE_NO_MEMORY;
		return 0;
	}
	memset(m_ptrFileHeader,0,sizeof(TGAFILEHEADER));
	m_ptrFileFooter = (TGAFILEFOOTER*)malloc(sizeof(TGAFILEFOOTER));
	if( !m_ptrFileFooter )
	{
		m_inError = IMAGE_NO_MEMORY;
		return 0;
	}
	memset(m_ptrFileFooter,0,sizeof(TGAFILEFOOTER));
	return true;
}

void SrImageTga::deallocMemory()
{
	if( m_ptrFileHeader )
	{
		free(m_ptrFileHeader);
		m_ptrFileHeader = NULL;
	}
	if( m_ptrStruInfo )
	{
		if( m_ptrStruInfo->tgaPtrDevData )
		{
			free(m_ptrStruInfo->tgaPtrDevData);
			m_ptrStruInfo->tgaPtrDevData = NULL;

		}
		if( m_ptrStruInfo->tgaPtrExtData )
		{
			free(m_ptrStruInfo->tgaPtrExtData);
			m_ptrStruInfo->tgaPtrExtData = NULL;
		}
		if( m_ptrStruInfo->tgaImageColorData.tgaImageId )
		{
			free(m_ptrStruInfo->tgaImageColorData.tgaImageId);
			m_ptrStruInfo->tgaImageColorData.tgaImageId = NULL;
		}
		if( m_ptrStruInfo->tgaImageColorData.tgaColorMap )
		{
			free(m_ptrStruInfo->tgaImageColorData.tgaColorMap);
			m_ptrStruInfo->tgaImageColorData.tgaColorMap = NULL;
		}
		if( m_ptrStruInfo->tgaImageColorData.tgaImageData )
		{
			free(m_ptrStruInfo->tgaImageColorData.tgaImageData);
			m_ptrStruInfo->tgaImageColorData.tgaImageData = NULL;
		}
		free(m_ptrStruInfo);
		m_ptrStruInfo = NULL;
	}
	if( m_ptrFileFooter )
	{
		free(m_ptrFileFooter);
		m_ptrFileFooter = NULL;
	}
	if( m_ptrImageData )
	{
		free(m_ptrImageData);
		m_ptrImageData = NULL;
	}
}

bool SrImageTga::readFileFooter(FILE* flPtrFile)
{
	if( fseek(flPtrFile,-26L,SEEK_END)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
	if( fread(m_ptrFileFooter,sizeof(TGAFILEFOOTER),1,flPtrFile)!=1 )
	{
		m_inError = IMAGE_READ_FAIL;
		return false;
	}
	char signature[] = "TRUEVISION-XFILE";
	if( memcmp(signature,m_ptrFileFooter->tgaSigniture,16)!=0 )
	{
		free(m_ptrFileFooter);
		m_ptrFileFooter = NULL;
	}
	return true;
}

bool SrImageTga::readFileDevExt(FILE * flPtrFile)
{
	int devLength = 0, extLength = 0;
	BYTE* ptrDev = NULL;
	BYTE* ptrExt = NULL;
	if( m_ptrFileFooter->tgaExtOffset )
	{//��ȡ��չ��������
		extLength = m_ptrStruInfo->tgaFileSize - m_ptrFileFooter->tgaExtOffset - 26;
		if( fseek(flPtrFile,m_ptrFileFooter->tgaExtOffset,SEEK_SET)==-1 )
		{
			m_inError = IMAGE_SEEK_FAIL;
			return false;
		}
		ptrExt = (BYTE*)malloc(extLength);
		if( fread(ptrExt,1,extLength,flPtrFile)!=extLength )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrExt);
			return false;
		}
	}
	if( m_ptrFileFooter->tgaDevOffset )
	{//��ȡ����������
		devLength = m_ptrFileFooter->tgaExtOffset - m_ptrFileFooter->tgaDevOffset;
		if( fseek(flPtrFile,m_ptrFileFooter->tgaDevOffset,SEEK_SET)==-1 )
		{
			m_inError = IMAGE_SEEK_FAIL;
			free(ptrDev);
			return false;
		}
		ptrDev = (BYTE*)malloc(devLength);
		if( fread(ptrDev,1,devLength,flPtrFile)!=devLength )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrDev);
			free(ptrExt);
			return false;
		}
	}

	m_ptrStruInfo->tgaPtrDevData	=	ptrDev;
	m_ptrStruInfo->tgaPtrExtData	=	ptrExt;
	return true;
}

bool SrImageTga::readUncompressed(FILE*flPtrFile ,BYTE* btPtrOutData)
{
	BYTE* ptrFileData;
	int pixCount = m_ptrFileHeader->tgaWidth * m_ptrFileHeader->tgaHeight;
	int dataSize = 0;
	if( m_ptrFileHeader->tgaPixelDepth==8 )
	{
		dataSize = pixCount;
		ptrFileData = (BYTE*)malloc(dataSize);
		if( !ptrFileData )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrFileData,1,dataSize,flPtrFile) != dataSize )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrFileData);
			return false;
		}
		BYTE*  tmpPtrData = ptrFileData;
		BYTE*  tmpPtrDataEnd = tmpPtrData + dataSize;
		while( tmpPtrData<tmpPtrDataEnd )
		{
			*(btPtrOutData + 2)	=  *tmpPtrData;
			*(btPtrOutData + 1) =  *tmpPtrData;
			*(btPtrOutData)		=  *tmpPtrData;

			btPtrOutData += 3;
			tmpPtrData += 1;
		}
		free(ptrFileData);
	}
	else if( m_ptrFileHeader->tgaPixelDepth==15 ||  m_ptrFileHeader->tgaPixelDepth==16 )
	{
		dataSize = pixCount<<1;
		ptrFileData = (BYTE*)malloc(dataSize);
		if( !ptrFileData )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrFileData,1,dataSize,flPtrFile) != dataSize )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrFileData);
			return false;
		}
		BYTE*  tmpPtrData = ptrFileData;
		BYTE*  tmpPtrDataEnd = tmpPtrData + dataSize;
		while( tmpPtrData<tmpPtrDataEnd )
		{
			*(btPtrOutData + 2)	=  (*tmpPtrData & 0x1F);
			*(btPtrOutData + 1) =  ((*(tmpPtrData+1)<<3)&0x1C) | ((*tmpPtrData>>5)&0x07);
			*(btPtrOutData)		=  (*(tmpPtrData+1)>>2)&0x1F;

			*(btPtrOutData)		= (*(btPtrOutData)<<3 ) | (*(btPtrOutData)>>2 );
			*(btPtrOutData + 1) = (*(btPtrOutData + 1)<<3 ) | (*(btPtrOutData + 1)>>2 );
			*(btPtrOutData + 2) = (*(btPtrOutData + 2)<<3 ) | (*(btPtrOutData + 2)>>2 );

			btPtrOutData += 3;
			tmpPtrData += 2;
		}
		free(ptrFileData);
	}
	else if( m_ptrFileHeader->tgaPixelDepth==24 )
	{
		dataSize = pixCount*3;
		ptrFileData = (BYTE*)malloc(dataSize);
		if( !ptrFileData )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrFileData,1,dataSize,flPtrFile) != dataSize )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrFileData);
			return false;
		}
		BYTE*  tmpPtrData = ptrFileData;
		BYTE*  tmpPtrDataEnd = tmpPtrData + dataSize;
		while( tmpPtrData<tmpPtrDataEnd )
		{
			*(btPtrOutData + 2)	=  *(tmpPtrData);
			*(btPtrOutData + 1) =  *(tmpPtrData + 1);
			*(btPtrOutData)		=  *(tmpPtrData + 2);

			tmpPtrData += 3;
			btPtrOutData += 3;
		}
		free(ptrFileData);
	}
	else
	{
		dataSize = pixCount<<2;
		ptrFileData = (BYTE*)malloc(dataSize);
		if( !ptrFileData )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(ptrFileData,1,dataSize,flPtrFile) != dataSize )
		{
			m_inError = IMAGE_READ_FAIL;
			free(ptrFileData);
			return false;
		}
		BYTE*  tmpPtrData = ptrFileData;
		BYTE*  tmpPtrDataEnd = tmpPtrData + dataSize;
		while( tmpPtrData<tmpPtrDataEnd )
		{
			*(btPtrOutData + 2)	=  *(tmpPtrData);
			*(btPtrOutData + 1) =  *(tmpPtrData + 1);
			*(btPtrOutData)		=  *(tmpPtrData + 2);
			*(btPtrOutData + 3) =  *(tmpPtrData + 3);
			tmpPtrData	 += 4;
			btPtrOutData += 4;
		}
		free(ptrFileData);
	}
	return true;
}

bool SrImageTga::readCompressed(FILE*flPtrFile ,BYTE* btPtrOutData)
{
	int countPixel = m_ptrFileHeader->tgaWidth * m_ptrFileHeader->tgaHeight;
	int i , curPixel = 0;
	BYTE chunkHeader = 0;
	int bytesPerPixel = (m_ptrFileHeader->tgaPixelDepth+7)>>3;
	BYTE* ptrColorBuffer = (BYTE*)malloc(bytesPerPixel*128);
	bool isRLE = false;
	do
	{
		chunkHeader = 0;
		if( fread(&chunkHeader,1,1,flPtrFile)!=1 )
		{
			free(ptrColorBuffer);
			m_inError = IMAGE_READ_FAIL;
			return false;
		}
		if( chunkHeader<128 )
		{//ԭʼ��
			chunkHeader ++;
			if( fread(ptrColorBuffer,chunkHeader*bytesPerPixel,1,flPtrFile)!=1 )
			{
				free(ptrColorBuffer);
				m_inError = IMAGE_READ_FAIL;
				return false;
			}
			isRLE = false;
		}
		else		
		{//�г̰�
			chunkHeader -= 127;
			if( fread(ptrColorBuffer,bytesPerPixel,1,flPtrFile)!= 1 )
			{
				free(ptrColorBuffer);
				m_inError = IMAGE_READ_FAIL;
				return false;
			}
			isRLE = true;
		}
		curPixel += chunkHeader;
		if( curPixel>countPixel )
		{
			free(ptrColorBuffer);
			m_inError = IMAGE_UNKNOWN_FORMAT;
			return false;
		}

		BYTE* ptrTmpBuffer = ptrColorBuffer;
		if( m_ptrFileHeader->tgaPixelDepth==8 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrTmpBuffer);
				*(btPtrOutData + 1) =  *(ptrTmpBuffer);
				*(btPtrOutData)		=  *(ptrTmpBuffer);
				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer+=1;
			}
		}
		else if( m_ptrFileHeader->tgaPixelDepth==15 || m_ptrFileHeader->tgaPixelDepth==16)
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  (*(ptrTmpBuffer) & 0x1F);
				*(btPtrOutData + 1) =  ((*(ptrTmpBuffer+1)<<3)&0x1C) | ((*ptrTmpBuffer>>5)&0x07);
				*(btPtrOutData)		=  (*(ptrTmpBuffer+1)>>2)&0x1F;

				*(btPtrOutData)		= (*(btPtrOutData)<<3 )		| (*(btPtrOutData)>>2 );
				*(btPtrOutData + 1) = (*(btPtrOutData + 1)<<3 ) | (*(btPtrOutData + 1)>>2 );
				*(btPtrOutData + 2) = (*(btPtrOutData + 2)<<3 ) | (*(btPtrOutData + 2)>>2 );
				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer+=2;
			}
		}
		else if( m_ptrFileHeader->tgaPixelDepth==24 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrTmpBuffer);
				*(btPtrOutData + 1) =  *(ptrTmpBuffer + 1);
				*(btPtrOutData) =  *(ptrTmpBuffer + 2);
				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer += 3;
			}
		}
		else if( m_ptrFileHeader->tgaPixelDepth==32 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrTmpBuffer);
				*(btPtrOutData + 1)	=  *(ptrTmpBuffer + 1);
				*(btPtrOutData)		=  *(ptrTmpBuffer + 2);
				*(btPtrOutData + 3) =  *(ptrTmpBuffer + 3);
				btPtrOutData += 4;
				if( !isRLE ) ptrTmpBuffer += 4;
			}
		}
	} while (curPixel<countPixel);

	free(ptrColorBuffer);

	return true;
}

bool SrImageTga::readCompressedMap(FILE*flPtrFile ,BYTE* btPtrColorMap,BYTE* btPtrOutData)
{
	int countPixel = m_ptrFileHeader->tgaWidth * m_ptrFileHeader->tgaHeight;
	int i=0 , curPixel=0 ,chunkHeader = 0;
	BYTE* ptrColorBuffer = (BYTE*)malloc(128);
	int bytesPerPixel = (m_ptrFileHeader->tgaColorMapEnSize+7)>>3;
	BYTE* ptrBase = btPtrColorMap + m_ptrFileHeader->tgaFirstEnIndex * bytesPerPixel;
	bool isRLE = false;

	do 
	{
		if( fread(&chunkHeader,1,1,flPtrFile)!=1 )
		{
			free(ptrColorBuffer);
			m_inError = IMAGE_READ_FAIL;
			return false;
		}
		if( chunkHeader<128 )
		{
			chunkHeader ++;
			isRLE = false;
			if( fread(ptrColorBuffer,chunkHeader,1,flPtrFile)!=1 )
			{
				free(ptrColorBuffer);
				m_inError = IMAGE_READ_FAIL;
				return false;
			}
		}
		else
		{
			chunkHeader -= 127;
			isRLE = true;
			if( fread(ptrColorBuffer,1,1,flPtrFile)!=1 )
			{
				free(ptrColorBuffer);
				m_inError = IMAGE_READ_FAIL;
				return false;
			}
		}
		curPixel += chunkHeader;
		if( curPixel > countPixel )
		{
			free(ptrColorBuffer);
			m_inError = IMAGE_UNKNOWN_FORMAT;;
			return false;
		}


		BYTE* ptrTmpBuffer = ptrColorBuffer;
		if( m_ptrFileHeader->tgaColorMapEnSize==15 || m_ptrFileHeader->tgaColorMapEnSize==16)
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  (*(ptrBase + (*ptrTmpBuffer<<1))&0x1F);
				*(btPtrOutData + 1) =  ((*(ptrBase + (*ptrTmpBuffer<<1) + 1)<<3)&0x1C) | ((*(ptrBase + (*ptrTmpBuffer<<1))>>5)&0x07);
				*(btPtrOutData)		=  (*(ptrBase + (*ptrTmpBuffer<<1) + 1)>>2)&0x1F;

				*(btPtrOutData)		= (*(btPtrOutData)<<3 ) | (*(btPtrOutData)>>2 );
				*(btPtrOutData + 1) = (*(btPtrOutData + 1)<<3 ) | (*(btPtrOutData  + 1)>>2 );
				*(btPtrOutData + 2) = (*(btPtrOutData + 2)<<3 ) | (*(btPtrOutData  + 2)>>2 );

				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer++;
			}
		}
		else if( m_ptrFileHeader->tgaColorMapEnSize==24 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrBase + (*ptrTmpBuffer<<1));
				*(btPtrOutData + 1) =  *(ptrBase + (*ptrTmpBuffer<<1) + 1);
				*(btPtrOutData)		=  *(ptrBase + (*ptrTmpBuffer<<1) + 2);
				btPtrOutData += 3;
				if( !isRLE ) ptrTmpBuffer++;
			}
		}
		else if( m_ptrFileHeader->tgaColorMapEnSize==32 )
		{
			for( i=0 ; i<chunkHeader ; i++ )
			{
				*(btPtrOutData + 2)	=  *(ptrBase + (*ptrTmpBuffer<<1));
				*(btPtrOutData + 1) =  *(ptrBase + (*ptrTmpBuffer<<1) + 1);
				*(btPtrOutData)		=  *(ptrBase + (*ptrTmpBuffer<<1) + 2);
				*(btPtrOutData + 3) =  *(ptrBase + (*ptrTmpBuffer<<1) + 3);
				btPtrOutData += 4;
				if( !isRLE ) ptrTmpBuffer++;
			}
		}
	} while (curPixel<countPixel);

	free(ptrColorBuffer);

	return true;
}

bool SrImageTga::readUncompressedMap(FILE*flPtrFile ,BYTE* btPtrColorMap,BYTE* btPtrOutData)
{
	BYTE* ptrFileData;
	int pixelCount = m_ptrFileHeader->tgaWidth * m_ptrFileHeader->tgaHeight;
	int base = m_ptrFileHeader->tgaFirstEnIndex;
	int bytesPerPixel = (m_ptrFileHeader->tgaColorMapEnSize + 7)>>3;
	BYTE* ptrBase = btPtrColorMap + m_ptrFileHeader->tgaFirstEnIndex * bytesPerPixel;
	ptrFileData = (BYTE*)malloc(pixelCount);
	if( !ptrFileData )
	{
		m_inError = IMAGE_NO_MEMORY;
		return false;
	}
	if( fread(ptrFileData,1,pixelCount,flPtrFile) != pixelCount )
	{
		m_inError = IMAGE_READ_FAIL;
		free(ptrFileData);
		return false;
	}
	BYTE*  tmpPtrData = ptrFileData;
	BYTE*  tmpPtrOutData = btPtrOutData;
	BYTE*  ptrFileDataEnd = tmpPtrData + pixelCount;

	if( m_ptrFileHeader->tgaColorMapEnSize==15 ||  m_ptrFileHeader->tgaColorMapEnSize==16 )
	{
		while( tmpPtrData<ptrFileDataEnd )
		{
			*(tmpPtrOutData+2)	=  (*(ptrBase+*tmpPtrData) & 0x1F);
			*(tmpPtrOutData+1)	=  ((*(ptrBase+(*tmpPtrData<<1)+1)<<3)&0x1C) | ((*(ptrBase+(*tmpPtrData<<1))>>5)&0x07);
			*(tmpPtrOutData)	=  (*(ptrBase+(*tmpPtrData<<1)+1)>>2)&0x1F;

			*(tmpPtrOutData)		= (*(tmpPtrOutData)<<3 ) | (*(tmpPtrOutData)>>2 );
			*(tmpPtrOutData + 1)	= (*(tmpPtrOutData + 1)<<3 ) | (*(tmpPtrOutData + 1)>>2 );
			*(tmpPtrOutData + 2)	= (*(tmpPtrOutData + 2)<<3 ) | (*(tmpPtrOutData + 2)>>2 );

			tmpPtrOutData += 3;
			tmpPtrData += 1;
		}
	}
	else if( m_ptrFileHeader->tgaColorMapEnSize==24 )
	{
		while( tmpPtrData<ptrFileDataEnd )
		{
			*(tmpPtrOutData + 2)	=	*(ptrBase + *tmpPtrData*3);
			*(tmpPtrOutData + 1)	=	*(ptrBase + 1 + *tmpPtrData*3);
			*(tmpPtrOutData)		=	*(ptrBase + 2 + *tmpPtrData*3);
			tmpPtrOutData += 3;
			tmpPtrData += 1;
		}
	}
	else
	{
		while( tmpPtrData<ptrFileDataEnd )
		{
			*(tmpPtrOutData + 2)=  *(ptrBase + (*tmpPtrData<<2));
			*(tmpPtrOutData + 1)= *(ptrBase + (*tmpPtrData<<2) + 1);
			*(tmpPtrOutData)	= *(ptrBase + (*tmpPtrData<<2) + 2);
			*(tmpPtrOutData + 3) = *(ptrBase + (*tmpPtrData<<2) + 3);
			tmpPtrData += 1;
			tmpPtrOutData += 4;
		}
	}
	free(ptrFileData);

	return true;
}

bool SrImageTga::checkFileFormat()const
{
	//ͷ�ļ��еĿ�͸߱������0
	if( m_ptrFileHeader->tgaHeight<=0 || m_ptrFileHeader->tgaWidth<=0 )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//ֻ֧��λ�����8,15,16,24,32�����
	if( !(m_ptrFileHeader->tgaPixelDepth==8) &&
		!(m_ptrFileHeader->tgaPixelDepth==15)&&
		!(m_ptrFileHeader->tgaPixelDepth==16)&&
		!(m_ptrFileHeader->tgaPixelDepth==24)&&
		!(m_ptrFileHeader->tgaPixelDepth==32) )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	if( m_ptrFileHeader->tgaPixelDepth==8 )
	{
		//���λ�����8����������ɫ����ֻ�ܱ�ʾ�ڰ�ͼ������
		if( !m_ptrFileHeader->tgaColorMapType&&(m_ptrFileHeader->tgaImageType&0x03)!=0x03 )
		{
			m_inError = IMAGE_UNKNOWN_FORMAT;
			return false;
		}
	}
	else if( m_ptrFileHeader->tgaColorMapType)
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//�ж�ͼ����������ɫ���Ƿ�ƥ��
	if( !(m_ptrFileHeader->tgaImageType==TGA_UN_COLOR_MAP&&m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_UN_TRUE_COLOR&&!m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_UN_BLACK_WHITE&&!m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_RLE_COLOR_MAP&&m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_RLE_TRUE_COLOR&&!m_ptrFileHeader->tgaColorMapType)&&
		!(m_ptrFileHeader->tgaImageType==TGA_RLE_BLACK_WHITE&&!m_ptrFileHeader->tgaColorMapType) )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
	//���������ɫ����ÿ������ռ��λ��
	if( m_ptrFileHeader->tgaColorMapType )
	{
		if( m_ptrFileHeader->tgaColorMapEnSize != 15 &&
			m_ptrFileHeader->tgaColorMapEnSize != 16 &&
			m_ptrFileHeader->tgaColorMapEnSize != 24 &&
			m_ptrFileHeader->tgaColorMapEnSize != 32 )
		{
			m_inError = IMAGE_UNKNOWN_FORMAT;
			return false;
		}
	}
#if SR_IMAGE_TGA_CHECK_ORIGIN
	if( m_ptrFileHeader->tgaXOrigin!=0 || m_ptrFileHeader->tgaYOrigin!=0)
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
#endif

#if SR_IMAGE_TGA_CHECK_IMAGEDESC
	if( m_ptrFileHeader->tgaImageDesc )
	{
		m_inError = IMAGE_UNKNOWN_FORMAT;
		return false;
	}
#endif
	return true;
}



bool SrImageTga::readFileImageData(FILE* flPtrFile, BYTE*& btPtrOutData, int& inRGBType )
{
	int inRGB;
	if( fseek(flPtrFile,sizeof(TGAFILEHEADER),SEEK_SET)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
	BYTE* rgbIdInfo = NULL;
	BYTE* ptrOutData = NULL;
	//ͼ����Ϣ�ֶγ��Ȳ�Ϊ0������Ҫ��ȡͼ����Ϣ�ֶ�
	if( m_ptrFileHeader->tgaIdLength )
	{
		rgbIdInfo = (BYTE*)malloc(m_ptrFileHeader->tgaIdLength);
		if( !rgbIdInfo )
		{
			m_inError = IMAGE_NO_MEMORY;
			return false;
		}
		if( fread(rgbIdInfo,m_ptrFileHeader->tgaIdLength,1,flPtrFile)!=1 )
		{
			m_inError = IMAGE_READ_FAIL;
			free(rgbIdInfo);
			return false;
		}
	}
	if( !m_ptrFileHeader->tgaColorMapType )
	{//�����������ɫ�������½���TGA�ļ�
		ptrOutData = NULL;
		//����洢RGB���ݵ��ڴ�ռ�
		if( m_ptrFileHeader->tgaPixelDepth==32 )
		{
			ptrOutData = (BYTE*)malloc(4*m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight);
			inRGB = IMAGE_RGBA;
		}
		else
		{
			ptrOutData = (BYTE*)malloc(3*m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight);
			inRGB = IMAGE_RGB;
		}
		if( !ptrOutData )
		{
			m_inError = IMAGE_NO_MEMORY;
			if( rgbIdInfo )
				free(rgbIdInfo);
			return false;
		}
		if( m_ptrFileHeader->tgaImageType&0x08 )
		{
			//����RLEѹ����TGA�ļ�
			if( !readCompressed(flPtrFile,ptrOutData) )
			{
				free(ptrOutData);
				if( rgbIdInfo )
					free(rgbIdInfo);
				return false;
			}
		}
		else
		{
			//����δ����RLEѹ����TGA�ļ�
			if( !readUncompressed(flPtrFile,ptrOutData) )
			{
				free(ptrOutData);
				if( rgbIdInfo )
					free(rgbIdInfo);
				return false;
			}
		}
	}
	else
	{//���������ɫ�������½���TGA�ļ�
		int bytesPerPixel = (m_ptrFileHeader->tgaColorMapLen + 7)>>3;
		int colorMapSize = m_ptrFileHeader->tgaColorMapEnSize*bytesPerPixel;
		//������ɫ����ڴ�ռ�
		BYTE* ptrColorMap = (BYTE*)malloc(colorMapSize);
		if( !ptrColorMap )
		{
			m_inError = IMAGE_NO_MEMORY;
			if( rgbIdInfo )
				free(rgbIdInfo);
			return false;
		}
		if( fread(ptrColorMap,1,colorMapSize,flPtrFile)!=colorMapSize )
		{
			free(ptrColorMap);
			if( rgbIdInfo )
				free(rgbIdInfo);
			m_inError = IMAGE_READ_FAIL;
			return false;
		}
		//����洢RGB���ݵ��ڴ�ռ�
		ptrOutData = NULL;
		if( m_ptrFileHeader->tgaColorMapEnSize==32 )
		{
			ptrOutData = (BYTE*)malloc(4*m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight);
			inRGB = IMAGE_RGBA;
		}
		else
		{
			ptrOutData = (BYTE*)malloc(3*m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight);
			inRGB = IMAGE_RGB;
		}
		if( !ptrOutData )
		{
			m_inError = IMAGE_NO_MEMORY;
			free(ptrColorMap);
			if( rgbIdInfo )
				free(rgbIdInfo);
			return false;
		}
		//����RLEѹ����TGA�ļ�
		if( m_ptrFileHeader->tgaImageType&0x08 )
		{
			if( !readCompressedMap(flPtrFile,ptrColorMap,ptrOutData) )
			{
				free(ptrOutData);
				free(ptrColorMap);
				if( rgbIdInfo )
					free(rgbIdInfo);
				return false;
			}
		}
		else
		{
			//����δ����RLEѹ����TGA�ļ�
			if( !readUncompressedMap(flPtrFile,ptrColorMap,ptrOutData) )
			{
				free(ptrOutData);
				free(ptrColorMap);
				if( rgbIdInfo )
					free(rgbIdInfo);
				return false;
			}
		}
		free(ptrColorMap);
	}


	//����TGA�ļ���ȷ������ͼ����Ϣ�ֶ�
	m_ptrStruInfo->tgaImageColorData.tgaImageId = rgbIdInfo;
	//�����ķ���ֵ
	btPtrOutData = ptrOutData;
	inRGBType = inRGB;
	return true;
}

bool SrImageTga::readFile(const char* chPtrFileName,BYTE*& btPtrImageData,int& lgPixelCount,int& inRGBType)
{
	if( m_inIsReadOnly!=IMAGE_READ_ONLY )
	{
		m_inError = IMAGE_OBJECT_WRITE_ONLY;
		return false;
	}
	FILE* ptrHandle = fopen(chPtrFileName,"rb");
	if( !ptrHandle )	
	{
		m_inError = IMAGE_OPEN_FAIL;
		return false;
	}
	//����TGAFILEHEADER��TGAFILEFOOTER��TGASTRUCTUREDATA�����ṹ����ڴ�ռ�
	if( !mallocMemory() )
	{
		fclose(ptrHandle);
		return false;
	}
	//��ȡ�ļ�ͷ
	if( fread(m_ptrFileHeader,sizeof(TGAFILEHEADER),1,ptrHandle)!=1 )
	{
		m_inError = IMAGE_READ_FAIL;
		deallocMemory();
		fclose(ptrHandle);
		return false;
	}
	//�ж��ļ���ʽ�Ƿ���Ͻ�����Ҫ��
	if( !checkFileFormat() )
	{
		deallocMemory();
		fclose(ptrHandle);
		return false;
	}
	BYTE* imageData = NULL;
	int rgbType = 0;
	//��ȡͼ����ɫ������
	if( !readFileImageData(ptrHandle,imageData,rgbType) )
	{
		deallocMemory();
		fclose(ptrHandle);
		return false;
	}
	else
	{
		m_ptrImageData = imageData;
	}
	//��ȡ��ǰ���ļ�ָ��λ��
	long imageDataOffset = ftell(ptrHandle);
	//�ƶ��ļ�ָ�����ļ���β����¼�ļ���С
	if( fseek(ptrHandle,0L,SEEK_END)==-1 )
	{
		m_inError = IMAGE_SEEK_FAIL;
		return false;
	}
	m_ptrStruInfo->tgaFileSize = ftell(ptrHandle);
	//��ȡ�ļ�ע�ţ�����ɹ���˵��TGA�ļ������µ�TGA�ļ���ʽ��
	//���ʧ�ܣ�˵��TGA�ļ����ھɵ�TGA�ļ���ʽ�����ͷ�ע�Žṹ�������,
	//m_ptrFileFooter��ֵΪNULL
	if(m_ptrStruInfo->tgaFileSize>=imageDataOffset + 26 && !readFileFooter(ptrHandle) )
	{
		deallocMemory();
		fclose(ptrHandle);
		return false;
	}
	if( m_ptrFileFooter )
	{//������µ�TGA��ʽ������ܴ��ڿ������������չ���򣬶�ȡ����������
		if( !readFileDevExt(ptrHandle) )
		{
			deallocMemory();
			fclose(ptrHandle);
			return false;
		}
	}
	fclose(ptrHandle);

	inRGBType = rgbType;
	lgPixelCount = m_ptrFileHeader->tgaWidth*m_ptrFileHeader->tgaHeight;
	btPtrImageData = imageData;
	return true;
}


bool SrImageTga::writeFile(const char* chPtrImageFile) const
{
	if( m_inIsReadOnly!=IMAGE_WRITE_ONLY )
	{
		m_inError = IMAGE_OBJECT_READ_ONLY;
		return false;
	}
	if( !m_ptrImageData )
	{
		m_inError = IMAGE_UNKNOWN;
		return false;
	}
	FILE* ptrHandle = fopen(chPtrImageFile,"wb");
	if( !ptrHandle )
	{
		m_inError = IMAGE_OPEN_FAIL;
		return false;
	}

	//д�ļ�ͷ
	if( fwrite(m_ptrFileHeader,sizeof(TGAFILEHEADER),1,ptrHandle)!=1 )
	{
		m_inError = IMAGE_WRITE_FAIL;
		return false;
	}
	if( m_ptrFileHeader->tgaIdLength )
	{
		assert(m_ptrStruInfo->tgaImageColorData.tgaImageId);
		if( fwrite(m_ptrStruInfo->tgaImageColorData.tgaImageId,1,m_ptrFileHeader->tgaIdLength,ptrHandle)!=m_ptrFileHeader->tgaIdLength )
		{
			m_inError = IMAGE_WRITE_FAIL;
			return false;
		}
	}
	//дͼ������
	int pixelCount = m_ptrFileHeader->tgaHeight * m_ptrFileHeader->tgaWidth;
	if( fwrite(m_ptrImageData,1,pixelCount*3,ptrHandle)!=3*pixelCount )
	{
		m_inError = IMAGE_WRITE_FAIL;
		return false;
	}
	//д�ļ�ע��
	if( fwrite(m_ptrFileFooter,sizeof(TGAFILEFOOTER),1,ptrHandle)!=1 )
	{
		m_inError = IMAGE_WRITE_FAIL;
		return false;
	}

	fclose(ptrHandle);
	return true;
}


bool SrImageTga::loadImageData(unsigned char* ucPtrRgbData ,unsigned short inWidth,unsigned short inHeight)
{
	if( m_inIsReadOnly!=IMAGE_WRITE_ONLY )
	{
		m_inError = IMAGE_OBJECT_READ_ONLY;
		return false;
	}
	if( !ucPtrRgbData )
	{
		m_inError = IMAGE_UNKNOWN;
		return false;
	}
	//���TGA������ԭʼ��ͼ������
	deallocMemory();
	//�����ļ�ͷ���ݣ��ļ��ṹ���ݣ��ļ�ע�������ڴ�
	if( !mallocMemory())
	{
		return false;
	}
	m_ptrFileHeader->tgaImageType = 0x02;
	m_ptrFileHeader->tgaWidth  = inWidth;
	m_ptrFileHeader->tgaHeight = inHeight;
	m_ptrFileHeader->tgaPixelDepth = 24;
	//����ͼ�������ڴ�
	int pixelCount = 3*inWidth*inHeight;
	m_ptrImageData = (BYTE*)malloc(pixelCount);
	if( !m_ptrImageData )
	{
		m_inError = IMAGE_NO_MEMORY;
		free(m_ptrFileHeader);
		m_ptrFileHeader = NULL;
		return false;
	}
	//�����ݸ��ƽ�TGA�ļ�������
	int i;
	for( i=0 ; i<pixelCount ; i++ )
		m_ptrImageData[i] = ucPtrRgbData[i];

	//��ʼ��ע����Ϣ
	char signature[] = "TRUEVISION-XFILE";
	m_ptrFileFooter->tgaAscii='.';
	for( i=0 ; i<16 ; i++ )
		m_ptrFileFooter->tgaSigniture[i] = signature[i];

	//��ʼ��ͼ����Ϣ�ֶ�
	char imageInfo[]="Copywrite by TwinkingStar";
	m_ptrFileHeader->tgaIdLength = strlen(imageInfo);
	m_ptrStruInfo->tgaImageColorData.tgaImageId = (BYTE*)malloc(m_ptrFileHeader->tgaIdLength);
	for( i=0 ; i<m_ptrFileHeader->tgaIdLength ; i++ )
	{
		m_ptrStruInfo->tgaImageColorData.tgaImageId[i] = imageInfo[i];
	}
	return true;
}

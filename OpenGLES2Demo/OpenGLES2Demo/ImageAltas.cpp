#include "ImageAltas.h"
#include <algorithm>
#include <GLES2/gl2.h>
#include <assert.h>
int CImageAltas::s_MaxTextureSize = 0;
CImageAltas::CImageAltas() :miCurWidth(0), miCurHeight(0), miIndex(-1)
{
	if (0 == s_MaxTextureSize)
	{
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s_MaxTextureSize);
	}

}

CImageAltas::~CImageAltas()
{
	for_each(mImageUnits.begin(), mImageUnits.end(), [](ImageUnit *unit){
		if (NULL != unit)
		{
			delete unit;
		}
	});
	mImageUnits.clear();
}

void CImageAltas::addImageUnit(ImageUnit*unit)
{
	if (NULL != unit && NULL != unit->data )
	{
		if (unit->width > s_MaxTextureSize || unit->height > s_MaxTextureSize)
		{
			throw "the image size is out of opengles maxsize.";
		}
		if (mImgData.empty())
		{
			createData(unit);
		}
		else
		{
			int tmpWidth = miCurWidth + unit->width;
			int tmpHeight = miCurHeight + unit->height;
			if ( tmpWidth > s_MaxTextureSize)
			{
				miCurWidth = 0;	 
				miCurHeight += unit->height;
				if ( tmpHeight >= s_MaxTextureSize)
				{ //合并纹理超出最大支持纹理则重新创建纹理
					createData(unit);
					return;
				}
			}
			insertData(unit);
			miCurWidth += unit->width;
			
		}
	};
}

void CImageAltas::createData(ImageUnit *unit )
{
	uchar *dt = new uchar[RGBFORMAT * s_MaxTextureSize * s_MaxTextureSize];
	const int defaultWidthSpace = RGBFORMAT * s_MaxTextureSize;
	const int unitWidthSpace = RGBFORMAT * unit->width;
	memset(dt, 0, defaultWidthSpace * s_MaxTextureSize);
	if (unit->width == s_MaxTextureSize)
	{
		miCurWidth = 0;
		miCurHeight = unit->height;
	}
	else
	{
		miCurWidth = unit->width;
		miCurHeight = 0;
	}
	
	for (int i = 0; i < unit->height; ++i)
	{
		const int dtSpace = i * defaultWidthSpace;
		const int utSpace = i * unitWidthSpace;
		for (int j = 0; j < unit->width * 3 ; j += 3)
		{
			dt[j + dtSpace + 0] = unit->data[j + utSpace];
			dt[j + dtSpace + 1] = unit->data[j + utSpace + 1];
			dt[j + dtSpace + 2] = unit->data[j + utSpace + 2];
		}
	}
	mImgData.insert(make_pair(++miIndex, dt));
	unit->index = miIndex;
}

CImageAltas& CImageAltas::getSingleton(void)
{
	static CImageAltas singleton;
	return singleton;
}

uchar* CImageAltas::getIndexBuffer(int index)
{
	if (index > miIndex)
		return NULL;
	return mImgData[index];
}

void CImageAltas::insertData(ImageUnit *unit)
{
	assert(NULL != mImgData[miIndex]);
	uchar *pdata = mImgData[miIndex];
	const int defaultWidthSpace = RGBFORMAT * s_MaxTextureSize;
	const int unitWidthSpace = RGBFORMAT * unit->width;
	const int offsetWidth = RGBFORMAT * miCurWidth;

	for (int i = 0; i < unit->height; ++i)
	{
		const int dtSpace = (i + miCurHeight ) * defaultWidthSpace;
		const int utSpace = i * unitWidthSpace;
		for (int j = 0; j < unit->width * 3; j += 3)
		{
			pdata[offsetWidth + j + dtSpace + 0] = unit->data[j + utSpace];
			pdata[offsetWidth + j + dtSpace + 1] = unit->data[j + utSpace + 1];
			pdata[offsetWidth + j + dtSpace + 2] = unit->data[j + utSpace + 2];
		}
	}
	unit->index = miIndex;
}

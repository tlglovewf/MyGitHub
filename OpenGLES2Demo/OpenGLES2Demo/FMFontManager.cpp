#include "FMFontManager.h"
#include <assert.h>
#include <ftglyph.h>
#include <GLES2/gl2.h>
#include <algorithm>
#include "ImageAltas.h"
FMFont::FMFont(FT_Library ftLib, const char* fontFile, int fontSize):
mFontSize(fontSize),
mFace(NULL),
mStartX(0),
mStartY(0),
mTexId(0)
{
	assert(NULL != ftLib);

	FT_Error error =  FT_New_Face(ftLib, fontFile,0, &mFace);
	FT_Set_Pixel_Sizes(mFace, 0 , fontSize);
	//FT_Set_Char_Size(mFace, fontSize << 6, 0, 400, 400);
}

FMFont::~FMFont()
{
	std::for_each(mChars.begin(), mChars.end(), [](std::pair<wchar_t, FMCharacter*> item)
	{
		if (NULL != item.second)
		{
			delete item.second;
			item.second = NULL;
		}
	});
}

void setfour( GLuint &val )
{
	while (val % 2 != 0)
	{
		++val;
	}
}

FMCharacter* FMFont::getCharactor(wchar_t ch)
{
	return NULL;
}

void FMFont::drawText(float x, float y, const wchar_t *text, GLuint ptLoc, GLuint texLoc)
{
	if (NULL != text)
	{

	}
}

void FMFont::initFontAtlasTexture(void)
{
	wchar_t initch[] = L"一二三四五七八九十"
					   L"上下左右东南西北"
					   L"赵钱孙李周五郑王"
					  // L"孔子孟子老子庄子孙子"
					   L"abcdefghijklmnopq"
					   L"rstuvwxyz"
					   L"ABCDEFG";
	const int ftLen =  wcslen(initch);
	typedef unsigned char u8;
	CImageAltas::setImageFormat(4);
	CImageAltas::getSingleton();
	const int MulSz = CImageAltas::s_MaxTextureSize * CImageAltas::s_MaxTextureSize;
	mData = new uint[ MulSz ];
	memset(mData, 0,  MulSz * 4);
	int offsetX = 0;
	int offsetY = 0;
	for (int iidx = 0; iidx < ftLen; ++iidx)
	{
		int idx = FT_Get_Char_Index(mFace, initch[iidx]);

		if (idx <= 0) continue;

		int top;
		int lft;
		int ttw;
		int tth;
		int igw;
		int igh;

		if (!FT_Load_Glyph(mFace, idx, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP))
		{
			FT_GlyphSlot glyph = mFace->glyph;
			FT_Bitmap bits;
			if (glyph->format == ft_glyph_format_outline)
			{
				if (!FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL))
				{
					bits = glyph->bitmap;
					
					u8 *pt = bits.buffer;
					top = glyph->bitmap_top;
					lft = glyph->bitmap_left;

					igw = 1;
					igh = 1;
					ttw = bits.width;
					tth = bits.rows;

					int Ascender = mFace->size->metrics.ascender >> 6;

					int charx = lft;
					int chary = Ascender - top;

					while (igw < ttw) igw <<= 1;
					while (igh < tth) igh <<= 1;
					igw > igh ? igh = igw : igw = igh;
					//igw = igh = 128;
					if ( offsetY +  igh >= 1024)
					{
						break;
					}

					int dt = (chary + offsetY )* CImageAltas::s_MaxTextureSize + offsetX + lft  ;
					
					unsigned int *texp = mData + dt;
					for (int i = 0; i < bits.rows; ++i)
					{
						unsigned int *rowp = texp ;
						for (int j = 0; j < bits.width; ++j)
						{
							if (0 != *pt)
							{
								*rowp = *pt;
								*rowp <<= 24;
								*rowp |= 0;// 0x0000ff;
							}
							else
							{
								*rowp = 0;
							}
							if (i == 0 || j == 0 || i == bits.rows - 1 || j == bits.width - 1)
							{
							//	*rowp = 0xffffffff;
							}
							pt++;
							rowp++;
						}
						texp += CImageAltas::s_MaxTextureSize;
					}
					offsetX += igw ;
					if (offsetX + igw > 1024)
					{
						offsetX = 0;
						offsetY += igh;
					}
				}

			}
		}
	}
}

FMFontManager::FMFontManager() :mCurIndex(0)
{
	FT_Init_FreeType(&mFontLib);
}


FMFontManager::~FMFontManager()
{
	deleteAllFont();
	assert(NULL != mFontLib);
	FT_Done_FreeType(mFontLib);
}

FMFontManager& FMFontManager::getSingleton(void)
{
	static FMFontManager singleton;
	return singleton;
}

FMFont* FMFontManager::createFont(const char *fontFile, int fontSize)
{
	FMFont *font = new FMFont(mFontLib, fontFile, fontSize);
	mFonts.push_back(font);
	font->initFontAtlasTexture();
	return font;
}

void FMFontManager::deleteAllFont(void)
{
	for_each(mFonts.begin(), mFonts.end(), [](FMFont *font)
	{
		if (NULL != font)
		{
			delete font;
			font = NULL;
		}
	});
}

void FMFontManager::deleteFont(FMFont *font)
{
	if (NULL != font)
	{
		delete font;
	}
}

void FMFontManager::setCurrentFont(int idx)
{
	mCurIndex = idx;
}

void FMFontManager::drawText(float x, float y, const wchar_t *text, GLuint ptLoc, GLuint texLoc)
{
	if (mCurIndex < mFonts.size())
	{
		mFonts[mCurIndex]->drawText(x, y, text, ptLoc, texLoc);
	}
}

void FMFontManager::beginDraw( GLuint texLoc)
{
	
}

void FMFontManager::endDraw(void)
{

}

void FMFontManager::setData(void *data)
{
}


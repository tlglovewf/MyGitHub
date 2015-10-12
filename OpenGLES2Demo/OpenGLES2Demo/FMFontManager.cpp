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
	FT_Set_Pixel_Sizes(mFace, 0, fontSize);
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
	wchar_t initch[] = L"世界你好么";

	const int ftLen =  wcslen(initch);
	typedef unsigned char u8;
	CImageAltas::setImageFormat(4);
	CImageAltas::getSingleton();
	mData = new u8[CImageAltas::s_MaxTextureSize * CImageAltas::s_MaxTextureSize * 4];
	memset(mData, 0, CImageAltas::s_MaxTextureSize * CImageAltas::s_MaxTextureSize * 4);
	int offset = 0;
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
					while (igw < ttw) igw <<= 1;
					while (igh < tth) igh <<= 1;
					igw > igh ? igh = igw : igw = igh;

					const int len = igw * igh ;
					uchar *data = new uchar[len * 4];
					memset(data, 0, len);
					const int defaultWidthspace = 4 * CImageAltas::s_MaxTextureSize;
					for(int j = 0; j < igh ; j++)
					{
						const int dt = j * defaultWidthspace + offset;
						for(int i = 0; i < igw; ++i)
						{
							mData[i + dt + 0] = mData[i + dt + 1] = mData[i + dt + 2] = mData[i + dt + 3] = (i >= bits.width || j >= bits.rows) ? 0 : bits.buffer[i + bits.width * j];
						}
					}
					offset += igw * 4;
					delete [] data;
#if 0
					for (int i = 0; i < bits.rows; ++i)
					{
						uchar *rowp = texp;
						for (int j = 0; j < bits.width; ++j)
						{
						/*	if (*pt)
							{
								u8 tmp = *pt;
								*pt = (u8)(sqrt((float)tmp / 256.0f) * 256.0f);
								*rowp = *pt;
								*rowp <<= 24;
								*rowp |= 0xffffff;
							}
							else
							{
								*rowp == 0;
							}
							pt++;
							rowp++;*/
							if (0 != *pt)
							{
								u8 tmp = *pt;
								*pt = (u8)(sqrt((float)tmp / 256.0f) * 256.0f);
								*rowp++ = 0xff;
								*rowp++ = 0xff;
								*rowp++ = 0xff;
								*rowp++ = 0xff;
							}
							else
							{
								rowp += 4;
							}
							pt++;
						}
						texp += igw * 4;
					}
					int Ascender = mFace->size->metrics.ascender >> 6;
					int DeAscender = mFace->size->metrics.descender >> 6;
					int height2 = mFace->size->metrics.height >> 6;

					int glyphTop = (int)(Ascender - ((Ascender - DeAscender) - mFontSize) / 2.0 - top);
					//
					memcpy(ftTexture + offset, data, len * 4);
					offset += 4 * len;
#endif


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


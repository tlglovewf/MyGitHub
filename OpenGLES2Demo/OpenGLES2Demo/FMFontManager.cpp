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
	wchar_t initch[] = L"你好我是中国人";

	const int ftLen =  wcslen(initch);
	typedef unsigned char u8;
	CImageAltas::setImageFormat(4);
	CImageAltas::getSingleton();
	u8 *ftTexture = new u8[CImageAltas::s_MaxTextureSize * CImageAltas::s_MaxTextureSize * 4];
	memset(ftTexture, 0, CImageAltas::s_MaxTextureSize * CImageAltas::s_MaxTextureSize * 4);
	ImageUnit unit;
	unit.width = 1024;
	unit.height = 1024;
	unit.data = ftTexture;
	
	for (int i = 0; i < ftLen; ++i)
	{
		int idx = FT_Get_Char_Index(mFace, initch[i]);

		if (idx <= 0) continue;

		int top;
		int lft;
		int ttw;
		int tth;
		int igw;
		int igh;

		int offset = 0;
		
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
					while (1)
					{
						if (igw > ttw)
							break;
						else
							igw <<= 1;
					}
					while (1)
					{
						if (igh > tth)
							break;
						else
							igh <<= 1;
					}

					igw > igh ? igh = igw : igw = igh;

					const int len = igw * igh ;
					int *data = new int[ len ];
					int *texp = data;
					memset(data, 0, len);

					for (int i = 0; i < bits.rows; ++i)
					{
						int *rowp = texp;
						for (int j = 0; j < bits.width; ++j)
						{
							if (*pt)
							{
								u8 tmp = *pt;
								*pt = (u8)(sqrt((float)tmp / 256.0f) * 256.0f);
								*rowp = *pt;
								*rowp != 0xffffff;
							}
							else
							{
								*rowp == 0;
							}
							pt++;
							rowp++;
						}
						texp += igw;
					}
					int Ascender = mFace->size->metrics.ascender >> 6;
					int DeAscender = mFace->size->metrics.descender >> 6;
					int height2 = mFace->size->metrics.height >> 6;

					int glyphTop = (int)(Ascender - ((Ascender - DeAscender) - mFontSize) / 2.0 - top);
					//
					memcpy(ftTexture + offset, data, len * 4);
					offset += 4 * len;
				}

			}
		}
	}
	CImageAltas::getSingleton().addImageUnit(&unit);
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


#include "FMFontManager.h"
#include <assert.h>
#include <ftglyph.h>
#include <GLES2/gl2.h>
#include <algorithm>
FMFont::FMFont(FT_Library ftLib, const char* fontFile, int fontSize):
mFontSize(fontSize),
mFace(NULL),
mStartX(0),
mStartY(0),
mTexId(0)
{
	assert(NULL != ftLib);

	FT_Error error =  FT_New_Face(ftLib, fontFile,0, &mFace);
	//FT_Select_Charmap(mFace, FT_ENCODING_UNICODE);
	//FT_Set_Char_Size(mFace, fontSize << 6, fontSize << 6, 500, 500);
	FT_Set_Pixel_Sizes(mFace, 0, fontSize);
	assert(NULL != mFace);
	glGenTextures(1, &mTexId);
	glBindTexture(GL_TEXTURE_2D, mTexId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, TEXTSIZE, TEXTSIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTSIZE, TEXTSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
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
	if ( 0 == mChars.count(ch))
	{//字符不存在纹理上

		mChars.insert(std::make_pair(ch, new FMCharacter));

		if (mStartX + mFontSize > TEXTSIZE)
		{//满一行，从新开始
			mStartX = 0;
			mStartY += mFontSize;
		}
 
		FT_Error error = FT_Load_Glyph(mFace, FT_Get_Char_Index(mFace, ch), FT_LOAD_DEFAULT);
		FT_Glyph glyph;
		FT_Get_Glyph(mFace->glyph, &glyph);
		FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
		FT_Bitmap &bitmap = bitmap_glyph->bitmap;

		//假如没有数据直接跳过
		if (0 == bitmap.width || 0 == bitmap.rows)
		{
			mStartX += mFontSize / 2;

			mChars[ch]->x0 = mStartX;
			mChars[ch]->y0 = mStartY;
			mChars[ch]->x1 = mStartX + bitmap.width;
			mChars[ch]->y1 = mStartX + bitmap.rows;
			mChars[ch]->offsetX = 0;
			mChars[ch]->offsetY = 0;
			
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, mTexId);
			int error = glGetError();
			mChars[ch]->x0 = mStartX;
			mChars[ch]->y0 = mStartY;
			mChars[ch]->x1 = mStartX + bitmap.width;
			mChars[ch]->y1 = mStartX + bitmap.rows;
			mChars[ch]->offsetX = bitmap_glyph->left;
			mChars[ch]->offsetY = bitmap_glyph->top;
			//memset(ch, 255,  );
			GLuint txwidth = bitmap.width;
			GLuint txheight = bitmap.rows;
			setfour(txwidth);
			setfour(txheight);
			typedef unsigned char u8;
			unsigned char *pt = bitmap.buffer;
			txwidth > txheight ? txheight = txwidth : txwidth = txheight;
			unsigned char *ch = new unsigned char[ txwidth * 4 * txheight ];
			memset(ch, 0, txwidth * 4);
			for (int i = 0; i < bitmap.rows; ++i)
			{
				unsigned char *rowp = ch;
				for (int j = 0; j < bitmap.width; ++j)
				{
					if (*pt)
					{
						u8 tmp = *pt;
						*pt = (u8)(sqrt((float)tmp / 256.0f) * 256.0f);

						*rowp = *pt << 24;
						*rowp |= 0xffffff;
					}
					else
					{
						*rowp = 0;
					}
					pt++;
					rowp++;
				}
				ch += txwidth;
			}

			glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
			glTexSubImage2D(GL_TEXTURE_2D, 0, mStartX, mStartY, txwidth, txheight, 
							GL_RGBA, GL_UNSIGNED_BYTE, ch);
			mStartX += txwidth + 1;
		}
	}

	return mChars[ch];
}

void FMFont::drawText(float x, float y, const wchar_t *text, GLuint ptLoc, GLuint texLoc)
{
	typedef float TextVerx[5];
	TextVerx vert[TEXTSIZE] = { 0 };

	float texWidth  = TEXTSIZE;
	float texHeight = TEXTSIZE;
	float xStart = x;
	float yStart = y + 16;
	float zStart = -1;
	int index = 0;
	unsigned nsize = wcslen(text);
	float fHeight = 0;
	int *pIdx = new int[nsize * 6];
	for (int i = 0; i < nsize; ++i)
	{
		FMCharacter *ch = getCharactor(text[i]);

		int height = ch->y1 - ch->y0;
		int width = ch->x1 - ch->x0;
		float offset = height - ch->offsetY;
		float offsetX = ch->offsetX;

		/**
		*   第一个点
		*/
		vert[index + 0][0] = xStart;
		vert[index + 0][1] = yStart - height + offset;
		vert[index + 0][2] = zStart;
		vert[index + 0][3] = ch->x0 / texWidth;
		vert[index + 0][4] = ch->y0 / texHeight;
		/**
		*   第二个点
		*/
		vert[index + 1][0] = xStart  + width;
		vert[index + 1][1] = yStart - height + offset;
		vert[index + 1][2] = zStart;
		vert[index + 1][3] = ch->x1 / texWidth;
		vert[index + 1][4] = ch->y0 / texHeight;

		/**
		*   第三个点
		*/
		vert[index + 2][0] = xStart + width;
		vert[index + 2][1] = yStart + offset;
		vert[index + 2][2] = zStart;
		vert[index + 2][3] = ch->x1 / texWidth;
		vert[index + 2][4] = ch->y1 / texHeight;


		/**
		*   第四个点
		*/
		vert[index + 3][0] = xStart;
		vert[index + 3][1] = yStart + offset;
		vert[index + 3][2] = zStart;
		vert[index + 3][3] = ch->x0 / texWidth;
		vert[index + 3][4] = ch->y1 / texHeight;

		const int dt = 4 * i;
		pIdx[i * 6 + 0] = 0 + dt;
		pIdx[i * 6 + 1] = 1 + dt;
		pIdx[i * 6 + 2] = 2 + dt;
		pIdx[i * 6 + 3] = 0 + dt;
		pIdx[i * 6 + 4] = 2 + dt;
		pIdx[i * 6 + 5] = 3 + dt;

		xStart += width + (ch->offsetX);
		index += 4;
	}

	static GLfloat pttex[] = { -0.5f, 0.5f, 0.0f,  // Position 0
							
								-0.5f, -0.5f, 0.0f,  // Position 1

								0.5f, -0.5f, 0.0f,  // Position 2

								0.5f, 0.5f, 0.0f,  // Position 3
							};
	//GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextVerx), pttex);
	//glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE,sizeof(TextVerx), &vert[3]);
	//glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(texLoc);

	//glDrawElements(GL_TRIANGLES, 6 * nsize, GL_UNSIGNED_INT, pIdx);
	//glDrawElements( GL_POINTS, 6, GL_UNSIGNED_SHORT, indices);
	delete[] pIdx;
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
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, mFonts[mCurIndex]->mTexId);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void FMFontManager::endDraw(void)
{

}

void FMFontManager::setData(void *data)
{
  uint id = 	mFonts[0]->mTexId;
  glBindTexture(GL_TEXTURE_2D, id);

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_RGB, GL_UNSIGNED_BYTE, data);
}

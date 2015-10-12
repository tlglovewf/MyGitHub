#ifndef _FONTMANAGER_H_H
#define _FONTMANAGER_H_H
#include <vector>
#include <ft2build.h>
#include <freetype.h>
#include <map>
typedef unsigned int uint;
struct FMCharacter
{
public:
	FMCharacter(void){ memset(this, sizeof(*this), 0); }

	uint x0 : 10;
	uint y0 : 10;
	uint x1 : 10;
	uint y1 : 10;
	uint offsetX : 10;
	uint offsetY : 10;
};

class FMFont
{
	friend class FMFontManager;
	typedef std::map< wchar_t, FMCharacter*> CharMapTable;
public:
	
	FMCharacter* getCharactor(wchar_t ch);

	void drawText(float x, float y, const wchar_t *text, unsigned int ptLoc, unsigned int texLoc);

	void initFontAtlasTexture(void);
private:
	FT_Face			mFace;
	int				mStartX;
	int				mStartY;
	int				mFontSize;
	uint			mTexId;
	CharMapTable	mChars;
	FMFont(FT_Library ftLib, const char* fontFile, int fontSize);
	~FMFont();
};

class FMFontManager
{
public:

	~FMFontManager();
	static FMFontManager& getSingleton(void);

	FMFont* createFont(const char *fontFile, int fontSize);
	
	void beginDraw( unsigned int texLoc);
	void endDraw(void);
	void setCurrentFont(int idx);

	void deleteFont(FMFont *font);
	void deleteAllFont(void);

	void drawText(float x, float y, const wchar_t *text, unsigned int ptLoc, unsigned int texLoc);

	void setData(void *data);

	int getTexId(void){ return mFonts[0]->mTexId; }
private:
	std::vector<FMFont*> mFonts;
	int					 mCurIndex;
	FT_Library mFontLib;
	FMFontManager();
};

#endif
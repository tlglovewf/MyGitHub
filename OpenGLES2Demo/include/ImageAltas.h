#ifndef _IMAGEALTAS_H_H
#define _IMAGEALTAS_H_H
#include <vector>
#include <map>
#include <set>
using namespace std;
typedef unsigned char uchar;
#define IMAGEDEFAULTSIZE (DEAFULTSIZE * DEAFULTSIZE)
#define RGBFORMAT 3
struct ImageUnit
{
	float u;
	float v;
	int   width;
	int	  height;
	int   index;
	uchar *data;
	ImageUnit():data(NULL), u(0), v(0), index(-1){}
	~ImageUnit(){ if (NULL != data){ delete data; data = NULL; } }
};


class CImageAltas
{
public:
	CImageAltas( );
	~CImageAltas();

	void addImageUnit( ImageUnit*unit);

	void createData( ImageUnit *unit );

	void insertData(ImageUnit *unit);

	static CImageAltas& getSingleton(void);

	uchar* getIndexBuffer(int index);

	static int s_MaxTextureSize;
private:
	vector< ImageUnit* > mImageUnits;
	map< int , uchar* > mImgData;
	int   miCurWidth;
	int   miCurHeight;
	int   miIndex;
	
};
#endif

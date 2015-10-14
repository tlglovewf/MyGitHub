#include <iostream>
#include "es2render.h"
#include <assert.h>
#include "FMFontManager.h"
#include "Matrix.h"
#include "FileManager.h"
#include "ImageAltas.h"
using namespace std;

#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#define INVERTED_BIT (1 << 5)
#pragma pack(push,x1)                            // Byte alignment (8-bit)
#pragma pack(1)

static int drawframe = 0;
static int ang = 0;
static int drawtex = 1;
typedef struct
{
	unsigned char	IdSize,
					MapType,
					ImageType;
	unsigned short	PaletteStart,
					PaletteSize;
	unsigned char	PaletteEntryDepth;
	unsigned short	X,
					Y,
					Width,
					Height;
	unsigned char	ColorDepth,
					Descriptor;

} TGA_HEADER;
#pragma pack(pop,x1)
void *pdata = NULL;
struct UserData
{
	GLuint programObj;

	GLint textureId;

	GLint textVLoc;
	GLint textSpLoc;

	GLint clrLoc;
	GLint dwtexLoc;

	GLint mtxLoc;
};
typedef struct
{
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE alpha;
}RGBAFORMAT;
typedef struct  
{
	BYTE red;
	BYTE green;
	BYTE blue;
}RGBFORMAT;
int WinTGALoad( const char *fileName, uchar **buffer, int *width, int *height )
{
	FILE        *fp;
	TGA_HEADER   Header;

	if ( fopen_s ( &fp, fileName, "rb" ) != 0 )
	{
		return FALSE;
	}

	if ( fp == NULL )
	{
		return FALSE;
	}

	fread ( &Header, sizeof(TGA_HEADER), 1, fp );

	*width = Header.Width;
	*height = Header.Height;
	CImageAltas::setImageFormat(Header.ColorDepth / 8);
	if ( Header.ColorDepth == 24 )
	{
		RGBFORMAT *Buffer24;

		Buffer24 = (RGBFORMAT*)malloc(sizeof(RGBFORMAT) * (*width) * (*height));

		if(Buffer24)
		{
			int i=0;
			int x,
				y;

			fread(Buffer24, sizeof(RGBFORMAT), (*width) * (*height), fp);

			*buffer = new uchar[sizeof(RGBFORMAT) * (*width) * (*height)];

			for ( y = 0; y < *height; y++ )
				for( x = 0; x < *width; x++ )
				{
					int Index= y * (*width) + x;

					if(!(Header.Descriptor & INVERTED_BIT))
						Index= ((*height) - 1 - y) * (*width) + x;

					(*buffer)[(i * 3)]=      Buffer24[Index].red;
					(*buffer)[(i * 3) + 1]=  Buffer24[Index].green;
					(*buffer)[(i * 3) + 2]=  Buffer24[Index].blue;

					i++;
				}

				fclose(fp);
				free(Buffer24);
				return(TRUE);
		}		
	}
	else if ( Header.ColorDepth == 32 )
	{
			RGBAFORMAT *Buffer32;

			Buffer32 = (RGBAFORMAT*)malloc(sizeof(RGBAFORMAT) * (*width) * (*height));

			if (Buffer32)
			{
				int i = 0;
				int x,
					y;
				const int szFormat = sizeof(RGBAFORMAT);
				fread(Buffer32, szFormat, (*width) * (*height), fp);
				*buffer = new uchar[sizeof(RGBAFORMAT) * (*width) * (*height)];

				for (y = 0; y < *height; y++)
					for (x = 0; x < *width; x++)
					{
						int Index = y * (*width) + x;

						if (!(Header.Descriptor & INVERTED_BIT))
							Index = ((*height) - 1 - y) * (*width) + x;

						(*buffer)[(i * szFormat)] = Buffer32[Index].red;
						(*buffer)[(i * szFormat) + 1] = Buffer32[Index].green;
						(*buffer)[(i * szFormat) + 2] = Buffer32[Index].blue;
						(*buffer)[(i * szFormat) + 3] = Buffer32[Index].alpha;
						i++;
					}
				fclose(fp);
				free(Buffer32);
				return(TRUE);
			}
	}

	return(FALSE);
}
GLuint picTex = 0;
GLuint LoadTexture( const char *fileName )
{
	static ImageUnit unit;
	WinTGALoad(fileName, &unit.data, &unit.width, &unit.height);
	static int left = 0;
	static int top = 0;
	if (NULL == unit.data)
	{
		printf("Error loading (%s) image.\n", fileName);
		return 0;
	}
	CImageAltas::getSingleton().addImageUnit(&unit);
	return picTex;
}


#define PI 3.141592657


void ES2STDCALL drawArrowLine( void *ctx )
{
	assert( NULL != ctx );
	glClear(GL_COLOR_BUFFER_BIT);

	ES2Context *es2Ctx = static_cast<ES2Context*>(ctx);
	UserData *pData = (UserData*)es2Ctx->userData;
	assert(NULL != pData );
	GLuint programObj = pData->programObj;
	if( 0 == programObj )
		return ;
	const float num = 0.5f;
	GLfloat vVertices[12] = { -0.1,  -num, 0,
							 0.1, num / 2  , 0};
	
	const float dt = 0.05;
	vVertices[6] = -dt;
	vVertices[7] = dt / 2; 
	vVertices[8] = 0.0f;
	vVertices[9] = -dt;
	vVertices[10] = -dt / 2;
	vVertices[11] = 0.0;

	float dtX = vVertices[3] - vVertices[0];
	float dtY = vVertices[4] - vVertices[1];
	const float mo = sqrtf( dtX * dtX + dtY * dtY );
	float iX =  dtX / mo;
	float iY =  dtY / mo;

	float dthe = iX / sqrtf( iX * iX + iY * iY );

	float theta = acos(dthe);
	float ang = acos(theta);
	float tmpx = vVertices[6] * cos(theta) - vVertices[7] * sin(theta) + vVertices[3];
	float tmpy = vVertices[6] * sin(theta) + vVertices[7] * cos(theta) + vVertices[4];
	vVertices[6] = tmpx;
	vVertices[7] = tmpy;

	tmpx = vVertices[9] * cos(theta) - vVertices[10] * sin(theta) + vVertices[3];
	tmpy = vVertices[9] * sin(theta) + vVertices[10] * cos(theta) + vVertices[4];
	vVertices[9] = tmpx;
 	vVertices[10] = tmpy;
	int indices[] = { 0, 1, 2, 1, 3};
	glUseProgram( programObj );
	const int posIndex = 0; 
	
	glVertexAttribPointer( posIndex, 3 , GL_FLOAT, GL_FALSE, 0, vVertices);
	
	glEnableVertexAttribArray( posIndex );
	
	glDrawElements( GL_LINE_STRIP, 5, GL_UNSIGNED_INT, indices);
	eglSwapBuffers( es2Ctx->eglDisplay, es2Ctx->eglSurface);
}

void ES2STDCALL draw(void *ctx )
{
	//glClearColor(255, 255, 255, 255);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	assert(NULL != ctx );
	ES2Context *es2ctx = static_cast<ES2Context*>(ctx);
	UserData *pData = (UserData *)es2ctx->userData;
	assert( NULL != pData );
	GLuint programObj =  pData->programObj;
	if( 0 == programObj)
		return;

	GLfloat *vVertices = NULL;
if(drawtex)
{
	static GLfloat quadtex[] = { -0.5f,  0.5f, 0.0f,  // Position 0
									0.0f,  0.0f,        // TexCoord 0 
									-0.5f, -0.5f, 0.0f,  // Position 1
									0.0f,  1.0f,        // TexCoord 1
									0.5f, -0.5f, 0.0f,  // Position 2
									1.0f,  1.0f,        // TexCoord 2
									0.5f,  0.5f, 0.0f,  // Position 3
									1.0f,  0.0f         // TexCoord 3
									};
	vVertices = quadtex;
}
else
{
	static GLfloat pttex[] = { -0.5f,  0.5f, 0.0f,  // Position 0
								-0.5f, -0.5f, 0.0f,  // Position 1
								0.5f, -0.5f, 0.0f,  // Position 2
								0.5f,  0.5f, 0.0f,  // Position 3
								};
	vVertices = pttex;
}
	
	glClear ( GL_COLOR_BUFFER_BIT );
	glUseProgram( programObj );
	if(drawtex)
	{
		glVertexAttribPointer( 0, 3 ,GL_FLOAT,GL_FALSE, 5 * sizeof(GLfloat),vVertices);
	}
	else
	{
		glVertexAttribPointer( 0, 3 ,GL_FLOAT,GL_FALSE, 0,vVertices);
	}
	

	if(drawtex)
	{
		glVertexAttribPointer( pData->textVLoc, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(GLfloat), &vVertices[3]);
		glEnableVertexAttribArray( pData->textVLoc );
	}

	glEnableVertexAttribArray( 0 );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, pData->textureId);

	glUniform1i( pData->textSpLoc, 0 );

	glUniform4f( pData->clrLoc, 1.0f, 1.0f, 1.0f, 0.0f);
	glUniform1i( pData->dwtexLoc, drawtex);

	//获取PointSize 大小返回
	GLfloat pointSizeRange[2];
	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE,pointSizeRange);
	//获取线宽范围
	GLfloat lineSzRange[2];
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE,lineSzRange);
	glDisable(GL_CULL_FACE); 
	GLint num;
	GLint maxsize;
	
	glGetProgramiv( programObj, GL_ACTIVE_UNIFORMS, &num);
	glGetProgramiv(programObj, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxsize);
 
	Matrix ortho;
	const float len = 0.5;
	ortho.ortho(-len,len, -len,len, -1, 1);
	glUniformMatrix4fv(pData->mtxLoc, 1, false, ortho.getMatrix() );
	if(drawtex)
	{
		GLushort indices[] = { 0, 1, 2, 0, 2, 3};
		glDrawElements( drawframe ? GL_LINE_LOOP : GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
	}
	else
	{
		glDrawArrays(GL_POINTS,0,1);
	}
	eglSwapBuffers( es2ctx->eglDisplay, es2ctx->eglSurface );
}

void ES2STDCALL drawText(void *ctx)
{
	glClear(GL_COLOR_BUFFER_BIT);
	ES2Context *es2ctx = static_cast<ES2Context*>(ctx);
	UserData *pData = (UserData *)es2ctx->userData;
	eglSwapBuffers(es2ctx->eglDisplay, es2ctx->eglSurface);

}

void ES2STDCALL keyboard(void *esCtx , unsigned char ch, int x, int y)
{
	if( 'q' == ch )
	{
		drawframe = !drawframe;
	}
	else if ( 'w' == ch )
	{
		drawtex = !drawtex;
	}
	else if ( 'e' == ch )
	{

	}
	else
	{

	}
}

GLuint CreateFontAltasMap()
{
	FMFontManager::getSingleton().createFont("msyh.ttf", 26);

	glGenTextures(1, &picTex);
	glBindTexture(GL_TEXTURE_2D, picTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, FMFontManager::getSingleton().getData());
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return picTex;
}
GLuint CreateTextureAltasMap(void)
{
#if 1
	LoadTexture("test.tga");
	LoadTexture("dll.tga");
	LoadTexture("111.tga");
#else
	LoadTexture("number.tga");
	LoadTexture("rockwall.tga");
	LoadTexture("number.tga");
	LoadTexture("rockwall.tga");
	LoadTexture("number.tga");
	LoadTexture("rockwall.tga");
#endif
	
	glGenTextures(1, &picTex);
	glBindTexture(GL_TEXTURE_2D, picTex);
	GLuint type = CImageAltas::s_IMAGEFORMATE == 4 ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, type, CImageAltas::s_MaxTextureSize, CImageAltas::s_MaxTextureSize, 0, type, GL_UNSIGNED_BYTE, CImageAltas::getSingleton().getIndexBuffer(0));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return picTex;
}
int main(void)
{
	ES2Context es2Ctx;
	UserData userData;
	es2Ctx.userData = &userData;
	ES2RenderManager renderMgr(&es2Ctx);
	renderMgr.CreateWdSystem("OpenGL ES2.0 Demo",600, 600, ES_WINDOW_RGB);

	 char vShaderStr[MAXFILELEN] = { 0 };
	 char fShaderStr[MAXFILELEN] = { 0 };

	strcpy(vShaderStr, FileManager::getSingleton().loadFile("svp.txt"));
	strcpy(fShaderStr, FileManager::getSingleton().loadFile("sfp.txt"));

	if(0 == (userData.programObj = renderMgr.CreateShaderProgram( vShaderStr, fShaderStr)))
		return 0;

	userData.textureId = CreateTextureAltasMap();
						 // CreateFontAltasMap();
	userData.textVLoc = glGetAttribLocation(userData.programObj, "texCrood");
	userData.textSpLoc = glGetUniformLocation( userData.programObj, "texSampler" );
	userData.clrLoc = glGetUniformLocation( userData.programObj, "uColor");
	userData.dwtexLoc = glGetUniformLocation( userData.programObj, "drawtex");
	userData.mtxLoc = glGetUniformLocation(userData.programObj, "vWorldMatrix");
	int error = glGetError();
	renderMgr.SetDisplay(draw);
	//renderMgr.SetDisplay(drawText);
	renderMgr.SetKeyBoard(keyboard);
	//glViewport ( 0, 0, es2Ctx.width, es2Ctx.height );
	error = glGetError();
	renderMgr.MainLoop();
	return 0;
};
#ifndef ES2RENDER_H_H
#define ES2RENDER_H_H
#include "rendercfg.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>


#define ES_WINDOW_RGB			0
#define ES_WINDOW_ALPHA			1
#define ES_WINDOW_DEPTH			2
#define ES_WINDOW_STENCIL		4
#define ES_WINDOW_MULTISAMPLE	8

typedef void (ES2STDCALL*DrawPtr)(void *);
typedef void (ES2STDCALL *KeyBordPtr)(void *,unsigned char,int,int);
typedef void (ES2STDCALL *MousePtr)(void *, int, int);
class RenderWdSystem;
struct ES2Context
{
	void *userData;
	GLint width;
	GLint height;
	EGLNativeWindowType hWnd;//window handle 
	EGLDisplay eglDisplay;
	EGLContext eglContext;
	EGLSurface eglSurface;
	RenderWdSystem *wdSystem;

	void (ES2STDCALL *drawFunc)(void*);
	void (ES2STDCALL *keyFunc)(void*,unsigned char, int, int );
	void (ES2STDCALL *mouseFunc)(void*, int, int);
	ES2Context(void){memset(this,0, sizeof(ES2Context));}
};

class RenderWdSystem
{
public:
	RenderWdSystem(void){}
	virtual ~RenderWdSystem(void){}
	virtual bool CreateWd(ES2Context *ctx, const char *title) = 0;
	virtual bool Render(ES2Context *ctx) = 0;
};


class RenderWdSystemFactory
{
public:
	RenderWdSystemFactory(){}
	virtual ~RenderWdSystemFactory(){}
	virtual RenderWdSystem* Create( void ) = 0;
	static void Destroy(RenderWdSystem *sys) ;
};



class ESRENDER_DLL ES2RenderManager
{
public:
	ES2RenderManager(ES2Context *esctx);
	GLboolean	CreateWdSystem(const char *title, GLint width, GLint height, GLuint flags);
	EGLBoolean	CreateEGLContext(EGLint attribList[]);
	GLuint		LoadShader(GLenum type, const char *shaderSrc );
	GLuint		CreateShaderProgram( const char *vertexShader, const char *fragmentShader );
	void		SetDisplay(DrawPtr funcPtr );
	void		SetKeyBoard(KeyBordPtr keyPtr );
	void		MainLoop( void );
private:
	ES2Context *mpEs2Ctx;
};
#endif
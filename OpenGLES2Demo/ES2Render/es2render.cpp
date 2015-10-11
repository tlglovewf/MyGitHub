#include "es2render.h"
#include "winrenderwdsys.h"
#include <assert.h>
#include <memory>



GLboolean ES2RenderManager::CreateWdSystem(const char *title, GLint width, GLint height, GLuint flags)
{
	if( NULL == mpEs2Ctx )
	{
		return GL_FALSE;
	}
	
	EGLint attribList[] =
	{
		EGL_RED_SIZE,	5,
		EGL_GREEN_SIZE,	6,
		EGL_BLUE_SIZE,	5,
		EGL_ALPHA_SIZE,		(flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
		EGL_DEPTH_SIZE,		(flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
		EGL_STENCIL_SIZE,	(flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
		EGL_SAMPLE_BUFFERS,	(flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
		EGL_NONE
	};

	mpEs2Ctx->width	= width;
	mpEs2Ctx->height	= height;

	std::auto_ptr<RenderWdSystemFactory> winFac(new WinRenderWdSystemFactory);
	mpEs2Ctx->wdSystem = winFac->Create();
	assert(NULL != mpEs2Ctx->wdSystem);

	if(!mpEs2Ctx->wdSystem->CreateWd(mpEs2Ctx,title))
	{
		return GL_FALSE;
	}
	
	if(!CreateEGLContext(attribList))
	{
		return GL_FALSE;
	}

	return GL_TRUE;
}

EGLBoolean ES2RenderManager::CreateEGLContext( EGLint attribList[])
{
	assert(NULL != mpEs2Ctx && NULL != mpEs2Ctx->hWnd);
	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	EGLConfig config;
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

	display = eglGetDisplay(GetDC(mpEs2Ctx->hWnd));
	if( display == EGL_NO_DISPLAY )
	{
		return EGL_FALSE;
	}
	//Initialize EGL
	if( !eglInitialize(display, &majorVersion,&minorVersion))
	{
		return EGL_FALSE;
	}
	//Get config
	if(!eglGetConfigs(display, NULL,0, &numConfigs))
	{
		return EGL_FALSE;
	}
	//Choose config
	if( !eglChooseConfig(display, attribList,&config, 1, &numConfigs))
	{
		return EGL_FALSE;
	}
	//Create a surface
	surface = eglCreateWindowSurface(display, config,(EGLNativeWindowType)mpEs2Ctx->hWnd,NULL);
	if( EGL_NO_SURFACE == surface )
	{
		return EGL_FALSE;
	}

	//Create a GL context
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
	if( EGL_NO_CONTEXT == context)
	{
		return EGL_FALSE;
	}

	//Make the context current
	if( !eglMakeCurrent(display, surface, surface, context))
	{
		return EGL_FALSE;
	}

	mpEs2Ctx->eglDisplay = display;
	mpEs2Ctx->eglSurface = surface;
	mpEs2Ctx->eglContext = context;
	return EGL_TRUE;
}

void ES2RenderManager::MainLoop( void )
{
	assert( NULL != mpEs2Ctx && NULL != mpEs2Ctx->wdSystem);
	mpEs2Ctx->wdSystem->Render( mpEs2Ctx );
}

ES2RenderManager::ES2RenderManager(ES2Context *esctx):mpEs2Ctx(esctx)
{

}

GLuint ES2RenderManager::LoadShader(GLenum type, const char *shaderSrc)
{
	GLuint shader;
	GLint compiled;

	shader = glCreateShader( type );

	if( 0 == shader )
	{
		return 0;
	}

	glShaderSource(shader, 1, &shaderSrc, NULL);
	glCompileShader( shader );
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

	if( !compiled )
	{
		GLint infoLen = 0;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLen );

		if( infoLen > 1 )
		{
			char *infoLog = (char*)malloc(sizeof(char ) * infoLen );

			glGetShaderInfoLog( shader, infoLen, NULL, infoLog );
			free(infoLog);
		}
		glDeleteShader( shader );
		return 0;
	}
	return shader;
}

GLuint ES2RenderManager::CreateShaderProgram( const char *vertexStr, const char *fragmentStr )
{
	assert( NULL != vertexStr && NULL != fragmentStr );

	if( NULL == mpEs2Ctx )
	{
		return false;
	}

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObj;
	GLint linked;

	vertexShader = LoadShader( GL_VERTEX_SHADER, vertexStr );
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragmentStr );

	programObj = glCreateProgram();

	if( 0 == programObj )
		return 0;

	glAttachShader( programObj, vertexShader );
	glAttachShader( programObj, fragmentShader);

	glBindAttribLocation( programObj, 0, "vPosition");
	glLinkProgram( programObj);

	glGetProgramiv( programObj, GL_LINK_STATUS, &linked );

	if( !linked )
	{
		GLint infoLen = 0;
		glGetProgramiv( programObj, GL_INFO_LOG_LENGTH, &infoLen );

		if( infoLen > 1 )
		{
			char *infoLog = (char*)malloc(sizeof(char) * infoLen);

			glGetProgramInfoLog( programObj, infoLen, NULL, infoLog);

			free(infoLog);
		}
		glDeleteShader(programObj);
		return false;
	}
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
	return programObj;
}

void ES2RenderManager::SetDisplay(DrawPtr funcPtr)
{
	assert( NULL != funcPtr && NULL != mpEs2Ctx);
	mpEs2Ctx->drawFunc = funcPtr;
}

void ES2RenderManager::SetKeyBoard(KeyBordPtr keyPtr)
{
	if ( NULL != keyPtr )
	{
		mpEs2Ctx->keyFunc = keyPtr;
	}
}


void RenderWdSystemFactory::Destroy(RenderWdSystem *sys)
{
	if( NULL != sys )
	{
		delete sys;
		sys = NULL;
	}
}

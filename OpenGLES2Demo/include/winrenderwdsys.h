#ifndef ES2WINRENDER_H_H
#define ES2WINRENDER_H_H
#include "es2render.h"
class WinRenderWdSystem : public RenderWdSystem
{
public:
	WinRenderWdSystem(void){}

	virtual bool CreateWd(ES2Context *ctx, const char *title);
	virtual bool Render(ES2Context *ctx );
};

class WinRenderWdSystemFactory : public RenderWdSystemFactory
{
public:
	virtual RenderWdSystem* Create(void);
};
#endif
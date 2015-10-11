#ifndef ESRENDER_CFG_H_H
#define ESRENDER_CFG_H_H

#ifdef ES2RENDER_EXPROT
#define ESRENDER_DLL __declspec(dllexport)
#else
#define ESRENDER_DLL __declspec(dllimport)
#endif

#define ES2STDCALL __stdcall
#endif 
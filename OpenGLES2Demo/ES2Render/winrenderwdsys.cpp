#include "winrenderwdsys.h"

LRESULT WINAPI ESWindowProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) 
{
	LRESULT  lRet = 1; 

	switch (uMsg) 
	{ 
	case WM_CREATE:
		break;

	case WM_PAINT:
		{
			ES2Context *esContext = (ES2Context*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );

			if ( esContext && esContext->drawFunc )
				esContext->drawFunc ( esContext );

			ValidateRect( esContext->hWnd, NULL );
		}
		break;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);             
		break; 

	case WM_CHAR:
		{
			POINT      point;
			ES2Context *esContext = (ES2Context*)(LONG_PTR) GetWindowLongPtr ( hWnd, GWL_USERDATA );

			GetCursorPos( &point );

			if ( esContext && esContext->keyFunc )
				esContext->keyFunc ( esContext, (unsigned char) wParam, 
				(int) point.x, (int) point.y );
		}
		break;
	case WM_LBUTTONDOWN:
		{
			POINT pt;
			ES2Context *es2Context = (ES2Context*)(LONG_PTR)GetWindowLongPtr(hWnd, GWL_USERDATA);
			GetCursorPos(&pt);
			if (es2Context && es2Context->mouseFunc)
				es2Context->mouseFunc(es2Context, pt.x, pt.y);
		}
	default: 
		lRet = DefWindowProc (hWnd, uMsg, wParam, lParam); 
		break; 
	} 

	return lRet; 
}

bool WinRenderWdSystem::Render(ES2Context *ctx)
{
	if(NULL != ctx )
	{
		MSG msg = { 0 };
		int done = 0;
		DWORD lastTime = GetTickCount();

		while (!done)
		{
			int gotMsg = (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
			DWORD curTime = GetTickCount();
			float deltaTime = (float)( curTime - lastTime ) / 1000.0f;
			lastTime = curTime;

			if ( gotMsg )
			{
				if (msg.message==WM_QUIT)
				{
					done=1; 
				}
				else
				{
					TranslateMessage(&msg); 
					DispatchMessage(&msg); 
				}
			}
			else
				SendMessage( ctx->hWnd, WM_PAINT, 0, 0 );

			// Call update function if registered
			// 			if ( ctx->updateFunc != NULL )
			// 				ctx->updateFunc ( ctx, deltaTime );
		}
	}
	return false;
}

bool WinRenderWdSystem::CreateWd(ES2Context *ctx, const char *title)
{
	WNDCLASS wndclass = {0}; 
	DWORD    wStyle   = 0;
	RECT     windowRect;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	int wd = GetSystemMetrics(SM_CXSCREEN);
	int hg = GetSystemMetrics(SM_CYSCREEN);
	const int len = 4;
	wndclass.style         = CS_OWNDC;
	wndclass.lpfnWndProc   = (WNDPROC)ESWindowProc; 
	wndclass.hInstance     = hInstance; 
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); 
	wndclass.lpszClassName = "opengles2.0"; 

	if (!RegisterClass (&wndclass) ) 
		return FALSE; 

	wStyle = WS_VISIBLE | WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION;

	// Adjust the window rectangle so that the client area has
	// the correct number of pixels
	windowRect.left = wd / len;
	windowRect.top =  hg / len;
	windowRect.right = wd  / len + ctx->width;
	windowRect.bottom = hg / len + ctx->height;

	AdjustWindowRect ( &windowRect, wStyle, FALSE );



	ctx->hWnd = CreateWindow(
		"opengles2.0",
		title,
		wStyle,
		windowRect.left,
		windowRect.top,
		ctx->width,
		ctx->height,
		NULL,
		NULL,
		hInstance,
		NULL);

	// Set the ESContext* to the GWL_USERDATA so that it is available to the 
	// ESWindowProc
	SetWindowLongPtr (  ctx->hWnd, GWL_USERDATA, (LONG) (LONG_PTR) ctx );


	//    MoveWindow(esContext->hWnd, wd / 2, hg / 2, wd / 2, hg / 2,FALSE);

	if ( ctx->hWnd == NULL )
		return GL_FALSE;
	ShowWindow ( ctx->hWnd, TRUE );

	return true;
}

RenderWdSystem* WinRenderWdSystemFactory::Create( void )
{
	RenderWdSystem *wdSys = new WinRenderWdSystem;
	return wdSys;
}
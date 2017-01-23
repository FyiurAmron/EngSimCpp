#ifndef WINDOW_H
#define WINDOW_H

#include "Resource.h"
#include <vector>
#include <Windows.h>

typedef double (*ONKEYPROC )( WPARAM keyCode, UINT message, UINT specialKeys, void *parameter );

#define KEYPRESSED_ALT 1<<0
#define KEYPRESSED_SHIFT 1<<1
#define KEYPRESSED_CONTROL 1<<2

class CWindow {
    static WNDCLASSEX wcex;

protected:
    void OnKeyPrivate( WPARAM keyCode, UINT message );

public:
    static HINSTANCE hInstance;
    HWND hWnd;
    int width;
    int height;
    std::vector<ONKEYPROC> onKeyExtProc;
    std::vector<void *> onKeyExtProcParam;

    CWindow( ) {
        ;
    }

    HWND Create( LPCTSTR szClassName, LPCTSTR szTitle, int nWidth, int nHeight );
    HWND Create( LPCTSTR szClassName, LPCTSTR szTitle, int nWidth, int nHeight, int nXpos, int nYpos, HWND hParent );

    static ATOM CreateWndClass( LPCTSTR className, WNDPROC WndProc, WORD menuID );
    static void SetInstance( HINSTANCE hInstance );
    void UpdateSize( );
    void Redraw( void );
    void AddKeyProc( ONKEYPROC proc, void *parameter );

    //static HINSTANCE	hInstance;
};

#endif
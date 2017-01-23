#include "stdafx.h"
#include "CWindow.h"

HINSTANCE CWindow::hInstance;
WNDCLASSEX CWindow::wcex;

ATOM CWindow::CreateWndClass( LPCTSTR className, WNDPROC WndProc, WORD menuID ) {
    wcex.cbSize = sizeof (WNDCLASSEX );

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_WINSIM ) );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH) ( COLOR_WINDOW );
    wcex.lpszMenuName = ( menuID != 0 ) ? MAKEINTRESOURCE( menuID ) : NULL;
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

    return RegisterClassEx( &wcex );
}

///

HWND CWindow::Create( LPCTSTR szClassName, LPCTSTR szTitle, int nWidth, int nHeight ) {
    hWnd = CreateWindowEx( 0, szClassName, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, NULL, NULL, hInstance, this );

    ShowWindow( hWnd, SW_SHOW );
    //DWORD error = GetLastError();
    UpdateWindow( hWnd );
    return hWnd;
}

HWND CWindow::Create( LPCTSTR szClassName, LPCTSTR szTitle, int nWidth, int nHeight, int nXpos, int nYpos, HWND hParent ) {
    hWnd = CreateWindowEx( WS_EX_TOOLWINDOW, szClassName, szTitle, 0 | 0, nXpos, nYpos, nWidth, nHeight, hParent, NULL, hInstance, this );
    ShowWindow( hWnd, SW_SHOW );
    UpdateWindow( hWnd );
    return hWnd;
}

void CWindow::SetInstance( HINSTANCE hInstance ) {
    CWindow::hInstance = hInstance;
}

void CWindow::UpdateSize( ) {
    RECT rect;

    GetClientRect( hWnd, &rect );
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
}

void CWindow::Redraw( void ) {
    InvalidateRect( hWnd, NULL, FALSE );
}

void CWindow::OnKeyPrivate( WPARAM keyCode, UINT message ) {
    UINT specialKeys = 0;

    if ( GetKeyState( VK_CONTROL )&0x0100 ) //jesli ustawiony bit w starszym bajcie - nacisniety przycisk
    {
        specialKeys |= KEYPRESSED_CONTROL;
    }
    if ( GetKeyState( VK_SHIFT )&0x0100 ) //jesli ustawiony bit w starszym bajcie - nacisniety przycisk
    {
        specialKeys |= KEYPRESSED_SHIFT;
    }
    if ( GetKeyState( VK_MENU )&0x0100 ) //jesli ustawiony bit w starszym bajcie - nacisniety przycisk
    {
        specialKeys |= KEYPRESSED_ALT;
    }

    for( int i = onKeyExtProc.size( ) - 1; i >= 0; i-- ) {
        onKeyExtProc[i]( keyCode, message, specialKeys, onKeyExtProcParam[i] );
    }

}

void CWindow::AddKeyProc( ONKEYPROC proc, void *parameter ) {
    onKeyExtProc.push_back( proc );
    onKeyExtProcParam.push_back( parameter );
}
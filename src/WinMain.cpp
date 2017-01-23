// WinSim.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "WinMain.h"
#include "SimConfig.h"

#include "CWindow.h"
#include "CChartWindow.h"
#include "CGraphics.h"
#include "CGDI.h"

#include "CChart.h"
//#include "CNeuralNet.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst; // current instance
wchar_t szTitle[MAX_LOADSTRING]; // The title bar text
wchar_t szWindowClass[MAX_LOADSTRING]; // the main window class name

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK About( HWND, UINT, WPARAM, LPARAM );


//CWindow wMain,wMain2,wChild;
extern void Simulations( );

int APIENTRY WinMain( HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow ) {
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
    UNREFERENCED_PARAMETER( nCmdShow );
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;
    //LCID myLangID;
    // Initialize global strings
    LoadString( hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING );
    LoadString( hInstance, IDC_WINSIM, szWindowClass, MAX_LOADSTRING );

    //CWindow::hInstance = hInstance;
    CWindow::SetInstance( hInstance );
    GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

    //myLangID = GetThreadLocale( );

    srand( (unsigned) time( NULL ) );

    Simulations( );

    hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_WINSIM ) );

    // Main message loop:
    while( GetMessage( &msg, NULL, 0, 0 ) ) {
        if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) ) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    FinishSimulations( );
    GdiplusShutdown( gdiplusToken );

    return (int) msg.wParam;
}

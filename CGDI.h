#ifndef GDI_H
#define GDI_H

#include <GdiPlus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#include "CWindow.h"
#include "CGraphics.h"

class CGDI : public CGraphics {
    PAINTSTRUCT ps;
    HDC hDC;

    Graphics *gp, *gp2;
    Bitmap *bmp;

    Matrix mModel;
    Matrix mViewport;

    float dashPattern[2];

    int doubleBuffer;

    BOOL Init( CWindow *window );
    void DeInit( );

    void UpdateWorldMatrix( );
    Color *curPenColor, *curBrushColor;
    Pen *curPen;
    Brush *curBrush;
    Brush *curTextBrush;
    Color *bgColor;
    FontFamily *curFontF;
    Font *curFont;
public:
    static BOOL PreInit( );
    static void AfterDeInit( );

    CGDI( CWindow *window ) {
        Init( window );
    }

    ~CGDI( ) {
        DeInit( );
    }

    BOOL Begin( int doubleBuffer = TRUE );
    BOOL End( );

    void ClearModelMatrix( );
    void ClearViewMatrix( );

    void SetClipping( float left, float right, float bottom, float top );
    void GetClipping( CRect *rect );
    void SetBgColor( DWORD color );

    void TranslateModel( double x, double y );
    void RotateModel( double degrees );
    void ScaleModel( double x, double y );

    void TranslateView( double x, double y );
    void RotateView( double degrees );
    void ScaleView( double x, double y );

    void DrawLine( float x1, float y1, float x2, float y2 );
    void DrawPoint( float x, float y );
    void DrawPoints( float *x, float *y, int num );
    void DrawLines( float *x, float *y, int num, double x_lim_left, double x_lim_right );
    int DrawLines( float *x, float *y, int num, double x_lim_left, double x_lim_right, double markerX );
    void DrawText( float x, float y, const TCHAR *string );

    void DrawRect( float x1, float y1, float x2, float y2 );
    void Cursor2Pos( int x, int y, CPoint *res );

    void SetLineColor( DWORD color );
    void SetLinePattern( int pattern );
    void SetLine( DWORD color, int pattern );
    void SetFillColor( DWORD color );
    void SetTextColor( DWORD color );
    void SetTextSize( int fontSize );

    int GetFontHeight( );
};

#endif
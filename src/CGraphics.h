#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "CWindow.h"

#define LINE_DASHED 1
#define LINE_SOLID 2

#define RGBA(r, g ,b, a)  ((DWORD) ((((BYTE) (b) | ((WORD) (g) << 8)) | (((DWORD) (BYTE) (r)) << 16)) | (((DWORD) (BYTE) (a)) << 24)))
#define RGB_(r, g ,b)  ((DWORD) ((((BYTE) (b) | ((WORD) (g) << 8)) | (((DWORD) (BYTE) (r)) << 16)) | (((DWORD) (BYTE) (255)) << 24)))
//#define DrawText DrawTextW

class CPoint {
public:
    double x;
    double y;
};

class CRect {
public:
    float left;
    float right;
    float bottom;
    float top;
};

class CGraphics {
protected:

    HDC hDC;

    double scale;
public:
    CWindow *window;

    CGraphics( ) {
        scale = 1.0;
    }

    virtual ~CGraphics( ) { };
    virtual BOOL Init( CWindow *window ) = 0;
    virtual void DeInit( ) = 0;

    virtual BOOL Begin( int doubleBuffer = TRUE ) = 0;
    virtual BOOL End( ) = 0;

    virtual void ClearModelMatrix( ) = 0;
    virtual void ClearViewMatrix( ) = 0;

    virtual void SetClipping( float left, float right, float bottom, float top ) = 0;
    virtual void GetClipping( CRect *rect ) = 0;
    virtual void SetBgColor( DWORD color ) = 0;

    virtual void TranslateModel( double x, double y ) = 0;
    virtual void RotateModel( double degrees ) = 0;
    virtual void ScaleModel( double x, double y ) = 0;

    virtual void TranslateView( double x, double y ) = 0;
    virtual void RotateView( double degrees ) = 0;
    virtual void ScaleView( double x, double y ) = 0;

    virtual void SetScale( double scale );

    virtual void DrawLine( float x1, float y1, float x2, float y2 ) = 0;
    virtual void DrawLines( float *x, float *y, int num, double x_lim_left, double x_lim_right ) = 0;
    virtual int DrawLines( float *x, float *y, int num, double x_lim_left, double x_lim_right, double markerX ) = 0;
    virtual void DrawPoint( float x, float y ) = 0;
    virtual void DrawPoints( float *x, float *y, int num ) = 0;
    virtual void DrawText( float x, float y, const wchar_t *string ) = 0;
    void DrawNumber( float x, float y, int number );
    void DrawNumber( float x, float y, float number, const wchar_t *precision = L".3" );

    virtual void DrawRect( float x1, float y1, float x2, float y2 ) = 0;
    virtual void Cursor2Pos( int x, int y, CPoint *res ) = 0;

    virtual void SetLineColor( DWORD color ) = 0;
    virtual void SetLinePattern( int pattern ) = 0;
    virtual void SetLine( DWORD color, int pattern ) = 0;
    virtual void SetFillColor( DWORD color ) = 0;
    virtual void SetTextColor( DWORD color ) = 0;
    virtual void SetTextSize( int fontSize ) = 0;

    virtual int GetFontHeight( ) = 0;
};

#endif
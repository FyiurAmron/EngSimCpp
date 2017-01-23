#include "stdafx.h"

#include "CGDI.h"

BOOL CGDI::PreInit( ) {
    //Status	res;
    //res = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    //return res==Ok ? TRUE : FALSE;
    return 1;
}

void CGDI::AfterDeInit( ) {
    //GdiplusShutdown(gdiplusToken);
}

BOOL CGDI::Init( CWindow *window ) {

    this->window = window;

    curPen = new Pen( Color( 0, 0, 0 ) );
    curPen->SetDashStyle( DashStyleSolid );
    curBrush = new SolidBrush( Color( 0, 0, 0 ) );
    curTextBrush = new SolidBrush( Color( 0, 0, 0 ) );
    curFontF = new FontFamily( _T( "Arial" ) );
    curFont = new Font( curFontF, 14, FontStyleRegular, UnitPixel );
    bgColor = new Color( RGB_( 255, 255, 255 ) );


    return 1;


    Pen pen( Color( 126, 0, 0, 155 ) );
    SolidBrush brush( Color( 126, 0, 0, 5 ) );
    FontFamily fontf( _T( "Arial" ) );
    Font font( &fontf, 7, FontStyleRegular, UnitPixel );

    Matrix world;
    Matrix model;

    window->UpdateSize( );

    //gp->SetClip(Rect(0,0,200,200));

    //gp->TranslateTransform(50,50);
    //gp->RotateTransform(30);




    world.Scale( window->width, window->height );
    world.Translate( 0.500, 0.450 );



    //model.Translate(0.5,0.5);

    model.Translate( 100, 100 );
    model.Rotate( 30, MatrixOrderAppend );
    //model.Translate(150,150,MatrixOrderAppend);


    model.Multiply( &world );

    gp->SetTransform( &world );
    //gp->Save();

    //model.Translate(150.0f,150.0f);
    //model.Rotate(30.0f);


    gp->TranslateTransform( 0.20, 0.20 );
    gp->RotateTransform( 30 );


    //gp->MultiplyTransform(&model);


    gp->DrawLine( &pen, -0.10f, .0f, 1.0f, 0.0f );
    gp->DrawLine( &pen, -0.00f, -0.10f, 0.0f, 1.0f );
    //gp->DrawString(_T("Uga bugąga teścik"),-1,&font,PointF(0.10,0.10),&brush);

    //gp->TranslateTransform(100,0);






}

void CGDI::DeInit( ) {
    delete curPen;
    delete curBrush;
}

BOOL CGDI::Begin( int doubleBuffer ) {
    hDC = BeginPaint( window->hWnd, &ps );
    this->doubleBuffer = doubleBuffer;
    if ( doubleBuffer ) {
        bmp = new Bitmap( window->width, window->height );
        gp = new Graphics( bmp );
        //gp->DrawRectangle(&Pen(Color(0,0,0)),0,0,window->width,window->height);
        gp->FillRectangle( &SolidBrush( *bgColor ), 0, 0, window->width, window->height );
    } else {
        gp = new Graphics( hDC );
    }
    //window->UpdateSize();

    SetScale( 1 );

    ClearModelMatrix( );
    ClearViewMatrix( );
    //UpdateWorldMatrix();

    return hDC ? TRUE : FALSE;
}

BOOL CGDI::End( ) {
    delete gp;

    if ( doubleBuffer ) {
        gp = new Graphics( hDC );
        gp->DrawImage( bmp, 0, 0 );
        delete gp;
        delete bmp;
    }

    EndPaint( window->hWnd, &ps );
    return 1;
}

void CGDI::ClearModelMatrix( ) {
    mModel.Reset( );
    UpdateWorldMatrix( );
}

void CGDI::ClearViewMatrix( ) {
    mViewport.Reset( );
    mViewport.Translate( 0, window->height, MatrixOrderAppend );
    //mViewport.Scale(1,-1);
    UpdateWorldMatrix( );
}

void CGDI::TranslateModel( double x, double y ) {
    mModel.Translate( x, -y );
    UpdateWorldMatrix( );
}

void CGDI::RotateModel( double degrees ) {
    mModel.Rotate( degrees );
    UpdateWorldMatrix( );
}

void CGDI::ScaleModel( double x, double y ) {
    mModel.Scale( x, y );
    UpdateWorldMatrix( );
}

void CGDI::TranslateView( double x, double y ) {
    mViewport.Translate( x, -y );
    UpdateWorldMatrix( );
}

void CGDI::RotateView( double degrees ) {
    mViewport.Rotate( degrees );
    UpdateWorldMatrix( );
}

void CGDI::ScaleView( double x, double y ) {
    mViewport.Scale( x, y );
    UpdateWorldMatrix( );
}

void CGDI::UpdateWorldMatrix( ) {
    Matrix *m;
    m = mViewport.Clone( );
    //m->Translate(0,window->height,MatrixOrderAppend);
    m->Multiply( &mModel );
    gp->SetTransform( m );
}

void CGDI::DrawLine( float x1, float y1, float x2, float y2 ) {
    curPen->SetWidth( -1/*1.0f/scale*/ );

    gp->DrawLine( curPen, x1, -y1, x2, -y2 );
}

void CGDI::DrawLines( float *x, float *y, int num, double x_lim_left, double x_lim_right ) {
    DrawLines( x, y, num, x_lim_left, x_lim_right, 0 );
}

void CGDI::DrawPoint( float x, float y ) {
    float size = 0.1f;
    DrawLine( x - size, y + size, x + size, y - size );
    DrawLine( x - size, y - size, x + size, y + size );

    //DrawRect(x-size,y+size,x+size,y-size);
}

void CGDI::DrawPoints( float *x, float *y, int num ) {
    for( int i = 0; i < num; i++ ) {
        DrawPoint( x[i], y[i] );
    }
}

int CGDI::DrawLines( float *x, float *y, int num, double x_lim_left, double x_lim_right, double markerX ) {
    ARGB nnn;
    PointF *points;
    int i, k, first_i = 0;
    int markerNum = -1;

    //num=0;
    curPen->SetWidth( -1/*1.0f/scale*/ );
    points = new PointF[num + 4];

    points[0].X = x[0];
    points[0].Y = -y[0];
    for( i = 0, k = 0; i < num; i++ ) {
        if ( x[i] < x_lim_left ) {
            points[k].X = x[i];
            points[k].Y = -y[i];
            continue;
        }
        k++;
        points[k].X = x[i];
        points[k].Y = -y[i];

        if ( x[i] > markerX && markerNum < 0 ) markerNum = i - 1;
        if ( x[i] > x_lim_right ) {
            /*if(i+1<num)
            {
                k++;
                points[k].X=x[i+1];
                points[k].Y=-y[i+1];
            }*/
            break;
        }
    }

    if ( i + 1 <= num + 1 ) {
        k++;
        points[k].X = x[i + 1];
        points[k].Y = -y[i + 1];
    }

    gp->DrawLines( curPen, points, k );

    delete[] points;

    return markerNum;
}

void CGDI::DrawRect( float x1, float y1, float x2, float y2 ) {
    gp->FillRectangle( curBrush, min( x1, x2 ), -max( y1, y2 ), abs( x1 - x2 ), abs( y2 - y1 ) );
    gp->DrawRectangle( curPen, min( x1, x2 ), -max( y1, y2 ), abs( x1 - x2 ), abs( y2 - y1 ) );
}

void CGDI::DrawText( float x, float y, const TCHAR *string ) {
    PointF p( x, -y );
    Matrix temp, mIdentity;
    //FontFamily	fontf(_T("Arial"));
    //Font		font(&fontf,14/1,FontStyleRegular,UnitPixel);
    //SolidBrush	brush(Color(126,0,0,5));

    gp->TransformPoints( CoordinateSpaceDevice, CoordinateSpaceWorld, &p, 1 );
    gp->GetTransform( &temp );

    mIdentity.Reset( );
    //mIdentity.Translate(0,window->height,MatrixOrderAppend);

    gp->SetTransform( &mIdentity );
    //p.Y*=-1;
    gp->DrawString( string, -1, curFont, p, curTextBrush );
    gp->SetTransform( &temp );
}

void CGDI::SetClipping( float left, float right, float bottom, float top ) {
    RectF rect( left, -top, right - left, abs( bottom - top ) );

    gp->SetClip( rect );
}

void CGDI::GetClipping( CRect *rect ) {
    RectF rectf;
    gp->GetClipBounds( &rectf );
    rect->bottom = -rectf.GetBottom( );
    rect->top = -rectf.GetTop( );
    rect->left = rectf.GetLeft( );
    rect->right = rectf.GetRight( );
}

void CGDI::SetLineColor( DWORD color ) {

    curPen->SetColor( color );
}

void CGDI::SetLinePattern( int pattern ) {
    switch( pattern ) {
        case LINE_DASHED:
            dashPattern[0] = 3.0f / scale; //scale;
            dashPattern[1] = dashPattern[0];
            curPen->SetDashStyle( DashStyleCustom );
            curPen->SetDashPattern( dashPattern, 2 );
            break;
        case LINE_SOLID:
        default:
            curPen->SetDashStyle( DashStyleSolid );
            break;
    }
}

void CGDI::SetLine( DWORD color, int pattern ) {
    SetLineColor( color );
    SetLinePattern( pattern );
}

void CGDI::SetFillColor( DWORD color ) {
    if ( curBrush ) delete curBrush;
    curBrush = new SolidBrush( Color( color ) );
}

void CGDI::SetTextColor( DWORD color ) {
    if ( curTextBrush ) delete curTextBrush;
    curTextBrush = new SolidBrush( Color( color ) );
}

void CGDI::SetTextSize( int fontSize ) {
    if ( curFont ) delete curFont;
    curFont = new Font( curFontF, fontSize, FontStyleRegular, UnitPixel );
}

void CGDI::SetBgColor( DWORD color ) {
    if ( bgColor ) delete bgColor;
    bgColor = new Color( color );
}

int CGDI::GetFontHeight( ) {
    return curFont->GetHeight( gp );
}

void CGDI::Cursor2Pos( int x, int y, CPoint *res ) {
    res->x = x;
    res->y = window->height - y;
}

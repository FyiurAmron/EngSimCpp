#include "stdafx.h"
#include "CPolesWindow.h"

bool CPolesWindow::bWndClassRegistered = false;
std::map<HWND, CPolesWindow*> CPolesWindow::allWindows;

HWND CPolesWindow::Create( LPCTSTR szTitle, int nWidth, int nHeight ) {
    if ( !bWndClassRegistered ) RegisterWndClass( );
    ( ( CWindow* )this )->Create( L"SimChartWindow", szTitle, nWidth, nHeight );
    if ( this->hWnd ) {
        allWindows[hWnd] = this;
    }
    return 0;
}

void CPolesWindow::RegisterWndClass( ) {
    CreateWndClass( L"SimChartWindow", StaticChartWndProc, 0 );
    bWndClassRegistered = true;
}

void CPolesWindow::AddChart( ) {
    charts.push_back( new CChart( graph ) );
}

void CPolesWindow::ChangeXScale( double scale, int forceRecalc ) {
    for( size_t i = 0; i < charts.size( ); i++ ) {
        charts[i]->ChangeScale( scale, 0, 0, forceRecalc );
    }
}

void CPolesWindow::SetXPos( double x ) {
    for( size_t i = 0; i < charts.size( ); i++ ) {
        charts[i]->x0 = x;
    }
}

LRESULT CALLBACK CPolesWindow::StaticChartWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    if ( message == WM_CREATE ) {
        SetWindowLong( hWnd, GWL_USERDATA, (LONG) ( (CREATESTRUCT*) lParam )->lpCreateParams );
        //return ((CChartWindow*)lParam)->ChartWndProc(hWnd,message,wParam,lParam);
    }
    return ((CPolesWindow*) GetWindowLong( hWnd, GWL_USERDATA ) )->ChartWndProc( hWnd, message, wParam, lParam );
}

LRESULT CPolesWindow::ChartWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    //CChartWindow *window = allWindows[hWnd];
    int wmId;
    //int wmEvent;

    switch( message ) {

            //********** w WM_CREATE w tej chwili wywolywane sa wszystkie istotne obliczenia **********
            //*****************************************************************************************
        case WM_CREATE:
            graph = new CGDI( ( CWindow* )this ); //inicjalizacja grafiki (GDI)
            graph->SetBgColor( RGB_( 0, 0, 0 ) );

            PostMessage( hWnd, WM_SIZE, 0, 0 );
            srand( (unsigned) time( NULL ) );

            SetTimer( hWnd, 1, 50, NULL );


            break;
        case WM_COMMAND:
            wmId = LOWORD( wParam );
            //wmEvent = HIWORD( wParam );
            // Parse the menu selections:
            switch( wmId ) {
                case IDM_INIT__GDI1:

                    break;
                case IDM_INIT__DIRECTX1:
                    //graph = new CDirectX;
                    break;
                case IDM_ABOUT:

                    //DialogBox(CWindow::hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                case IDM_EXIT:
                    DestroyWindow( hWnd );
                    break;


                default:
                    return DefWindowProc( hWnd, message, wParam, lParam );
            }
            break;


        case WM_KEYDOWN:
            switch( wParam ) {
                case VK_ESCAPE:
                    PostQuitMessage( 0 );
                    break;
                case VK_PRIOR:
                    SetXPos( charts[0]->x0 + charts[0]->xWidth );
                    break;
                case VK_NEXT:
                    SetXPos( charts[0]->x0 - charts[0]->xWidth );
                    break;
            }
            InvalidateRect( hWnd, NULL, FALSE );
            break;
        case WM_PAINT:

            graph->Begin( TRUE );

            //graph->DrawText(prevPosX,wMain.height-prevPosY,sErrorVal);
            graph->SetTextColor( RGBA( 50, 50, 50, 128 ) );
            graph->DrawText( prevPosX, height - prevPosY, L"BOO!!!" );

            for( size_t i = 0; i < charts.size( ); i++ ) {
                charts[i]->SetArea( 0, height * i / charts.size( ), -1, height * ( i + 1 ) / charts.size( ) );
                charts[i]->DrawChart( );
                //charts[i]->SetArea(0,wMain.height*4/charts.size(),-1,wMain.height*5/charts.size());
            }




            //graph->ClearViewMatrix();
            //graph->ClearModelMatrix();
            //graph->SetScale(1);

            //graph->DrawText(200,200,_T("hhhh"));


            graph->End( );



            //delete graph;
            //SendMessage(hWnd,WM_PAINT,0,0);

            //InvalidateRect(hWnd,NULL,FALSE);
            break;
        case WM_LBUTTONDOWN:
            for( size_t i = 0; i < charts.size( ); i++ ) {
                if ( charts[i]->Contains( LOWORD( lParam ), HIWORD( lParam ) ) ) charts[i]->transforming = 1;
            }
            break;
        case WM_RBUTTONDOWN:
            for( size_t i = 0; i < charts.size( ); i++ ) {
                if ( charts[i]->Contains( LOWORD( lParam ), HIWORD( lParam ) ) ) charts[i]->transforming = 1;
            }
            break;
        case WM_RBUTTONUP:
            for( size_t i = 0; i < charts.size( ); i++ ) {
                charts[i]->RewriteSignalsAfterScale( );
            }
            break;
        case WM_MOUSEMOVE:


            if ( wParam == MK_LBUTTON ) {
                for( size_t i = 0; i < charts.size( ); i++ ) {
                    charts[i]->SetPlotOffset( ( prevPosX - LOWORD( lParam ) )*2, 0 );
                    if ( charts[i]->transforming ) charts[i]->SetPlotOffset( 0, ( prevPosY - HIWORD( lParam ) )*2 );
                }
            }

            if ( wParam == MK_RBUTTON ) {
                for( size_t i = 0; i < charts.size( ); i++ ) {
                    charts[i]->ChangeScale( prevPosX - LOWORD( lParam ), 0, true, false );
                    if ( charts[i]->transforming ) charts[i]->ChangeScale( 0, -( prevPosY - HIWORD( lParam ) ), true, false );
                }
            }

            if ( wParam != MK_RBUTTON && wParam != MK_LBUTTON ) {
                for( size_t i = 0; i < charts.size( ); i++ ) {
                    charts[i]->transforming = 0;
                    charts[i]->UpdateMarker( LOWORD( lParam ), HIWORD( lParam ) );
                }


            }

            prevPosX = LOWORD( lParam );
            prevPosY = HIWORD( lParam );
            InvalidateRect( hWnd, NULL, FALSE );

            break;
        case WM_SIZE:
            UpdateSize( );
            scale = min( width, height );
            break;
        case WM_DESTROY:
            delete graph;
            PostQuitMessage( 0 );
            break;
        case WM_TIMER:
            switch( wParam ) {
                case 1:
                    //UpdateWindow(hWnd);
                    //SendMessage(hWnd,WM_PAINT,0,0);
                    //InvalidateRect(hWnd,NULL,FALSE);
                    break;
            }
            break;
        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}
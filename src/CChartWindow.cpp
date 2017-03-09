#include "stdafx.h"
#include "CChartWindow.h"
#include "CSignalsFile.h"
#include "CSignal.h"

bool CChartWindow::bWndClassRegistered = false;
std::map<HWND, CChartWindow*> CChartWindow::allWindows;

HWND CChartWindow::Create( LPCTSTR szTitle, int nWidth, int nHeight ) {
    if ( !bWndClassRegistered ) RegisterWndClass( );
    ( ( CWindow* )this )->Create( L"SimChartWindow", szTitle, nWidth, nHeight );
    if ( this->hWnd ) {
        allWindows[hWnd] = this;
    }
    return 0;
}

void CChartWindow::RegisterWndClass( ) {
    CreateWndClass( L"SimChartWindow", StaticChartWndProc, 0 );
    bWndClassRegistered = true;
}

void CChartWindow::AddChart( ) {
    AddCharts( 1 );
}

void CChartWindow::AddCharts( int num ) {
    CChart *tempCh;
    for( int i = 0; i < num; i++ ) {
        tempCh = new CChart( graph );
        charts.push_back( tempCh );
        //delete tempCh;
    }
}

void CChartWindow::ChangeXScale( double scale, int forceRecalc ) {
    for( size_t i = 0; i < charts.size( ); i++ ) {
        charts[i]->ChangeScale( scale, 0, 0, forceRecalc );
    }
}

void CChartWindow::ChangeYScale( double scale ) {
    for( size_t i = 0; i < charts.size( ); i++ ) {
        charts[i]->ChangeScale( 0, scale, 0, false );
    }
}

void CChartWindow::SetXPos( double x ) {
    for( size_t i = 0; i < charts.size( ); i++ ) {
        //charts[i]->xWidth += x-charts[i]->x0;
        charts[i]->x0 = x;
    }
}

void CChartWindow::SetWidth( double width ) {
    for( size_t i = 0; i < charts.size( ); i++ ) {
        charts[i]->SetWidth( width );
    }
}

void CChartWindow::SetColors( DWORD cBackground, DWORD cGrid, DWORD cLabels ) {
    for( size_t i = 0; i < charts.size( ); i++ ) {
        charts[i]->SetColors( cBackground, cGrid, cLabels );
    }
}

LRESULT CALLBACK CChartWindow::StaticChartWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    if ( message == WM_CREATE ) {
        SetWindowLong( hWnd, GWL_USERDATA, (LONG) ( (CREATESTRUCT*) lParam )->lpCreateParams );
        //return ((CChartWindow*)lParam)->ChartWndProc(hWnd,message,wParam,lParam);
    }
    return ((CChartWindow*) GetWindowLong( hWnd, GWL_USERDATA ) )->ChartWndProc( hWnd, message, wParam, lParam );
}

LRESULT CChartWindow::ChartWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
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
            //srand((unsigned)time(NULL));

            SetTimer( hWnd, 1, 50, NULL );
            Redraw( );

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
            OnKeyPrivate( wParam, message );
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
                case 'Z':
                    if ( GetKeyState( VK_CONTROL )&0x0100 ) {
                        Autoscale( );
                    }
                    break;
                case 'S':
                    if ( GetKeyState( VK_CONTROL )&0x0100 ) {
                        CSignalsFile file;
                        file.SaveSignals( charts, SaveSignalCSVMultipleTimeColumn );
                    }
                    break;
            }

            InvalidateRect( hWnd, NULL, FALSE );
            break;
        case WM_KEYUP:
            switch( wParam ) {
                case VK_CONTROL:
                    for( size_t i = 0; i < charts.size( ); i++ ) {
                        charts[i]->RewriteSignalsAfterScale( );
                    }
                    break;
            }
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


            if ( GetKeyState( VK_F1 )&0x0100 ) {
                wchar_t tempStr[255];
                int line;
                int lineHeigth;

                graph->ClearViewMatrix( );
                graph->ClearModelMatrix( );
                graph->SetScale( 1 );
                graph->SetClipping( 0, graph->window->width, 0, graph->window->height );

                graph->SetTextColor( RGB_( 255, 255, 255 ) );
                graph->SetTextSize( 14 );
                lineHeigth = 20;
                graph->SetFillColor( RGBA( 0, 0, 0, 200 ) );
                graph->SetLineColor( RGBA( 100, 100, 100, 200 ) );

                graph->DrawRect( 20, graph->window->height - 20, 420, graph->window->height - 320 ); //tlo pomocy


                line = 0;
                wsprintf( tempStr, L"F1\tPomoc - wyświetla to okno" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"Spacja\tKontynuacja obliczeń" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"Ctrl+Z\tAutomatyczny zoom" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"Ctrl+S\tZapis przebiegów do pliku" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"PageDown/Up\tPrzemieszczenie wykresów (oś X)" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"LPM+ruch\tPrzemieszczanie wykresów (oś X i Y)" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"PPM+ruch\tSkalowanie wykresów (oś X i Y)" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"Rolka pionowa\tSkalowanie wykresów (oś Y)" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"Ctrl+Rolka pionowa\tSkalowanie wykresów (oś X)" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;
                wsprintf( tempStr, L"Esc\tWyjście z programu" );
                graph->DrawText( 40, graph->window->height - 40 - line*lineHeigth, tempStr );
                line++;

                graph->SetTextSize( 9 );
                wsprintf( tempStr, L"Autor GUI: Daniel Wachowiak" );
                graph->DrawText( 280, graph->window->height - 320 + 15, tempStr );
            }

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

        case WM_MOUSEWHEEL:
            int nWheelMove;
            nWheelMove = (short) HIWORD( wParam );
            if ( GetKeyState( VK_CONTROL )&0x0100 ) //jesli ustawiony bit w starszym bajcie - nacisniety przycisk
            {
                for( size_t i = 0; i < charts.size( ); i++ ) {
                    charts[i]->ChangeScale( -nWheelMove / WHEEL_DELTA * 25, 0, true, false );
                }
            } else {
                for( size_t i = 0; i < charts.size( ); i++ ) {
                    if ( charts[i]->Contains( prevPosX, prevPosY ) ) {
                        charts[i]->ChangeScale( 0, -nWheelMove / WHEEL_DELTA * 25, true, false );
                    }
                }
            }
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
                    if ( orderedRedraw ) {
                        Redraw( );
                        orderedRedraw = 0;
                    }
                    break;
            }
            break;
        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}

void CChartWindow::OrderRedraw( ) {
    orderedRedraw = 1;
}

int CChartWindow::PointOutsideWindowX( double x ) {
    if ( charts.size( ) ) {
        if ( x > charts[0]->x0 + charts[0]->xWidth ) {
            return 1;
        }

        if ( x < charts[0]->x0 ) {
            return -1;
        }
    } else {
        return 2;
    }

    return 0;
}

void CChartWindow::Autoscale( ) {
    for( size_t chart = 0; chart < charts.size( ); chart++ ) {
        charts[chart]->FindMargins( );
        charts[chart]->SetMargins( );
    }

}
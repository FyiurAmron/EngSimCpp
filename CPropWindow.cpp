#include "stdafx.h"
#include "CPropWindow.h"
#include "CFile.h"

bool CPropWindow::bWndClassRegistered = false;
std::map<HWND, CPropWindow*> CPropWindow::allWindows;
CSimulation* CPropWindow::sim = 0;

HWND CPropWindow::Create( LPCTSTR szTitle, int nWidth, int nHeight, CPropertiesGroup *props ) {
    if ( !bWndClassRegistered ) RegisterWndClass( );
    this->props = props;
    ( ( CWindow* )this )->Create( _T( "SimPropsWindow" ), szTitle, nWidth, nHeight );
    if ( this->hWnd ) {
        allWindows[hWnd] = this;
    }

    return 0;
}

void CPropWindow::RegisterWndClass( ) {
    CreateWndClass( _T( "SimPropsWindow" ), StaticPropsWndProc, NULL );
    bWndClassRegistered = true;
}

LRESULT CALLBACK CPropWindow::StaticPropsWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    CPropWindow* temp;
    if ( message == WM_CREATE ) {
        SetWindowLong( hWnd, GWL_USERDATA, (LONG) ( (CREATESTRUCT*) lParam )->lpCreateParams );
        //return ((CChartWindow*)lParam)->ChartWndProc(hWnd,message,wParam,lParam);
    }
    temp = ( (CPropWindow*) GetWindowLong( hWnd, GWL_USERDATA ) );
    return temp->PropsWndProc( hWnd, message, wParam, lParam );
}

LRESULT CPropWindow::PropsWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    //if(hWnd != this->hWnd)
    switch( message ) {
        case WM_CREATE:
            int i;
            wchar_t varTitle[255];

            this->hWnd = hWnd;
            //but.Create(this,10,10,50,20,_T("TestButton"),&CPropWindow::OnButtonOKClick);
            //number.Create(this,10,10,30,20,0);
            //track.SetRange(-1,1);
            //track.Create(this,10,30,200,50,&number);
            inputNums = new CInputNum[props->props.size( )];
            inputTracks = new CTrackBar[props->props.size( )];
            inputTexts = new CText[props->props.size( )];
            for( i = 0; i < props->props.size( ); i++ ) {
                if ( props->props[i].shortcut ) {
                    wsprintf( varTitle, _T( "%s [%c]" ), props->props[i].name.c_str( ), props->props[i].shortcut );
                    inputTexts[i].Create( this, 10, 10 + i * ( 20 + 10 ), 50, 20, varTitle );
                } else {
                    inputTexts[i].Create( this, 10, 10 + i * ( 20 + 10 ), 50, 20, props->props[i].name.c_str( ) );
                }
                inputNums[i].Create( this, 60, 10 + i * ( 20 + 10 ), 50, 20 );
                inputNums[i].AddBuff( props->props[i].pointer );
                inputTracks[i].SetRange( props->props[i].sliderMin, props->props[i].sliderMax );
                inputTracks[i].Create( this, 110, 10 + i * ( 20 + 10 ), 250, 20, &inputNums[i] );
            }
            resetButton.Create( this, 10, 10 + i * ( 20 + 10 ), 50, 20, _T( "Reset" ), &CPropWindow::OnButtonResetClick );
            break;
        case WM_COMMAND:
            if ( CControl::ControlProc( (HWND) lParam, HIWORD( wParam ) ) == ControlChangedValue ) {
                //sim->ResetSimulation();
                //sim->StartSimulation();
            }
            break;
        case WM_HSCROLL:
            CControl::ControlProc( (HWND) lParam, wParam );
            break;
        case WM_KEYDOWN:
            switch( wParam ) {
                case 0x53: //s
                    if ( GetKeyState( VK_CONTROL )&0x0100 ) //jesli ustawiony bit w starszym bajcie - nacisniety przycisk
                    {
                        CFile file;
                        int line = 0;

                        if ( file.SelectAndSave( ) ) {
                            for( i = 0; i < props->props.size( ); i++ ) {
                                file.printf( _T( "%s = %f\r\n" ), props->props[i].name.c_str( ), props->props[i].GetVal( ) );
                            }

                            file.CloseFile( );
                        }
                    }
                    break;
                case 0x4f: //o
                    if ( GetKeyState( VK_CONTROL )&0x0100 ) //jesli ustawiony bit w starszym bajcie - nacisniety przycisk
                    {
                        CFile file;
                        int line = 0;
                        wchar_t name[255];
                        float val;

                        if ( file.SelectAndOpen( ) ) {
                            for( i = 0; i < props->props.size( ); i++ ) {
                                file.scanf( _T( "%s = %f\r\n" ), name, &val );
                                inputNums[i].SetValue( val );
                                //props->props[i].SetVal(val);
                            }

                            file.CloseFile( );
                        }
                    }
                    break;
            }
            break;
        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}

void CPropWindow::RereadValues( ) {
    for( int i = 0; i < props->props.size( ); i++ ) {
        inputNums[i].changedByPC++;
        inputNums[i].SetValue( props->props[i].GetVal( ) );
        //inputTracks[i].Create(this,110,10+i*(20+10),250,20,&inputNums[i]);
    }
}

void CPropWindow::SetParent( CWindow *parent ) {
    parentWidnow = parent;
    parentWidnow->AddKeyProc( (ONKEYPROC) KeyProc, this );
}

void CPropWindow::KeyProc( WPARAM keyCode, UINT message, UINT specialKeys, void *parameter ) {
    CPropWindow* wnd;
    wnd = (CPropWindow*) parameter;

    if ( message == WM_KEYDOWN ) {
        for( int i = 0; i < wnd->props->props.size( ); i++ ) {
            if ( keyCode == wnd->props->props[i].shortcut ) {
                wnd->inputNums[i].SetFocus( SelectAll );
                break;
            }
        }
    }
}
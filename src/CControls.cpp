#include "stdafx.h"
#include "CPropWindow.h"
#include "CControls.h"

std::map<HWND, CControl*> CControl::controls;
HFONT CControl::hCtrlFont;

CControl::CControl( ) {
    if ( !hCtrlFont ) {
        hCtrlFont = CreateFont( -11, 0, 0, 0, FW_DONTCARE, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, L"Ms Shell Dlg" );
    }

    type = ControlTypeUndefined;
}

CControl::~CControl( ) {
    DeleteObject( hCtrlFont );
}

ControlAction CControl::ControlProc( HWND hWnd, int code ) {
    if ( code == BN_CLICKED ) //nacisniecie przycisku
    {
        CButton *p = dynamic_cast<CButton*> ( controls[hWnd] );
        if ( p != NULL ) {
            ( (CPropWindow*) p->parent->*( p->onClickFcn ) )( );
        }
    }

    if ( code == EN_UPDATE ) //aktualizacja EDITa
    {
        CInputNum *p = dynamic_cast<CInputNum*> ( controls[hWnd] );
        if ( p != NULL ) {
            if ( p->changedByPC ) {
                p->changedByPC--;
            } else if ( p->trackbar ) {
                //p->trackbar->SetValue(p->GetValue());
                return ControlChangedValue;
            }
        }
    }

    if ( LOWORD( code ) == TB_THUMBTRACK ) //przesuniecie paska przez usera
    {
        CTrackBar *p = dynamic_cast<CTrackBar*> ( controls[hWnd] );
        if ( p != NULL ) {
            p->UpdateValue( );
        }
    }

    return ControlNothing;
}

LRESULT CControl::CtrlStaticWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    CControl *wnd;
    LRESULT retVal;

    wnd = (CControl*) GetWindowLong( hWnd, GWL_USERDATA );


    retVal = CallWindowProc( wnd->systemProc, wnd->hWnd, message, wParam, lParam );

    switch( wnd->type ) {
        case ControlTypeInputNum:
            switch( message ) {
                case WM_KEYDOWN:
                    if ( wParam == VK_RETURN ) {
                        SetFocus( ( (CPropWindow*) wnd->parent )->parentWidnow->hWnd );
                    }
                    break;
            }
            break;
        default:
            break;
    }

    return retVal;
}

void CControl::OverrideWndProc( ) {
    SetWindowLong( hWnd, GWL_USERDATA, ( LONG )this );
    systemProc = (WNDPROC) GetWindowLong( hWnd, GWL_WNDPROC );
    SetWindowLong( hWnd, GWL_WNDPROC, (LONG) CtrlStaticWndProc );
}

void CInputText::Create( const CWindow *parent, int x, int y, int width, int height ) {
    Create( parent, x, y, width, height, NULL );
}

void CInputText::Create( const CWindow *parent, int x, int y, int width, int height, const wchar_t *initVal ) {
    //HWND hWnd;
    hWnd = CreateWindowEx( 0, L"edit", initVal, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, x, y, width, height, parent->hWnd, ( HMENU )this, parent->hInstance, this );
    if ( hWnd ) {
        controls[hWnd] = this;
        SendMessage( hWnd, WM_SETFONT, (WPARAM) hCtrlFont, FALSE );
        this->parent = parent;
    }
}

void CInputText::SetValue( const wchar_t *str ) {
    SetWindowText( hWnd, str );
}

void CInputText::GetValue( wchar_t *str ) {
    GetWindowText( hWnd, str, 255 );
}

void CInputText::SetValue( ) {
    if ( buff ) SetWindowText( hWnd, buff );
}

void CInputText::GetValue( ) {
    if ( buff ) GetWindowText( hWnd, buff, 255 );
}

void CInputNum::Create( const CWindow *parent, int x, int y, int width, int height ) {
    Create( parent, x, y, width, height, 0 );
}

void CInputNum::Create( const CWindow *parent, int x, int y, int width, int height, double initVal ) {
    //HWND hWnd;
    wchar_t temp[255];

    swprintf( temp, L"%.3f", initVal );

    hWnd = CreateWindowEx( 0, L"edit", temp, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, x, y, width, height, parent->hWnd, ( HMENU )this, parent->hInstance, this );
    if ( hWnd ) {
        controls[hWnd] = this;
        SendMessage( hWnd, WM_SETFONT, (WPARAM) hCtrlFont, FALSE );
        this->parent = parent;
    }

    OverrideWndProc( );
}

void CInputNum::SetValue( double val ) {
    wchar_t temp[255];

    swprintf( temp, L"%.3f", val );
    SetWindowText( hWnd, temp );
    if ( trackbar ) trackbar->SetValue( val );
}

double CInputNum::GetValue( ) {
    double temp_num = ReadValue( );
    if ( buff ) *buff = temp_num;
    return temp_num;
}

double CInputNum::ReadValue( ) {
    wchar_t temp[255];
    float temp_num;

    GetWindowText( hWnd, temp, 255 );
    for( int i = 0;; i++ ) {
        if ( temp[i] == 0 ) break;
        if ( temp[i] == ',' ) temp[i] = '.';
    }
    swscanf( temp, L"%f", &temp_num );
    return temp_num;
}

void CInputNum::SetValue( ) {
    /*wchar_t temp[255];

    swprintf(temp,_T("%.3f"),buff);
    if(buff) SetWindowText(hWnd,temp);*/
    if ( buff ) SetValue( *buff );
    //if(trackbar) trackbar->SetValue(GetValue());
}

void CInputNum::SetFocus( FocusSelect select ) {
    ::SetFocus( hWnd );
    if ( select == SelectAll ) {
        SendMessage( hWnd, EM_SETSEL, 0, -1 );
    }
}

void CInputNum::AddBuff( double *buff ) {
    this->buff = buff;
    SetValue( );
}

void CTrackBar::Create( const CWindow *parent, int x, int y, int width, int height ) {
    Create( parent, x, y, width, height, (double) 0 );
}

void CTrackBar::Create( const CWindow *parent, int x, int y, int width, int height, double initVal ) {
    wchar_t temp[255];

    wsprintf( temp, L"%.3f", initVal );

    hWnd = CreateWindowEx( 0, TRACKBAR_CLASS, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, x, y, width, height, parent->hWnd, ( HMENU )this, parent->hInstance, this );
    if ( hWnd ) {
        controls[hWnd] = this;
        SendMessage( hWnd, WM_SETFONT, (WPARAM) hCtrlFont, FALSE );
        this->parent = parent;
    }
}

void CTrackBar::Create( const CWindow *parent, int x, int y, int width, int height, CInputNum *input ) {
    Create( parent, x, y, width, height, (double) 0 );
    this->input = input;
    input->trackbar = this;
    SetValue( );
}

void CTrackBar::SetValue( double val ) {
    SendMessage( hWnd, TBM_SETPOS, TRUE, Real2Logical( val ) );
    /*if(input)
    {
        input->SetValue(val);
    }*/
}

void CTrackBar::UpdateValue( ) {
    if ( input ) {
        input->SetValue( GetValue( ) );
    }
}

double CTrackBar::GetValue( ) {
    int pos;

    pos = SendMessage( hWnd, TBM_GETPOS, 0, 0 );

    return Logical2Real( pos );
}

void CTrackBar::SetValue( ) {
    //if(buff) SetValue(*buff);
    if ( input ) SetValue( input->ReadValue( ) );
}

double CTrackBar::Logical2Real( int pos ) {
    return (maxVal - minVal ) / ( maxLogicalVal - minLogicalVal )*( pos - minLogicalVal ) + minVal;
}

int CTrackBar::Real2Logical( double val ) {
    return (maxLogicalVal - minLogicalVal ) / ( maxVal - minVal )*( val - minVal ) + minLogicalVal;
}

void CTrackBar::SetLogicalRange( int min, int max ) {
    minLogicalVal = min;
    maxLogicalVal = max;
}

void CTrackBar::SetRange( double min, double max ) {
    minVal = min;
    maxVal = max;
}

void CButton::Create( const CWindow *parent, int x, int y, int width, int height ) {
    Create( parent, x, y, width, height, NULL, NULL );
}

void CButton::Create( const CWindow *parent, int x, int y, int width, int height, const wchar_t *initVal, CTRLFCNPTR onClick ) {
    //HWND hWnd;
    this->onClickFcn = onClick;
    hWnd = CreateWindowEx( 0, L"button", initVal, WS_CHILD | WS_VISIBLE | WS_TABSTOP, x, y, width, height, parent->hWnd, ( HMENU )this, parent->hInstance, this );
    if ( hWnd ) {
        controls[hWnd] = this;
        SendMessage( hWnd, WM_SETFONT, (WPARAM) hCtrlFont, FALSE );
        this->parent = parent;
    }
}

void CText::Create( const CWindow *parent, int x, int y, int width, int height ) {
    Create( parent, x, y, width, height, NULL );
}

void CText::Create( const CWindow *parent, int x, int y, int width, int height, const wchar_t *initVal ) {
    //HWND hWnd;
    hWnd = CreateWindowEx( 0, L"static", initVal, WS_CHILD | WS_VISIBLE | ES_LEFT, x, y, width, height, parent->hWnd, ( HMENU )this, parent->hInstance, this );
    if ( hWnd ) {
        controls[hWnd] = this;
        SendMessage( hWnd, WM_SETFONT, (WPARAM) hCtrlFont, FALSE );
        this->parent = parent;
    }

}

void CText::SetValue( const wchar_t *str ) {
    SetWindowText( hWnd, str );
}

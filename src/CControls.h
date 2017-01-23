#ifndef CONTROLS_H
#define CONTROLS_H

#include <map>
//#include "CWindow.h"
#include <CommCtrl.h>

class CPropWindow;
class CTrackBar;

typedef void(CPropWindow::*CTRLFCNPTR )( );

enum ControlAction {
    ControlNothing, ControlChangedValue
};

enum FocusSelect {
    NoSelect,
    SelectAll
};

enum ControlType {
    ControlTypeUndefined, ControlTypeInputNum, ControlTypeInputText, ControlTypeButton, ControlTypeSlider
};

class CControl {
protected:
    HWND hWnd;
    const CWindow *parent;
    ControlType type;
    WNDPROC systemProc;

    void OverrideWndProc( );

public:
    CControl( );
    ~CControl( );
    virtual void Create( const CWindow *parent, int x, int y, int width, int height ) = 0;



    static std::map<HWND, CControl*> controls;
    static ControlAction ControlProc( HWND hWnd, int code );

    static LRESULT CtrlStaticWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

    static HFONT hCtrlFont;
};

class CButton : public CControl {
public:
    CTRLFCNPTR onClickFcn;

    CButton( ) {
        type = ControlTypeButton;
    }
    void Create( const CWindow *parent, int x, int y, int width, int height );
    void Create( const CWindow *parent, int x, int y, int width, int height, const wchar_t *text, CTRLFCNPTR onClick );
};

class CInputText : public CControl {
public:
    wchar_t *buff;

    CInputText( ) {
        buff = 0;
        type = ControlTypeInputText;
    }

    ~CInputText( ) {
        buff = 0;
    }
    void Create( const CWindow *parent, int x, int y, int width, int height );
    void Create( const CWindow *parent, int x, int y, int width, int height, const wchar_t *initVal );
    void SetValue( const wchar_t *str );
    void GetValue( wchar_t *str );
    void SetValue( );
    void GetValue( );
};

class CInputNum : public CControl {
public:
    double *buff;
    CTrackBar *trackbar;
    int changedByPC; //jesli wartosc zostala zakutalizowana przez program zmienna przyjmuje wartosc wieksza od zera - zapobiega resetowaniu symulacji przy zmianie nie przez uzytkownika

    CInputNum( ) : CControl( ) {
        buff = 0;
        trackbar = 0;
        changedByPC = 0;
        type = ControlTypeInputNum;
    }

    ~CInputNum( ) {
        buff = 0;
    }
    void Create( const CWindow *parent, int x, int y, int width, int height );
    void Create( const CWindow *parent, int x, int y, int width, int height, double initVal );
    void SetValue( double val );
    void SetValue( );
    void SetFocus( FocusSelect select );
    double GetValue( );
    double ReadValue( );
    void AddBuff( double *buff );
};

class CTrackBar : public CControl {
    CInputNum *input;
    double minVal, maxVal;
    int minLogicalVal, maxLogicalVal;

    double Logical2Real( int pos );
    int Real2Logical( double val );
public:
    double *buff;

    CTrackBar( ) : CControl( ) {
        minLogicalVal = 0;
        maxLogicalVal = 100;
        minVal = 0;
        maxVal = 100;
        buff = 0;
        input = 0;
        type = ControlTypeSlider;
    }

    ~CTrackBar( ) {
        buff = 0;
    }
    void Create( const CWindow *parent, int x, int y, int width, int height );
    void Create( const CWindow *parent, int x, int y, int width, int height, double initVal );
    void Create( const CWindow *parent, int x, int y, int width, int height, CInputNum *input );
    void SetValue( double val );
    void SetValue( );
    void UpdateValue( );
    void SetLogicalRange( int min, int max );
    void SetRange( double min, double max );
    double GetValue( );
};

class CText : public CControl {
public:
    void Create( const CWindow *parent, int x, int y, int width, int height );
    void Create( const CWindow *parent, int x, int y, int width, int height, const wchar_t *initVal );
    void SetValue( const wchar_t *str );
};

#endif
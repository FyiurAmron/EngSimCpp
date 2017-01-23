#ifndef PROPS_WND_H
#define PROPS_WND_H

#include <map>
#include "CWindow.h"
#include "CControls.h"
#include "CSimulation.h"
#include "CProperties.h"

class CPropWindow : public CWindow {
    /*CButton but;
    CTrackBar track;
    CInputNum number;*/

    CInputNum *inputNums;
    CTrackBar *inputTracks;
    CText *inputTexts;
    CButton resetButton;

    void OnButtonResetClick( ) {
        if ( sim ) {
            sim->ResetSimulation( );
            //sim->StartSimulation();
        }
    }
public:
    CPropertiesGroup *props;
    static CSimulation *sim;
    CWindow *parentWidnow;

    HWND Create( LPCTSTR szTitle, int nWidth, int nHeight, CPropertiesGroup *props );
    static bool bWndClassRegistered;
    static void RegisterWndClass( );
    LRESULT PropsWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    static LRESULT CALLBACK StaticPropsWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    static std::map<HWND, CPropWindow*> allWindows;
    void RereadValues( );
    void SetParent( CWindow *parent );
    static void KeyProc( WPARAM keyCode, UINT message, UINT specialKeys, void *prameter );
};

#endif
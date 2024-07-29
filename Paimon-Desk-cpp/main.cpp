#include <tchar.h>
#include <windows.h>

#include "framework.h"
#include "Paimon-Desk-cpp.h"
#include "Entity.cpp"
#include "Brain.cpp"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text

struct Component {
    const wchar_t* rightIdleBody = L"C:\\Users\\davis\\OneDrive\\Documents\\Code Projects\\paimon-desk-c\\entity\\paimonRight.gif";
    const wchar_t* leftIdleBody = L"C:\\Users\\davis\\OneDrive\\Documents\\Code Projects\\paimon-desk-c\\entity\\paimonLeft.gif";
};

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance, Entity*);
BOOL                InitInstance(Entity*, HINSTANCE, int);
LRESULT CALLBACK    WindowsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    Component paimon;
    Entity creature = Entity(
        szTitle,
        paimon.rightIdleBody,
        paimon.leftIdleBody,
        hInstance
    );

    MyRegisterClass(hInstance, &creature);

    if (!InitInstance (&creature, hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PAIMONDESKCPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance, Entity* creature)
{

    creature->lpfnWndProc = WindowsProc;
    //WNDCLASSEXW wcex;

    //wcex.cbSize = sizeof(WNDCLASSEX);

    /*wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAIMONDESKCPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PAIMONDESKCPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));*/

    return RegisterClassExW(creature);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(Entity* entity, HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   
   entity->setHandle();

   if (!entity->hWindow)
   {
      return FALSE;
   }

   entity->materializeEntity();

   SetWindowPos(entity->hWindow,
       HWND_TOPMOST,
       0, 0, 0, 0,
       SWP_NOMOVE | SWP_NOSIZE
   );

   ShowWindow(entity->hWindow, nCmdShow);
   UpdateWindow(entity->hWindow);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WindowsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    Entity* pEntity = reinterpret_cast<Entity*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (pEntity)
        return pEntity->onAlive(hwnd, message, wParam, lParam);
    return DefWindowProc(hwnd, message, wParam, lParam);
}

#pragma once
#include <Windows.h>
#include <winuser.h>
#include "./Brain.cpp"
#include "./Gif.cpp"
#include "./Canvas.cpp"
#include "./resource.h"

class Entity : public WNDCLASSEXW {
public:
    typedef WNDCLASSEX super;
    HWND hWindow;

    Brain* entityBrain;
    Canvas* windowCanvas;

    Gif* rightBody;
    Gif* leftBody;

    Gif* currentBody;

    int totalFrame = 0;
    int frameIdx = 0;
    UINT move = 0;
    UINT TIMER_ANIM = 1;

    Entity(
        const WCHAR className[],
        const wchar_t* rightIdleBody,
        const wchar_t* leftIdleBody,
        HINSTANCE hInstance) : WNDCLASSEX()
    {
        this->lpszClassName = className;
        this->hInstance = hInstance;
        this->hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
        this->hCursor = LoadCursor(NULL, IDC_ARROW);
        this->style = CS_HREDRAW | CS_VREDRAW;
        this->cbSize = sizeof(WNDCLASSEX);

        this->rightBody = new Gif(rightIdleBody);
        this->leftBody = new Gif(leftIdleBody);
        this->currentBody = this->rightBody;
    }

    bool materializeEntity() {
        this->entityBrain = new Brain();

        HRESULT hr = CoInitializeEx(
            NULL,
            COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
        );
        if (hr != S_OK) return false;

        windowCanvas = new Canvas(&this->hWindow);
        windowCanvas->initializeCanvas();
        this->rightBody->initilizeGif();
        this->leftBody->initilizeGif();

        return true;
    }

    void setHandle() {
        this->hWindow = createHandle();
        if (this->hWindow == NULL) return;
        SetWindowLongPtr(this->hWindow, GWLP_USERDATA, (LONG_PTR)this);
    }

    HWND createHandle() {
        return CreateWindowEx(
            WS_EX_NOREDIRECTIONBITMAP, //expmn 0x00200000L
            this->lpszClassName,
            this->lpszClassName,
            WS_POPUP | WS_VISIBLE,
            300, 200, 150, 150,
            nullptr,
            nullptr,
            this->hInstance,
            nullptr
        );
    }

    RECT getSelfBody(HWND hwnd) {
        RECT body;
        GetWindowRect(hwnd, &body);
        return body;
    }

    void animateEntity(HWND hwnd) {
        ComPtr<ID2D1Bitmap> frame =
            this->currentBody->getBitmapFrameAt(
                frameIdx,
                windowCanvas->d2dContext
            );

        windowCanvas->draw(frame);

        frameIdx = (frameIdx + 1) % this->currentBody->getFrameCount(); //this.totalFrame ;
        setTimer(hwnd, 123, this->currentBody->getFrameDelay());
    }

    void moveEntity(HWND hwnd) {
        POINT targ = entityBrain->getTargetPosition(hwnd, 1);
        RECT bodyPos = entityBrain->getBodyPosition(hwnd);
        POINT moveTo = entityBrain->calculatePosition(targ, bodyPos);
        SetWindowPos(
            hwnd,
            HWND_TOPMOST,
            moveTo.x,
            moveTo.y,
            0, 0,
            SWP_NOSIZE | SWP_NOZORDER
        );
    }

    void disMaterializeEntity() {
        windowCanvas->destroy();
        rightBody->erase();
        leftBody->erase();
        free(entityBrain);
        free(windowCanvas);
    }

    void setTimer(HWND hwnd, UINT timerId, UINT value) {
        SetTimer(hwnd, timerId, value, NULL);
    }

    LRESULT onAlive(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc(hwnd, message, wParam, lParam);
            if (hit == HTCLIENT) hit = HTCAPTION;
            return hit;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            disMaterializeEntity();
            return 0;
        case WM_PAINT:
            animateEntity(hwnd);
            moveEntity(hwnd);
            ValidateRect(hwnd, NULL);
            return 0;
        case WM_DISPLAYCHANGE:
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_TIMER:
            KillTimer(hwnd, 123);
            setTimer(hwnd, 123, this->currentBody->getFrameDelay());
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
};


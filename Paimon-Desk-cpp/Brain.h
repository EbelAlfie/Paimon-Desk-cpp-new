#pragma once
#include <Windows.h>
#include <math.h>

class Brain {
public:
    Brain() { }

    void chooseAction() {

    }

    POINT getTargetPosition(HWND hwnd, int mode) {
        POINT lpPoint;
        switch (mode) {
        case 1:
            GetCursorPos(&lpPoint);
            break;
        case 2:
            break;
        default:
            break;
        }
        return lpPoint;
    }

    RECT getBodyPosition(HWND hwnd) {
        RECT body;
        GetWindowRect(hwnd, &body);
        MapWindowPoints(hwnd, GetParent(hwnd), (LPPOINT)&body, 2);
        return body;
    }

    bool determineOrientation(HWND hwnd, int mode) {
        POINT target = getTargetPosition(hwnd, mode);
        RECT self = getBodyPosition(hwnd);
        return true; //sementara 
    }

    POINT calculatePosition(POINT target, RECT self) {
        //pivotnya top left (0,0)
        int centerX = abs(self.right - self.left) / 2;
        int centerY = abs(self.bottom - self.top) / 2;
        int targX = target.x - centerX;
        int targY = target.y - centerY;
        //return { self.left, self.top };
        return { centerX + targX, centerY + targY };
    }
};


#include <Windows.h>
#include <shellapi.h>
#include <iostream>


// tray bar menu options identifiers
#define EXIT_ID 1001 // "exit" button ID
#define OPEN_TOOL_MANAGER_ID 1002 // "open tool manager" button ID

// tray bar menu icon options
NOTIFYICONDATA icon_data = {};

bool alwaysOnTopActivated = false;


// this function creates the popup menu from the tray bar menu icon
void CreateTrayMenu(HWND hwnd) {
    HMENU popupMenu = CreatePopupMenu();
    AppendMenu(popupMenu, MF_STRING, EXIT_ID, "Exit"); // create "Exit" option
    AppendMenu(popupMenu, MF_STRING, OPEN_TOOL_MANAGER_ID, "Open Tool Manager"); // create "Open Tool Manager" option

    // make the "exit" button and the "Open Tool Manager" button clickable
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(popupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
}

// this function assigns actions to tray bar menu options 
void HandleTrayMenu(HWND hwnd, WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case EXIT_ID: // "Exit" button
            DestroyWindow(hwnd);
            break;
        case OPEN_TOOL_MANAGER_ID: // "Open Tool Manager" button
            // WILL BE IMPLEMENTED AS SOON AS POSSIBLE
            break;
    }
}

// called when the user press win + shift + t
void SetWindowAlwaysOnTop(HWND hwnd) {
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    alwaysOnTopActivated = true;
    InvalidateRect(hwnd, NULL, TRUE);
}

// called when the user press win + shift + y
void RemoveWindowAlwaysOnTop(HWND hwnd) { 
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    alwaysOnTopActivated = false;
    InvalidateRect(hwnd, NULL, TRUE);
}

// background function of the thread
DWORD WINAPI BackgroundThread(LPVOID lpParam) {
    // background code
    while (true) {
        // enable Always On Top to the current window    
        if ((GetAsyncKeyState(VK_LWIN) & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState('T') & 0x8000)) { // WIN + CTRL + T
            HWND hwnd = GetForegroundWindow();
            if (hwnd != NULL) {
                SetWindowAlwaysOnTop(hwnd);
                std::cout << "Window set as Always On Top." << std::endl;
            } else {
                std::cout << "Unable to get active window handle." << std::endl;
            }
        }
        // disable Always On Top to the current window  
        if ((GetAsyncKeyState(VK_LWIN) & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState('Y') & 0x8000)) { // WIN + CTRL + Y
            HWND hwnd = GetForegroundWindow();
            if (hwnd != NULL) {
                RemoveWindowAlwaysOnTop(hwnd);
                std::cout << "Always On Top mode disabled." << std::endl;
            } else {
                std::cout << "Unable to get active window handle." << std::endl;
            }
        }
        // little delay (100ms)
        Sleep(100);
    }

    return 0;
}

// called from main() when setting window class wc (to create the tray bar menu icon)
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            icon_data.cbSize = sizeof(NOTIFYICONDATA);
            icon_data.hWnd = hwnd;
            icon_data.uID = 1;
            icon_data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            icon_data.uCallbackMessage = WM_USER + 1;

            // load the icon from the folder /graphics
            icon_data.hIcon = (HICON)LoadImage(NULL, "graphics\\traybar_icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
            if (!Shell_NotifyIcon(NIM_ADD, &icon_data)) {
                DWORD error = GetLastError();
                std::cerr << "Error while loading the tray bar menu icon: " << error << std::endl;
            }
            break;
        case WM_USER + 1:
            switch (lParam) {
                case WM_RBUTTONDOWN:
                case WM_CONTEXTMENU:
                    CreateTrayMenu(hwnd); // call the function to create the tray menu icon
                    break;
            }
            break;
        case WM_COMMAND:
            HandleTrayMenu(hwnd, wParam); // call the function to handle actions from the tray menu icon
            break;
        case WM_PAINT:
            if (alwaysOnTopActivated) {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                RECT rect;
                GetClientRect(hwnd, &rect);
                // Set the color for the border
                HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0)); // Red color
                // Draw a thicker border
                FrameRect(hdc, &rect, hBrush); // Draw the outer border
                InflateRect(&rect, -5, -5); // Inflate the rectangle to create a thicker border
                FrameRect(hdc, &rect, hBrush); // Draw the inner border
                DeleteObject(hBrush);
                EndPaint(hwnd, &ps);
            }
            break;
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &icon_data);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
    HANDLE hThread; // to manage the thread
    hThread = CreateThread(NULL, 0, BackgroundThread, NULL, 0, NULL); // create the thread in background (call the function BackgroundThread())
    if (hThread == NULL) {
        std::cerr << "Error while creating the thread...." << GetLastError() << std::endl;
        return 1;
    }

    // window class manage the tray bar menu icon and a lot of stuff 
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance; // GetModuleHandle(nullptr);
    wc.lpszClassName = "TrayWindowClass";
    RegisterClass(&wc);

    // place the icon in the traybar menu (creating an invisible window)
    HWND g_hWnd = CreateWindow(wc.lpszClassName, "Tray Window", 0, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_hWnd) {
        std::cerr << "Unable to create the icon..." << std::endl;
        return 1;
    }

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0) {
        if (bRet == -1) {
            std::cerr << "Errore durante la ricezione del messaggio." << std::endl;
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}
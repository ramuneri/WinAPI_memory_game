#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>

// Window event handler.
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

const TCHAR szClassName[] = _T("MemoryGameWindow");

int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int nCmdShow)
{
    WNDCLASSEX wincl = {};
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_HREDRAW | CS_VREDRAW;

    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassEx(&wincl))
    {
        MessageBox(NULL, _T("Could not register the window."),
                   _T("Error"), MB_OK | MB_ICONERROR);
        return 1;
    }

    // Fixed-size window.
    DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION |
                        WS_SYSMENU | WS_MINIMIZEBOX;

    // Set the usable area inside the window to 430 x 525 pixels.
    RECT windowRect = {0, 0, 430, 525};
    AdjustWindowRectEx(&windowRect, windowStyle, FALSE, 0);

    HWND hwnd = CreateWindowEx(
        0,
        szClassName,
        _T("Memory Game"),
        windowStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        hThisInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        MessageBox(NULL, _T("Could not create the window."),
                   _T("Error"), MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG messages = {};
    int result;

    while ((result = GetMessage(&messages, NULL, 0, 0)) > 0)
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    if (result == -1)
        return 1;

    return (int)messages.wParam;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message,
                                 WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            HINSTANCE instance =
                ((LPCREATESTRUCT)lParam)->hInstance;

            // Attempt counter.
            CreateWindowEx(
                0,
                _T("STATIC"),
                _T("Attempts: 0"),
                WS_CHILD | WS_VISIBLE,
                20, 20, 200, 25,
                hwnd,
                (HMENU)(INT_PTR)200,
                instance,
                NULL
            );

            // Four rows and four columns of cards.
            for (int i = 0; i < 16; ++i)
            {
                int row = i / 4;
                int column = i % 4;

                CreateWindowEx(
                    0,
                    _T("BUTTON"),
                    _T("?"),
                    WS_CHILD | WS_VISIBLE |
                    WS_TABSTOP | BS_PUSHBUTTON,
                    20 + column * 100,
                    60 + row * 100,
                    90, 90,
                    hwnd,
                    (HMENU)(INT_PTR)(100 + i),
                    instance,
                    NULL
                );
            }

            // New Game button.
            CreateWindowEx(
                0,
                _T("BUTTON"),
                _T("New Game"),
                WS_CHILD | WS_VISIBLE |
                WS_TABSTOP | BS_PUSHBUTTON,
                20, 470, 140, 35,
                hwnd,
                (HMENU)(INT_PTR)201,
                instance,
                NULL
            );

            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

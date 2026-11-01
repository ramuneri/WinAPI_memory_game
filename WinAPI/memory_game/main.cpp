#include <windows.h>
#include <string>

// IDs used by our controls and timer.
const int CARD_FIRST_ID = 100;
const int STATUS_ID = 200;
const int NEW_GAME_ID = 201;
const UINT_PTR HIDE_TIMER_ID = 1;

// Function types matching the DLL exports.
typedef void (*NewGameFunction)(unsigned int);
typedef int (*FlipCardFunction)(int);
typedef void (*HideCardsFunction)();
typedef int (*GetCardValueFunction)(int);
typedef int (*IsCardVisibleFunction)(int);
typedef int (*GetAttemptsFunction)();
typedef int (*GetMatchedPairsFunction)();

// DLL handle and function pointers.
HMODULE gameDLL = NULL;

NewGameFunction dllNewGame = NULL;
FlipCardFunction dllFlipCard = NULL;
HideCardsFunction dllHideCards = NULL;
GetCardValueFunction dllGetCardValue = NULL;
IsCardVisibleFunction dllIsCardVisible = NULL;
GetAttemptsFunction dllGetAttempts = NULL;
GetMatchedPairsFunction dllGetMatchedPairs = NULL;

// Window controls.
HWND cardButtons[16] = {};
HWND statusLabel = NULL;

bool waitingToHide = false;

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

// Load the DLL from the same folder as the executable.
bool LoadGameDLL()
{
    char executablePath[MAX_PATH] = {};

    DWORD length = GetModuleFileNameA(
        NULL, executablePath, MAX_PATH
    );

    if (length == 0 || length >= MAX_PATH)
    {
        MessageBoxA(
            NULL,
            "Could not find the application folder.",
            "Error",
            MB_OK | MB_ICONERROR
        );
        return false;
    }

    std::string path(executablePath);
    std::string::size_type separator = path.find_last_of("\\/");

    if (separator == std::string::npos)
        return false;

    path = path.substr(0, separator + 1) + "MemoryGameDLL.dll";

    gameDLL = LoadLibraryA(path.c_str());

    if (gameDLL == NULL)
    {
        char errorText[1024];

        wsprintfA(
            errorText,
            "Could not load MemoryGameDLL.dll.\n\n"
            "Keep it beside memory_game.exe.\n"
            "Both projects must use the same compiler architecture.\n\n"
            "Windows error: %lu",
            GetLastError()
        );

        MessageBoxA(
            NULL, errorText, "DLL error",
            MB_OK | MB_ICONERROR
        );

        return false;
    }

    dllNewGame = reinterpret_cast<NewGameFunction>(
        GetProcAddress(gameDLL, "NewGame")
    );

    dllFlipCard = reinterpret_cast<FlipCardFunction>(
        GetProcAddress(gameDLL, "FlipCard")
    );

    dllHideCards = reinterpret_cast<HideCardsFunction>(
        GetProcAddress(gameDLL, "HideCards")
    );

    dllGetCardValue = reinterpret_cast<GetCardValueFunction>(
        GetProcAddress(gameDLL, "GetCardValue")
    );

    dllIsCardVisible = reinterpret_cast<IsCardVisibleFunction>(
        GetProcAddress(gameDLL, "IsCardVisible")
    );

    dllGetAttempts = reinterpret_cast<GetAttemptsFunction>(
        GetProcAddress(gameDLL, "GetAttempts")
    );

    dllGetMatchedPairs = reinterpret_cast<GetMatchedPairsFunction>(
        GetProcAddress(gameDLL, "GetMatchedPairs")
    );

    if (!dllNewGame || !dllFlipCard || !dllHideCards ||
        !dllGetCardValue || !dllIsCardVisible ||
        !dllGetAttempts || !dllGetMatchedPairs)
    {
        MessageBoxA(
            NULL,
            "The DLL is missing required game functions.\n"
            "Rebuild MemoryGameDLL and copy the new DLL here.",
            "DLL error",
            MB_OK | MB_ICONERROR
        );

        FreeLibrary(gameDLL);
        gameDLL = NULL;
        return false;
    }

    return true;
}

// Refresh all card captions and the score display.
void RefreshBoard()
{
    for (int i = 0; i < 16; ++i)
    {
        if (dllIsCardVisible(i))
        {
            char text[16];
            wsprintfA(text, "%d", dllGetCardValue(i));
            SetWindowTextA(cardButtons[i], text);
        }
        else
        {
            SetWindowTextA(cardButtons[i], "?");
        }
    }

    char statusText[100];

    wsprintfA(
        statusText,
        "Attempts: %d       Pairs: %d / 8",
        dllGetAttempts(),
        dllGetMatchedPairs()
    );

    SetWindowTextA(statusLabel, statusText);
}

// Cancel any pending card hiding and start again.
void StartNewGame(HWND hwnd)
{
    KillTimer(hwnd, HIDE_TIMER_ID);
    waitingToHide = false;

    dllNewGame(static_cast<unsigned int>(GetTickCount()));
    RefreshBoard();
}

int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int nCmdShow)
{
    if (!LoadGameDLL())
        return 1;

    const char className[] = "MemoryGameWindow";

    WNDCLASSEXA wincl = {};
    wincl.cbSize = sizeof(WNDCLASSEXA);
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = className;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_HREDRAW | CS_VREDRAW;
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassExA(&wincl))
    {
        MessageBoxA(
            NULL, "Could not register the window.",
            "Error", MB_OK | MB_ICONERROR
        );

        FreeLibrary(gameDLL);
        return 1;
    }

    DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION |
                        WS_SYSMENU | WS_MINIMIZEBOX;

    RECT windowRect = {0, 0, 430, 525};
    AdjustWindowRectEx(&windowRect, windowStyle, FALSE, 0);

    HWND hwnd = CreateWindowExA(
        0,
        className,
        "Memory Game",
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
        MessageBoxA(
            NULL, "Could not create the window.",
            "Error", MB_OK | MB_ICONERROR
        );

        FreeLibrary(gameDLL);
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

    FreeLibrary(gameDLL);
    gameDLL = NULL;

    if (result == -1)
        return 1;

    return static_cast<int>(messages.wParam);
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

            HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

            statusLabel = CreateWindowExA(
                0, "STATIC", "",
                WS_CHILD | WS_VISIBLE,
                20, 20, 390, 25,
                hwnd,
                (HMENU)(INT_PTR)STATUS_ID,
                instance, NULL
            );

            SendMessage(
                statusLabel, WM_SETFONT, (WPARAM)font, TRUE
            );

            for (int i = 0; i < 16; ++i)
            {
                int row = i / 4;
                int column = i % 4;

                cardButtons[i] = CreateWindowExA(
                    0, "BUTTON", "?",
                    WS_CHILD | WS_VISIBLE |
                    WS_TABSTOP | BS_PUSHBUTTON,
                    20 + column * 100,
                    60 + row * 100,
                    90, 90,
                    hwnd,
                    (HMENU)(INT_PTR)(CARD_FIRST_ID + i),
                    instance, NULL
                );

                SendMessage(
                    cardButtons[i],
                    WM_SETFONT, (WPARAM)font, TRUE
                );
            }

            HWND newGameButton = CreateWindowExA(
                0, "BUTTON", "New Game",
                WS_CHILD | WS_VISIBLE |
                WS_TABSTOP | BS_PUSHBUTTON,
                20, 470, 140, 35,
                hwnd,
                (HMENU)(INT_PTR)NEW_GAME_ID,
                instance, NULL
            );

            SendMessage(
                newGameButton, WM_SETFONT, (WPARAM)font, TRUE
            );

            StartNewGame(hwnd);
            return 0;
        }

        case WM_COMMAND:
        {
            if (HIWORD(wParam) != BN_CLICKED)
                return 0;

            int id = LOWORD(wParam);

            if (id == NEW_GAME_ID)
            {
                StartNewGame(hwnd);
                return 0;
            }

            if (id >= CARD_FIRST_ID &&
                id < CARD_FIRST_ID + 16)
            {
                if (waitingToHide)
                    return 0;

                int index = id - CARD_FIRST_ID;
                int result = dllFlipCard(index);

                RefreshBoard();

                if (result == 3)
                {
                    // Show different cards for 900 milliseconds.
                    waitingToHide = true;

                    if (SetTimer(hwnd, HIDE_TIMER_ID, 900, NULL) == 0)
                    {
                        dllHideCards();
                        waitingToHide = false;
                        RefreshBoard();
                    }
                }
                else if (result == 4)
                {
                    // Paint the final pair before displaying the message.
                    RedrawWindow(
                        hwnd, NULL, NULL,
                        RDW_INVALIDATE | RDW_UPDATENOW |
                        RDW_ALLCHILDREN
                    );

                    char winText[128];

                    wsprintfA(
                        winText,
                        "You found all eight pairs in %d attempts!",
                        dllGetAttempts()
                    );

                    MessageBoxA(
                        hwnd, winText, "You won!",
                        MB_OK | MB_ICONINFORMATION
                    );
                }
            }

            return 0;
        }

        case WM_TIMER:
        {
            if (wParam == HIDE_TIMER_ID && waitingToHide)
            {
                KillTimer(hwnd, HIDE_TIMER_ID);
                dllHideCards();
                waitingToHide = false;
                RefreshBoard();
            }

            return 0;
        }

        case WM_DESTROY:
            KillTimer(hwnd, HIDE_TIMER_ID);
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcA(hwnd, message, wParam, lParam);
    }
}

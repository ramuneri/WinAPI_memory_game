#include <windows.h>
#include <string>
#include "resource.h"

// IDs - used by our controls and timer
const int CARD_FIRST_ID = 100;
const int STATUS_ID = 200;
const int NEW_GAME_ID = 201;
const UINT_PTR HIDE_TIMER_ID = 1;

// Function types matching the DLL exports
typedef void (*NewGameFunction)(unsigned int);
typedef int (*FlipCardFunction)(int);
typedef void (*HideCardsFunction)();
typedef int (*GetCardValueFunction)(int);
typedef int (*IsCardVisibleFunction)(int);
typedef int (*GetAttemptsFunction)();
typedef int (*GetMatchedPairsFunction)();

// Other variables
HMODULE gameDLL = NULL;

NewGameFunction dllNewGame = NULL;
FlipCardFunction dllFlipCard = NULL;
HideCardsFunction dllHideCards = NULL;
GetCardValueFunction dllGetCardValue = NULL;
IsCardVisibleFunction dllIsCardVisible = NULL;
GetAttemptsFunction dllGetAttempts = NULL;
GetMatchedPairsFunction dllGetMatchedPairs = NULL;

// Window controls
HWND cardButtons[16] = {};
HWND statusLabel = NULL;


bool waitingToHide = false;

HBITMAP cardImages[8] = {};

const COLORREF BACKGROUND_COLOR = RGB(20, 20, 38);
const COLORREF TEXT_COLOR = RGB(232, 225, 250);

HBRUSH backgroundBrush = CreateSolidBrush(BACKGROUND_COLOR);

int bestScore = 0;
HWND bestScoreLabel = NULL;
std::string scoreFilePath;

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

// Load the DLL from the same folder as the .exe
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


    std::string folder = path.substr(0, separator + 1);

    scoreFilePath = folder + "best_score.ini";
    path = folder + "MemoryGameDLL.dll";


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

void LoadBestScore()
{
    UINT savedScore = GetPrivateProfileIntA(
        "Scores",
        "BestAttempts",
        0,
        scoreFilePath.c_str()
    );

    // A completed game must have at least 8 attempts
    if (savedScore >= 8 && savedScore <= 1000000)
        bestScore = static_cast<int>(savedScore);
    else
        bestScore = 0;
}

void SaveBestScore(HWND hwnd)
{
    char text[32];
    wsprintfA(text, "%d", bestScore);

    if (!WritePrivateProfileStringA(
            "Scores",
            "BestAttempts",
            text,
            scoreFilePath.c_str()))
    {
        MessageBoxA(
            hwnd,
            "The best score could not be saved.\n"
            "Check that the game folder is writable.",
            "Save error",
            MB_OK | MB_ICONWARNING
        );
    }
}

// Refresh all cards and the score display
void RefreshBoard()
{
    for (int i = 0; i < 16; ++i)
    {
        InvalidateRect(cardButtons[i], NULL, FALSE);
    }

    char statusText[100];

    wsprintfA(
        statusText,
        "Attempts: %d       Pairs: %d / 8",
        dllGetAttempts(),
        dllGetMatchedPairs()
    );

    SetWindowTextA(statusLabel, statusText);

    char bestText[64];

    if (bestScore == 0)
        lstrcpyA(bestText, "Best: --");
    else
        wsprintfA(bestText, "Best: %d attempts", bestScore);

    SetWindowTextA(bestScoreLabel, bestText);
}

void StartNewGame(HWND hwnd)
{
    KillTimer(hwnd, HIDE_TIMER_ID);
    waitingToHide = false;

    dllNewGame(static_cast<unsigned int>(GetTickCount()));
    RefreshBoard();
}

INT_PTR CALLBACK AboutDialogProcedure(HWND dialog, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK ||
                LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(dialog, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(dialog, IDCANCEL);
            return TRUE;
    }

    return FALSE;
}

void DrawGameButton(const DRAWITEMSTRUCT* item)
{
    HDC dc = item->hDC;
    RECT area = item->rcItem;

    int savedState = SaveDC(dc);

    bool pressed = (item->itemState & ODS_SELECTED) != 0;
    int offset = pressed ? 1 : 0;

    bool isCard =
        item->CtlID >= CARD_FIRST_ID &&
        item->CtlID < CARD_FIRST_ID + 16;

    COLORREF fillColor = pressed
        ? RGB(64, 49, 91)
        : RGB(40, 34, 62);

    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    FillRect(dc, &area, fillBrush);
    DeleteObject(fillBrush);

    HBRUSH borderBrush = CreateSolidBrush(RGB(139, 116, 183));
    FrameRect(dc, &area, borderBrush);
    DeleteObject(borderBrush);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, TEXT_COLOR);

    SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT));

    if (isCard)
    {
        int index = item->CtlID - CARD_FIRST_ID;

        if (dllIsCardVisible(index))
        {
            int value = dllGetCardValue(index);

            if (value >= 1 && value <= 8 &&
                cardImages[value - 1] != NULL)
            {
                HDC imageDC = CreateCompatibleDC(dc);

                HGDIOBJ oldBitmap = SelectObject(
                    imageDC, cardImages[value - 1]
                );

                int x = area.left +
                    (area.right - area.left - 72) / 2 + offset;

                int y = area.top +
                    (area.bottom - area.top - 72) / 2 + offset;

                BitBlt(dc, x, y, 72, 72, imageDC, 0, 0, SRCCOPY);

                SelectObject(imageDC, oldBitmap);
                DeleteDC(imageDC);
            }
        }
        else
        {
            // Design on top of a closed card - diamond
            int centerX = (area.left + area.right) / 2 + offset;
            int centerY = (area.top + area.bottom) / 2 + offset;

            POINT diamond[4] = {
                {centerX,      centerY - 13},
                {centerX + 9,  centerY},
                {centerX,      centerY + 13},
                {centerX - 9,  centerY}
            };

            HBRUSH diamondBrush =
                CreateSolidBrush(RGB(183, 157, 226));

            HPEN diamondPen =
                CreatePen(PS_SOLID, 1, RGB(210, 190, 245));

            HGDIOBJ oldBrush = SelectObject(dc, diamondBrush);
            HGDIOBJ oldPen = SelectObject(dc, diamondPen);

            Polygon(dc, diamond, 4);

            SelectObject(dc, oldPen);
            SelectObject(dc, oldBrush);

            DeleteObject(diamondPen);
            DeleteObject(diamondBrush);
        }
    }
    else
    {
        char caption[100];
        GetWindowTextA(item->hwndItem, caption, 100);

        RECT textArea = area;
        OffsetRect(&textArea, offset, offset);

        DrawTextA(
            dc, caption, -1, &textArea,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE
        );
    }

    if ((item->itemState & ODS_FOCUS) &&
        !(item->itemState & ODS_NOFOCUSRECT))
    {
        RECT focusArea = area;
        InflateRect(&focusArea, -4, -4);
        DrawFocusRect(dc, &focusArea);
    }

    RestoreDC(dc, savedState);
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
    wincl.lpszMenuName = MAKEINTRESOURCEA(IDR_MAIN_MENU);
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_HREDRAW | CS_VREDRAW;
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.hbrBackground = backgroundBrush;

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
    AdjustWindowRectEx(&windowRect, windowStyle, TRUE, 0);

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

    UnregisterClassA(className, hThisInstance);

    DeleteObject(backgroundBrush);
    backgroundBrush = NULL;

    FreeLibrary(gameDLL);
    gameDLL = NULL;

    if (result == -1)
        return 1;

    return static_cast<int>(messages.wParam);
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message,                                 WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            HINSTANCE instance =
                ((LPCREATESTRUCT)lParam)->hInstance;

            HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

            for (int i = 0; i < 8; ++i)
            {
                cardImages[i] = (HBITMAP)LoadImageA(
                    instance,
                    MAKEINTRESOURCEA(IDB_CARD1 + i),
                    IMAGE_BITMAP,
                    0, 0,
                    LR_CREATEDIBSECTION
                );

                if (cardImages[i] == NULL)
                {
                    MessageBoxA(
                        hwnd,
                        "Could not load a card picture.",
                        "Image error",
                        MB_OK | MB_ICONERROR
                    );

                    return -1;
                }
            }

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
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
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
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                20, 470, 140, 35,
                hwnd,
                (HMENU)(INT_PTR)NEW_GAME_ID,
                instance, NULL
            );

            SendMessage(
                newGameButton, WM_SETFONT, (WPARAM)font, TRUE
            );

            bestScoreLabel = CreateWindowExA(
                0, "STATIC", "",
                WS_CHILD | WS_VISIBLE,
                180, 478, 230, 25,
                hwnd, NULL, instance, NULL
            );

            SendMessage(
                bestScoreLabel, WM_SETFONT, (WPARAM)font, TRUE
            );

            LoadBestScore();
            StartNewGame(hwnd);
            return 0;
        }

        case WM_COMMAND:
        {
            if (HIWORD(wParam) != BN_CLICKED)
                return 0;

            int id = LOWORD(wParam);

            if (id == IDM_NEW_GAME)
            {
                StartNewGame(hwnd);
                return 0;
            }

            if (id == IDM_EXIT)
            {
                DestroyWindow(hwnd);
                return 0;
            }

            if (id == IDM_ABOUT)
            {
                INT_PTR result = DialogBoxParamA(
                    GetModuleHandle(NULL),
                    MAKEINTRESOURCEA(IDD_ABOUT),
                    hwnd,
                    AboutDialogProcedure,
                    0
                );

                if (result == -1)
                {
                    MessageBoxA(
                        hwnd,
                        "Could not open the About dialog.",
                        "Error",
                        MB_OK | MB_ICONERROR
                    );
                }

                return 0;
            }

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
                    int completedAttempts = dllGetAttempts();

            if (bestScore == 0 || completedAttempts < bestScore)
            {
                bestScore = completedAttempts;
                SaveBestScore(hwnd);
                RefreshBoard();
            }

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

        case WM_DRAWITEM:
        {
            const DRAWITEMSTRUCT* item =
                reinterpret_cast<const DRAWITEMSTRUCT*>(lParam);

            if (item->CtlType == ODT_BUTTON)
            {
                DrawGameButton(item);
                return TRUE;
            }

            return FALSE;
        }

        case WM_CTLCOLORSTATIC:
        {
            HDC dc = (HDC)wParam;

            SetTextColor(dc, TEXT_COLOR);
            SetBkColor(dc, BACKGROUND_COLOR);

            return (LRESULT)backgroundBrush;
        }

        case WM_DESTROY:
            KillTimer(hwnd, HIDE_TIMER_ID);

            for (int i = 0; i < 8; ++i)
            {
                if (cardImages[i] != NULL)
                {
                    DeleteObject(cardImages[i]);
                    cardImages[i] = NULL;
                }
            }

            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcA(hwnd, message, wParam, lParam);
    }
}

// Wordle.cpp : Definiuje punkt wejścia dla aplikacji.
//
#pragma comment(lib, "Msimg32.lib")
#include "framework.h"
#include "Wordle.h"
#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>
#include <Windows.h>


#define MAX_LOADSTRING 100
#define GAME_ROWS 5


// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szTitlePuzzle[MAX_LOADSTRING];            // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego


typedef struct TILE 
{
    RECT rec;
    char letter;
    char color[4];
};

typedef struct WND
{
    HWND hWnd;
    std::string password;
    int finish{};
};

// kolorowanie planszy
std::map<std::pair<int, int>, TILE*> puzzleCaps;
std::map<char, TILE*> keyCaps;
int I = 0;
int J = 0;

// zmiana poziomu trudnosci
int noWindows = 1;
int currentWindow = 0;
int difficulty = 6;
WND puzzleWnds[4];
bool lose[4];
bool MODE;


// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                PuzzleRegisterClass(HINSTANCE hInstance);
ATOM                WinRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    PuzzleWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WinWndProc(HWND, UINT, WPARAM, LPARAM);
void                Paint(HDC);
void                PaintKeyboard(HDC);
void                PaintKey(HDC, RECT*, char);
void                ClearMap(std::map<std::pair<int, int>, TILE*>*);
bool                ValidateWord();
void                Request(char);
std::string         GeneratePassword(int);
void                LoadTiles();
void                LoadKeycaps();
void                ClearKeyboard(std::map<char, TILE*>*);
void                win(HDC ,char);




int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
 

    // Inicjuj ciągi globalne
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WORDLE, szWindowClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_GAME, szTitlePuzzle, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    PuzzleRegisterClass(hInstance);
    WinRegisterClass(hInstance);

    // Wykonaj inicjowanie aplikacji:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WORDLE));

    MSG msg;

    // Główna pętla komunikatów:
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
//  FUNKCJA: MyRegisterClass()
//
//  PRZEZNACZENIE: Rejestruje klasę okna.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WORDLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(RGB(255, 255, 255));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WORDLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WORDLE));

    return RegisterClassExW(&wcex);
}

ATOM PuzzleRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = PuzzleWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;    
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"PUZZLE";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WORDLE));

    return RegisterClassExW(&wcex);
}

ATOM WinRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WinWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 255, 0));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"WIN";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WORDLE));

    return RegisterClassExW(&wcex);
}
//
//   FUNKCJA: InitInstance(HINSTANCE, int)
//
//   PRZEZNACZENIE: Zapisuje dojście wystąpienia i tworzy okno główne
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Przechowuj dojście wystąpienia w naszej zmiennej globalnej

   int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = GetSystemMetrics(SM_CYSCREEN);

   int windowWidth = 635;
   int windowHeight = 260;

   int windowX = (screenWidth - windowWidth) / 2;
   int windowY = (screenHeight - windowHeight) / 2;


   DWORD Style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, Style,
       windowX, windowY + 300, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

   // Set WS_EX_LAYERED on this window
   SetWindowLong(hWnd, GWL_EXSTYLE,
   GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
   
   // Make this window 50% alpha
   SetLayeredWindowAttributes(hWnd, 0, (255 * 80) / 100, LWA_ALPHA);

   if (!hWnd)
   {
       return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNKCJA: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PRZEZNACZENIE: Przetwarza komunikaty dla okna głównego.
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        
        // Parse the menu selections:
        switch (wmId)
        {
        case ID_DIFFICULTY_EASY:
        {
            auto menu = GetMenu(hWnd);
            CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_CHECKED);
            
            for (int i = 0; i < noWindows; i++)
            {
                DestroyWindow(puzzleWnds[i].hWnd);
                puzzleWnds[i].finish = 0;
                lose[i] = 0;
            }

            ClearMap(&puzzleCaps);
            InvalidateRect(hWnd, NULL, TRUE);
            ClearKeyboard(&keyCaps);
            noWindows = 1;
            difficulty = 6;
            SendMessage(hWnd, WM_CREATE, wParam, lParam);
        }
        break;
        case ID_DIFFICULTY_MEDIUM:
        {
            auto menu = GetMenu(hWnd);
            CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_CHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
            
            for (int i = 0; i < noWindows; i++)
            {
                DestroyWindow(puzzleWnds[i].hWnd);
                puzzleWnds[i].finish = 0;
                lose[i] = 0;
            }
 

            ClearMap(&puzzleCaps);
            InvalidateRect(hWnd, NULL, TRUE);
            ClearKeyboard(&keyCaps);
            noWindows = 2;
            difficulty = 8;
            SendMessage(hWnd, WM_CREATE, wParam, lParam);

        }
        break;
        case ID_DIFFICULTY_HARD:
        {
            auto menu = GetMenu(hWnd);
            CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_CHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
            
            for (int i = 0; i < noWindows; i++)
            {
                DestroyWindow(puzzleWnds[i].hWnd);
                puzzleWnds[i].finish = 0;
                lose[i] = 0;
            }

            ClearMap(&puzzleCaps);
            InvalidateRect(hWnd, NULL, TRUE);
            ClearKeyboard(&keyCaps);
            noWindows = 4;
            difficulty = 10;
            SendMessage(hWnd, WM_CREATE, wParam, lParam);
        }
        break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CREATE:
    {
        if (!MODE) 
        {
            char diff[256];
            GetPrivateProfileStringA("WORDLE", "DIFFICULTY", "1", diff, 256, ".\\Wordle.ini");
            noWindows = atoi(diff);
            MODE = true;

            auto menu = GetMenu(hWnd);
            CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_CHECKED);

            switch (noWindows)
            {
            case 1:
            {
                difficulty = 6;
                CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_CHECKED);
            }   
            break;
            case 2:
            {
                difficulty = 8;
                CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_CHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
            }    
            break;
            case 4:
            {
                difficulty = 10;
                CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_CHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
            }   
            break;
            }
        }
        
        
        LoadTiles();
        LoadKeycaps();

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        switch (noWindows)
        {
        case 1:
        {
            int windowWidth = 326;
            int windowHeight = 415;
            
            int windowX = (screenWidth - windowWidth) / 2;
            int windowY = (screenHeight - windowHeight) / 2;


            DWORD Style = ( WS_BORDER | WS_VISIBLE) & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
            // https://stackoverflow.com/questions/2398746/removing-window-border


            puzzleWnds[0].hWnd = CreateWindowEx(0,L"PUZZLE", szTitlePuzzle, Style,
                windowX, windowY -200, windowWidth, windowHeight, hWnd, NULL, hInst, NULL);

            puzzleWnds[0].password = GeneratePassword(100);

            
            break;
        }
        case 2:
        {
            int windowWidth = 326;
            int windowHeight = 537;

            int windowX1 = (screenWidth/2 - windowWidth) / 2;
            int windowY = (screenHeight - windowHeight) / 2;

            int windowX2 = (screenWidth - windowWidth) / 2 + windowX1 + windowWidth/2;


            DWORD Style = (WS_BORDER | WS_VISIBLE) & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
            // https://stackoverflow.com/questions/2398746/removing-window-border


            puzzleWnds[0].hWnd = CreateWindowEx(0, L"PUZZLE", szTitlePuzzle, Style,
                windowX1, windowY -200, windowWidth, windowHeight, hWnd, NULL, hInst, NULL);
            puzzleWnds[1].hWnd = CreateWindowEx(0, L"PUZZLE", szTitlePuzzle, Style,
                windowX2, windowY -200, windowWidth, windowHeight, hWnd, NULL, hInst, NULL);

            puzzleWnds[0].password = GeneratePassword(100);
            puzzleWnds[1].password = GeneratePassword(10000);
            break;
        }
        case 4:
        {
            int windowWidth = 326;
            int windowHeight = 659;

            int windowX1 = (screenWidth / 2 - windowWidth) / 2;
            int windowY1 = (screenHeight /2 - windowHeight) / 2;

            int windowX2 = (screenWidth - windowWidth) / 2 + windowX1 + windowWidth / 2;
            int windowY2 = (screenHeight - windowHeight) / 2 + windowY1 + windowHeight / 2;

            DWORD Style = (WS_BORDER | WS_VISIBLE) & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
            // https://stackoverflow.com/questions/2398746/removing-window-border


            puzzleWnds[0].hWnd = CreateWindowEx(0, L"PUZZLE", szTitlePuzzle, Style,
                windowX1, windowY1, windowWidth, windowHeight, hWnd, NULL, hInst, NULL);
            puzzleWnds[1].hWnd = CreateWindowEx(0, L"PUZZLE", szTitlePuzzle, Style,
                windowX2, windowY1, windowWidth, windowHeight, hWnd, NULL, hInst, NULL);
            puzzleWnds[2].hWnd = CreateWindowEx(0, L"PUZZLE", szTitlePuzzle, Style,
                windowX1, windowY2, windowWidth, windowHeight, hWnd, NULL, hInst, NULL);
            puzzleWnds[3].hWnd = CreateWindowEx(0, L"PUZZLE", szTitlePuzzle, Style,
                windowX2, windowY2, windowWidth, windowHeight, hWnd, NULL, hInst, NULL);

            puzzleWnds[0].password = GeneratePassword(50);
            puzzleWnds[1].password = GeneratePassword(500);
            puzzleWnds[2].password = GeneratePassword(5000);
            puzzleWnds[3].password = GeneratePassword(10000);
            break;
        }
        }


    }
    break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_BACK:
        {
            if (J == GAME_ROWS-1 && puzzleCaps.at({ I,J })->letter != '\0')
            {
                Request('\0');
                break;
            }
            else if (J == GAME_ROWS - 1 && puzzleCaps.at({ I,J })->letter == '\0')
            {
                J--;
                Request('\0');
                break;
            }
            else if (J == 0)
            {
                Request('\0');
                break;
            }
            else if (J == GAME_ROWS-1 && I == difficulty-1 && puzzleCaps.at({ I,J })->letter != '\0')
            {
                break;
            }
            else
            {
                J--;
                Request('\0');
                break;
            }
        }
        case VK_RETURN:
        {
            
            if (J == GAME_ROWS - 1 && puzzleCaps.at({ I,J })->letter != '\0' && puzzleCaps.at({ difficulty-1,GAME_ROWS-1})->letter == '\0')
            {

                
                if (ValidateWord() == true)
                {
                    for (currentWindow = 0; currentWindow < noWindows; currentWindow++)
                    {
                        InvalidateRect(hWnd, NULL, TRUE);

                        if (puzzleWnds[currentWindow].finish == 2) continue;
                        else if (puzzleWnds[currentWindow].finish == 1)
                        {

                            puzzleWnds[currentWindow].finish = 2;
                        }
                      
                        InvalidateRect(puzzleWnds[currentWindow].hWnd, NULL, TRUE);
                        UpdateWindow(puzzleWnds[currentWindow].hWnd);
                        
                    }
                    I++;
                    J = 0;
                    break;
                }
                else
                {
                    for (; J > 0; J--)
                        puzzleCaps.at({ I,J })->letter = '\0';

                    Request('\0');
                    break;
                }
                
            }
            else if (J == GAME_ROWS - 1 && puzzleCaps.at({ I,J })->letter != '\0' && puzzleCaps.at({ difficulty - 1,GAME_ROWS - 1 })->letter != '\0')
            {
                
                if (ValidateWord() == true)
                {
                    for (currentWindow = 0; currentWindow < noWindows; currentWindow++)
                    {
                        InvalidateRect(hWnd, NULL, TRUE);
                        
                        if (puzzleWnds[currentWindow].finish == 2) continue;
                        else if (puzzleWnds[currentWindow].finish == 1)
                        {

                            puzzleWnds[currentWindow].finish = 2;
                        }
                        else
                        {
                            lose[currentWindow] = true;
                        }

                        InvalidateRect(puzzleWnds[currentWindow].hWnd, NULL, TRUE);
                        UpdateWindow(puzzleWnds[currentWindow].hWnd);
                        
                    }
                    break;
                }
                else
                {
                    for (; J > 0; J--)
                        puzzleCaps.at({ I,J })->letter = '\0';

                    Request('\0');
                    break;
                }
            }
            break;
        }
        default:
        {
            if (J == GAME_ROWS-1 && puzzleCaps.at({ I,J })->letter != '\0') break;
            
            Request(static_cast<char>(wParam));
            break;
        }
        }
    }
    break;
    case WM_PAINT:
    {
    
        PAINTSTRUCT psM;
        HDC hdcM = BeginPaint(hWnd, &psM);
        HBRUSH hBrush = CreateSolidBrush(RGB(230, 252, 255));
        HPEN hPen = CreatePen(PS_NULL, 1, RGB(255, 0, 0));
        SelectObject(hdcM, hBrush);
        SelectObject(hdcM, hPen);

        PaintKeyboard(hdcM);

        ReleaseDC(hWnd, hdcM);
        DeleteObject(hPen);
        DeleteObject(hBrush);
        EndPaint(hWnd, &psM);


    }
    break;
    case WM_ERASEBKGND:
    {
        return 1;
    }
    case WM_CLOSE:
    {
        std::string tmp = std::to_string(noWindows);
        LPCSTR diff = tmp.c_str();
        WritePrivateProfileStringA("WORDLE", "DIFFICULTY", diff, ".\\Wordle.ini");
        PostQuitMessage(0);
    }
    break;
    case WM_DESTROY:
    {
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK PuzzleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdcPuzzle = BeginPaint(hWnd, &ps);
        HBRUSH hBrush = CreateSolidBrush(RGB(230, 252, 255));
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(181, 188, 189));

        SelectObject(hdcPuzzle, hPen);
        SelectObject(hdcPuzzle, hBrush);

        if (puzzleWnds[currentWindow].finish == 2 || lose[currentWindow] == true)
        {
            Rectangle(hdcPuzzle, 0, 0, 1000, 1000);
        }
        

        Paint(hdcPuzzle);
        for(int i=0; i< difficulty; i++)
            for (int j = 0; j < GAME_ROWS; j++)
                PaintKey(hdcPuzzle, &puzzleCaps.at({ i,j })->rec, puzzleCaps.at({i,j})->letter);

        //win check
        if(puzzleWnds[currentWindow].finish == 2)
            win(hdcPuzzle,'w');
        else if (lose[currentWindow] == true)
            win(hdcPuzzle, 'l');
        else
        {
            RECT rec = puzzleCaps.at({ I,J })->rec;
            PaintKey(hdcPuzzle, &rec, puzzleCaps.at({ I,J })->letter);
            if (puzzleCaps.at({ I,J })->letter != '\0' && J < GAME_ROWS - 1) J++;
        }

        
        ReleaseDC(hWnd, hdcPuzzle);
        DeleteObject(hPen);
        DeleteObject(hBrush);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_NCHITTEST:
    {
        return HTCAPTION;
    }
    break;
    case WM_ERASEBKGND:
    {
        return 1;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK WinWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_MOVE:
    {

    }
    case WM_NCHITTEST:
    {
        return HTCAPTION;
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}


//
//  Definicje pozostalych funkcji:
//
//
//

void Paint(HDC hdc)
{
    int square = 55;
    int margin = 6;
    int ew = 12;
    int eh = 12;

    HBRUSH Green = CreateSolidBrush(RGB(0, 255,0));
    HBRUSH Yellow = CreateSolidBrush(RGB(251, 252,0));
    HBRUSH Grey = CreateSolidBrush(RGB(105, 105, 105));
    HBRUSH Bg = CreateSolidBrush(RGB(251, 252, 255));

    
    for(int i=0; i<difficulty; i++)
        for (int j = 0; j < GAME_ROWS; j++)
        {
            auto tile = puzzleCaps.at({ i,j });
            auto rec = tile->rec;
            switch (tile->color[currentWindow])
            {
            case 'G':
            {
                SelectObject(hdc, Green);
                RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom,ew,eh);
                break;
            }
            case 'Y':
            {
                SelectObject(hdc, Yellow);
                RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom, ew, eh);
                break;
            }
            case 'D':
            {
                SelectObject(hdc, Grey);
                RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom, ew, eh);
                break;
            }
            default:
            {
                SelectObject(hdc, Bg);
                RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom, ew, eh);
                break;
            }
            }
        }

    SelectObject(hdc, Bg);
}

void PaintKeyboard(HDC hdc)
{
    int shift = 27;
    int eh = 10;
    int ew = 10;
    HBRUSH Green = CreateSolidBrush(RGB(0, 255, 0));
    HBRUSH Yellow = CreateSolidBrush(RGB(251, 252, 0));
    HBRUSH Grey = CreateSolidBrush(RGB(105, 105, 105));
    HBRUSH Bg = CreateSolidBrush(RGB(251, 252, 255));
    HPEN hPen = CreatePen(PS_NULL, 1, RGB(255, 0, 0));
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(181, 188, 189));
    
    UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
    for (auto i = keyCaps.begin(); i != keyCaps.end(); i++)
    {
        
        auto rec = i->second->rec;
        switch (i->second->color[0])
        {
        case 'G':
        {
            SelectObject(hdc, Green);
            RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom, ew, eh);
            break;
        }
        case 'Y':
        {
            SelectObject(hdc, Yellow);
            RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom, ew, eh);
            break;
        }
        case 'D':
        {
            SelectObject(hdc, Grey);
            RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom, ew, eh);
            break;
        }
        default:
        {
            SelectObject(hdc, Bg);
            RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom,ew,eh);
            break;
        }
        }

        if (noWindows >= 2)
        {
            switch (i->second->color[1])
            {
            case 'G':
            {
                SelectObject(hdc, Green);
                RoundRect(hdc, rec.left + shift, rec.top, rec.right, rec.bottom,ew,eh);
                break;
            }
            case 'Y':
            {
                SelectObject(hdc, Yellow);
                RoundRect(hdc, rec.left + shift, rec.top, rec.right, rec.bottom, ew, eh);
                break;
            }
            case 'D':
            {
                SelectObject(hdc, Grey);
                RoundRect(hdc, rec.left + shift, rec.top, rec.right, rec.bottom, ew, eh);
                break;
            }
            default:
            {
                SelectObject(hdc, Bg);
                RoundRect(hdc, rec.left + shift, rec.top, rec.right, rec.bottom, ew, eh);
                break;
            }
            }
        }

        if (noWindows == 4)
        {
            switch (i->second->color[2])
            {
            case 'G':
            {
                SelectObject(hdc, Green);
                RoundRect(hdc, rec.left, rec.top + shift, rec.right - shift, rec.bottom,ew,eh);
                break;
            }
            case 'Y':
            {
                SelectObject(hdc, Yellow);
                RoundRect(hdc, rec.left, rec.top + shift, rec.right - shift, rec.bottom, ew, eh);
                break;
            }
            case 'D':
            {
                SelectObject(hdc, Grey);
                RoundRect(hdc, rec.left, rec.top + shift, rec.right - shift, rec.bottom, ew, eh);
                break;
            }
            default:
            {
                SelectObject(hdc, Bg);
                RoundRect(hdc, rec.left, rec.top + shift, rec.right - shift, rec.bottom, ew, eh);
                break;
            }
            }

            switch (i->second->color[3])
            {
            case 'G':
            {
                SelectObject(hdc, Green);
                RoundRect(hdc, rec.left + shift, rec.top + shift, rec.right, rec.bottom,ew,eh);
                break;
            }
            case 'Y':
            {
                SelectObject(hdc, Yellow);
                RoundRect(hdc, rec.left + shift, rec.top + shift, rec.right, rec.bottom, ew, eh);
                break;
            }
            case 'D':
            {
                SelectObject(hdc, Grey);
                RoundRect(hdc, rec.left + shift, rec.top + shift, rec.right, rec.bottom, ew, eh);
                break;
            }
            default:
            {
                SelectObject(hdc, Bg);
                RoundRect(hdc, rec.left + shift, rec.top + shift, rec.right, rec.bottom, ew, eh);
                break;
            }
            }
        }

        
        // rysowanie obramowania
        SelectObject(hdc, pen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, rec.left, rec.top, rec.right, rec.bottom, ew+2, eh+2);
        SelectObject(hdc, hPen);

        HFONT font = CreateFont(
            MulDiv(24, GetDeviceCaps(hdc, LOGPIXELSY), 72),

            0, // Width
            0, // Escapement
            0, // Orientation
            FW_BOLD, // Weight
            false, // Italic
            FALSE, // Underline
            0, // StrikeOut
            EASTEUROPE_CHARSET, // CharSet
            OUT_DEFAULT_PRECIS, // OutPrecision
            CLIP_DEFAULT_PRECIS, // ClipPrecision
            DEFAULT_QUALITY, // Quality
            DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
            _T(" Verdana ")); // Facename

        SelectObject(hdc, font);
        SetTextColor(hdc, RGB(0, 0, 0));
        
        
        wchar_t wch[2] = {};
        swprintf(wch, 2, L"%c", i->first);

        SetBkMode(hdc, TRANSPARENT);
        DrawText(hdc, wch, 1, &(i->second->rec), format);
        SetBkMode(hdc, OPAQUE);
    }
    DeleteObject(hPen);
    DeleteObject(pen);
}

void PaintKey(HDC hdc, RECT *rec, char c)
{
    UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE;

    wchar_t wch[2] = {};
    swprintf(wch, 2, L"%c", c);

    HFONT font = CreateFont(
        MulDiv(24, GetDeviceCaps(hdc, LOGPIXELSY), 72),

        0, // Width
        0, // Escapement
        0, // Orientation
        FW_BOLD, // Weight
        false, // Italic
        FALSE, // Underline
        0, // StrikeOut
        EASTEUROPE_CHARSET, // CharSet
        OUT_DEFAULT_PRECIS, // OutPrecision
        CLIP_DEFAULT_PRECIS, // ClipPrecision
        DEFAULT_QUALITY, // Quality
        DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
        _T(" Verdana ")); // Facename

    SelectObject(hdc, font);
    SetTextColor(hdc, RGB(0, 0, 0));
   

    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, wch, 1, rec, format);
    SetBkMode(hdc, OPAQUE);
}

void ClearMap(std::map<std::pair<int, int>, TILE*>* map)
{
    for (auto i =map->begin(); i!=map->end();i++)
    {
        i->second->letter = '\0';
        for (int j = 0; j < 4; j++)
            i->second->color[j] = '\0';
    }
    I = 0;
    J = 0;
    
}

void ClearKeyboard(std::map<char, TILE*>* map)
{
    for (auto i = map->begin(); i != map->end(); i++)
    {
        for (int j = 0; j < 4; j++)
            i->second->color[j] = '\0';
    }
}

bool ValidateWord()
{
    std::ifstream str;
    str.open("Wordle.txt");

    if (!str.is_open()) {

        std::cerr << "COULDN'T OPEN FILE";
    }
    
    std::string word;
    std::string input;
    std::string validate;
    for (int i = 0; i < GAME_ROWS; i++)
    {
        char ch = puzzleCaps.at({ I,i })->letter;
        char c = std::tolower(ch);
        input += c;
        validate += ch;
    }

    while (std::getline(str, word))
    {
        if (word == input)
        {
            for (int i = 0; i < GAME_ROWS; i++)
            {
                for (int j = 0; j < noWindows; j++)
                {
                    // kolorowanie literk
                    if (puzzleWnds[j].password[i] == puzzleCaps.at({ I,i })->letter)
                    {
                        puzzleCaps.at({ I,i })->color[j] = 'G'; // green
                        keyCaps.at(puzzleWnds[j].password[i])->color[j] = 'G';
                        
                    }   
                    else if (puzzleWnds[j].password.find(puzzleCaps.at({ I,i })->letter) != -1)
                    {

                        puzzleCaps.at({ I,i })->color[j] = 'Y'; // yellow
                        keyCaps.at(puzzleCaps.at({ I,i })->letter)->color[j] = 'Y';
                        
                    }   
                    else
                    {
                        puzzleCaps.at({ I,i })->color[j] = 'D'; // default
                        keyCaps.at(puzzleCaps.at({ I,i })->letter)->color[j] = 'D';
                        
                    }   
                }
            }

            for (int u = 0; u < noWindows; u++)
            {
                if (puzzleWnds[u].password == validate && puzzleWnds[u].finish < 1)
                {

                    puzzleWnds[u].finish++;
                }
            }
            return true;
        }
    }

    str.close();
    return false;
}

void Request(char c)
{
    puzzleCaps.at({ I,J })->letter = c;
    for (currentWindow = 0; currentWindow < noWindows; currentWindow++) 
    {
        if (puzzleWnds[currentWindow].finish) continue;
        InvalidateRect(puzzleWnds[currentWindow].hWnd, NULL, TRUE);
        UpdateWindow(puzzleWnds[currentWindow].hWnd);
    }
        
}

void LoadTiles()
{
    int square = 55;
    int margin = 6;

    auto tiles = new TILE[difficulty][GAME_ROWS];
    for (int i = 0; i < difficulty; i++)
        for (int j = 0; j < GAME_ROWS; j++)
        {
            RECT rec = { margin + j * (margin + square), margin + i * (margin + square), (margin + square) + j * (margin + square), (margin + square) + i * (margin + square) };
            tiles[i][j] = { rec,'\0','\0'};
            puzzleCaps.insert({ {i,j},&tiles[i][j] });
        }

}

void LoadKeycaps()
{
    int square = 55;
    int margin = 6;
    int shift = 28;
    int iter = 0;

    RECT* recs = new RECT[26];
    char qwerty[26] = { 'Q','W','E','R','T','Y','U','I','O','P','A','S','D','F','G','H','J','K','L','Z','X','C','V','B','N','M' };
    TILE* tiles = new TILE[26];
    // first row
    for (int i = 0; i < 10; i++, iter++)
    {
        recs[iter].left = margin + i * (square + margin);
        recs[iter].top = margin;
        recs[iter].right = square + margin + i * (square + margin);
        recs[iter].bottom = square + margin;
    }

    // second row
    for (int i = 0; i < 9; i++, iter++)
    {
        recs[iter].left = margin + shift + i * (square + margin);
        recs[iter].top = margin + square + margin;
        recs[iter].right = shift + square + margin + i * (square + margin);
        recs[iter].bottom = 2 * (square + margin);
    }

    // third row
    for (int i = 1; i <= 7; i++, iter++)
    {
        recs[iter].left = margin + shift + i * (square + margin);
        recs[iter].top = margin + (square + margin) * 2;
        recs[iter].right = shift + square + margin + i * (square + margin);
        recs[iter].bottom = (square + margin) * 3;
    }

    for (int i = 0; i < 26; i++)
    {
        tiles[i].rec = recs[i];
        keyCaps.insert({ qwerty[i],&tiles[i]});
    }
}

std::string GeneratePassword(int x)
{
    std::ifstream str;
    str.open("Wordle.txt");

    if (!str.is_open()) {

        std::cerr << "COULDN'T OPEN FILE";
    }
    
    std::srand(std::time(nullptr));
    int index = std::rand()%x; 
    int iter = 0;
    std::string password;

    while (std::getline(str, password))
    {
        if (iter++ == index) break;
    }

    for (auto i = password.begin(); i < password.end(); i++)
        *i = toupper(*i);

    str.close();
    return password;
}

void win(HDC hdc,char m)
{
    
    int x = 326;
    int y = 0;


    if (noWindows == 1) y = 415;
    if (noWindows == 2) y = 537;
    if (noWindows == 4) y = 659;

    HDC ovDC = CreateCompatibleDC(hdc);
    HBITMAP bovDC = CreateCompatibleBitmap(hdc, x, y);
    HBRUSH Green = CreateSolidBrush(RGB(0, 255, 0));
    HBRUSH Red = CreateSolidBrush(RGB(255, 0, 0));
    SelectObject(ovDC, bovDC);

    BLENDFUNCTION blendFunction = { 0 };
    blendFunction.BlendOp = AC_SRC_OVER;
    blendFunction.BlendFlags = 0;
    blendFunction.SourceConstantAlpha = 150;
    blendFunction.AlphaFormat = 0;


    switch (m)
    {
    case 'w':
    {
        SelectObject(ovDC, Green);

        Rectangle(ovDC, 0, 0, x, y);

        AlphaBlend(hdc, 0, 0, x, y, ovDC, 0, 0, x, y, blendFunction);
        break;
    }
    case 'l':
    {

        SelectObject(ovDC, Red);

        Rectangle(ovDC, 0, 0, x, y);

        AlphaBlend(hdc, 0, 0, x, y, ovDC, 0, 0, x, y, blendFunction);

        
        
        HFONT font = CreateFont(
             MulDiv(24, GetDeviceCaps(hdc, LOGPIXELSY), 72),
            
             0, // Width
             0, // Escapement
             0, // Orientation
             FW_BOLD, // Weight
             false, // Italic
             FALSE, // Underline
             0, // StrikeOut
             EASTEUROPE_CHARSET, // CharSet
             OUT_DEFAULT_PRECIS, // OutPrecision
             CLIP_DEFAULT_PRECIS, // ClipPrecision
             DEFAULT_QUALITY, // Quality
             DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
             _T(" Verdana ")); // Facename

        SelectObject(hdc, font);
        SetTextColor(hdc, RGB(255, 255, 255));
       

        UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
        RECT rec = { 0, 0, x, y };

        std::string str = puzzleWnds[currentWindow].password;
        std::wstring wstr(str.begin(), str.end());

        SetBkMode(hdc, TRANSPARENT);
        DrawText(hdc, wstr.c_str(), 6, &rec, format);
        SetBkMode(hdc, OPAQUE);
        break;
    }
    }

    DeleteObject(ovDC);
    DeleteObject(bovDC);
    DeleteObject(Green);
    DeleteObject(Red);




}







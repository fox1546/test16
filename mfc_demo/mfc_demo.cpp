// mfc_demo.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "mfc_demo.h"
#include <strsafe.h>

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

HWND g_hWnd = nullptr;
CLinkGame g_linkGame;
int g_boardWidth = 8;
int g_boardHeight = 8;
bool g_gameStarted = false;
Point g_selectedTile(0, 0);

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MFCDEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MFCDEMO));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MFCDEMO));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MFCDEMO);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    int windowWidth = g_boardWidth * TILE_SIZE + 40;
    int windowHeight = g_boardHeight * TILE_SIZE + 80;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, 0, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    g_hWnd = hWnd;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void ResizeWindowForBoard()
{
    if (!g_hWnd) return;

    RECT rect;
    GetWindowRect(g_hWnd, &rect);

    int windowWidth = g_boardWidth * TILE_SIZE + 40;
    int windowHeight = g_boardHeight * TILE_SIZE + 80;

    SetWindowPos(g_hWnd, nullptr, rect.left, rect.top, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOMOVE);
}

INT_PTR CALLBACK SettingsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
    {
        WCHAR buf[16];
        StringCchPrintfW(buf, 16, L"%d", g_boardWidth);
        SetDlgItemTextW(hDlg, IDC_EDIT_WIDTH, buf);
        StringCchPrintfW(buf, 16, L"%d", g_boardHeight);
        SetDlgItemTextW(hDlg, IDC_EDIT_HEIGHT, buf);
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (wmId == IDOK)
        {
            WCHAR buf[16];
            GetDlgItemTextW(hDlg, IDC_EDIT_WIDTH, buf, 16);
            int width = _wtoi(buf);
            GetDlgItemTextW(hDlg, IDC_EDIT_HEIGHT, buf, 16);
            int height = _wtoi(buf);

            if (width < 4 || width > 20 || height < 4 || height > 20)
            {
                MessageBoxW(hDlg, L"宽度和高度必须在 4-20 之间！", L"错误", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            if ((width * height) % 2 != 0)
            {
                MessageBoxW(hDlg, L"宽度 × 高度 必须是偶数！", L"错误", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE;
            }

            g_boardWidth = width;
            g_boardHeight = height;

            g_gameStarted = false;
            g_selectedTile = Point(0, 0);

            ResizeWindowForBoard();
            InvalidateRect(g_hWnd, nullptr, TRUE);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (wmId == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    }
    return (INT_PTR)FALSE;
}

void DrawTile(HDC hdc, int x, int y, int iconIndex)
{
    int offsetX = 20;
    int offsetY = 20;

    int left = offsetX + (x - 1) * TILE_SIZE;
    int top = offsetY + (y - 1) * TILE_SIZE;

    RECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = left + TILE_SIZE;
    rect.bottom = top + TILE_SIZE;

    if (iconIndex == 0)
    {
        FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
        return;
    }

    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);

    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    Rectangle(hdc, left, top, left + TILE_SIZE, top + TILE_SIZE);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    COLORREF colors[] = {
        RGB(255, 0, 0),
        RGB(0, 255, 0),
        RGB(0, 0, 255),
        RGB(255, 255, 0),
        RGB(255, 0, 255),
        RGB(0, 255, 255),
        RGB(255, 128, 0),
        RGB(128, 0, 255),
        RGB(255, 128, 128),
        RGB(128, 255, 128),
        RGB(128, 128, 255),
        RGB(255, 192, 0)
    };

    int colorIndex = (iconIndex - 1) % 12;
    COLORREF color = colors[colorIndex];

    HBRUSH hIconBrush = CreateSolidBrush(color);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hIconBrush);

    int centerX = left + TILE_SIZE / 2;
    int centerY = top + TILE_SIZE / 2;
    int radius = TILE_SIZE / 3;

    switch ((iconIndex - 1) % 6)
    {
    case 0:
        Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
        break;
    case 1:
        Rectangle(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
        break;
    case 2:
        MoveToEx(hdc, centerX, centerY - radius, nullptr);
        LineTo(hdc, centerX + radius, centerY);
        LineTo(hdc, centerX, centerY + radius);
        LineTo(hdc, centerX - radius, centerY);
        LineTo(hdc, centerX, centerY - radius);
        break;
    case 3:
    {
        POINT trianglePts[] = {
            {centerX, centerY - radius},
            {centerX + radius, centerY + radius / 2},
            {centerX - radius, centerY + radius / 2}
        };
        Polygon(hdc, trianglePts, 3);
        break;
    }
    case 4:
        MoveToEx(hdc, centerX, centerY - radius, nullptr);
        LineTo(hdc, centerX + radius, centerY + radius);
        LineTo(hdc, centerX - radius, centerY + radius);
        LineTo(hdc, centerX, centerY - radius);
        break;
    case 5:
        for (int i = 0; i < 5; i++)
        {
            double angle1 = -90 + i * 72;
            double angle2 = angle1 + 36;
            int x1 = (int)(centerX + radius * cos(angle1 * 3.14159 / 180));
            int y1 = (int)(centerY + radius * sin(angle1 * 3.14159 / 180));
            int x2 = (int)(centerX + radius / 2 * cos(angle2 * 3.14159 / 180));
            int y2 = (int)(centerY + radius / 2 * sin(angle2 * 3.14159 / 180));
            if (i == 0)
                MoveToEx(hdc, x1, y1, nullptr);
            else
                LineTo(hdc, x1, y1);
            LineTo(hdc, x2, y2);
        }
        LineTo(hdc, (int)(centerX + radius * cos(-90 * 3.14159 / 180)), 
                  (int)(centerY + radius * sin(-90 * 3.14159 / 180)));
        break;
    }

    SelectObject(hdc, hOldBrush);
    DeleteObject(hIconBrush);

    if (g_selectedTile.x == x && g_selectedTile.y == y)
    {
        HPEN hSelectPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
        HPEN hOldPen2 = (HPEN)SelectObject(hdc, hSelectPen);
        HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HBRUSH hOldBrush2 = (HBRUSH)SelectObject(hdc, hNullBrush);
        Rectangle(hdc, left, top, left + TILE_SIZE, top + TILE_SIZE);
        SelectObject(hdc, hOldBrush2);
        SelectObject(hdc, hOldPen2);
        DeleteObject(hSelectPen);
    }
}

void DrawBoard(HDC hdc)
{
    if (!g_gameStarted)
    {
        RECT rect;
        GetClientRect(g_hWnd, &rect);
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        SetTextColor(hdc, RGB(128, 128, 128));
        SetBkMode(hdc, TRANSPARENT);
        HFONT hFont = CreateFontW(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        TextOutW(hdc, 50, 100, L"请选择【目录】->【开始】开始游戏", 26);
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        return;
    }

    for (int y = 1; y <= g_linkGame.GetHeight(); y++)
    {
        for (int x = 1; x <= g_linkGame.GetWidth(); x++)
        {
            int iconIndex = g_linkGame.GetTile(x, y);
            DrawTile(hdc, x, y, iconIndex);
        }
    }
}

void CheckGameState()
{
    if (g_linkGame.IsGameOver())
    {
        g_gameStarted = false;
        g_selectedTile = Point(0, 0);
        MessageBoxW(g_hWnd, L"恭喜！游戏胜利！", L"游戏结束", MB_OK | MB_ICONINFORMATION);
        InvalidateRect(g_hWnd, nullptr, TRUE);
        return;
    }

    if (!g_linkGame.HasValidMove())
    {
        int result = MessageBoxW(g_hWnd, L"没有可消除的配对了！是否重新洗牌？", L"提示", MB_YESNO | MB_ICONQUESTION);
        if (result == IDYES)
        {
            g_linkGame.Shuffle();
            g_selectedTile = Point(0, 0);
            InvalidateRect(g_hWnd, nullptr, TRUE);
        }
    }
}

void OnLButtonDown(HWND hWnd, int mouseX, int mouseY)
{
    if (!g_gameStarted)
        return;

    int offsetX = 20;
    int offsetY = 20;

    int x = (mouseX - offsetX) / TILE_SIZE + 1;
    int y = (mouseY - offsetY) / TILE_SIZE + 1;

    if (x < 1 || x > g_linkGame.GetWidth() || y < 1 || y > g_linkGame.GetHeight())
        return;

    int iconIndex = g_linkGame.GetTile(x, y);
    if (iconIndex == 0)
        return;

    if (g_selectedTile.x == 0 && g_selectedTile.y == 0)
    {
        g_selectedTile = Point(x, y);
        InvalidateRect(hWnd, nullptr, TRUE);
        return;
    }

    if (g_selectedTile.x == x && g_selectedTile.y == y)
    {
        g_selectedTile = Point(0, 0);
        InvalidateRect(hWnd, nullptr, TRUE);
        return;
    }

    if (g_linkGame.CanEliminate(g_selectedTile.x, g_selectedTile.y, x, y))
    {
        g_linkGame.Eliminate(g_selectedTile.x, g_selectedTile.y, x, y);
        g_selectedTile = Point(0, 0);
        InvalidateRect(hWnd, nullptr, TRUE);

        CheckGameState();
    }
    else
    {
        g_selectedTile = Point(x, y);
        InvalidateRect(hWnd, nullptr, TRUE);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsDlg);
            break;
        case IDM_START:
            g_linkGame.Init(g_boardWidth, g_boardHeight);
            g_gameStarted = true;
            g_selectedTile = Point(0, 0);
            ResizeWindowForBoard();
            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawBoard(hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        OnLButtonDown(hWnd, x, y);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

#pragma once

#include "resource.h"
#include "LinkGame.h"

#define TILE_SIZE 50

extern HWND g_hWnd;
extern CLinkGame g_linkGame;
extern int g_boardWidth;
extern int g_boardHeight;
extern bool g_gameStarted;
extern Point g_selectedTile;

INT_PTR CALLBACK SettingsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void DrawBoard(HDC hdc);
void DrawTile(HDC hdc, int x, int y, int iconIndex);
void OnLButtonDown(HWND hWnd, int x, int y);
void CheckGameState();

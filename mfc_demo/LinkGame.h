#pragma once
#include <vector>
#include <windows.h>

struct Point
{
    int x;
    int y;
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
};

struct Path
{
    Point p1;
    Point p2;
    Point p3;
    int turnCount;
    Path() : turnCount(0) {}
};

class CLinkGame
{
public:
    CLinkGame();
    ~CLinkGame();

    void Init(int width, int height);
    void Shuffle();
    bool CanLink(int x1, int y1, int x2, int y2, Path* pPath = nullptr);
    bool CanEliminate(int x1, int y1, int x2, int y2);
    void Eliminate(int x1, int y1, int x2, int y2);
    bool HasValidMove();
    bool IsGameOver();
    int GetTile(int x, int y);
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetIconCount() const { return 12; }

private:
    std::vector<std::vector<int>> m_board;
    int m_width;
    int m_height;
    int m_remainingCount;

    bool CanLinkDirect(Point p1, Point p2);
    bool CanLinkOneTurn(Point p1, Point p2, Point* pMid = nullptr);
    bool CanLinkTwoTurns(Point p1, Point p2, Point* pMid1 = nullptr, Point* pMid2 = nullptr);
    bool IsEmpty(int x, int y);
    bool IsInBoard(int x, int y);
};

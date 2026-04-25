#include "framework.h"
#include "LinkGame.h"
#include <algorithm>
#include <ctime>

CLinkGame::CLinkGame()
    : m_width(0)
    , m_height(0)
    , m_remainingCount(0)
{
    srand((unsigned)time(nullptr));
}

CLinkGame::~CLinkGame()
{
}

void CLinkGame::Init(int width, int height)
{
    m_width = width;
    m_height = height;
    m_remainingCount = width * height;

    m_board.clear();
    m_board.resize(height + 2, std::vector<int>(width + 2, 0));

    int totalTiles = width * height;
    int iconCount = GetIconCount();
    int pairsPerIcon = totalTiles / (iconCount * 2);

    std::vector<int> tiles;
    for (int icon = 1; icon <= iconCount; icon++)
    {
        for (int i = 0; i < pairsPerIcon * 2; i++)
        {
            tiles.push_back(icon);
        }
    }

    int remaining = totalTiles - (int)tiles.size();
    if (remaining > 0)
    {
        for (int i = 0; i < remaining / 2; i++)
        {
            int icon = (i % iconCount) + 1;
            tiles.push_back(icon);
            tiles.push_back(icon);
        }
    }

    std::random_shuffle(tiles.begin(), tiles.end());

    int index = 0;
    for (int y = 1; y <= height; y++)
    {
        for (int x = 1; x <= width; x++)
        {
            m_board[y][x] = tiles[index++];
        }
    }
}

void CLinkGame::Shuffle()
{
    std::vector<int> tiles;
    for (int y = 1; y <= m_height; y++)
    {
        for (int x = 1; x <= m_width; x++)
        {
            if (m_board[y][x] != 0)
            {
                tiles.push_back(m_board[y][x]);
            }
        }
    }

    std::random_shuffle(tiles.begin(), tiles.end());

    int index = 0;
    for (int y = 1; y <= m_height; y++)
    {
        for (int x = 1; x <= m_width; x++)
        {
            if (m_board[y][x] != 0)
            {
                m_board[y][x] = tiles[index++];
            }
        }
    }
}

bool CLinkGame::CanLink(int x1, int y1, int x2, int y2, Path* pPath)
{
    Point p1(x1, y1);
    Point p2(x2, y2);

    if (pPath)
    {
        pPath->p1 = p1;
        pPath->p2 = p2;
        pPath->turnCount = 0;
    }

    if (CanLinkDirect(p1, p2))
    {
        if (pPath)
        {
            pPath->turnCount = 0;
        }
        return true;
    }

    Point mid;
    if (CanLinkOneTurn(p1, p2, &mid))
    {
        if (pPath)
        {
            pPath->p2 = mid;
            pPath->p3 = p2;
            pPath->turnCount = 1;
        }
        return true;
    }

    Point mid1, mid2;
    if (CanLinkTwoTurns(p1, p2, &mid1, &mid2))
    {
        if (pPath)
        {
            pPath->p2 = mid1;
            pPath->p3 = mid2;
            pPath->turnCount = 2;
        }
        return true;
    }

    return false;
}

bool CLinkGame::CanEliminate(int x1, int y1, int x2, int y2)
{
    if (x1 == x2 && y1 == y2)
        return false;

    int tile1 = GetTile(x1, y1);
    int tile2 = GetTile(x2, y2);

    if (tile1 == 0 || tile2 == 0)
        return false;

    if (tile1 != tile2)
        return false;

    return CanLink(x1, y1, x2, y2);
}

void CLinkGame::Eliminate(int x1, int y1, int x2, int y2)
{
    m_board[y1][x1] = 0;
    m_board[y2][x2] = 0;
    m_remainingCount -= 2;
}

bool CLinkGame::HasValidMove()
{
    for (int y1 = 1; y1 <= m_height; y1++)
    {
        for (int x1 = 1; x1 <= m_width; x1++)
        {
            if (m_board[y1][x1] == 0)
                continue;

            for (int y2 = 1; y2 <= m_height; y2++)
            {
                for (int x2 = 1; x2 <= m_width; x2++)
                {
                    if (x1 == x2 && y1 == y2)
                        continue;

                    if (CanEliminate(x1, y1, x2, y2))
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool CLinkGame::IsGameOver()
{
    return m_remainingCount == 0;
}

int CLinkGame::GetTile(int x, int y)
{
    if (!IsInBoard(x, y))
        return 0;
    return m_board[y][x];
}

bool CLinkGame::CanLinkDirect(Point p1, Point p2)
{
    if (p1.x == p2.x)
    {
        int minY = (p1.y < p2.y) ? p1.y : p2.y;
        int maxY = (p1.y > p2.y) ? p1.y : p2.y;
        for (int y = minY + 1; y < maxY; y++)
        {
            if (!IsEmpty(p1.x, y))
                return false;
        }
        return true;
    }
    else if (p1.y == p2.y)
    {
        int minX = (p1.x < p2.x) ? p1.x : p2.x;
        int maxX = (p1.x > p2.x) ? p1.x : p2.x;
        for (int x = minX + 1; x < maxX; x++)
        {
            if (!IsEmpty(x, p1.y))
                return false;
        }
        return true;
    }
    return false;
}

bool CLinkGame::CanLinkOneTurn(Point p1, Point p2, Point* pMid)
{
    Point c1(p1.x, p2.y);
    if (IsEmpty(c1.x, c1.y))
    {
        if (CanLinkDirect(p1, c1) && CanLinkDirect(c1, p2))
        {
            if (pMid)
                *pMid = c1;
            return true;
        }
    }

    Point c2(p2.x, p1.y);
    if (IsEmpty(c2.x, c2.y))
    {
        if (CanLinkDirect(p1, c2) && CanLinkDirect(c2, p2))
        {
            if (pMid)
                *pMid = c2;
            return true;
        }
    }

    return false;
}

bool CLinkGame::CanLinkTwoTurns(Point p1, Point p2, Point* pMid1, Point* pMid2)
{
    for (int x = 0; x <= m_width + 1; x++)
    {
        Point c1(x, p1.y);
        Point c2(x, p2.y);

        if (IsEmpty(c1.x, c1.y) && IsEmpty(c2.x, c2.y))
        {
            if (CanLinkDirect(p1, c1) && CanLinkDirect(c1, c2) && CanLinkDirect(c2, p2))
            {
                if (pMid1 && pMid2)
                {
                    *pMid1 = c1;
                    *pMid2 = c2;
                }
                return true;
            }
        }
    }

    for (int y = 0; y <= m_height + 1; y++)
    {
        Point c1(p1.x, y);
        Point c2(p2.x, y);

        if (IsEmpty(c1.x, c1.y) && IsEmpty(c2.x, c2.y))
        {
            if (CanLinkDirect(p1, c1) && CanLinkDirect(c1, c2) && CanLinkDirect(c2, p2))
            {
                if (pMid1 && pMid2)
                {
                    *pMid1 = c1;
                    *pMid2 = c2;
                }
                return true;
            }
        }
    }

    return false;
}

bool CLinkGame::IsEmpty(int x, int y)
{
    if (x == 0 || x == m_width + 1 || y == 0 || y == m_height + 1)
        return true;
    if (!IsInBoard(x, y))
        return true;
    return m_board[y][x] == 0;
}

bool CLinkGame::IsInBoard(int x, int y)
{
    return x >= 1 && x <= m_width && y >= 1 && y <= m_height;
}

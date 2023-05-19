#include <unordered_set>
#include "raylib.h"
#include <iostream>

using namespace std;

float cellSize = 10;
bool paused = true;
int iterations = 0;

struct Point {
    int x, y;

    // Constructor
    Point(int x, int y) {
        this->x = x;
        this->y = y;
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
};



struct HashFunction {
    size_t operator()(const Point& point) const {
        std::hash<int> hasher;
        return (hasher(point.y) * 0x1f1f1f1f) ^ hasher(point.y);
    }
};

unordered_set<Point, HashFunction> aliveCells;
unordered_set<Point, HashFunction> emptyBorderCells;
list<Point> toKill;
list<Point> toRevive;

list<Point> getEmptyNeighbors(Point p) {
    list<Point> empties = {};
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            Point neighborPoint = { p.x + i, p.y + j };
            auto it = aliveCells.find(neighborPoint);
            if (it == aliveCells.end()) {
                empties.push_front(neighborPoint);
            }
        }
    }

    return empties;
}

list<Point> getNeighbors(Point p) {
    list<Point> neighbors = {};
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            Point neighborPoint = { p.x + i, p.y + j };
            auto it = aliveCells.find(neighborPoint);
            if (it != aliveCells.end() && !(i == 0 && j == 0)) {
                neighbors.push_front(p);
            }
        }
    }
    
    return neighbors;
}


void ReviveAllTagged() {
    //cout << "Reviving all tagged..." << endl;
    for (const auto& p : toRevive) {
        emptyBorderCells.erase(p);
        aliveCells.insert(p);
    }
    toRevive.clear();
}

void KillAllTagged() {
    for (const auto& p : toKill) {
        aliveCells.erase(p);
    }
    toKill.clear();
}

void DrawCells(unordered_set<Point, HashFunction> points, Color c) {
    for (const Point& point : points) {
        DrawRectangle(point.x * cellSize, point.y * cellSize, cellSize, cellSize, c);
    }
}

Vector2 mouseOffset = { 0, 0 };
Vector2 camOffset = { 0,0 };
int main(void)
{

    InitWindow(500, 500, "Conway's Game of Life - Keon Davoudi");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    MaximizeWindow();

    int targetFPS = 120;
    SetTargetFPS(targetFPS);
    
    

    Camera2D camera = { 0 };
    camera.offset = camOffset;
    camera.rotation = 0;
    camera.target = { 0,0 };
    camera.zoom = 1.0f;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        
            ClearBackground(RAYWHITE);

            if (IsMouseButtonDown(1)) {
                Vector2 mouseMovement = GetMouseDelta();
                camera.offset.x += mouseMovement.x;
                camera.offset.y += mouseMovement.y;
                mouseOffset = camera.offset;
                //cout << "offset: " << mouseOffset.x << ", " << mouseOffset.y << endl;
            }
            

            // if left mouse button down, create new point at mouse position and queue it to be revived
            if (IsMouseButtonDown(0)) {
                Point newPoint = { (int)(GetMouseX() / cellSize - mouseOffset.x / cellSize), (int)(GetMouseY() / cellSize - mouseOffset.y / cellSize) };
                //cout << "adding point " << newPoint.x << ", " << newPoint.y << endl;
                toRevive.push_front(newPoint);
            }

            // put all the revived cells into the aliveCells and draw them.
            ReviveAllTagged();

            BeginMode2D(camera);
                DrawCells(aliveCells, BLACK);
            EndMode2D();

            
            //DrawCells(emptyBorderCells, RED);
            DrawFPS(0, 40);
            DrawText(paused ? "PAUSED | |" : "", 0, 0, 28, BLACK);
            DrawText("space: pause", 0, 80, 20, BLACK);
            DrawText("e: erase all cells", 0, 120, 20, BLACK);
            if (IsKeyPressed(32)) {
                paused = !paused;
            }

            if (IsKeyPressed(69)) {
                aliveCells.clear();
            }
            
            if (paused || iterations++ % 5 != 0) {
                EndDrawing();
                continue;
            }
            
            // put the cells on the chopping block
            for (const auto& p : aliveCells) {
                int neighborCount = getNeighbors(p).size();

                // dying from under or overpopulation
                if (neighborCount <= 1 || neighborCount >= 4) {
                    toKill.push_front(p);
                }
            }
            
            
            // go through the alive cells and insert their empty border cells
            for (const auto& p : aliveCells) {
                list<Point> empties = getEmptyNeighbors(p);
                for (const auto& emptyCell : empties) {
                    emptyBorderCells.insert(emptyCell);
                }
            }

            // go through the empty border cells and see which ones can be discarded and which ones should be revived
            list<Point> toErase;
            for (const auto& p : emptyBorderCells) {
                int neighborCount = getNeighbors(p).size();
                if (neighborCount == 3) {
                    toRevive.push_front(p);
                }
                else if (neighborCount == 0) {
                    toErase.push_front(p);
                }
                
            }
            ReviveAllTagged();
            KillAllTagged();

            for (const auto& p : toErase) {
                emptyBorderCells.erase(p);
            }
            
            
            
        EndDrawing();
    }

    CloseWindow();
    
    return 0;
}


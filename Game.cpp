/***********************************************
Conway's Game of Life
Implementation by Keon Davoudi using hashmaps on an infinite grid and the RayLib C++ library.
5/19/2023
************************************************/

#include <unordered_set>
#include "raylib.h"
#include <iostream>

using namespace std;

float cellSize = 10;
bool paused = true;
int iterations = 0;

// Point struct, represents a location on the grid.
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


// hashfunction for grid points.
struct HashFunction {
    size_t operator()(const Point& point) const {
        std::hash<int> hasher;

        // hashing function which should produce unique keys given two different points 100% of the time.
        return (hasher(point.y) * 0x1f1f1f1f) ^ hasher(point.y);
    }
};

// two unordered sets containing alive cells and bordering empty cells
unordered_set<Point, HashFunction> aliveCells;
unordered_set<Point, HashFunction> emptyBorderCells;

// lists that hold a given iteration's cells, which are tagged to be killed or revived.
list<Point> toKill;
list<Point> toRevive;

// returns a list of points where there are empty cells bordering a given cell at point p
list<Point> getEmptyNeighbors(Point p) {
    list<Point> empties = {};
    
    // loop through the 3x3 area excluding point p
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            Point neighborPoint = { p.x + i, p.y + j };
            auto it = aliveCells.find(neighborPoint);

            // if the current point is not alive, aka empty, add it to the list.
            if (it == aliveCells.end()) {
                empties.push_front(neighborPoint);
            }
        }
    }

    // return the points where there are empty cells
    return empties;
}

// similar to getEmptyNeighbors but returns a list of alive cells.
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

// loop through toRevive and remove from emptyBorderCells if exists and add to aliveCells.
void ReviveAllTagged() {
    //cout << "Reviving all tagged..." << endl;
    for (const auto& p : toRevive) {
        emptyBorderCells.erase(p);
        aliveCells.insert(p);
    }

    // clear for the next iteration.
    toRevive.clear();
}

// similar to ReviveAllTagged() but instead just erases each point in toKill from aliveCells
void KillAllTagged() {
    for (const auto& p : toKill) {
        aliveCells.erase(p);
    }

    // clear toKill for next iteration.
    toKill.clear();
}

// Function to draw cells at the given points in the set with a given color
void DrawCells(unordered_set<Point, HashFunction> points, Color c) {
    for (const Point& point : points) {
        DrawRectangle(point.x * cellSize, point.y * cellSize, cellSize, cellSize, c);
    }
}

int main(void)
{
    // initialize window and maximize to screen dimensions
    InitWindow(500, 500, "Conway's Game of Life - Keon Davoudi");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    MaximizeWindow();

    // target FPS at 120 frames per second
    int targetFPS = 120;
    SetTargetFPS(targetFPS);
    
    
    // setup 2D camera
    Camera2D camera = { 0 };
    camera.offset = { 0, 0 };
    camera.rotation = 0;
    camera.target = { 0,0 };
    camera.zoom = 1.0f;

    // main game loop
    while (!WindowShouldClose())
    {

        BeginDrawing();
            
            // set white background
            ClearBackground(RAYWHITE);

            // if RMB is held down, spawn alive cell at the mouse position
            if (IsMouseButtonDown(1)) {
                // get mouse change during that frame, add it to the camera offset (panning functionality)
                Vector2 mouseMovement = GetMouseDelta();
                camera.offset.x += mouseMovement.x;
                camera.offset.y += mouseMovement.y;
                //cout << "offset: " << mouseOffset.x << ", " << mouseOffset.y << endl;
            }
            

            // if left mouse button down, create new point at mouse position and queue it to be revived
            if (IsMouseButtonDown(0)) {
                Point newPoint = { (int)(GetMouseX() / cellSize - camera.offset.x / cellSize), (int)(GetMouseY() / cellSize - camera.offset.y / cellSize) };
                //cout << "adding point " << newPoint.x << ", " << newPoint.y << endl;
                toRevive.push_front(newPoint);
            }

            // put all the revived cells into the aliveCells and draw them.
            ReviveAllTagged();

            // draw cells
            BeginMode2D(camera);
                DrawCells(aliveCells, BLACK);
            EndMode2D();

            // Draw data and controls 
            DrawFPS(0, 40);
            DrawText(paused ? "PAUSED | |" : "", 0, 0, 28, BLACK);
            DrawText("space: pause", 0, 80, 20, BLACK);
            DrawText("e: erase all cells", 0, 120, 20, BLACK);
            DrawText("RMB drag: pan", 0, 160, 20, BLACK);

            // if pressed space, toggle paused
            if (IsKeyPressed(32)) {
                paused = !paused;
            }

            // if "e" pressed, kill all cells.
            if (IsKeyPressed(69)) {
                aliveCells.clear();
            }
            
            // every 5 iterations, update game data.
            // if paused, stop drawing and continue from the top of loop.
            if (paused || iterations++ % 5 != 0) {
                EndDrawing();
                continue;
            }
            
            // put the appropriate cells on the chopping block
            for (const auto& p : aliveCells) {
                // get number of alive cells neighboring p
                int neighborCount = getNeighbors(p).size();

                // dying from under or overpopulation
                if (neighborCount <= 1 || neighborCount >= 4) {
                    toKill.push_front(p);
                }
            }
            
            
            // go through the alive cells and insert their empty border cells
            for (const auto& p : aliveCells) {
                // get empty cells
                list<Point> empties = getEmptyNeighbors(p);

                // insert empty border cells into emptyBorderCells
                for (const auto& emptyCell : empties) {
                    emptyBorderCells.insert(emptyCell);
                }
            }

            // go through the empty border cells and see which ones should be revived
            for (const auto& p : emptyBorderCells) {
                int neighborCount = getNeighbors(p).size();

                // if an empty cell has 3 alive neighbors, revive it.
                if (neighborCount == 3) {
                    toRevive.push_front(p);
                }
            }

            // revive and kill appropriate cells.
            ReviveAllTagged();
            KillAllTagged();

            // clear border cells for this iteration.
            emptyBorderCells.clear();
        EndDrawing();
    }

    // de-initialization
    CloseWindow();
    
    return 0;
}


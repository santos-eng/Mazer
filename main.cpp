#include <iostream>
#include <string>
#include <Windows.h>
#include <cmath>

int screenWidth = 120;
int screenHeight = 40;

// Player Position
float playerX = 8.0f;
float playerY = 8.0f;
float playerA = 0.0f; // player view angle

// Minimap size
int mapHeight = 16;
int mapWidth = 16;

float FOV = 3.14159f / 4.0f;
float depth = 16.0f;

int main() {
    wchar_t *screen = new wchar_t[screenWidth * screenHeight];
    HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);    
    SetConsoleActiveScreenBuffer(console);
    DWORD bytesWritten = 0;

    std::wstring map;
    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    // Game Loop
    while (1) {
        for (int x = 0; x < screenWidth; x++) {
            // playerA bisects the FOV, so calculate rayAngles from -FOV/2 to +FOV/2
            float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)screenWidth) * FOV;

            float distanceToWall = 0;
            bool hitWall = false;

            // Create unit vector to represent view direction
            float eyeX = std::sinf(rayAngle);
            float eyeY = std::cosf(rayAngle);
 

            while (!hitWall && distanceToWall < depth) {
                distanceToWall += 0.1f;

                // Ray marching to determine distance each ray in FOV is from walls
                // Walls only on integer boundaries, i.e. dist of 1.5 away is only 1 cell away
                int testX = (int)(playerX + eyeX * distanceToWall);
                int testY = (int)(playerY + eyeY * distanceToWall);
                
                // Check out of bounds
                if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight) {
                    hitWall = true;
                    distanceToWall = depth;
                }
                // When ray is indounds need to check if its cell is a wall
                else {
                    // Convert x and y into a 1D plane
                    if (map[testY * mapWidth + testX] == '#') { 
                        hitWall = true; 
                    }
                }
            } 
            // Walls that are further away appear to have larger ceilings and floors
            int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
            int floor = screenHeight - ceiling;

            for (int y = 0; y < screenHeight; y++) {
                if (y < ceiling) // top to bottom y coord
                    screen[y * screenWidth + x] = ' ';
                else if (y > ceiling && y <= floor)
                    screen[y * screenWidth + x] = '#';
                else 
                    screen[y * screenWidth + x] = ' '; 
            }

        }
        screen[screenWidth * screenHeight - 1] = '\0'; // Stop outputting the string 
        WriteConsoleOutputCharacterW(console, screen, screenWidth * screenHeight, {0,0}, &bytesWritten);
    }



    return 0;
}
#include <iostream>
#include <Windows.h>

int screenWidth = 120;
int screenHeight = 40;

// Player Position
float playerX = 0.0f;
float playerY = 0.0f;
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
            float eyeX = sinf(rayAngle);
            float eyeY = cosf(rayAngle);


            while (!hitWall && distanceToWall < depth) {
                distanceToWall += 0.1f;

                // Ray marching to determine distance each ray in FOV is from walls
                int testX = (int)(playerX + eyeX * distanceToWall);
                int testY = (int)(playerY + eyeY * distanceToWall);
            }
        }
    }


    screen[screenWidth * screenHeight - 1] = '\0'; // Stop outputting the string 
    WriteConsoleOutputCharacter(console, screen, screenWidth * screenHeight, {0,0}, &bytesWritten);
    return 0;
}
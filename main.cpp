#include <iostream>
#include <string>
#include <Windows.h>
#include <cmath>
#include <chrono>
#include <vector>
#include <algorithm>
// to do, use the mouse for moving around in ccw and cw dir
// pause screen to change fov

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
    map += L"#.......#......#";
    map += L"#.......#......#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..........#...#";
    map += L"#..........#...#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#......#########";
    map += L"#..............#";
    map += L"################";

    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = std::chrono::system_clock::now();

    // Game Loop
    while (1) {
        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float timeDelta = elapsedTime.count(); // tick count


        // CCW Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000) // MSB is high when key pressed
            playerA -= (1.5f) * timeDelta;
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            playerA += (1.5f) * timeDelta;
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
            playerX += std::sinf(playerA) * 5.0f * timeDelta;
            playerY += std::cosf(playerA) * 5.0f * timeDelta;
            if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
                playerX -= std::sinf(playerA) * 5.0f * timeDelta;
                playerY -= std::cosf(playerA) * 5.0f * timeDelta;
            }
        }
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000 && map[(int)playerY * mapWidth + (int)playerX] != '#') {
            playerX -= std::sinf(playerA) * 5.0f * timeDelta;
            playerY -= std::cosf(playerA) * 5.0f * timeDelta;
            if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
                playerX += std::sinf(playerA) * 5.0f * timeDelta;
                playerY += std::cosf(playerA) * 5.0f * timeDelta;
            }
        }
        
        // for every ray in the fov
        for (int x = 0; x < screenWidth; x++) {
            // playerA bisects the FOV, so calculate rayAngles from -FOV/2 to +FOV/2
            float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)screenWidth) * FOV;

            float distanceToWall = 0;
            bool hitWall = false;
            bool boundary = false; // checking for the corner of a cell

            // Create unit vector to represent view direction. Unit vec of (0,1) indicates straight down on minimap
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

                        // Calculate the two closest perfect corners and cast a ray from said corner to player
                        // Compare this angle to rays cast from player. 
                        // Two smallest angle deltas will be corners of boundary cells

                        std::vector<std::pair<float,float>> p;

                        // loop over four corners of each cell
                        for (int tx = 0; tx < 2; tx++) {
                            for (int ty = 0; ty < 2; ty++) {
                                //vector from perfect corner to player
                                float vy = (float)testY + ty - playerY;
                                float vx = (float)testX + tx - playerX;
                                float dist = sqrt(vx*vx + vy*vy);
                                float dotProd = (eyeX * vx / dist) + (eyeY * vy / dist);
                                p.push_back(std::make_pair(dist,dotProd));
                            }
                        }

                        // Find the two smallest angles, only O(4) ~ O(1) time
                        std::sort(p.begin(),p.end(),[](const std::pair<float,float>& a, const std::pair<float,float>& b) {return a.first < b.first; });
                        float bound = 0.01;
                        if (acos(p.at(0).second) < bound) // If the angle between the perfect corner ray and current ray < 0.05 rad
                            boundary = true;
                        if (acos(p.at(1).second) < bound) 
                            boundary = true;
                        //if (acos(p.at(2).second) < bound) 
                            //boundary = true;
                
                    }
                }
            } 
            // Walls that are further away appear to have larger ceilings and floors
            int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
            int floor = screenHeight - ceiling;

            // hex vals: dark shade 2593, medium shade 2592, light shade 2591, full shade 2588
            short shade = ' ';

            if (distanceToWall <= depth / 4.0f)
                shade = 0x2588; // very close full shade
            else if (distanceToWall < depth / 3.0f)
                shade = 0x2593; // dark shade
            else if (distanceToWall < depth / 2.0f)
                shade = 0x2592; // medium shade
            else if (distanceToWall < depth)
                shade = 0x2591; // light shade
            else
                shade = ' '; // very far no shade

            if (boundary)
                shade = ' ';

            // top to bottom y coord
            for (int y = 0; y < screenHeight; y++) {
                if (y < ceiling) 
                    screen[y * screenWidth + x] = ' ';
                else if (y > ceiling && y <= floor)
                    screen[y * screenWidth + x] = shade;
                else {
                    float brightness = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f)); // close floors brighter
                    short floorShade = ' ';
                    if (brightness < 0.25)
                        floorShade = '#';
                    else if (brightness < 0.5) 
                        floorShade = 'x';
                    else if (brightness < 0.75) 
                        floorShade = '.';
                    else if (brightness < 0.9) 
                        floorShade = '-';
                    screen[y * screenWidth + x] = floorShade; 
                }
            }

        }
        //Stats
        swprintf_s(screen, 40, L"X=%3.2f, Y =%3.2f, A=%3.2f FPS=%3.2f", playerX, playerY, playerA, 1.0f / timeDelta);

        //minimap
        for (int nx = 0; nx < mapWidth; nx++) {
            for (int ny = 0; ny < mapHeight; ny++) {
                screen[(ny + 1)  *screenWidth + nx] = map[ny * mapWidth + nx];
            }   
        }

        screen[((int)playerY + 1) * screenWidth + (int)playerX] = 'P';

        screen[screenWidth * screenHeight - 1] = '\0'; // Stop outputting the string 
        WriteConsoleOutputCharacterW(console, screen, screenWidth * screenHeight, {0,0}, &bytesWritten);
    }



    return 0;
}
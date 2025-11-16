#include "olcConsoleGameEngine.h"
#include <string>
#include <cmath>
#include <algorithm>
// pause screen to change fov

class MazerFPS : public olcConsoleGameEngine {
    // Player Position
    float playerX = 8.0f;
    float playerY = 8.0f;
    float playerA = 0.0f; // player view angle

    // Minimap size
    int mapHeight = 16;
    int mapWidth = 16;

    float FOV = 3.14159f / 4.0f;
    float depth = 16.0f;

    std::wstring map;
public:
    MazerFPS() {
        m_sAppName = L"Mazer First Person Shooter";
    }

    virtual bool OnUserCreate() override {
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
        return true;
    }
    virtual bool OnUserUpdate(float timeDelta) override {
        // CCW Rotation
        if (m_keys[L'A'].bHeld)
            playerA -= (1.5f) * timeDelta;

        // CW Rotation
        if (m_keys[L'D'].bHeld)
            playerA += (1.5f) * timeDelta;
        
        // Forward Movement
        if (m_keys[L'W'].bHeld) {
            playerX += std::sinf(playerA) * 5.0f * timeDelta;
            playerY += std::cosf(playerA) * 5.0f * timeDelta;
            // Prevent walking out of bounds
            if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
                playerX -= std::sinf(playerA) * 5.0f * timeDelta;
                playerY -= std::cosf(playerA) * 5.0f * timeDelta;
            }
        }

        // Backward Movement     
        if (m_keys[L'S'].bHeld) {
            playerX -= std::sinf(playerA) * 5.0f * timeDelta;
            playerY -= std::cosf(playerA) * 5.0f * timeDelta;
            // Prevent reversing out of bounds
            if (map[(int)playerY * mapWidth + (int)playerX] == '#') {
                playerX += std::sinf(playerA) * 5.0f * timeDelta;
                playerY += std::cosf(playerA) * 5.0f * timeDelta;
            }
        }

        // for every ray in the fov
        for (int x = 0; x < ScreenWidth(); x++) {
            // playerA bisects the FOV, so calculate rayAngles from -FOV/2 to +FOV/2
            float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)ScreenWidth()) * FOV;

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
            int ceiling = (float)(ScreenHeight() / 2.0) - ScreenHeight() / ((float)distanceToWall);
            int floor = ScreenHeight() - ceiling;

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
            for (int y = 0; y < ScreenHeight(); y++) {
                if (y <= ceiling) 
                    Draw(x, y, L' ');
                else if (y > ceiling && y <= floor)
                    Draw(x, y, shade);
                else {
                    float brightness = 1.0f - (((float)y - ScreenHeight() / 2.0f) / ((float)ScreenHeight() / 2.0f)); // close floors brighter
                    short floorShade = ' ';
                    if (brightness < 0.25)
                        floorShade = '#';
                    else if (brightness < 0.5) 
                        floorShade = 'x';
                    else if (brightness < 0.75) 
                        floorShade = '.';
                    else if (brightness < 0.9) 
                        floorShade = '-';
                    Draw(x, y, floorShade);
                }
            }

        }

        //minimap
        for (int nx = 0; nx < mapWidth; nx++)
            for (int ny = 0; ny < mapHeight; ny++)
                Draw(nx + 1, ny + 1, map[ny * mapWidth + nx]);
        Draw(1 + (int)playerY, 1 + (int)playerX, L'P');

        return true;
    }

};



int main() {
    MazerFPS game;
    game.ConstructConsole(120, 80, 8, 8);
    game.Start();
    return 0;
}
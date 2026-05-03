#pragma once
#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")


//游戏常量定义
const int WIDTH = 1280;
const int HEIGHT = 720;
const int laneX[3] = { 446, 640, 834 };
const int OBSTACLE_GAP = 240;
const int SAFE_ROOM_RANGE = 160;

//障碍物结构体
struct Obstacle {
    int lane;
    int y;
    int type;
    bool exist;
    bool isWarning;
    DWORD warningStartTime;
    int aerialIdx;
};

//全局变量声明
extern int currentLane;
extern int pY, pW, pH;
extern bool isJump;
extern bool isCrouch;
extern int jumpVel;
extern const int gravity;
extern Obstacle obs[50];
extern int speed;
extern int score;
extern wchar_t scoreText[50];
extern DWORD lastSpawn;
extern const int SPAWN_INTERVAL;
extern int gameState;
extern bool animPlaying;
extern DWORD animStart;
extern float charY[4];
extern int difficulty;
extern bool showDifficulty;
extern IMAGE imgBackgroundStart;
extern IMAGE imgBackgroundGame;
extern IMAGE imgBackgroundEnd;
extern IMAGE imgObsGround;
extern IMAGE imgObsAerial;
extern IMAGE imgObsHead;
extern IMAGE imgObsHead2;
extern IMAGE imgPlayer1;
extern IMAGE imgPlayer2;
extern IMAGE imgPlayer3;
extern IMAGE imgPlayer4;
extern bool bgImageLoaded;
extern DWORD gameEndTime;
extern DWORD gameStartTime;
extern int currentMusicIndex;
extern bool isMusicPlaying;
extern bool musicStopped;
extern DWORD playerAnimTimer;
extern int playerAnimFrame;

//函数声明
void spawnObstacle();
void resetGame();
void drawStartUI();
void checkAllMouse();
void drawEndUI();
void drawExitButton();
void drawMusicButton();
void playMusic(int index);
void playRandomMusic();
void stopMusic();
void nextMusic();
void loadBackgroundImage();
void loadObstacleImages();
void loadPlayerImages();
void updatePlayerAnimation();
bool safeLoadImage(IMAGE* img, const TCHAR* filename); // 安全加载图片

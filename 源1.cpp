#include<graphics.h>
#include <conio.h>
#include <time.h>
#include <cstdio>
#include <stdlib.h>
#include <cstring>

const int WIDTH = 800;
const int HEIGHT = 600;

const int laneX[3] = { 300, 400, 500 };
const int OBSTACLE_GAP = 240;
const int SAFE_ROOM_RANGE = 160;

int currentLane = 1;
int pY = HEIGHT - 130;
int pW = 28;
int pH = 28;

bool isJump = false;
bool isCrouch = false;
int jumpVel = 0;
const int gravity = 2;

struct Obstacle {
    int lane;
    int y;
    int type;
    bool exist;
    bool isWarning;
    DWORD warningStartTime;
};
Obstacle obs[50];

int speed = 9;
int score = 0;
wchar_t scoreText[50];

DWORD lastSpawn = 0;
const int SPAWN_INTERVAL = 800;
int gameState = 0;

bool animPlaying = true;
DWORD animStart = 0;
float charY[4] = { 0,0,0,0 };

int difficulty = 0;
bool showDifficulty = false;

bool canSpawnInLane(int lane, int y) {
    for (int i = 0; i < 50; i++) {
        if (obs[i].exist && obs[i].lane == lane && abs(obs[i].y - y) < OBSTACLE_GAP)
            return false;
    }
    return true;
}

bool isLaneSafe(int y) {
    int cnt = 0;
    for (int i = 0; i < 50; i++) {
        if (obs[i].exist && obs[i].type == 1 && abs(obs[i].y - y) < SAFE_ROOM_RANGE) {
            cnt++;
            if (cnt >= 2) return false;
        }
    }
    return true;
}

void spawnObstacle() {
    int spawnY = -120;
    int lanes[3] = { 0,1,2 };

    for (int i = 0; i < 3; i++) {
        int r = rand() % 3;
        int tmp = lanes[i];
        lanes[i] = lanes[r];
        lanes[r] = tmp;
    }

    for (int k = 0; k < 3; k++) {
        int lane = lanes[k];
        if (!canSpawnInLane(lane, spawnY)) continue;

        int type = rand() % 3;
        if (type == 1 && !isLaneSafe(spawnY)) continue;

        for (int i = 0; i < 50; i++) {
            if (!obs[i].exist) {
                obs[i].lane = lane;
                obs[i].exist = true;
                obs[i].isWarning = false;
                obs[i].y = spawnY;
                obs[i].type = type;

                if (type == 2) {
                    obs[i].isWarning = true;
                    obs[i].warningStartTime = GetTickCount();
                }
                lastSpawn = GetTickCount();
                return;
            }
        }
    }
}

void resetGame() {
    memset(obs, 0, sizeof(obs));
    score = 0;
    currentLane = 1;
    pY = HEIGHT - 130;
    isJump = false;
    isCrouch = false;
    lastSpawn = GetTickCount();

    if (difficulty == 0) speed = 9;
    else if (difficulty == 1) speed = 13;
    else speed = 17;
}

void drawStartUI() {
    setbkcolor(RGB(20, 20, 40));
    cleardevice();

    DWORD t = GetTickCount() - animStart;

    if (animPlaying) {
        for (int i = 0; i < 4; i++) {
            if (t > i * 150 && charY[i] < 180) {
                charY[i] += 1.5f + (180 - charY[i]) * 0.08f;
                if (charY[i] > 180) charY[i] = 180;
            }
        }

        bool finished = true;
        for (int i = 0; i < 4; i++) {
            if (charY[i] < 178) { finished = false; break; }
        }
        if (finished) animPlaying = false;
    }

    settextstyle(100, 0, _T("华文行楷"));
    settextcolor(RGB(255, 200, 0));

    int charW = 100;
    int totalW = charW * 4;
    int startX = (WIDTH - totalW) / 2;

    wchar_t buf[2] = { 0 };
    buf[1] = L'\0';

    buf[0] = L'佐'; outtextxy(startX + charW * 0, (int)charY[0], buf);
    buf[0] = L'助'; outtextxy(startX + charW * 1, (int)charY[1], buf);
    buf[0] = L'快'; outtextxy(startX + charW * 2, (int)charY[2], buf);
    buf[0] = L'跑'; outtextxy(startX + charW * 3, (int)charY[3], buf);

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(GetHWnd(), &p);
    bool hoverStart = (p.x >= WIDTH / 2 - 130 && p.x <= WIDTH / 2 + 130 && p.y >= 350 && p.y <= 430);

    if (hoverStart) setfillcolor(RGB(80, 160, 255));
    else setfillcolor(RGB(50, 130, 255));
    fillroundrect(WIDTH / 2 - 130, 350, WIDTH / 2 + 130, 430, 25, 25);

    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(40, 0, _T("黑体"));
    outtextxy(WIDTH / 2 - textwidth(_T("开始游戏")) / 2, 375, _T("开始游戏"));

    if (hoverStart) {
        showDifficulty = true;
    }

    if (showDifficulty) {
        int diffY[] = { 450, 510, 570 };
        const wchar_t* diffText[] = { L"简单", L"中等", L"困难" };
        int diffColor[] = { RGB(100,200,100), RGB(255,180,0), RGB(255,80,80) };

        for (int i = 0; i < 3; i++) {
            bool hoverDiff = (p.x >= WIDTH / 2 - 100 && p.x <= WIDTH / 2 + 100 && p.y >= diffY[i] && p.y <= diffY[i] + 50);

            if (hoverDiff) setfillcolor(diffColor[i]);
            else setfillcolor(RGB(70, 70, 100));

            fillroundrect(WIDTH / 2 - 100, diffY[i], WIDTH / 2 + 100, diffY[i] + 50, 15, 15);
            settextcolor(WHITE);
            settextstyle(32, 0, _T("黑体"));
            outtextxy(WIDTH / 2 - textwidth(diffText[i]) / 2, diffY[i] + 8, diffText[i]);
        }
    }
}

void checkAllMouse() {
    if (!MouseHit()) return;
    MOUSEMSG m = GetMouseMsg();
    if (m.uMsg != WM_LBUTTONDOWN) return;

    POINT p = { m.x, m.y };

    if (gameState == 0) {
        if (p.x >= WIDTH / 2 - 130 && p.x <= WIDTH / 2 + 130 && p.y >= 350 && p.y <= 430) {
            difficulty = 0;
            resetGame();
            gameState = 1;
            return;
        }

        int diffY[] = { 450, 510, 570 };
        for (int i = 0; i < 3; i++) {
            if (p.x >= WIDTH / 2 - 100 && p.x <= WIDTH / 2 + 100 && p.y >= diffY[i] && p.y <= diffY[i] + 50) {
                difficulty = i;
                resetGame();
                gameState = 1;
                return;
            }
        }
    }

    if (gameState == 2) {
        int textH = textheight(L"我愚蠢的弟弟啊......");
        int textY = HEIGHT / 2 - textH / 2 - 60;
        int btnY = textY + textH + 40;

        if (p.x >= WIDTH / 2 - 130 && p.x <= WIDTH / 2 + 130 && p.y >= btnY && p.y <= btnY + 80) {
            resetGame();
            gameState = 1;
            return;
        }

        int btnHomeY = btnY + 80 + 20;
        if (p.x >= WIDTH / 2 - 130 && p.x <= WIDTH / 2 + 130 && p.y >= btnHomeY && p.y <= btnHomeY + 80) {
            gameState = 0;
            showDifficulty = false;
            return;
        }
    }

    int x1 = WIDTH - 160, y1 = HEIGHT - 70, x2 = WIDTH - 30, y2 = HEIGHT - 20;
    if (p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2) {
        closegraph();
        exit(0);
    }
}

void drawEndUI() {
    settextstyle(60, 0, _T("华文行楷"));
    settextcolor(RGB(255, 200, 0));
    setbkmode(TRANSPARENT);
    wchar_t endText[] = L"我愚蠢的弟弟啊......";

    int textW = textwidth(endText);
    int textH = textheight(endText);
    int textX = WIDTH / 2 - textW / 2 + 20;
    int textY = HEIGHT / 2 - textH / 2 - 60;

    outtextxy(textX, textY, endText);

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(GetHWnd(), &p);

    int btnY = textY + textH + 40;
    bool hoverRestart = (p.x >= WIDTH / 2 - 130 && p.x <= WIDTH / 2 + 130 && p.y >= btnY && p.y <= btnY + 80);
    if (hoverRestart) setfillcolor(RGB(80, 160, 255));
    else setfillcolor(RGB(50, 130, 255));
    fillroundrect(WIDTH / 2 - 130, btnY, WIDTH / 2 + 130, btnY + 80, 25, 25);

    settextcolor(WHITE);
    settextstyle(40, 0, _T("黑体"));
    outtextxy(WIDTH / 2 - textwidth(L"重新开始") / 2, btnY + 25, L"重新开始");

    int btnHomeY = btnY + 80 + 20;
    bool hoverHome = (p.x >= WIDTH / 2 - 130 && p.x <= WIDTH / 2 + 130 && p.y >= btnHomeY && p.y <= btnHomeY + 80);
    if (hoverHome) setfillcolor(RGB(130, 130, 130));
    else setfillcolor(RGB(100, 100, 100));
    fillroundrect(WIDTH / 2 - 130, btnHomeY, WIDTH / 2 + 130, btnHomeY + 80, 25, 25);

    settextcolor(WHITE);
    outtextxy(WIDTH / 2 - textwidth(L"返回主页") / 2, btnHomeY + 25, L"返回主页");
}

void drawExitButton() {
    int x1 = WIDTH - 160, y1 = HEIGHT - 70, x2 = WIDTH - 30, y2 = HEIGHT - 20;
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(GetHWnd(), &p);
    bool h = (p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2);
    if (h) setfillcolor(RGB(255, 60, 60));
    else setfillcolor(RGB(220, 40, 40));
    fillroundrect(x1, y1, x2, y2, 15, 15);
    setlinecolor(WHITE);
    setlinestyle(PS_SOLID, 2);
    roundrect(x1, y1, x2, y2, 15, 15);
    settextcolor(WHITE);
    settextstyle(26, 0, _T("黑体"));
    int tx = (x1 + x2) / 2 - textwidth(L"退出游戏") / 2;
    int ty = (y1 + y2) / 2 - 13;
    outtextxy(tx, ty, L"退出游戏");
}

int main() {
    initgraph(WIDTH, HEIGHT);
    srand((unsigned)time(NULL));
    BeginBatchDraw();

    const int FPS = 60;
    const DWORD frameDelay = 1000 / FPS;
    DWORD frameStart;

    while (true) {
        frameStart = GetTickCount();
        cleardevice();
        checkAllMouse();

        if (gameState == 0) {
            drawStartUI();
            drawExitButton();
            FlushBatchDraw();
        }
        else if (gameState == 2) {
            setbkcolor(RGB(20, 20, 40));
            line(240, 0, 240, HEIGHT);
            line(340, 0, 340, HEIGHT);
            line(440, 0, 440, HEIGHT);
            line(540, 0, 540, HEIGHT);

            swprintf_s(scoreText, L"分数：%d", score);
            settextcolor(RGB(255, 210, 0));
            outtextxy(20, 20, scoreText);

            drawEndUI();
            drawExitButton();
            FlushBatchDraw();
        }
        else {
            setbkcolor(RGB(20, 20, 40));
            setlinecolor(WHITE);
            line(240, 0, 240, HEIGHT);
            line(340, 0, 340, HEIGHT);
            line(440, 0, 440, HEIGHT);
            line(540, 0, 540, HEIGHT);

            static DWORD keyCool = 0;
            DWORD now = GetTickCount();
            if (now - keyCool > 50) {
                if (GetAsyncKeyState('A') & 0x8000 && currentLane > 0) {
                    currentLane--; keyCool = now;
                }
                if (GetAsyncKeyState('D') & 0x8000 && currentLane < 2) {
                    currentLane++; keyCool = now;
                }
            }

            isCrouch = (GetAsyncKeyState('S') & 0x8000) != 0;

            if (GetAsyncKeyState('W') & 0x8000 && !isJump && !isCrouch) {
                isJump = true; jumpVel = -22;
            }

            if (isJump) {
                pY += jumpVel; jumpVel += gravity;
                if (pY >= HEIGHT - 130) {
                    pY = HEIGHT - 130; isJump = false; jumpVel = 0;
                }
            }

            if (GetTickCount() - lastSpawn > SPAWN_INTERVAL) {
                spawnObstacle();
            }

            bool crash = false;
            int pr, pb;

            if (isCrouch) {
                pr = pY + pH / 2 + 20;
                pb = pr + pH / 2 - 20;
            }
            else {
                pr = pY;
                pb = pr + pH;
            }

            for (int i = 0; i < 50; i++) {
                if (!obs[i].exist) continue;

                if (obs[i].type == 2 && obs[i].isWarning) {
                    if (now - obs[i].warningStartTime >= 1500) {
                        obs[i].isWarning = false;
                        obs[i].y = HEIGHT - 90;
                    }
                }
                else if (!obs[i].isWarning) {
                    if (obs[i].type == 2)
                        obs[i].y -= speed;
                    else
                        obs[i].y += speed;
                }

                if ((obs[i].type == 2 && obs[i].y < -500) || (obs[i].type != 2 && obs[i].y > HEIGHT + 300)) {
                    obs[i].exist = false;
                    score++;
                }

                if (obs[i].exist && !obs[i].isWarning && obs[i].lane == currentLane) {
                    int oy = obs[i].y;
                    int oh = 18;
                    if (obs[i].type == 1) oh = 28;
                    if (obs[i].type == 2) oh = 90;

                    // 核心躲避逻辑
                    if (obs[i].type == 0 && isJump) { // 红色 -> 跳跃躲避
                        continue;
                    }
                    if (obs[i].type == 2 && isCrouch) { // 绿色 -> 蹲下躲避
                        continue;
                    }

                    if (!(pb < oy || pr > oy + oh)) {
                        crash = true;
                        break;
                    }
                }
            }

            if (crash) {
                gameState = 2;
            }

            for (int i = 0; i < 50; i++) {
                if (!obs[i].exist) continue;
                int x = laneX[obs[i].lane];

                if (obs[i].type == 0) {
                    setfillcolor(RGB(230, 90, 90));
                    solidrectangle(x - 10, obs[i].y, x + 10, obs[i].y + 18);
                }
                else if (obs[i].type == 1) {
                    setfillcolor(RGB(60, 180, 255));
                    solidrectangle(x - 14, obs[i].y, x + 14, obs[i].y + 28);
                }
                else if (obs[i].type == 2) {
                    if (obs[i].isWarning) {
                        setcolor(RED);
                        line(x, HEIGHT - 180, x, HEIGHT);
                    }
                    else {
                        setfillcolor(RGB(80, 190, 110));
                        solidrectangle(x - 16, obs[i].y, x + 16, obs[i].y + 90);
                    }
                }
            }

            setfillcolor(RGB(255, 200, 0));
            if (isCrouch) {
                solidrectangle(laneX[currentLane] - 15, pr, laneX[currentLane] + 15, pr + 10);
            }
            else {
                solidrectangle(laneX[currentLane] - 15, pr, laneX[currentLane] + 15, pr + pH);
            }

            settextcolor(RGB(255, 210, 0));
            swprintf_s(scoreText, L"分数：%d", score);
            outtextxy(20, 20, scoreText);

            drawExitButton();
            FlushBatchDraw();
        }

        DWORD cost = GetTickCount() - frameStart;
        if (cost < frameDelay) Sleep(frameDelay - cost);
    }

    closegraph();
    return 0;
}

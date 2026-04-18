#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <cstdio>
#include <stdlib.h>
#include <cstring>

const int WIDTH = 800;
const int HEIGHT = 600;

const int laneX[3] = { 300, 400, 500 };
const int OBSTACLE_GAP = 240;
const int SAFE_ROW_RANGE = 160;

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

int speed = 7;
int score = 0;
wchar_t scoreText[50];

DWORD lastSpawn = 0;
const int SPAWN_INTERVAL = 800;
int gameState = 0; // 0:开始界面 1:游戏中 2:结束界面

bool animPlaying = true;
DWORD animStart = 0;
float charY[4] = { 0,0,0,0 };

bool canSpawnInLane(int lane, int y) {
    for (int i = 0; i < 50; i++) {
        if (obs[i].exist && obs[i].lane == lane && abs(obs[i].y - y) < OBSTACLE_GAP)
            return false;
    }
    return true;
}

bool isLaneChangeSafe(int y) {
    int cnt = 0;
    for (int i = 0; i < 50; i++) {
        if (obs[i].exist && obs[i].type == 1 && abs(obs[i].y - y) < SAFE_ROW_RANGE) {
            cnt++;
            if (cnt >= 2) return false;
        }
    }
    return true;
}

void spawnObstacle() {
    int spawnY = -120;
    int lanes[3] = { 0, 1, 2 };

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
        if (type == 1 && !isLaneChangeSafe(spawnY)) continue;

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
}

void drawStartUI() {
    IMAGE bg;

    if (loadimage(&bg, _T("start_bg.jpg"))) {
        putimage(0, 0, &bg);
    }
    else {

        setbkcolor(RGB(20, 20, 40));
        cleardevice();
    }

    DWORD t = GetTickCount() - animStart;

    if (animPlaying) {
        for (int i = 0; i < 4; i++) {
            if (t > i * 150 && charY[i] < 180) {
                charY[i] += 1.5f + (180 - charY[i]) * 0.08f;
            }
        }

        bool finished = true;
        for (int i = 0; i < 4; i++) {
            if (charY[i] < 178) finished = false;
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

    setfillcolor(RGB(50, 130, 255));
    fillroundrect(WIDTH / 2 - 130, 350, WIDTH / 2 + 130, 430, 25, 25);

    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(40, 0, _T("黑体"));
    outtextxy(WIDTH / 2 - textwidth(_T("开始游戏")) / 2, 375, _T("开始游戏"));
}

void checkStartClick() {
    if (MouseHit()) {
        MOUSEMSG m = GetMouseMsg();
        if (m.uMsg == WM_LBUTTONDOWN) {
            if (m.x >= WIDTH / 2 - 130 && m.x <= WIDTH / 2 + 130 && m.y >= 350 && m.y <= 430) {
                gameState = 1;
                resetGame();
            }
        }
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

    int btnY = textY + textH + 40;
    setfillcolor(RGB(50, 130, 255));
    fillroundrect(WIDTH / 2 - 130, btnY, WIDTH / 2 + 130, btnY + 80, 25, 25);

    settextcolor(WHITE);
    settextstyle(40, 0, _T("黑体"));
    wchar_t btnText[] = L"重新开始";
    int btnTextX = WIDTH / 2 - textwidth(btnText) / 2;
    int btnTextY = btnY + 25;
    outtextxy(btnTextX, btnTextY, btnText);

    int btnHomeY = btnY + 80 + 20;
    setfillcolor(RGB(100, 100, 100));
    fillroundrect(WIDTH / 2 - 130, btnHomeY, WIDTH / 2 + 130, btnHomeY + 80, 25, 25);

    wchar_t btnHomeText[] = L"返回主页";
    int btnHomeTextX = WIDTH / 2 - textwidth(btnHomeText) / 2;
    int btnHomeTextY = btnHomeY + 25;
    outtextxy(btnHomeTextX, btnHomeTextY, btnHomeText);
}

void checkRestartClick() {
    if (MouseHit()) {
        MOUSEMSG m = GetMouseMsg();
        if (m.uMsg == WM_LBUTTONDOWN) {
            wchar_t endText[] = L"我愚蠢的弟弟啊......";
            int textH = textheight(endText);
            int textY = HEIGHT / 2 - textH / 2 - 60;
            int btnY = textY + textH + 40;

            if (m.x >= WIDTH / 2 - 130 && m.x <= WIDTH / 2 + 130 && m.y >= btnY && m.y <= btnY + 80) {
                resetGame();
                gameState = 1;
                return;
            }

            int btnHomeY = btnY + 80 + 20;
            if (m.x >= WIDTH / 2 - 130 && m.x <= WIDTH / 2 + 130 && m.y >= btnHomeY && m.y <= btnHomeY + 80) {
                gameState = 0;
                animStart = GetTickCount();
                animPlaying = true;
                for (int i = 0; i < 4; i++) charY[i] = 0;
                return;
            }
        }
    }
}

void drawExitButton() {
    int x1 = WIDTH - 160;
    int y1 = HEIGHT - 70;
    int x2 = WIDTH - 30;
    int y2 = HEIGHT - 20;

    setfillcolor(RGB(220, 40, 40));
    fillroundrect(x1, y1, x2, y2, 15, 15);
    setlinecolor(WHITE);
    setlinestyle(PS_SOLID, 2);
    roundrect(x1, y1, x2, y2, 15, 15);

    settextcolor(WHITE);
    settextstyle(26, 0, _T("黑体"));
    int tx = (x1 + x2) / 2 - textwidth(_T("退出游戏")) / 2;
    int ty = (y1 + y2) / 2 - 13;
    outtextxy(tx, ty, _T("退出游戏"));
}

void checkExitClick() {
    if (MouseHit()) {
        MOUSEMSG m = GetMouseMsg();
        if (m.uMsg == WM_LBUTTONDOWN) {
            if (m.x >= WIDTH - 160 && m.x <= WIDTH - 30 && m.y >= HEIGHT - 70 && m.y <= HEIGHT - 20) {
                closegraph();
                exit(0);
            }
        }
    }
}

int main() {
    initgraph(WIDTH, HEIGHT);
    srand((unsigned)time(NULL));
    BeginBatchDraw();

    animStart = GetTickCount();
    animPlaying = true;
    for (int i = 0; i < 4; i++) charY[i] = 0;

    while (true) {
        DWORD now = GetTickCount();
        cleardevice();

        if (gameState == 0) {
            drawStartUI();
            checkStartClick();
            FlushBatchDraw();
            Sleep(10);
            continue;
        }

        if (gameState == 2) {
            setbkcolor(RGB(20, 20, 40));
            setlinecolor(WHITE);
            setlinestyle(PS_SOLID, 1);
            line(240, 0, 240, HEIGHT);
            line(340, 0, 340, HEIGHT);
            line(440, 0, 440, HEIGHT);
            line(540, 0, 540, HEIGHT);

            setbkmode(TRANSPARENT);
            settextcolor(RGB(255, 210, 0));
            settextstyle(28, 0, _T("黑体"));
            swprintf_s(scoreText, L"分数：%d", score);
            outtextxy(20, 20, scoreText);

            for (int i = 0; i < 50; i++) {
                if (!obs[i].exist) continue;
                int lx = laneX[obs[i].lane];
                if (obs[i].type == 2 && obs[i].isWarning) {
                    setcolor(RED);
                    line(lx, HEIGHT - 180, lx, HEIGHT);
                }
                else if (obs[i].type == 0) {
                    setfillcolor(RGB(230, 90, 90));
                    solidrectangle(lx - 10, obs[i].y, lx + 10, obs[i].y + 18);
                }
                else if (obs[i].type == 1) {
                    setfillcolor(RGB(60, 180, 255));
                    solidrectangle(lx - 14, obs[i].y, lx + 14, obs[i].y + 28);
                }
            }

            int realH = isCrouch ? pH / 2 : pH;
            int pRenderY = isCrouch ? pY + pH / 2 : pY;
            int pX = laneX[currentLane];
            setfillcolor(RGB(255, 200, 0));
            solidrectangle(pX - 15, pRenderY, pX + 15, pRenderY + realH);

            for (int i = 0; i < 50; i++) {
                if (!obs[i].exist || obs[i].type != 2 || obs[i].isWarning) continue;
                int lx = laneX[obs[i].lane];
                setfillcolor(RGB(80, 190, 110));
                solidrectangle(lx - 16, obs[i].y, lx + 16, obs[i].y + 90);
            }

            drawEndUI();
            drawExitButton();
            checkRestartClick();
            checkExitClick();

            FlushBatchDraw();
            Sleep(10);
            continue;
        }

        setbkcolor(RGB(20, 20, 40));
        setlinecolor(WHITE);
        setlinestyle(PS_SOLID, 1);
        line(240, 0, 240, HEIGHT);
        line(340, 0, 340, HEIGHT);
        line(440, 0, 440, HEIGHT);
        line(540, 0, 540, HEIGHT);

        setbkmode(TRANSPARENT);
        settextcolor(RGB(255, 210, 0));
        settextstyle(28, 0, _T("黑体"));
        swprintf_s(scoreText, L"分数：%d", score);
        outtextxy(20, 20, scoreText);

        drawExitButton();
        checkExitClick();

        if (GetAsyncKeyState('A') & 0x8000 && currentLane > 0) { currentLane--; Sleep(120); }
        if (GetAsyncKeyState('D') & 0x8000 && currentLane < 2) { currentLane++; Sleep(120); }
        if (GetAsyncKeyState('W') & 0x8000 && !isJump) { isJump = true; jumpVel = -22; }
        isCrouch = (GetAsyncKeyState('S') & 0x8000) > 0;

        if (isJump) {
            pY += jumpVel; jumpVel += gravity;
            if (pY >= HEIGHT - 130) { pY = HEIGHT - 130; isJump = false; jumpVel = 0; }
        }

        int realH = isCrouch ? pH / 2 : pH;
        int pRenderY = isCrouch ? pY + pH / 2 : pY;
        int playerBottom = pRenderY + realH;

        if (now - lastSpawn >= SPAWN_INTERVAL) spawnObstacle();

        for (int i = 0; i < 50; i++) {
            if (!obs[i].exist) continue;
            if (obs[i].type == 2 && obs[i].isWarning) {
                if (now - obs[i].warningStartTime >= 1500) {
                    obs[i].isWarning = false;
                    obs[i].y = HEIGHT;
                }
            }
            if (!obs[i].isWarning) {
                obs[i].y += (obs[i].type == 2) ? -speed : speed;
            }
            if (obs[i].y < -300 || obs[i].y > HEIGHT + 300) {
                obs[i].exist = false;
                score++;
                continue;
            }
        }

        for (int i = 0; i < 50; i++) {
            if (!obs[i].exist) continue;
            int lx = laneX[obs[i].lane];
            if (obs[i].type == 2 && obs[i].isWarning) {
                setcolor(RED);
                line(lx, HEIGHT - 180, lx, HEIGHT);
            }
            else if (obs[i].type == 0) {
                setfillcolor(RGB(230, 90, 90));
                solidrectangle(lx - 10, obs[i].y, lx + 10, obs[i].y + 18);
            }
            else if (obs[i].type == 1) {
                setfillcolor(RGB(60, 180, 255));
                solidrectangle(lx - 14, obs[i].y, lx + 14, obs[i].y + 28);
            }
        }

        bool crash = false;
        for (int i = 0; i < 50; i++) {
            if (!obs[i].exist || obs[i].lane != currentLane || obs[i].isWarning) continue;
            int oy = obs[i].y;
            int oh = 18;
            if (obs[i].type == 1) oh = 28;
            if (obs[i].type == 2) oh = 90;

            if (!(playerBottom < oy || pRenderY > oy + oh)) {
                if ((obs[i].type == 0 && !isJump) || (obs[i].type == 2 && !isCrouch) || obs[i].type == 1) {
                    crash = true;
                    break;
                }
            }
        }

        if (crash) {
            gameState = 2;
        }

        int pX = laneX[currentLane];
        setfillcolor(RGB(255, 200, 0));
        solidrectangle(pX - 15, pRenderY, pX + 15, pRenderY + realH);

        for (int i = 0; i < 50; i++) {
            if (!obs[i].exist || obs[i].type != 2 || obs[i].isWarning) continue;
            int lx = laneX[obs[i].lane];
            setfillcolor(RGB(80, 190, 110));
            solidrectangle(lx - 16, obs[i].y, lx + 16, obs[i].y + 90);
        }

        FlushBatchDraw();
        Sleep(1);
    }

    closegraph();
    return 0;
}
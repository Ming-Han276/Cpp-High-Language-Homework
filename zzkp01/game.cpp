#include "game.h"

//全局变量定义
int currentLane = 1;
int pY = HEIGHT * 3 / 4;
int pW = 28;
int pH = 28;
bool isJump = false;
bool isCrouch = false;
int jumpVel = 0;
const int gravity = 2;
Obstacle obs[50];
int speed = 9;
int score = 0;
wchar_t scoreText[50];
DWORD lastSpawn = 0;
const int SPAWN_INTERVAL = 800;
int gameState = 0;
bool animPlaying = true;
DWORD animStart = 0;
float charY[4] = { 0 };
int difficulty = 0;
bool showDifficulty = false;
IMAGE imgBackgroundStart;
IMAGE imgBackgroundGame;
IMAGE imgBackgroundEnd;
IMAGE imgObsGround;
IMAGE imgObsAerial;
IMAGE imgObsHead;
IMAGE imgObsHead2;
bool bgImageLoaded = false;
DWORD gameEndTime;
DWORD gameStartTime;
int altHead = 0;    // 头顶障碍物图片交替索引
int currentMusicIndex = 0;
bool isMusicPlaying = false;
bool musicStopped = false;  // 是否手动停止音乐
IMAGE imgPlayer1;      // 正常状态
IMAGE imgPlayer2;      // 正常状态2
IMAGE imgPlayer3;      // 跳跃状态
IMAGE imgPlayer4;      // 下蹲状态
DWORD playerAnimTimer = 0;  // 计时器
int playerAnimFrame = 0;

//加载背景图片
void loadBackgroundImage() {
    bgImageLoaded = false;

    if (loadimage(&imgBackgroundStart, _T("bg_start.png"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    } else if (loadimage(&imgBackgroundStart, _T("bg_start.jpg"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    } else if (loadimage(&imgBackgroundStart, _T("bg_start.bmp"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    }

    if (loadimage(&imgBackgroundGame, _T("bg_game.png"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    } else if (loadimage(&imgBackgroundGame, _T("bg_game.jpg"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    } else if (loadimage(&imgBackgroundGame, _T("bg_game.bmp"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    }

    if (loadimage(&imgBackgroundEnd, _T("bg_end.png"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    } else if (loadimage(&imgBackgroundEnd, _T("bg_end.jpg"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    } else if (loadimage(&imgBackgroundEnd, _T("bg_end.bmp"), WIDTH, HEIGHT)) {
        bgImageLoaded = true;
    }
}

//安全加载图片函数
bool safeLoadImage(IMAGE* img, const TCHAR* filename) {
    if (img == NULL) return false;
    if (!loadimage(img, filename)) {
        return false;
    }
    return true;
}

//加载障碍物图片
void loadObstacleImages() {
    safeLoadImage(&imgObsGround, _T("obs_ground.png"));
    safeLoadImage(&imgObsAerial, _T("obs_aerial.png"));
    safeLoadImage(&imgObsHead, _T("obs_head.png"));
    safeLoadImage(&imgObsHead2, _T("obs_head2.png"));
}

//加载玩家图片
void loadPlayerImages() {
    safeLoadImage(&imgPlayer1, _T("pic1.png"));
    safeLoadImage(&imgPlayer2, _T("pic2.png"));
    safeLoadImage(&imgPlayer3, _T("pic3.png"));
    safeLoadImage(&imgPlayer4, _T("pic4.png"));
}

//检查指定车道指定位置是否可以生成障碍物
bool canSpawnInLane(int lane, int y) {
    for (int i = 0; i < 50; i++) {
        if (obs[i].exist && obs[i].lane == lane && abs(obs[i].y - y) < OBSTACLE_GAP)
            return false;
    }
    return true;
}

//检查指定位置是否安全
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

//障碍物生成函数
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
        if (lane < 0 || lane >= 3) continue;
        if (!canSpawnInLane(lane, spawnY)) continue;

        int type = rand() % 3;
        if (type < 0 || type >= 3) type = 0;
        if (type == 1 && !isLaneSafe(spawnY)) continue;

        for (int i = 0; i < 50; i++) {
            if (!obs[i].exist) {
                obs[i].lane = lane;
                obs[i].exist = true;
                obs[i].isWarning = false;
                obs[i].y = spawnY;
                obs[i].type = type;
                obs[i].aerialIdx = 0;

                if (type == 2) {
                    obs[i].isWarning = true;
                    obs[i].warningStartTime = GetTickCount();
                    obs[i].aerialIdx = altHead;
                    altHead = 1 - altHead;
                }
                lastSpawn = GetTickCount();
                return;
            }
        }
    }
}

//游戏重置
void resetGame() {
    memset(obs, 0, sizeof(obs));
    score = 0;
    currentLane = 1;
    pY = HEIGHT * 3 / 4;
    isJump = false;
    isCrouch = false;
    lastSpawn = GetTickCount();
    playerAnimTimer = GetTickCount();
    playerAnimFrame = 0;

    if (difficulty == 0) speed = 9;
    else if (difficulty == 1) speed = 13;
    else speed = 17;
}

//获取鼠标当前位置
void getMousePosInWindow(POINT* p) {
    GetCursorPos(p);
    ScreenToClient(GetHWnd(), p);
}

//检查鼠标是否在指定矩形区域内
bool isMouseInRect(int px, int py, int rx1, int ry1, int rx2, int ry2) {
    return (px >= rx1 && px <= rx2 && py >= ry1 && py <= ry2);
}

//播放千夜
void playMusic(int index) {
    stopMusic();
    if (index < 1 || index > 3) index = 1;
    wchar_t cmd[256];
    wchar_t alias[16];
    swprintf_s(alias, L"music%d", index);
    swprintf_s(cmd, L"open music%d.mp3 alias %s", index, alias);
    mciSendString(cmd, NULL, 0, NULL);
    swprintf_s(cmd, L"play %s repeat", alias);
    mciSendString(cmd, NULL, 0, NULL);
    currentMusicIndex = index;
    isMusicPlaying = true;
}

//随机播放
void playRandomMusic() {
    int randomIndex = rand() % 3 + 1;
    playMusic(randomIndex);
}

//停止播放
void stopMusic() {
    mciSendString(L"stop music1", NULL, 0, NULL);
    mciSendString(L"close music1", NULL, 0, NULL);
    mciSendString(L"stop music2", NULL, 0, NULL);
    mciSendString(L"close music2", NULL, 0, NULL);
    mciSendString(L"stop music3", NULL, 0, NULL);
    mciSendString(L"close music3", NULL, 0, NULL);
    isMusicPlaying = false;
}

//切换到下一首
void nextMusic() {
    int nextIndex = currentMusicIndex % 3 + 1;
    playMusic(nextIndex);
}

void drawMusicButton() {
    settextstyle(28, 0, _T("\x9ED1\x4F53"));
    setbkmode(TRANSPARENT);
    
    int btnNextX1 = WIDTH - 160;
    int btnNextY1 = 20;
    int btnNextX2 = WIDTH - 90;
    int btnNextY2 = 70;

    POINT p;
    getMousePosInWindow(&p);
    bool hoverNext = isMouseInRect(p.x, p.y, btnNextX1, btnNextY1, btnNextX2, btnNextY2);

    if (hoverNext) setfillcolor(RGB(120, 80, 180));
    else setfillcolor(RGB(80, 50, 120));
    fillroundrect(btnNextX1, btnNextY1, btnNextX2, btnNextY2, 10, 10);

    settextcolor(WHITE);
    int nextTextW = textwidth(_T("next"));
    int nextTextH = textheight(_T("next"));
    outtextxy(btnNextX1 + (btnNextX2 - btnNextX1 - nextTextW) / 2, btnNextY1 + (btnNextY2 - btnNextY1 - nextTextH) / 2, _T("next"));

    int btnStopX1 = WIDTH - 80;
    int btnStopY1 = 20;
    int btnStopX2 = WIDTH - 20;
    int btnStopY2 = 70;

    bool hoverStop = isMouseInRect(p.x, p.y, btnStopX1, btnStopY1, btnStopX2, btnStopY2);

    if (hoverStop) setfillcolor(RGB(180, 60, 60));
    else setfillcolor(RGB(120, 40, 40));
    fillroundrect(btnStopX1, btnStopY1, btnStopX2, btnStopY2, 10, 10);

    settextcolor(WHITE);
    int stopTextW = textwidth(_T("stop"));
    int stopTextH = textheight(_T("stop"));
    outtextxy(btnStopX1 + (btnStopX2 - btnStopX1 - stopTextW) / 2, btnStopY1 + (btnStopY2 - btnStopY1 - stopTextH) / 2, _T("stop"));
}

//绘制开始界面
void drawStartUI() {
    setbkcolor(RGB(20, 20, 40));
    cleardevice();

    if (bgImageLoaded) {
        putimage(0, 0, &imgBackgroundStart);
    }

    DWORD t = GetTickCount() - animStart;

    if (animPlaying) {
        for (int i = 0; i < 4; i++) {
            if (t > i * 150 && charY[i] < 200) {
                charY[i] += 1.5f + (200 - charY[i]) * 0.08f;
                if (charY[i] > 200) charY[i] = 200;
            }
        }

        bool finished = true;
        for (int i = 0; i < 4; i++) {
            if (charY[i] < 198) { finished = false; break; }
        }
        if (finished) animPlaying = false;
    }

    settextstyle(160, 0, _T("\x534E\x6587\x65B0\x9B4F"));
    settextcolor(RGB(180, 0, 0));

    int charW = 160;
    int totalW = charW * 4;
    int startX = (WIDTH - totalW) / 2;

    wchar_t buf[2] = { 0 };
    buf[1] = L'\0';

    buf[0] = L'\x4F50'; outtextxy(startX + charW * 0, (int)charY[0], buf);
    buf[0] = L'\x52A9'; outtextxy(startX + charW * 1, (int)charY[1], buf);
    buf[0] = L'\x5FEB'; outtextxy(startX + charW * 2, (int)charY[2], buf);
    buf[0] = L'\x8DD1'; outtextxy(startX + charW * 3, (int)charY[3], buf);

    POINT p;
    getMousePosInWindow(&p);

    int btnStartX = WIDTH / 2 - 130;
    int btnStartY = 450;
    int btnEndX = WIDTH / 2 + 130;
    int btnEndY = 530;

    bool hoverStart = isMouseInRect(p.x, p.y, btnStartX, btnStartY, btnEndX, btnEndY);

    if (hoverStart) setfillcolor(RGB(80, 160, 255));
    else setfillcolor(RGB(50, 130, 255));
    fillroundrect(btnStartX, btnStartY, btnEndX, btnEndY, 25, 25);

    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(40, 0, _T("\x9ED1\x4F53"));
    outtextxy(WIDTH / 2 - textwidth(_T("\x5F00\x59CB\x6E38\x620F")) / 2, 475, _T("\x5F00\x59CB\x6E38\x620F"));

    if (hoverStart) {
        showDifficulty = true;
    }

    if (showDifficulty) {
        int diffY[] = { 550, 610, 670 };
        const wchar_t* diffText[] = { L"\x7B80\x5355", L"\x4E2D\x7B49", L"\x56F0\x96BE" };
        int diffColor[] = { RGB(100,200,100), RGB(255,180,0), RGB(255,80,80) };

        for (int i = 0; i < 3; i++) {
            int diffBtnX1 = WIDTH / 2 - 100;
            int diffBtnX2 = WIDTH / 2 + 100;
            bool hoverDiff = isMouseInRect(p.x, p.y, diffBtnX1, diffY[i], diffBtnX2, diffY[i] + 50);

            if (hoverDiff) setfillcolor(diffColor[i]);
            else setfillcolor(RGB(70, 70, 100));

            fillroundrect(diffBtnX1, diffY[i], diffBtnX2, diffY[i] + 50, 15, 15);
            settextcolor(WHITE);
            settextstyle(32, 0, _T("\x9ED1\x4F53"));
            outtextxy(WIDTH / 2 - textwidth(diffText[i]) / 2, diffY[i] + 8, diffText[i]);
        }
    }
    
    drawMusicButton();
}

//鼠标点击
void checkAllMouse() {
    if (!MouseHit()) return;
    MOUSEMSG m = GetMouseMsg();
    if (m.uMsg != WM_LBUTTONDOWN) return;

    POINT p = { m.x, m.y };

    if (gameState == 0) {
        int btnStartX = WIDTH / 2 - 130;
        int btnStartY = 450;
        int btnEndX = WIDTH / 2 + 130;
        int btnEndY = 530;

        if (isMouseInRect(p.x, p.y, btnStartX, btnStartY, btnEndX, btnEndY)) {
            difficulty = 0;
            resetGame();
            gameStartTime = GetTickCount();
            gameState = 1;
            return;
        }

        int diffY[] = { 550, 610, 670 };
        for (int i = 0; i < 3; i++) {
            int diffBtnX1 = WIDTH / 2 - 100;
            int diffBtnX2 = WIDTH / 2 + 100;
            if (isMouseInRect(p.x, p.y, diffBtnX1, diffY[i], diffBtnX2, diffY[i] + 50)) {
                difficulty = i;
                resetGame();
                gameStartTime = GetTickCount();
                gameState = 1;
                if (!musicStopped) {
                    playRandomMusic();
                }
                return;
            }
        }
    }

    if (gameState == 2) {
        int textH = textheight(L"\x6211\x611A\x8822\x7684\x5F1F\x5F1F\x554A......");
        int textY = HEIGHT / 2 - textH / 2 - 60;
        int btnY = textY + textH + 40;

        int btnRestartX1 = WIDTH / 2 - 130;
        int btnRestartX2 = WIDTH / 2 + 130;

        if (isMouseInRect(p.x, p.y, btnRestartX1, btnY, btnRestartX2, btnY + 80)) {
            resetGame();
            gameStartTime = GetTickCount();
            gameState = 1;
            if (!musicStopped) {
                playRandomMusic();
            }
            return;
        }

        int btnHomeY = btnY + 80 + 20;
        if (isMouseInRect(p.x, p.y, btnRestartX1, btnHomeY, btnRestartX2, btnHomeY + 80)) {
            gameState = 0;
            showDifficulty = false;
            return;
        }
    }

    int nextX1 = WIDTH - 160, nextY1 = 20, nextX2 = WIDTH - 90, nextY2 = 70;
    if (isMouseInRect(p.x, p.y, nextX1, nextY1, nextX2, nextY2)) {
        nextMusic();
        return;
    }

    int stopX1 = WIDTH - 80, stopY1 = 20, stopX2 = WIDTH - 20, stopY2 = 70;
    if (isMouseInRect(p.x, p.y, stopX1, stopY1, stopX2, stopY2)) {
        if (isMusicPlaying) {
            stopMusic();
            musicStopped = true;
        } else {
            musicStopped = false;
            if (gameState == 0) {
                playMusic(1);
            } else if (gameState == 1) {
                playRandomMusic();
            } else {
                playRandomMusic();
            }
        }
        return;
    }
    
    int exitX1 = WIDTH - 160, exitY1 = HEIGHT - 70, exitX2 = WIDTH - 30, exitY2 = HEIGHT - 20;
    if (isMouseInRect(p.x, p.y, exitX1, exitY1, exitX2, exitY2)) {
        closegraph();
        exit(0);
    }
}

//绘制结束界面
void drawEndUI() {
    int scoreSeconds = (gameEndTime - gameStartTime) / 1000;
    wchar_t scoreText[64];
    swprintf_s(scoreText, L"\x5F97\x5206\xFF1A%d", scoreSeconds);

    settextstyle(100, 0, _T("\x534E\x6587\x65B0\x9B4F"));
    settextcolor(RGB(180, 0, 0));
    setbkmode(TRANSPARENT);

    int textW = textwidth(scoreText);
    int textH = textheight(scoreText);
    int textX = WIDTH / 2 - textW / 2 + 20;
    int textY = HEIGHT / 2 - textH / 2 - 60;

    outtextxy(textX, textY, scoreText);

    POINT p;
    getMousePosInWindow(&p);

    int btnY = textY + textH + 40;
    int btnX1 = WIDTH / 2 - 130;
    int btnX2 = WIDTH / 2 + 130;

    bool hoverRestart = isMouseInRect(p.x, p.y, btnX1, btnY, btnX2, btnY + 80);
    if (hoverRestart) setfillcolor(RGB(80, 160, 255));
    else setfillcolor(RGB(50, 130, 255));
    fillroundrect(btnX1, btnY, btnX2, btnY + 80, 25, 25);

    settextcolor(WHITE);
    settextstyle(40, 0, _T("\x9ED1\x4F53"));
    outtextxy(WIDTH / 2 - textwidth(L"\x91CD\x65B0\x5F00\x59CB") / 2, btnY + 25, L"\x91CD\x65B0\x5F00\x59CB");

    int btnHomeY = btnY + 80 + 20;
    bool hoverHome = isMouseInRect(p.x, p.y, btnX1, btnHomeY, btnX2, btnHomeY + 80);
    if (hoverHome) setfillcolor(RGB(130, 130, 130));
    else setfillcolor(RGB(100, 100, 100));
    fillroundrect(btnX1, btnHomeY, btnX2, btnHomeY + 80, 25, 25);

    settextcolor(WHITE);
    outtextxy(WIDTH / 2 - textwidth(L"\x8FD4\x56DE\x4E3B\x9875") / 2, btnHomeY + 25, L"\x8FD4\x56DE\x4E3B\x9875");
    
    drawMusicButton();
}

//绘制退出按钮
void drawExitButton() {
    int x1 = WIDTH - 160, y1 = HEIGHT - 70, x2 = WIDTH - 30, y2 = HEIGHT - 20;

    POINT p;
    getMousePosInWindow(&p);

    bool hover = isMouseInRect(p.x, p.y, x1, y1, x2, y2);

    if (hover) setfillcolor(RGB(255, 60, 60));
    else setfillcolor(RGB(220, 40, 40));
    fillroundrect(x1, y1, x2, y2, 15, 15);

    setlinecolor(WHITE);
    setlinestyle(PS_SOLID, 2);
    roundrect(x1, y1, x2, y2, 15, 15);

    settextcolor(WHITE);
    settextstyle(26, 0, _T("\x9ED1\x4F53"));
    int tx = (x1 + x2) / 2 - textwidth(L"\x9000\x51FA\x6E38\x620F") / 2;
    int ty = (y1 + y2) / 2 - 13;
    outtextxy(tx, ty, L"\x9000\x51FA\x6E38\x620F");
}

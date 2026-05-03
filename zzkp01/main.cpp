#include "game.h"

//玩家位置计算
void calculatePlayerBounds(int& playerTop, int& playerBottom) {
    int playerHeight;
    if (isCrouch) {
        playerHeight = 55;
    }
    else if (isJump) {
        playerHeight = 75;
    }
    else {
        playerHeight = 100;
    }
    playerTop = pY - playerHeight;
    playerBottom = pY;
}

//障碍物碰撞检测
bool checkObstacleCollision(int obstacleIndex, int playerTop, int playerBottom) {
    // 边界检查
    if (obstacleIndex < 0 || obstacleIndex >= 50) {
        return false;
    }

    // 检查障碍物是否有效
    if (!obs[obstacleIndex].exist) {
        return false;
    }

    // 地面障碍
    if (obs[obstacleIndex].type == 0 && isJump) {
        return false;
    }
    // 头顶障碍
    if (obs[obstacleIndex].type == 2 && isCrouch) {
        return false;
    }

    int obstacleTop = obs[obstacleIndex].y;
    int obstacleBottom;
    int obstacleWidth;
    int obstacleX = laneX[obs[obstacleIndex].lane];

    if (obs[obstacleIndex].type == 0) {
        obstacleBottom = obstacleTop + 35;
        obstacleWidth = 99;
    }
    else if (obs[obstacleIndex].type == 1) {
        obstacleBottom = obstacleTop + 63;
        obstacleWidth = 40;
    }
    else if (obs[obstacleIndex].type == 2) {
        if (obs[obstacleIndex].aerialIdx == 0) {
            obstacleBottom = obstacleTop + 78;
            obstacleWidth = 45;
        }
        else {
            obstacleBottom = obstacleTop + 90;
            obstacleWidth = 30;
        }
    }
    else {
        return false;
    }

    // 获取玩家宽度
    int playerWidth;
    if (isCrouch) {
        playerWidth = 31;
    }
    else if (isJump) {
        playerWidth = 40;
    }
    else {
        playerWidth = (playerAnimFrame == 0) ? 42 : 40;
    }

    int playerX = laneX[currentLane];

    // X轴碰撞检测
    int playerLeft = playerX - playerWidth / 2;
    int playerRight = playerX + playerWidth / 2;
    int obsLeft = obstacleX - obstacleWidth / 2;
    int obsRight = obstacleX + obstacleWidth / 2;

    // 只在障碍物在屏幕可见范围内时才检测碰撞
    if (obs[obstacleIndex].type == 0 || obs[obstacleIndex].type == 1) {
        if (obstacleBottom < 0 || obstacleTop > HEIGHT) {
            return false;
        }
    }
    else {
        if (obstacleBottom < 0 || obstacleTop > HEIGHT) {
            return false;
        }
    }

    bool xCollision = !(playerRight < obsLeft || playerLeft > obsRight);
    bool yCollision = !(playerBottom < obstacleTop || playerTop > obstacleBottom);

    return xCollision && yCollision;
}

//图片绘制
void drawAlphaImage(int x, int y, IMAGE* img) {
    if (img == NULL) return;
    if (img->getwidth() <= 0 || img->getheight() <= 0) return;
    putimage(x, y, img);
}

//障碍物绘制
void drawObstacle(int i) {
    // 安全检查
    if (i < 0 || i >= 50) return;
    if (!obs[i].exist) return;

    int x = laneX[obs[i].lane];

    if (obs[i].type == 0) {
        drawAlphaImage(x - 49, obs[i].y, &imgObsGround);
    }
    else if (obs[i].type == 1) {
        drawAlphaImage(x - 20, obs[i].y, &imgObsAerial);
    }
    else if (obs[i].type == 2) {
        if (obs[i].isWarning) {
            setcolor(RED);
            line(x, HEIGHT - 180, x, HEIGHT);
        }
        else {
            if (obs[i].aerialIdx == 0)
                drawAlphaImage(x - 22, obs[i].y, &imgObsHead);
            else
                drawAlphaImage(x - 15, obs[i].y, &imgObsHead2);
        }
    }
}

//玩家绘制
void drawPlayer(int playerTop, int playerBottom) {
    int x = laneX[currentLane];
    IMAGE* img = NULL;

    if (isCrouch) {
        img = &imgPlayer4;
    }
    else if (isJump) {
        img = &imgPlayer3;
    }
    else {
        img = (playerAnimFrame == 0) ? &imgPlayer1 : &imgPlayer2;
    }

    if (img) {
        int imgW = img->getwidth();
        int imgH = img->getheight();
        // 安全检查
        if (imgW > 0 && imgH > 0) {
            int drawX = x - imgW / 2;
            int drawY = playerTop;
            putimage(drawX, drawY, img);
        }
    }
}

//分数显示
void drawScore(DWORD gameStartTime) {
    settextcolor(RGB(255, 210, 0));
    DWORD displayTime = (gameState == 2) ? (gameEndTime - gameStartTime) / 1000 : (GetTickCount() - gameStartTime) / 1000;
    swprintf_s(scoreText, L"\x5F97\x5206\xFF1A%d", displayTime);
    outtextxy(20, 20, scoreText);
}

//键盘输入处理
void handleKeyboardInput() {
    static DWORD keyCool = 0;
    DWORD now = GetTickCount();

    if (now - keyCool > 50) {
        if (GetAsyncKeyState('A') & 0x8000 && currentLane > 0) {
            currentLane--;
            keyCool = now;
        }
        if (GetAsyncKeyState('D') & 0x8000 && currentLane < 2) {
            currentLane++;
            keyCool = now;
        }
    }

    isCrouch = (GetAsyncKeyState('S') & 0x8000) != 0;

    if (GetAsyncKeyState('W') & 0x8000 && !isJump && !isCrouch) {
        isJump = true;
        jumpVel = -22;
    }
}

//跳跃物理更新
void updateJumpPhysics() {
    if (isJump) {
        pY += jumpVel;
        jumpVel += gravity;
        if (pY >= HEIGHT * 3 / 4) {
            pY = HEIGHT * 3 / 4;
            isJump = false;
            jumpVel = 0;
        }
    }
}

//障碍物状态更新
void updateObstacles() {
    DWORD now = GetTickCount();

    for (int i = 0; i < 50; i++) {
        if (!obs[i].exist) continue;

        // 检查类型有效性
        if (obs[i].type < 0 || obs[i].type > 2) {
            obs[i].exist = false;
            continue;
        }

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
    }
}

//碰撞检测函数
bool checkCollisions() {
    int playerTop, playerBottom;
    calculatePlayerBounds(playerTop, playerBottom);

    // 玩家边界检查
    if (playerTop > playerBottom) {
        return false;
    }

    for (int i = 0; i < 50; i++) {
        if (!obs[i].exist) continue;
        if (obs[i].isWarning) continue;
        if (obs[i].lane != currentLane) continue;

        if (checkObstacleCollision(i, playerTop, playerBottom)) {
            return true;
        }
    }
    return false;
}

//游戏主循环
void gameLoop() {
    const int FPS = 60;
    const DWORD frameDelay = 1000 / FPS;
    DWORD frameStart;
    gameStartTime = GetTickCount();
    playerAnimTimer = GetTickCount();

    animStart = GetTickCount();
    playMusic(1);

    while (true) {
        frameStart = GetTickCount();
        cleardevice();
        checkAllMouse();

        if (gameState == 0) {
            if (!musicStopped && (currentMusicIndex != 1 || !isMusicPlaying)) {
                playMusic(1);
            }
            drawStartUI();
            drawMusicButton();
            drawExitButton();
            FlushBatchDraw();
        }
        else if (gameState == 2) {
            setbkcolor(RGB(20, 20, 40));
            cleardevice();
            if (bgImageLoaded) {
                putimage(0, 0, &imgBackgroundEnd);
            }
            drawScore(gameStartTime);
            drawEndUI();
            drawMusicButton();
            drawExitButton();
            FlushBatchDraw();
        }
        else {
            setbkcolor(RGB(20, 20, 40));
            cleardevice();
            if (bgImageLoaded) {
                putimage(0, 0, &imgBackgroundGame);
            }
            

            handleKeyboardInput();
            updateJumpPhysics();

            if (GetTickCount() - playerAnimTimer > 150) {
                playerAnimFrame = 1 - playerAnimFrame;
                playerAnimTimer = GetTickCount();
            }

            if (GetTickCount() - lastSpawn > SPAWN_INTERVAL) {
                spawnObstacle();
            }

            updateObstacles();

            if (checkCollisions()) {
                gameState = 2;
                gameEndTime = GetTickCount();
                if (!musicStopped) {
                    playRandomMusic();
                }
            }

            int playerTop, playerBottom;
            calculatePlayerBounds(playerTop, playerBottom);
            drawPlayer(playerTop, playerBottom);
            
            for (int i = 0; i < 50; i++) {
                if (obs[i].exist) {
                    drawObstacle(i);
                }
            }

            drawScore(gameStartTime);
            drawMusicButton();
            drawExitButton();
            FlushBatchDraw();
        }

        DWORD cost = GetTickCount() - frameStart;
        if (cost < frameDelay) Sleep(frameDelay - cost);
    }
}

int main() {
    initgraph(WIDTH, HEIGHT);
    srand((unsigned)time(NULL));
    BeginBatchDraw();
    loadBackgroundImage();
    loadObstacleImages();
    loadPlayerImages();

    gameLoop();

    closegraph();
    return 0;
}

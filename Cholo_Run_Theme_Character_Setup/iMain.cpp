#define _CRT_SECURE_NO_WARNINGS
#include "iGraphics.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

// 1. Define Game States
#define MENU 0
#define LOADING 7
#define GAME 1
#define CHARACTER_SELECT 2
#define THEME_SELECT 6
#define SHOP 3         
#define HIGHSCORE 4
#define ABOUT 5
#define PAUSE 8 
#define GAME_OVER 9
#define THEME_DAY 10
#define THEME_NIGHT 11
#define THEME_RAIN 13
#define INSTRUCTION_SCREEN 14 // New State

//timer of double jump
clock_t lastUpPressTime = 0;
int doublePressThreshold = 300;
bool wasUpPressed = false;
bool wasDownPressed = false;

// Player Selection theme and character
int currentTheme = THEME_NIGHT;
int currentCharacter = 1;

// Loading Progress Trackers
int currentAssetsLoaded = 0;
int totalAssets = 197; // Adjusted down from 200

// Dynamic Lane Information
int totalLanes = 4;
double laneTopX[4];
double laneBottomX[4];
double horizonY;
double bottomY;

int playerLaneX[4];
int playerCurrentLane = 0;

// --- SHIELD POWER UP VARIABLES ---
int shieldImg;
int shieldTimer = 0;
int nextShieldScore = 2000;
bool isShieldActive = false;

// Character Shield Sprites
int characterShield[2];
int characterNetShield[2];
int jumpShieldImg;

// --- RAIN & LIGHTNING EFFECT VARIABLES ---
#define MAX_DROPS 350
#define MAX_CLOUDS 15

struct RainDrop {
	double x, y;
	double speed;
	int length;
	bool isSplashing;
	int splashTimer;
	double targetY;
} drops[MAX_DROPS];

struct Cloud {
	double x, y;
	double speed;
	double scale;
} clouds[MAX_CLOUDS];

int lightningTimer = 0;
int lightningDuration = 0;
double lightningPoints[6][2];
double branchPoints[3][2];

void initRain() {
	for (int i = 0; i < MAX_DROPS; i++) {
		drops[i].x = rand() % 1600;
		drops[i].y = 900 + (rand() % 600);
		drops[i].speed = 15 + (rand() % 10);
		drops[i].length = 10 + (rand() % 15);
		drops[i].isSplashing = false;
		drops[i].splashTimer = 0;
		drops[i].targetY = rand() % (int)horizonY;
	}
	for (int i = 0; i < MAX_CLOUDS; i++) {
		clouds[i].x = rand() % 1600;
		clouds[i].y = 650 + (rand() % 200);
		clouds[i].speed = 0.5 + (rand() % 20) / 10.0;
		clouds[i].scale = 0.8 + (rand() % 8) / 10.0;
	}
}

int setupThemeRules() {
	horizonY = 575; bottomY = 0; totalLanes = 4;
	laneTopX[0] = 763; laneTopX[1] = 780; laneTopX[2] = 798; laneTopX[3] = 821;
	laneBottomX[0] = 330; laneBottomX[1] = 570; laneBottomX[2] = 800; laneBottomX[3] = 1030;
	playerLaneX[0] = 460; playerLaneX[1] = 630; playerLaneX[2] = 780; playerLaneX[3] = 970;
	return 1;
}

int bug = setupThemeRules();
int lastLane = -1;

int getUniqueLane() {
	int newLane = rand() % totalLanes;
	while (newLane == lastLane) newLane = rand() % totalLanes;
	lastLane = newLane;
	return newLane;
}

// UI Variables
int background, woodPlank, logoImg, shopBg, themeBg, aboutBg, pauseBtn;
int scoreBoardImg;
int pausedScreenImg;
int gameoverImg;
int heartImg;

// Shop & Selection Variables
int shopMainBg, frameImg, lockImg;
int charThumbs[4];
int themeThumbs[3];
int instructionImg; // Instruction Image Variable
int instructionTimer = 0; // Timer for 10 seconds

int gameState = INSTRUCTION_SCREEN; // Start with Instruction Screen
int gender = 0;
int loadingTime = 0;
int loadingImage;
int score = 0;
int highScore = 0;

// --- HEART & POWER UP VARIABLES ---
int hearts = 0;
int nextHeartScore = 1500;
bool heartTextActive = false;
int heartTextY = 0;
int heartTextTimer = 0;

int kalojamImg;
int powerTimer = 0;

int netImg;
int coinImg;
int characterNet[2];
int netTimer = 0;
int nextNetScore = 700;

// --- UNLOCK VARIABLES & PRICES ---
bool unlockedChars[4] = { true, false, false, false };
bool unlockedThemes[3] = { false, true, false };
int charPrices[4] = { 0, 200, 250, 300 };
int themePrices[3] = { 350, 0, 500 };

// --- AUDIO MANAGEMENT VARIABLES ---
int currentAudioState = -1;

void saveHighScore() {
	FILE *fp = fopen("highscore.txt", "w");
	if (fp != NULL) { fprintf(fp, "%d", highScore); fclose(fp); }
}

void loadHighScore() {
	FILE *fp = fopen("highscore.txt", "r");
	if (fp != NULL) { if (fscanf(fp, "%d", &highScore) != 1) highScore = 0; fclose(fp); }
}

void saveHearts() {
	FILE *fp = fopen("hearts.txt", "w");
	if (fp != NULL) { fprintf(fp, "%d", hearts); fclose(fp); }
}

void loadHearts() {
	FILE *fp = fopen("hearts.txt", "r");
	if (fp != NULL) { if (fscanf(fp, "%d", &hearts) != 1) hearts = 0; fclose(fp); }
}

void saveUnlocks() {
	FILE* fp = fopen("unlocks.txt", "w");
	if (fp != NULL) {
		fprintf(fp, "%d %d %d %d\n", unlockedChars[0], unlockedChars[1], unlockedChars[2], unlockedChars[3]);
		fprintf(fp, "%d %d %d\n", unlockedThemes[0], unlockedThemes[1], unlockedThemes[2]);
		fclose(fp);
	}
}

void loadUnlocks() {
	FILE* fp = fopen("unlocks.txt", "r");
	if (fp != NULL) {
		int c1, c2, c3, c4, t1, t2, t3;
		if (fscanf(fp, "%d %d %d %d", &c1, &c2, &c3, &c4) == 4) {
			unlockedChars[0] = c1; unlockedChars[1] = c2;
			unlockedChars[2] = c3; unlockedChars[3] = c4;
		}
		int args = fscanf(fp, "%d %d %d", &t1, &t2, &t3);
		if (args >= 3) {
			unlockedThemes[0] = t1; unlockedThemes[1] = t2; unlockedThemes[2] = t3;
		}
		fclose(fp);
	}
}

int coins = 0;
int sessionCoins = 0;

struct Coin { double x, y; int lane; bool active; } gameCoins[5];
struct Power { double x, y; int lane; bool active; } kalojamIcon, heartPower, netPower, shieldPower;

void saveCoins() {
	FILE *fp = fopen("coins.txt", "w");
	if (fp != NULL) { fprintf(fp, "%d", coins); fclose(fp); }
}

void loadCoins() {
	FILE *fp = fopen("coins.txt", "r");
	if (fp != NULL) { if (fscanf(fp, "%d", &coins) != 1) coins = 0; fclose(fp); }
}

// --- SHARED UI COMPONENTS ---
void drawBackButton() {
	iShowImage(50, 800, 150, 60, woodPlank);
	iSetColor(255, 255, 255);
	iText(95, 822, "BACK", GLUT_BITMAP_HELVETICA_18);
}

void drawBalanceUI() {
	iSetColor(30, 30, 30);
	iFilledRectangle(680, 815, 240, 50);
	iSetColor(255, 255, 255);
	iRectangle(680, 815, 240, 50);

	iShowImage(695, 825, 30, 30, coinImg);

	char balStr[30];
	sprintf(balStr, "BALANCE: %d", coins);
	iSetColor(255, 255, 255);
	iText(735, 832, balStr, GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawMenu() {
	iShowImage(0, 0, 1600, 900, background);
	char hsText[30];
	sprintf(hsText, "BEST RECORD: %d", highScore);
	iSetColor(0, 0, 0);
	iText(720, 590, hsText, GLUT_BITMAP_HELVETICA_18);

	iShowImage(765, 555, 40, 30, heartImg);
	char heartStr[20];
	sprintf(heartStr, " = %d", hearts);
	iSetColor(0, 0, 0);
	iText(800, 565, heartStr, GLUT_BITMAP_HELVETICA_18);

	int btnW = 300, btnH = 80, btnX = (1600 - btnW) / 2;
	char labels[6][20] = { "START", "CHARACTER", "THEME", "SHOP", "ABOUT US", "EXIT" };
	for (int i = 0; i < 6; i++) {
		int currentBtnY = 500 - (i * 85);
		iShowImage(btnX, currentBtnY, btnW, btnH, woodPlank);
		iSetColor(0, 0, 0);
		iText(btnX + 80, currentBtnY + 32, labels[i], GLUT_BITMAP_HELVETICA_18);
	}
}

void drawPauseMenu() { iShowImage(0, 0, 1600, 900, pausedScreenImg); }

int keyDelay = 0;
int backgroundImages[191];
int bgIndex = 0;

int inix() {
	playerCurrentLane = rand() % totalLanes;
	return playerCurrentLane;
}

int charIndex = 0;
int character[2];
int x = 0;
int y = 100;
int currentImage;
int j = 0;
int duckTimer = 0;
bool gameOver = false;

int obstacle[18];

int animFrame = 0;
int animationTimer = 0;

void animateObstacles() {
	animationTimer++;
	if (animationTimer >= 5) {
		animFrame = !animFrame;
		animationTimer = 0;
	}
}

int getThemeObstacle() {
	int newObs;
	while (1) {
		newObs = rand() % 17;
		if (newObs != 8 && newObs != 9) {
			break;
		}
	}
	return newObs;
}

struct Obstacle{
	double x, y;
	int lane;
	int type;
	bool active;
	double flyHeight;
} obs[6];

void resetGame() {
	score = 0;
	sessionCoins = 0;
	y = 100;
	loadingTime = 0;
	gameOver = false;
	playerCurrentLane = inix();
	x = playerLaneX[playerCurrentLane];
	powerTimer = 0;
	netTimer = 0;
	nextHeartScore = 1500;
	nextNetScore = 700;
	heartTextActive = false;

	shieldTimer = 0;
	isShieldActive = false;
	shieldPower.active = false;
	shieldPower.y = horizonY + 2000;
	nextShieldScore = score + 1800 + (rand() % 401);

	for (int i = 0; i<5; i++) {
		gameCoins[i].y = 575 + rand() % 5 + (i * 30);
		gameCoins[i].lane = getUniqueLane();
		gameCoins[i].active = (i == 0);
	}

	kalojamIcon.y = 575 + rand() % 1000; kalojamIcon.lane = getUniqueLane(); kalojamIcon.active = false;
	heartPower.y = 575; heartPower.lane = getUniqueLane(); heartPower.active = false;
	netPower.y = 575; netPower.lane = getUniqueLane(); netPower.active = false;

	for (int i = 0; i < 6; i++) {
		obs[i].active = false;
	}

	for (int i = 0; i < 3; i++) {
		obs[i].y = horizonY + (i * 250);
		obs[i].lane = getUniqueLane();
		obs[i].type = getThemeObstacle();
		obs[i].active = true;
		if (obs[i].type == 15 || obs[i].type == 16) obs[i].flyHeight = 800;
		else obs[i].flyHeight = 0;
	}
	initRain();
}

void useHeartToRevive() {
	if (hearts > 0) {
		hearts--;
		saveHearts();
		gameOver = false;

		for (int i = 0; i < 6; i++) {
			obs[i].active = false;
		}
		for (int i = 0; i < 3; i++) {
			obs[i].y = horizonY + (i * 250);
			obs[i].lane = getUniqueLane();
			obs[i].type = getThemeObstacle();
			obs[i].active = true;
			if (obs[i].type == 15 || obs[i].type == 16) obs[i].flyHeight = 800;
			else obs[i].flyHeight = 0;
		}
	}
}

void playCoinSound() {
	mciSendStringA("close sfx", NULL, 0, NULL);
	mciSendStringA("open \"Photo/Audio/coin audio.mp3\" type mpegvideo alias sfx", NULL, 0, NULL);
	mciSendStringA("play sfx from 0", NULL, 0, NULL);
}

void checkCollision() {
	int playerLeft = x + 40; int playerRight = x + 140;
	int playerBottom = y + 20; int playerTop = y + 120;

	for (int i = 0; i<5; i++){
		if (gameCoins[i].active) {
			int cLeft = gameCoins[i].x; int cRight = gameCoins[i].x + 40;
			int cBottom = gameCoins[i].y; int cTop = gameCoins[i].y + 40;
			if (cRight > playerLeft && cLeft < playerRight && cTop > playerBottom && cBottom < playerTop) {
				if (j == 0) {
					coins += 1; sessionCoins += 1; saveCoins();
					gameCoins[i].active = false;
					playCoinSound();
				}
			}
		}
	}

	if (kalojamIcon.active) {
		int cLeft = kalojamIcon.x; int cRight = kalojamIcon.x + 30;
		int cBottom = kalojamIcon.y; int cTop = kalojamIcon.y + 30;
		if (cRight > playerLeft && cLeft < playerRight && cTop > playerBottom && cBottom < playerTop) {
			if (j == 0) {
				powerTimer = 250; kalojamIcon.active = false;
				playCoinSound();
			}
		}
	}

	if (shieldPower.active) {
		int cLeft = shieldPower.x; int cRight = shieldPower.x + 35;
		int cBottom = shieldPower.y; int cTop = shieldPower.y + 35;
		if (cRight > playerLeft && cLeft < playerRight && cTop > playerBottom && cBottom < playerTop) {
			shieldTimer = 600; isShieldActive = true; shieldPower.active = false; playCoinSound();
		}
	}

	if (heartPower.active) {
		int cLeft = heartPower.x; int cRight = heartPower.x + 30;
		int cBottom = heartPower.y; int cTop = heartPower.y + 30;
		if (cRight > playerLeft && cLeft < playerRight && cTop > playerBottom && cBottom < playerTop) {
			if (j == 0) {
				hearts++; saveHearts(); heartPower.active = false;
				heartTextActive = true; heartTextY = y + 150; heartTextTimer = 100; playCoinSound();
			}
		}
	}

	if (netPower.active) {
		int cLeft = netPower.x; int cRight = netPower.x + 30;
		int cBottom = netPower.y; int cTop = netPower.y + 30;
		if (cRight > playerLeft && cLeft < playerRight && cTop > playerBottom && cBottom < playerTop) {
			if (j == 0) { netTimer = 500; netPower.active = false; playCoinSound(); }
		}
	}

	for (int i = 0; i < 6; i++) {
		if (!obs[i].active || obs[i].y > horizonY) continue;

		int obsLeft = obs[i].x; int obsRight = obs[i].x + 30;
		int obsBottom; int obsTop;

		if (obs[i].type == 15 || obs[i].type == 16) {
			obsBottom = obs[i].flyHeight; obsTop = obs[i].flyHeight + 80;
		}
		else {
			obsBottom = obs[i].y; obsTop = obs[i].y + 30;
		}

		if (obs[i].type == 12){ if (j >= 230) continue; }
		else if ((obs[i].type == 13 || obs[i].type == 14)) { if (j > 0) continue; }
		if (obs[i].type == 15 || obs[i].type == 16) { if (duckTimer > 0) continue; }

		if (obsRight > playerLeft && obsLeft < playerRight && obsTop > playerBottom && obsBottom < playerTop) {
			if (isShieldActive) {
				isShieldActive = false; shieldTimer = 0; obs[i].active = false;
			}
			else {
				gameOver = true;
				if (score > highScore) { highScore = score; saveHighScore(); }
			}
		}
	}
}

int speedBoost = 0;

void updateRain() {
	if (gameState != GAME || gameOver) return;
	if (currentTheme != THEME_RAIN) return;

	if (lightningDuration > 0) lightningDuration--;
	else {
		lightningTimer++;
		if (lightningTimer >= 33) {
			lightningDuration = 4;
			lightningTimer = 0;
			lightningPoints[0][0] = 200 + rand() % 1200; lightningPoints[0][1] = 900;
			for (int i = 1; i < 6; i++) {
				lightningPoints[i][0] = lightningPoints[i - 1][0] + (rand() % 160 - 80);
				lightningPoints[i][1] = lightningPoints[i - 1][1] - (rand() % 80 + 40);
				if (lightningPoints[i][1] < horizonY) lightningPoints[i][1] = horizonY;
			}
			branchPoints[0][0] = lightningPoints[2][0]; branchPoints[0][1] = lightningPoints[2][1];
			for (int i = 1; i < 3; i++) {
				branchPoints[i][0] = branchPoints[i - 1][0] + 40 + (rand() % 60);
				branchPoints[i][1] = branchPoints[i - 1][1] - (rand() % 60 + 30);
			}
		}
	}

	for (int i = 0; i < MAX_CLOUDS; i++) {
		clouds[i].x -= clouds[i].speed;
		if (clouds[i].x < -150) {
			clouds[i].x = 1600 + (rand() % 200);
			clouds[i].y = 650 + (rand() % 200);
		}
	}

	int playerLeft = x + 40; int playerRight = x + 140;
	int playerTop = y + 120; int playerBottom = y + 20;

	for (int i = 0; i < MAX_DROPS; i++) {
		if (!drops[i].isSplashing) {
			drops[i].x -= 2; drops[i].y -= drops[i].speed;
			bool hitSomething = false;

			if (drops[i].x > playerLeft && drops[i].x < playerRight && drops[i].y <= playerTop && drops[i].y > playerBottom) {
				drops[i].y = playerTop; hitSomething = true;
			}
			if (!hitSomething) {
				for (int k = 0; k < 6; k++) {
					if (!obs[k].active || obs[k].y > horizonY) continue;
					int obsLeft = obs[k].x; int obsRight = obs[k].x + 60;
					int obsTop, obsBottom;
					if (obs[k].type == 15 || obs[k].type == 16) { obsTop = obs[k].flyHeight + 80; obsBottom = obs[k].flyHeight; }
					else { obsTop = obs[k].y + 50; obsBottom = obs[k].y; }
					if (drops[i].x > obsLeft && drops[i].x < obsRight && drops[i].y <= obsTop && drops[i].y > obsBottom) {
						drops[i].y = obsTop; hitSomething = true; break;
					}
				}
			}
			if (!hitSomething && drops[i].y <= drops[i].targetY) { drops[i].y = drops[i].targetY; hitSomething = true; }
			if (hitSomething) { drops[i].isSplashing = true; drops[i].splashTimer = 4; }
		}
		else {
			drops[i].splashTimer--;
			if (drops[i].splashTimer <= 0) {
				drops[i].x = rand() % 1600; drops[i].y = 900 + (rand() % 600);
				drops[i].speed = 15 + (rand() % 10); drops[i].targetY = rand() % (int)horizonY;
				drops[i].isSplashing = false;
			}
		}
	}
}

void moveObstacles() {
	if (gameState != GAME) return;
	if (gameOver) return;

	double globalSpeed = 7 + speedBoost;

	bool anyCoinActive = false;
	for (int i = 0; i<5; i++){
		if (gameCoins[i].active) {
			anyCoinActive = true;
			if (netTimer > 0 && gameCoins[i].y < 450) {
				double targetX = x + 60; double targetY = y + 90;
				if (gameCoins[i].x < targetX) gameCoins[i].x += 15;
				if (gameCoins[i].x > targetX) gameCoins[i].x -= 15;
				gameCoins[i].y -= 20;
			}
			else {
				gameCoins[i].y -= (5 + (score / 1500));
				if (gameCoins[i].y <= horizonY) {
					double t = (horizonY - gameCoins[i].y) / (horizonY - bottomY);
					gameCoins[i].x = laneTopX[gameCoins[i].lane] + t * (laneBottomX[gameCoins[i].lane] - laneTopX[gameCoins[i].lane]);
				}
				else {
					gameCoins[i].x = laneTopX[gameCoins[i].lane];
				}
			}
			if (gameCoins[i].y < -50) { gameCoins[i].active = false; }
		}
	}

	if (!anyCoinActive) {
		int l = getUniqueLane();
		gameCoins[0].y = horizonY + (rand() % 10); gameCoins[0].lane = l; gameCoins[0].active = true;
	}

	if (!shieldPower.active && score >= nextShieldScore) {
		shieldPower.active = true; shieldPower.y = horizonY + (rand() % 10);
		shieldPower.lane = getUniqueLane(); nextShieldScore += (1800 + rand() % 401);
	}
	if (shieldPower.active) {
		shieldPower.y -= (5 + (score / 1500));
		if (shieldPower.y <= horizonY) {
			double t = (horizonY - shieldPower.y) / (horizonY - bottomY);
			shieldPower.x = laneTopX[shieldPower.lane] + t * (laneBottomX[shieldPower.lane] - laneTopX[shieldPower.lane]);
		}
		else { shieldPower.x = laneTopX[shieldPower.lane]; }
		if (shieldPower.y < -50) shieldPower.active = false;
	}

	if (kalojamIcon.active) {
		kalojamIcon.y -= (5 + (score / 1500));
		if (kalojamIcon.y <= horizonY) {
			double t = (horizonY - kalojamIcon.y) / (horizonY - bottomY);
			kalojamIcon.x = laneTopX[kalojamIcon.lane] + t * (laneBottomX[kalojamIcon.lane] - laneTopX[kalojamIcon.lane]);
		}
		else { kalojamIcon.x = laneTopX[kalojamIcon.lane]; }
		if (kalojamIcon.y < -50) { kalojamIcon.y = horizonY + 1500 + (rand() % 1000); kalojamIcon.lane = getUniqueLane(); }
	}
	else {
		kalojamIcon.y -= 2;
		if (kalojamIcon.y < -50) { kalojamIcon.y = horizonY + 1500 + (rand() % 1000); kalojamIcon.lane = getUniqueLane(); kalojamIcon.active = true; }
	}

	if (!netPower.active && score >= nextNetScore) {
		netPower.active = true; netPower.y = horizonY + (rand() % 10);
		netPower.lane = getUniqueLane(); nextNetScore += 1000;
	}
	if (netPower.active) {
		netPower.y -= (5 + (score / 1500));
		if (netPower.y <= horizonY) {
			double t = (horizonY - netPower.y) / (horizonY - bottomY);
			netPower.x = laneTopX[netPower.lane] + t * (laneBottomX[netPower.lane] - laneTopX[netPower.lane]);
		}
		else { netPower.x = laneTopX[netPower.lane]; }
		if (netPower.y < -50) { netPower.active = false; }
	}

	if (!heartPower.active && score >= nextHeartScore) {
		heartPower.active = true; heartPower.y = horizonY + (rand() % 10);
		heartPower.lane = getUniqueLane(); nextHeartScore += 1500;
	}
	if (heartPower.active) {
		heartPower.y -= (5 + (score / 1500));
		if (heartPower.y <= horizonY) {
			double t = (horizonY - heartPower.y) / (horizonY - bottomY);
			heartPower.x = laneTopX[heartPower.lane] + t * (laneBottomX[heartPower.lane] - laneTopX[heartPower.lane]);
		}
		else { heartPower.x = laneTopX[heartPower.lane]; }
		if (heartPower.y < -50) heartPower.active = false;
	}

	for (int i = 0; i < 6; i++) {
		if (!obs[i].active) continue;

		obs[i].y -= globalSpeed;

		if (obs[i].type == 15 || obs[i].type == 16) {
			obs[i].flyHeight -= (globalSpeed * 1.5);
			double startX = laneTopX[obs[i].lane]; double endX = playerLaneX[obs[i].lane] + 60;
			double progress = (800.0 - obs[i].flyHeight) / 800.0;
			if (progress > 1.0) progress = 1.0;
			obs[i].x = startX + progress * (endX - startX);
		}
		else {
			if (obs[i].y <= horizonY) {
				double t = (horizonY - obs[i].y) / (horizonY - bottomY);
				int lane = obs[i].lane;
				obs[i].x = laneTopX[lane] + t * (laneBottomX[lane] - laneTopX[lane]);
			}
			else { obs[i].x = laneTopX[obs[i].lane]; }
		}

		if (obs[i].y < -200) {
			obs[i].active = false;
		}
	}

	int activeObsCount = 0;
	double maxY = horizonY;
	for (int i = 0; i<6; i++) {
		if (obs[i].active) {
			activeObsCount++;
			if (obs[i].y > maxY) maxY = obs[i].y;
		}
	}

	if (activeObsCount < 3) {
		double newY = maxY + 250 + (rand() % 100);
		int chance = rand() % 10;

		if (chance == 0 && totalLanes >= 3 && activeObsCount <= 3) {
			int openLane = rand() % totalLanes;
			for (int lane = 0; lane<totalLanes; lane++) {
				if (lane == openLane) continue;
				for (int j = 0; j<6; j++) {
					if (!obs[j].active) {
						obs[j].y = newY; obs[j].lane = lane;
						obs[j].type = getThemeObstacle();
						while (obs[j].type == 15 || obs[j].type == 16) obs[j].type = getThemeObstacle();
						obs[j].flyHeight = 0; obs[j].active = true;
						break;
					}
				}
			}
		}
		else if (chance == 1 && totalLanes >= 2 && activeObsCount <= 4) {
			int lane1 = rand() % (totalLanes - 1);
			int lane2 = lane1 + 1;

			for (int j = 0; j<6; j++) {
				if (!obs[j].active) {
					obs[j].y = newY; obs[j].lane = lane1;
					obs[j].type = 15;
					obs[j].flyHeight = 800; obs[j].active = true;
					break;
				}
			}
			for (int j = 0; j<6; j++) {
				if (!obs[j].active) {
					obs[j].y = newY; obs[j].lane = lane2;
					obs[j].type = 13;
					obs[j].flyHeight = 0; obs[j].active = true;
					break;
				}
			}

			int spawned = 0;
			for (int i = 0; i<5; i++){
				if (!gameCoins[i].active) {
					gameCoins[i].y = newY + (spawned * 40);
					gameCoins[i].lane = lane1;
					gameCoins[i].active = true;
					spawned++;
				}
			}
		}
		else {
			for (int j = 0; j<6; j++) {
				if (!obs[j].active) {
					obs[j].y = newY; obs[j].lane = getUniqueLane();
					obs[j].type = getThemeObstacle();
					if (obs[j].type == 15 || obs[j].type == 16) obs[j].flyHeight = 800;
					else obs[j].flyHeight = 0;
					obs[j].active = true;
					break;
				}
			}
		}
	}

	checkCollision();
}

void drawObstacles() {
	for (int i = 0; i < 6; i++) {
		if (obs[i].active) {
			if (obs[i].type == 15 || obs[i].type == 16) {
				if (obs[i].flyHeight > -100) {
					double fixedScale = 80;
					iShowImage(obs[i].x - (fixedScale / 2), obs[i].flyHeight, fixedScale, fixedScale, obstacle[15 + animFrame]);
				}
			}
			else if (obs[i].type == 12) {
				if (obs[i].y <= horizonY) {
					double scale = (1.0 - (obs[i].y / 575.0)) * 120 + 50;
					int dogImgIndex = (animFrame == 0) ? obstacle[12] : obstacle[17];
					iShowImage(obs[i].x, obs[i].y, scale, scale, dogImgIndex);
				}
			}
			else {
				if (obs[i].y <= horizonY) {
					double scale = (1.0 - (obs[i].y / 575.0)) * 120 + 50;
					iShowImage(obs[i].x, obs[i].y, scale, scale, obstacle[obs[i].type]);
				}
			}
		}
	}
}

void drawRain() {
	if (currentTheme != THEME_RAIN) return;

	if (lightningDuration > 0) {
		iSetColor(255, 255, 220);
		for (int i = 0; i < 5; i++) {
			for (int j = -2; j <= 2; j++) {
				iLine(lightningPoints[i][0] + j, lightningPoints[i][1], lightningPoints[i + 1][0] + j, lightningPoints[i + 1][1]);
			}
		}
		for (int i = 0; i < 2; i++) {
			for (int j = -1; j <= 1; j++) {
				iLine(branchPoints[i][0] + j, branchPoints[i][1], branchPoints[i + 1][0] + j, branchPoints[i + 1][1]);
			}
		}
	}

	

	iSetColor(180, 200, 230);
	for (int i = 0; i < MAX_DROPS; i++) {
		if (!drops[i].isSplashing) {
			iLine(drops[i].x, drops[i].y, drops[i].x + 2, drops[i].y + drops[i].length);
		}
		else {
			iLine(drops[i].x, drops[i].y, drops[i].x - 4, drops[i].y + 4);
			iLine(drops[i].x, drops[i].y, drops[i].x + 4, drops[i].y + 4);
		}
	}
}

void iMouseMove(int mx, int my){}
void iPassiveMouseMove(int mx, int my){}
void iMouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (gameState == MENU) {
			int btnXStart = 660;
			if (mx >= btnXStart && mx <= btnXStart + 280) {
				if (my >= 515 && my <= 555) { gameState = LOADING; currentAssetsLoaded = 0; setupThemeRules(); }
				else if (my >= 430 && my <= 480) gameState = CHARACTER_SELECT;
				else if (my >= 345 && my <= 385) gameState = THEME_SELECT;
				else if (my >= 260 && my <= 290) gameState = SHOP;
				else if (my >= 175 && my <= 215) gameState = ABOUT;
				else if (my >= 90 && my <= 130) exit(0);
			}
		}
		else if (gameState == GAME) {
			if (mx >= 1500 && mx <= 1560 && my >= 820 && my <= 880) gameState = PAUSE;
		}
		else if (gameState == SHOP || gameState == THEME_SELECT || gameState == ABOUT || gameState == CHARACTER_SELECT) {
			if (mx >= 50 && mx <= 200 && my >= 800 && my <= 860) gameState = MENU;

			if (gameState == SHOP) {
				for (int i = 1; i < 4; i++) {
					if (!unlockedChars[i]) {
						int bx = 200 + (i * 300) + 50; int by = 460;
						if (mx >= bx && mx <= bx + 100 && my >= by && my <= by + 30) {
							if (coins >= charPrices[i]) { coins -= charPrices[i]; unlockedChars[i] = true; saveCoins(); saveUnlocks(); }
						}
					}
				}
				for (int i = 0; i < 3; i++) {
					if (i == 1) continue;
					if (!unlockedThemes[i]) {
						int bx = 350 + (i * 300) + 50; int by = 160;
						if (mx >= bx && mx <= bx + 100 && my >= by && my <= by + 30) {
							if (coins >= themePrices[i]) { coins -= themePrices[i]; unlockedThemes[i] = true; saveCoins(); saveUnlocks(); }
						}
					}
				}
			}

			if (gameState == CHARACTER_SELECT) {
				int startX = 200;
				for (int i = 0; i < 4; i++) {
					int xPos = startX + (i * 300); int yPos = 400;
					if (mx >= xPos && mx <= xPos + 200 && my >= yPos && my <= yPos + 200) {
						if (unlockedChars[i]) currentCharacter = i + 1;
					}
				}
			}

			if (gameState == THEME_SELECT) {
				int startX = 350;
				for (int i = 0; i < 3; i++) {
					int xPos = startX + (i * 300); int yPos = 400;
					if (mx >= xPos && mx <= xPos + 200 && my >= yPos && my <= yPos + 200) {
						if (unlockedThemes[i]) {
							if (i == 0) currentTheme = THEME_DAY;
							else if (i == 1) currentTheme = THEME_NIGHT;
							else if (i == 2) currentTheme = THEME_RAIN;
							setupThemeRules();
						}
					}
				}
			}
		}
	}
}

void updateAudio() {
	int desiredAudio = 0;
	if (gameState == PAUSE) desiredAudio = 0;
	// Play loading audio during instruction screen as well
	else if (gameState == MENU || gameState == INSTRUCTION_SCREEN || gameState == LOADING || gameState == SHOP || gameState == THEME_SELECT || gameState == CHARACTER_SELECT || gameState == ABOUT) desiredAudio = 1;
	else if (gameState == GAME) { if (gameOver) desiredAudio = 3; else desiredAudio = 2; }

	if (currentAudioState != desiredAudio) {
		mciSendStringA("close bgm", NULL, 0, NULL);
		if (desiredAudio == 1) { mciSendStringA("open \"Photo/Audio/loading audio.mp3\" type mpegvideo alias bgm", NULL, 0, NULL); mciSendStringA("play bgm repeat", NULL, 0, NULL); }
		else if (desiredAudio == 2) { mciSendStringA("open \"Photo/Audio/gameplay audio.mp3\" type mpegvideo alias bgm", NULL, 0, NULL); mciSendStringA("play bgm repeat", NULL, 0, NULL); }
		else if (desiredAudio == 3) { mciSendStringA("open \"Photo/Audio/gameover audio.mp3\" type mpegvideo alias bgm", NULL, 0, NULL); mciSendStringA("play bgm", NULL, 0, NULL); }
		currentAudioState = desiredAudio;
	}
}

void fixedUpdate(){
	updateAudio();

	// --- INSTRUCTION SCREEN LOGIC & BACKGROUND LOADING ---
	if (gameState == INSTRUCTION_SCREEN) {
		instructionTimer++;

		// Load main assets at tick 5 to ensure instruction image displays first
		if (instructionTimer == 5) {
			background = iLoadImage("Photo/Bg for Main Menu/Background.png");
			woodPlank = iLoadImage("Photo/Logo/wood_plank.png");
			logoImg = iLoadImage("Photo/Logo/Logo.png");
			loadingImage = iLoadImage("Photo/Loading/Loading.png");
			shopBg = iLoadImage("Photo/Bg for Main Menu/Shop.png");
			themeBg = iLoadImage("Photo/Bg for Main Menu/Theme.png");
			aboutBg = iLoadImage("Photo/Bg for Main Menu/About_us.png");
			pauseBtn = iLoadImage("Photo/Pause Button/Pause_button.png");
			pausedScreenImg = iLoadImage("Photo/Pause Button/paused.png");
			gameoverImg = iLoadImage("Photo/gameover.png");
			heartImg = iLoadImage("Photo/Power/heart.png");
			shopMainBg = iLoadImage("Photo/Bg for Main Menu/background.png");
			frameImg = iLoadImage("Photo/Shop/frame.png");
			lockImg = iLoadImage("Photo/Shop/lock.png");

			charThumbs[0] = iLoadImage("Photo/Shop/Character1.png"); charThumbs[1] = iLoadImage("Photo/Shop/Character2.png");
			charThumbs[2] = iLoadImage("Photo/Shop/Character3.png"); charThumbs[3] = iLoadImage("Photo/Shop/Character4.png");
			themeThumbs[0] = iLoadImage("Photo/Shop/theme1.bmp"); themeThumbs[1] = iLoadImage("Photo/Shop/theme2.bmp");
			themeThumbs[2] = iLoadImage("Photo/Shop/theme4.bmp");
			coinImg = iLoadImage("Photo/Power/coin.png");
		}

		// 500 ticks * 20ms = 10,000ms (10 seconds)
		if (instructionTimer >= 500) {
			gameState = MENU; // Automatically transition to main menu after 10 seconds
		}
		return; // Skip rest of game logic while in instruction screen
	}

	if (keyDelay > 0) keyDelay--;
	if (gameState == GAME && !gameOver) animateObstacles();

	if (gameState == GAME && !gameOver) {
		if (powerTimer > 0) { score += 2; powerTimer--; }
		else score += 1;
		speedBoost = score / 1500;
		if (speedBoost > 12) speedBoost = 12;

		if (netTimer > 0) netTimer--;
		if (shieldTimer > 0) { shieldTimer--; if (shieldTimer <= 0) isShieldActive = false; }

		if (heartTextActive) { heartTextY += 2; heartTextTimer--; if (heartTextTimer <= 0) heartTextActive = false; }

		bool isUpPressed = (isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP));
		bool isDownPressed = (isKeyPressed('s') || isSpecialKeyPressed(GLUT_KEY_DOWN));
		bool goLeft = (isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT));
		bool goRight = (isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT));

		if (isUpPressed && !wasUpPressed && y < 1020) {
			if (duckTimer == 0) {
				clock_t currentTime = clock();
				double timeDiff = (double)(currentTime - lastUpPressTime) * 1000 / CLOCKS_PER_SEC;
				if (timeDiff < doublePressThreshold && j > 0) {
					if (speedBoost < 5) j = 1000 - speedBoost * 50; else j = 800;
				}
				else if (j == 0) {
					if (speedBoost < 5) j = 800 - speedBoost * 50; else j = 600;
				}
				lastUpPressTime = currentTime;
			}
		}
		wasUpPressed = isUpPressed;

		if (isDownPressed && !wasDownPressed) {
			if (j == 0 && duckTimer == 0) {
				if (speedBoost < 5) duckTimer = 800 - speedBoost * 50; else duckTimer = 600;
			}
		}
		wasDownPressed = isDownPressed;

		if (goLeft && keyDelay == 0) {
			if (playerCurrentLane > 0) { playerCurrentLane--; x = playerLaneX[playerCurrentLane]; }
			keyDelay = (currentTheme == THEME_RAIN) ? 30 : 15;
		}

		if (goRight && keyDelay == 0) {
			if (playerCurrentLane < totalLanes - 1) { playerCurrentLane++; x = playerLaneX[playerCurrentLane]; }
			keyDelay = (currentTheme == THEME_RAIN) ? 30 : 15;
		}
	}

	if ((isKeyPressed('r') || isKeyPressed('R')) && keyDelay == 0) {
		if (gameState == PAUSE || (gameState == GAME && gameOver)) { resetGame(); gameState = GAME; keyDelay = 15; }
	}
	if ((isKeyPressed('m') || isKeyPressed('M')) && keyDelay == 0 && (gameState == PAUSE || (gameState == GAME && gameOver) || gameState == CHARACTER_SELECT || gameState == THEME_SELECT || gameState == SHOP || gameState == ABOUT)) {
		gameState = MENU; gameOver = false; keyDelay = 15;
	}
	if ((isKeyPressed('p') || isKeyPressed('P')) && keyDelay == 0) {
		if (gameState == GAME && !gameOver) gameState = PAUSE; else if (gameState == PAUSE) gameState = GAME;
		keyDelay = 15;
	}
	if ((isKeyPressed('h') || isKeyPressed('H')) && keyDelay == 0) {
		if (gameState == GAME && gameOver) { useHeartToRevive(); keyDelay = 15; }
	}
	if ((isKeyPressed('e') || isKeyPressed('E')) && keyDelay == 0) {
		if (gameState == MENU) exit(0);
	}
}

void changeBackground() { bgIndex++; if (bgIndex >= 172) bgIndex = 0; }
void changeCharacter(){ charIndex++; if (charIndex == 2) charIndex = 0; }

void loadCharacterSprites() {
	char pathLeft[100], pathRight[100], pathJump[100];
	sprintf(pathLeft, "Photo/Character/Character%d_Left.png", currentCharacter);
	sprintf(pathRight, "Photo/Character/Character%d_Right.png", currentCharacter);
	sprintf(pathJump, "Photo/Character/Character%d_Jump.png", currentCharacter);
	character[0] = iLoadImage(pathLeft); character[1] = iLoadImage(pathRight); currentImage = iLoadImage(pathJump);

	char netPathLeft[100], netPathRight[100];
	sprintf(netPathLeft, "Photo/Character/Character%d_Left_Net.png", currentCharacter);
	sprintf(netPathRight, "Photo/Character/Character%d_Right_Net.png", currentCharacter);
	characterNet[0] = iLoadImage(netPathLeft); characterNet[1] = iLoadImage(netPathRight);

	char sPathLeft[100], sPathRight[100], sPathJump[100], snPathLeft[100], snPathRight[100];
	sprintf(sPathLeft, "Photo/Character/Character%d_Left_Shield.png", currentCharacter);
	sprintf(sPathRight, "Photo/Character/Character%d_Right_Shield.png", currentCharacter);
	sprintf(sPathJump, "Photo/Character/Character%d_Jump_Shield.png", currentCharacter);
	sprintf(snPathLeft, "Photo/Character/Character%d_Left_Net_Shield.png", currentCharacter);
	sprintf(snPathRight, "Photo/Character/Character%d_Right_Net_Shield.png", currentCharacter);

	characterShield[0] = iLoadImage(sPathLeft); characterShield[1] = iLoadImage(sPathRight); jumpShieldImg = iLoadImage(sPathJump);
	characterNetShield[0] = iLoadImage(snPathLeft); characterNetShield[1] = iLoadImage(snPathRight);
}

void loadNextAsset() {
	if (currentAssetsLoaded < 172) {
		char filename[100];
		if (currentTheme == THEME_DAY) sprintf(filename, "Photo/Background/Day/bg%05d.bmp", currentAssetsLoaded + 19);
		else if (currentTheme == THEME_NIGHT ) sprintf(filename, "Photo/Background/Night/bg%05d.bmp", currentAssetsLoaded + 19);
		else if (currentTheme == THEME_RAIN) sprintf(filename, "Photo/Background/Rain/bg%05d.bmp", currentAssetsLoaded + 6);
		else sprintf(filename, "Photo/Background/Day/bg%05d.bmp", currentAssetsLoaded + 19);
		backgroundImages[currentAssetsLoaded] = iLoadImage(filename);
	}
	else if (currentAssetsLoaded == 172) {}
	else if (currentAssetsLoaded == 173) {}
	else if (currentAssetsLoaded == 174) loadCharacterSprites();
	else if (currentAssetsLoaded >= 175 && currentAssetsLoaded < 187) {
		int obsIdx = currentAssetsLoaded - 175; char filename[100];
		if (obsIdx < 4) sprintf(filename, "Photo/Obstacle/Bus/bus%d.png", obsIdx + 1);
		else if (obsIdx < 6) sprintf(filename, "Photo/Obstacle/Truck/truck%d.png", obsIdx - 3);
		else if (obsIdx < 8) sprintf(filename, "Photo/Obstacle/Rickshaw/rickshaw%d.png", obsIdx - 5);
		else if (obsIdx < 10) sprintf(filename, "Photo/Obstacle/Van/van%d.png", obsIdx - 7);
		else if (obsIdx < 12) sprintf(filename, "Photo/Obstacle/cc/cc%d.png", obsIdx - 9);
		obstacle[obsIdx] = iLoadImage(filename);
	}
	else if (currentAssetsLoaded == 187) { obstacle[12] = iLoadImage("Photo/Obstacle/dog1.png"); obstacle[17] = iLoadImage("Photo/Obstacle/dog2.png"); }
	else if (currentAssetsLoaded == 188) obstacle[13] = iLoadImage("Photo/Obstacle/crack.png");
	else if (currentAssetsLoaded == 189) obstacle[14] = iLoadImage("Photo/Obstacle/manhole.png");
	else if (currentAssetsLoaded == 190) obstacle[15] = iLoadImage("Photo/Obstacle/Eagle/eagle1.png");
	else if (currentAssetsLoaded == 191) obstacle[16] = iLoadImage("Photo/Obstacle/Eagle/eagle2.png");
	else if (currentAssetsLoaded == 192) kalojamImg = iLoadImage("Photo/Power/kalojam.png");
	else if (currentAssetsLoaded == 193) scoreBoardImg = iLoadImage("Photo/HighScore_board.png");
	else if (currentAssetsLoaded == 194) heartImg = iLoadImage("Photo/Power/heart.png");
	else if (currentAssetsLoaded == 195) netImg = iLoadImage("Photo/Power/net.png");
	else if (currentAssetsLoaded == 196) shieldImg = iLoadImage("Photo/Power/shield.png");
	// Removed load image requests for poisonImg, anchorImg, and policeImg here.
	currentAssetsLoaded++;
}

void drawLoading() {
	iSetColor(247, 207, 56); iFilledRectangle(0, 0, 1600, 900);
	if (loadingImage != -1) iShowImage(0, 0, 1600, 900, loadingImage);

	iSetColor(100, 100, 100); iRectangle(400, 150, 800, 25);
	double progressRatio = (double)currentAssetsLoaded / totalAssets; double progressWidth = progressRatio * 800;
	iSetColor(0, 0, 0); iFilledRectangle(400, 150, (int)progressWidth, 25);
	char percentText[10]; int percent = (int)(progressRatio * 100); if (percent > 100) percent = 100;
	sprintf(percentText, "%d%%", percent);
	iSetColor(0, 0, 0); iText(780, 120, percentText, GLUT_BITMAP_HELVETICA_18);
	double angle = currentAssetsLoaded * 0.2; iFilledCircle(800 + (int)(30 * cos(angle)), 210 + (int)(30 * sin(angle)), 6);

	if (currentAssetsLoaded < totalAssets) loadNextAsset(); else { gameState = GAME; resetGame(); }
}

void iDraw()
{
	iClear();

	// Render the instruction screen
	if (gameState == INSTRUCTION_SCREEN) {
		iShowImage(0, 0, 1600, 900, instructionImg);
	}
	else if (gameState == MENU) drawMenu();
	else if (gameState == LOADING) drawLoading();
	else if (gameState == SHOP) {
		iShowImage(0, 0, 1600, 900, shopMainBg); drawBackButton(); drawBalanceUI();
		iSetColor(255, 255, 255); iShowImage(670, 730, 190, 60, woodPlank); iText(700, 750, "ITEM SHOP", GLUT_BITMAP_TIMES_ROMAN_24);

		for (int i = 0; i < 4; i++) {
			int px = 200 + (i * 300); int py = 500;
			iShowImage(px, py, 200, 200, frameImg); iShowImage(px + 10, py + 10, 180, 180, charThumbs[i]);
			if (unlockedChars[i]) { iSetColor(30, 30, 30); iFilledRectangle(px + 10, py + 80, 180, 40); iSetColor(0, 255, 0); iText(px + 50, py + 90, "OWNED", GLUT_BITMAP_TIMES_ROMAN_24); }
			else { iSetColor(0, 150, 0); iFilledRectangle(px + 50, py - 40, 100, 30); iSetColor(255, 255, 255); char priceStr[20]; sprintf(priceStr, "BUY: %d", charPrices[i]); iText(px + 60, py - 32, priceStr, GLUT_BITMAP_HELVETICA_12); }
		}
		for (int i = 0; i < 3; i++) {
			int px = 350 + (i * 300); int py = 200;
			iShowImage(px, py, 200, 200, frameImg); iShowImage(px + 10, py + 10, 180, 180, themeThumbs[i]);
			if (unlockedThemes[i]) { iSetColor(30, 30, 30); iFilledRectangle(px + 10, py + 80, 180, 40); iSetColor(0, 255, 0); iText(px + 50, py + 90, "OWNED", GLUT_BITMAP_TIMES_ROMAN_24); }
			else { iSetColor(0, 150, 0); iFilledRectangle(px + 50, py - 40, 100, 30); iSetColor(255, 255, 255); char priceStr[20]; sprintf(priceStr, "BUY: %d", themePrices[i]); iText(px + 60, py - 32, priceStr, GLUT_BITMAP_HELVETICA_12); }
		}
	}
	else if (gameState == THEME_SELECT) {
		iShowImage(0, 0, 1600, 900, background); drawBackButton(); iSetColor(255, 255, 255); iText(700, 800, "SELECT THEME", GLUT_BITMAP_TIMES_ROMAN_24);
		int startX = 350;
		for (int i = 0; i < 3; i++) {
			int xPos = startX + (i * 300); int yPos = 400;
			if (currentTheme == (i == 0 ? THEME_DAY : (i == 1 ? THEME_NIGHT : THEME_RAIN))) { iSetColor(0, 255, 0); iFilledRectangle(xPos - 10, yPos - 10, 220, 220); }
			if (unlockedThemes[i]) { iShowImage(xPos, yPos, 200, 200, frameImg); iShowImage(xPos + 10, yPos + 10, 180, 180, themeThumbs[i]); }
			else { iShowImage(xPos, yPos, 200, 200, frameImg); iShowImage(xPos + 10, yPos + 10, 180, 180, themeThumbs[i]); iSetColor(20, 20, 20); iFilledRectangle(xPos + 10, yPos + 10, 180, 180); iShowImage(xPos + 60, yPos + 60, 80, 80, lockImg); }
		}
	}
	else if (gameState == ABOUT) { iShowImage(0, 0, 1600, 900, aboutBg); drawBackButton(); }
	else if (gameState == PAUSE) drawPauseMenu();
	else if (gameState == CHARACTER_SELECT) {
		iShowImage(0, 0, 1600, 900, background); drawBackButton(); iSetColor(255, 255, 255); iText(650, 800, "SELECT CHARACTER", GLUT_BITMAP_TIMES_ROMAN_24);
		int startX = 200;
		for (int i = 0; i < 4; i++) {
			int xPos = startX + (i * 300); int yPos = 400;
			if (currentCharacter == i + 1) { iSetColor(0, 255, 0); iFilledRectangle(xPos - 10, yPos - 10, 220, 220); }
			if (unlockedChars[i]) { iShowImage(xPos, yPos, 200, 200, frameImg); iShowImage(xPos + 10, yPos + 10, 180, 180, charThumbs[i]); }
			else { iShowImage(xPos, yPos, 200, 200, frameImg); iShowImage(xPos + 10, yPos + 10, 180, 180, charThumbs[i]); iSetColor(20, 20, 20); iFilledRectangle(xPos + 10, yPos + 10, 180, 180); iShowImage(xPos + 60, yPos + 60, 80, 80, lockImg); }
		}
	}
	else if (gameState == GAME) {
		if (gameOver) {
			iShowImage(0, 0, 1600, 900, gameoverImg);
			iSetColor(0, 0, 0);

			char gmHeartStr[50];
			sprintf(gmHeartStr, "YOU HAVE %d", hearts);
			iText(720, 610, gmHeartStr, GLUT_BITMAP_TIMES_ROMAN_24);
			iShowImage(890, 600, 30, 40, heartImg);
			iSetColor(218, 165, 32);

			iFilledRectangle(90, 700, 300, 80);
			iSetColor(0, 0, 0);

			char gmScoreStr[50];
			sprintf(gmScoreStr, "FINAL SCORE: %d", score);
			iText(100, 750, gmScoreStr, GLUT_BITMAP_TIMES_ROMAN_24);

			char gmCoinStr[50];
			sprintf(gmCoinStr, "COINS COLLECTED: %d", sessionCoins);
			iText(100, 710, gmCoinStr, GLUT_BITMAP_TIMES_ROMAN_24);
		}
		else {
			iShowImage(0, 0, 1600, 900, backgroundImages[bgIndex]);
			drawRain();

			iShowImage(1500, 820, 60, 60, pauseBtn);

			iShowImage(20, 650, 250, 250, scoreBoardImg);
			iSetColor(60, 30, 0);
			char scoreStr[20], hsStr[20], coinStr[20];
			sprintf(scoreStr, "SCORE: %d", score); sprintf(hsStr, "HI-SCORE: %d", highScore);
			iText(65, 810, scoreStr, GLUT_BITMAP_HELVETICA_18); iText(65, 780, hsStr, GLUT_BITMAP_HELVETICA_12);
			sprintf(coinStr, "COINS: %d", sessionCoins); iText(85, 715, coinStr, GLUT_BITMAP_HELVETICA_18);
			iShowImage(60, 708, 20, 20, coinImg);

			// --- DRAW POWER BARS (Left Side) ---
			if (shieldTimer > 0) {
				int barWidth = (shieldTimer * 150) / 600; iShowImage(45, 540, 30, 30, shieldImg);
				iSetColor(255, 105, 180); iFilledRectangle(85, 545, barWidth, 15); iSetColor(0, 0, 0); iRectangle(85, 545, 150, 15);
			}
			if (powerTimer > 0) {
				int barWidth = (powerTimer * 150) / 250; iShowImage(45, 620, 30, 30, kalojamImg);
				iSetColor(255, 255, 0); iFilledRectangle(85, 625, barWidth, 15); iSetColor(0, 0, 0); iRectangle(85, 625, 150, 15);
			}
			if (netTimer > 0) {
				int barWidth = (netTimer * 150) / 500; iShowImage(45, 580, 30, 30, netImg);
				iSetColor(255, 0, 0); iFilledRectangle(85, 585, barWidth, 15); iSetColor(0, 0, 0); iRectangle(85, 585, 150, 15);
			}

			drawObstacles();

			int k = 0; if (speedBoost > 5) k = 4 - speedBoost; else k = 0;
			int currentSprite;
			if (j > 800 + k * 50) {
				currentSprite = isShieldActive ? jumpShieldImg : currentImage; iShowImage(x, 170, 120, 180, currentSprite); j--;
			}
			else if (j > 0) {
				currentSprite = isShieldActive ? jumpShieldImg : currentImage; iShowImage(x, 130, 120, 180, currentSprite); j--;
			}
			else if (duckTimer > 0) {
				if (isShieldActive) currentSprite = (netTimer > 0) ? characterNetShield[charIndex] : characterShield[charIndex];
				else currentSprite = (netTimer > 0) ? characterNet[charIndex] : character[charIndex];
				iShowImage(x, y, 120, 90, currentSprite); duckTimer--;
			}
			else {
				if (isShieldActive) currentSprite = (netTimer > 0) ? characterNetShield[charIndex] : characterShield[charIndex];
				else currentSprite = (netTimer > 0) ? characterNet[charIndex] : character[charIndex];
				iShowImage(x, y, 120, 180, currentSprite);
			}

			for (int i = 0; i<5; i++){
				if (gameCoins[i].active && gameCoins[i].y <= horizonY) {
					double scale = (1.0 - (gameCoins[i].y / 575.0)) * 40 + 20;
					iShowImage(gameCoins[i].x, gameCoins[i].y, scale, scale, coinImg);
				}
			}

			if (shieldPower.active && shieldPower.y <= horizonY) { double scale = (1 - (shieldPower.y / 575.0)) * 50 + 20; iShowImage(shieldPower.x, shieldPower.y, scale + 25, scale + 25, shieldImg); }
			if (kalojamIcon.active && kalojamIcon.y <= horizonY) { double scale = (1 - (kalojamIcon.y / 575.0)) * 50 + 20; iShowImage(kalojamIcon.x, kalojamIcon.y, scale, scale, kalojamImg); }
			if (heartPower.active && heartPower.y <= horizonY) { double scale = (1 - (heartPower.y / 575.0)) * 50 + 20; iShowImage(heartPower.x, heartPower.y, scale + 20, scale + 20, heartImg); }
			if (netPower.active && netPower.y <= horizonY) { double scale = (1 - (netPower.y / 575.0)) * 50 + 20; iShowImage(netPower.x, netPower.y, scale + 25, scale + 25, netImg); }

			if (heartTextActive) { iSetColor(255, 0, 0); iText(x, heartTextY, "One Heart Added!", GLUT_BITMAP_TIMES_ROMAN_24); }

		}
	}
}

int main()
{
	srand((unsigned int)time(NULL));
	setupThemeRules();
	x = playerLaneX[inix()];

	loadHighScore();
	loadCoins();
	loadHearts();
	loadUnlocks();

	for (int i = 0; i<5; i++){
		gameCoins[i].y = 575 + (i * 40);
		gameCoins[i].lane = rand() % totalLanes;
		gameCoins[i].active = (i == 0);
	}

	initRain();

	iInitialize(1600, 900, "Cholo Run");

	
	instructionImg = iLoadImage("Photo/instruction.png");

	for (int i = 0; i < 6; i++) { obs[i].active = false; }
	for (int i = 0; i < 3; i++) {
		obs[i].y = horizonY + (i * 250);
		obs[i].lane = getUniqueLane();
		obs[i].type = getThemeObstacle();
		obs[i].active = true;
	}

	iSetTimer(30, moveObstacles);
	iSetTimer(30, updateRain);
	iSetTimer(200, changeCharacter);
	iSetTimer(20, fixedUpdate);
	iSetTimer(33.33, changeBackground);

	iStart();
	return 0;
}
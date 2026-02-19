#define _CRT_SECURE_NO_WARNINGS
#include "iGraphics.h"
#include <math.h>
#include <stdio.h>

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

//variable for main menu pic load
int background, woodPlank, logoImg, shopBg, themeBg, aboutBg, pauseBtn;

// score & data variables value assign
int gameState = MENU;
int gender = 0;
int loadingTime = 0;
int loadingImage;
int score = 0;
int highScore = 0;

//  DATA PERSISTENCE 
void saveHighScore() {
	FILE *fp = fopen("highscore.txt", "w");
	if (fp != NULL) {
		fprintf(fp, "%d", highScore);
		fclose(fp);
	}
}

void loadHighScore() {

	FILE *fp = fopen("highscore.txt", "r");
	if (fp != NULL) {
		if (fscanf(fp, "%d", &highScore) != 1) highScore = 0;
		fclose(fp);
	}
}

//variable for coin
int coins = 0;
int sessionCoins = 0; 

struct Coin {
	double x, y;
	int lane;
	bool active;
} gameCoin;

void saveCoins() {
	FILE *fp = fopen("coins.txt", "w");
	if (fp != NULL) {
		fprintf(fp, "%d", coins);
		fclose(fp);
	}
}

void loadCoins() {
	FILE *fp = fopen("coins.txt", "r");
	if (fp != NULL) {
		if (fscanf(fp, "%d", &coins) != 1) coins = 0;
		fclose(fp);
	}
}

// --- SHARED UI COMPONENTS ---

void drawBackButton() {
	iShowImage(50, 800, 150, 60, woodPlank);
	iSetColor(255, 255, 255);
	iText(95, 822, "BACK", GLUT_BITMAP_HELVETICA_18);
}

void drawBalanceUI() {
	// 1. Draw the UI Box for Balance
	iSetColor(30, 30, 30);
	iFilledRectangle(680, 815, 240, 50);
	iSetColor(255, 255, 255);
	iRectangle(680, 815, 240, 50);

	// 2. Draw the Gold Coin Icon
	iSetColor(255, 215, 0); // Gold Color
	iFilledCircle(710, 840, 12);
	iSetColor(218, 165, 32); // Darker Gold border
	iCircle(710, 840, 12);
	iSetColor(0, 0, 0);
	iText(707, 834, "$", GLUT_BITMAP_HELVETICA_12);

	// 3. Draw the Balance Text with NEW FONT
	char balStr[30];
	sprintf(balStr, "BALANCE: %d", coins);
	iSetColor(255, 255, 255);
	// Using Times Roman 24 for a larger, "Full Font" look
	iText(735, 832, balStr, GLUT_BITMAP_TIMES_ROMAN_24);
}

// --- STATE DRAWING FUNCTIONS ---

void drawMenu() {
	iShowImage(0, 0, 1600, 900, background);
	iShowImage(600, 650, 400, 200, logoImg);
	char hsText[30];
	sprintf(hsText, "BEST RECORD: %d", highScore);
	iSetColor(255, 255, 255);
	iText(720, 620, hsText, GLUT_BITMAP_HELVETICA_18);

	int btnW = 300, btnH = 80, btnX = (1600 - btnW) / 2;
	char labels[6][20] = { "START", "CHARACTER", "THEME", "SHOP", "ABOUT US", "EXIT" };
	for (int i = 0; i < 6; i++) {
		int currentBtnY = 520 - (i * 95);
		iShowImage(btnX, currentBtnY, btnW, btnH, woodPlank);
		iSetColor(255, 255, 255);
		iText(btnX + 80, currentBtnY + 32, labels[i], GLUT_BITMAP_HELVETICA_18);
	}
}

void drawPauseMenu() {
	iShowImage(0, 0, 1600, 900, background);
	iSetColor(20, 20, 20);
	iFilledRectangle(550, 300, 500, 400);
	iSetColor(255, 255, 255);
	iRectangle(550, 300, 500, 400);

	iText(750, 620, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

	iText(690, 520, "PRESS 'P' TO RESUME", GLUT_BITMAP_HELVETICA_18);

	iText(690, 470, "PRESS 'R' TO RESTART", GLUT_BITMAP_HELVETICA_18);

	iText(660, 420, "PRESS 'M' FOR MAIN MENU", GLUT_BITMAP_HELVETICA_18);
}

void drawLoading() {
	iSetColor(247, 207, 56);
	iFilledRectangle(0, 0, 1600, 900);
	if (loadingImage != -1) iShowImage(400, 250, 800, 450, loadingImage);
	iSetColor(100, 100, 100);
	iRectangle(400, 150, 800, 25);
	double progressWidth = (loadingTime / 80.0) * 800;
	iSetColor(0, 0, 0);
	iFilledRectangle(400, 150, (int)progressWidth, 25);
	char percentText[10];
	int percent = (int)((loadingTime / 80.0) * 100);
	if (percent > 100) percent = 100;
	sprintf(percentText, "%d%%", percent);
	iSetColor(0, 0, 0);
	iText(780, 120, percentText, GLUT_BITMAP_HELVETICA_18);
	double angle = loadingTime * 0.2;
	iFilledCircle(800 + (int)(30 * cos(angle)), 210 + (int)(30 * sin(angle)), 6);
}

//this variable help to press button
int keyDelay = 0;
//variable for background;
int backgroundImages[191]; // array for 191 images
int bgIndex = 0;

//for initialize x
int ix[4] = { 460, 630, 780, 970 };
//this function initialize x different everytime we start game
int inix()
{
	return rand() % 4;
}



//variable for character
int charIndex = 0;
int character[2];
int x = ix[inix()];
int y = 100;

//variable for load character jump image 
int currentImage;

//variable for load character in manhole image
int menInManhole;

//variable for jump in Crack & Manhole 
int j;

bool gameOver = false;

//variable for load obstacle image
int obstacle[11];
int lastLane = -1;
int lastObs = -2;

//this variabel for take all info about obstacle
struct Obstacle{
	double x, y;
	int lane;   // 0, 1, 2, or 3
	int type;   // index for your obstacle image array (bus, truck, etc.)
	bool active;
	double speed;
} obs[3];

//this function for get unique lane everytime if this function prevent old and new lane same
int getUniqueLane() {
	int newLane = rand() % 4;

	while (newLane == lastLane) {
		newLane = rand() % 4;
	}

	lastLane = newLane;
	return newLane;
}

//this function for get unique obs everytime if this function prevent old and new obstacle same 
int getUniqueobs() {
	int newObs = rand() % 11;

	while (newObs == lastObs) {
		newObs = rand() % 11;
	}

	lastObs = newObs;
	return newObs;
}

void resetGame() {
	score = 0;
	sessionCoins = 0;
	y = 100;
	loadingTime = 0;
	gameOver = false;

	gameCoin.y = 575+rand()%5 ;
	gameCoin.lane = getUniqueLane() ;
	gameCoin.active = true;

	// push all obstacles back to the start
	for (int i = 0; i < 3; i++) {
		obs[i].y = 575;
		obs[i].lane = getUniqueLane();
		obs[i].type = getUniqueobs();
		obs[i].active = true;
	}
}

void moveLoading() {
	if (gameState == LOADING) {
		loadingTime++;
		if (loadingTime >= 80) {
			gameState = GAME;
			resetGame(); // Calls the new function to set up the board
		}
	}
}

//this function for check collison
void checkCollision() {

	for (int i = 0; i < 3; i++) {

		//chech coin hit or miss
		if (gameCoin.active) {
			int cLeft = gameCoin.x;
			int cRight = gameCoin.x + 30;
			int cBottom = gameCoin.y;
			int cTop = gameCoin.y + 30;

			int playerLeft = x + 40;
			int playerRight = x + 140;
			int playerBottom = y + 20;
			int playerTop = y + 120;

			// if player touches the coin
			if (cRight > playerLeft && cLeft < playerRight &&
				cTop > playerBottom && cBottom < playerTop)
			{
				if (j == 0) { 
					printf("COIN COLLECTED!\n");
					coins += 1;   
					sessionCoins += 1;
					saveCoins();      
					gameCoin.active = false; 
				}
			}
		}

		if (!obs[i].active) continue;

		//double scale = (1.0 - (obs[i].y / 575.0)) * 120 + 50;

		int obsLeft = obs[i].x;
		int obsRight = obs[i].x + 30;
		int obsBottom = obs[i].y;
		int obsTop = obs[i].y + 30;

		int playerLeft = x + 40;
		int playerRight = x + 140;
		int playerBottom = y + 20;
		int playerTop = y + 120;


		if ((obs[i].type == 9 || obs[i].type == 10) ) {
			if (j > 0) continue;
		}

		if (obsRight > playerLeft && obsLeft < playerRight &&
			obsTop > playerBottom && obsBottom < playerTop)
		{
			printf("COLLISION!\n");
			gameOver = true;
			if (score > highScore) {
				highScore = score;
				saveHighScore();
			}
		}
	}
}

//this function for move obstacle 
void moveObstacles() {

	//this function check our game loading is compleate or not
	if (gameState != GAME) {
		return;
	}

	if (gameOver)    //this if condition for check if character hit obstacle then gameover is true then this function return or stop
	{
		return;
	}
	double horizonY = 575; // obstacle appear for here
	double bottomY = 0;

	int speedBoost = score / 1500;
	if (speedBoost > 12) {
		speedBoost = 12;
	}

	// horizon lane er x coordinate (white line)
	double laneTopX[] = { 763, 780, 798, 821 };

	//bottom lane er x coordinate (white line)
	double laneBottomX[] = { 330, 570, 800, 1030 };

	for (int i = 0; i < 3; i++) {
		
		//coin move function
		double horizonY = 575;
		double bottomY = 0;
		double laneTopX[] = { 763, 780, 798, 821 };
		double laneBottomX[] = { 330, 570, 800, 1030 };

		if (gameCoin.active) {
			int speedBoost = score / 1500;
			gameCoin.y -= 5 + speedBoost; // coin speed

			
			double t = (horizonY - gameCoin.y) / (horizonY - bottomY);
			gameCoin.x = laneTopX[gameCoin.lane] + t * (laneBottomX[gameCoin.lane] - laneTopX[gameCoin.lane]);

			
			if (gameCoin.y < -50) {
				gameCoin.y = horizonY + (rand() % 20);
				gameCoin.lane = getUniqueLane();
			}
		}
		else {
			
			gameCoin.y -= 7;
			if (gameCoin.y < -50) {
				gameCoin.y = horizonY + (rand() % 400); // Delay next coin spawn
				gameCoin.lane = rand() % 4;
				gameCoin.active = true;
			}
		}

		if (obs[i].active) {
			obs[i].y -= obs[i].speed;

			// t when obstacle go down value of t increase
			double t = (horizonY - obs[i].y) / (horizonY - bottomY);

			int lane = obs[i].lane;

			// lane interpolation get x value 
			obs[i].x = laneTopX[lane] + t * (laneBottomX[lane] - laneTopX[lane]);

			// reset logic
			if (obs[i].y < -200) {
				obs[i].y = horizonY;
				obs[i].lane = getUniqueLane();
				obs[i].type = getUniqueobs();
				obs[i].speed = 4 + (rand() % 5) + speedBoost;
				obs[i].active = true;
			}
		}
		checkCollision();
	}
}

//this function for drow obstacle 
void drawObstacles() {
	for (int i = 0; i < 3; i++) {
		if (obs[i].active) {

			//this condition for check our character jame and obstacle are crack or manhole then the obstacle do not draw
			if (j>0 && (obs[i].type == 9 || obs[i].type == 10) )
				continue;

			// dynamic width/height based on Y position (perspective scaling)
			double scale = (1.0 - (obs[i].y / 575.0)) * 120 + 50;


			iShowImage(obs[i].x, obs[i].y, scale, scale, obstacle[obs[i].type]);
		}
	}
}


void iMouseMove(int mx, int my){
}
void iPassiveMouseMove(int mx, int my){
}
void iMouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (gameState == MENU) {
			int btnXStart = 660;
			if (mx >= btnXStart && mx <= btnXStart + 280) {
				if (my >= 535 && my <= 575) { gameState = LOADING; loadingTime = 0; }
				else if (my >= 440 && my <= 480) gameState = CHARACTER_SELECT;
				else if (my >= 345 && my <= 385) gameState = THEME_SELECT;
				else if (my >= 250 && my <= 290) gameState = SHOP;
				else if (my >= 155 && my <= 195) gameState = ABOUT;
				else if (my >= 60 && my <= 100) exit(0);
			}
		}
		else if (gameState == GAME) {
			if (mx >= 1500 && mx <= 1560 && my >= 820 && my <= 880) gameState = PAUSE;
		}
		else if (gameState == SHOP || gameState == THEME_SELECT || gameState == ABOUT) {
			if (mx >= 50 && mx <= 200 && my >= 800 && my <= 860) gameState = MENU;
		}
	}
}

//this function is for button
void fixedUpdate(){
	if (keyDelay > 0) {
		keyDelay--;
	}

	//score increase
	if (gameState == GAME && !gameOver) {
		score++;

		if ((isKeyPressed('w') || isSpecialKeyPressed(GLUT_KEY_UP)) && y < 1020)
		{
			j = 50;
		}

		if ((isKeyPressed('a') || isSpecialKeyPressed(GLUT_KEY_LEFT)) && keyDelay == 0)
		{
			if (x == 970)
				x = 780;
			else if (x == 780)
				x = 630;
			else if (x == 630)
				x = 460;
			else
				x = 460;
			keyDelay = 15;
		}

		if ((isKeyPressed('d') || isSpecialKeyPressed(GLUT_KEY_RIGHT)) && keyDelay == 0)
		{
			if (x == 460)
				x = 630;
			else if (x == 630)
				x = 780;
			else if (x == 780)
				x = 970;
			else
				x = 970;
			keyDelay = 15;
		}
	}

	if ((isKeyPressed('r') || isKeyPressed('R')) && keyDelay == 0) {
		// Allow restart if game is PAUSED or if GAME OVER is true
		if (gameState == PAUSE || (gameState == GAME && gameOver)) {
			resetGame();
			gameState = GAME;
			keyDelay = 15;
		}
	}

	if ((isKeyPressed('m') || isKeyPressed('M')) && keyDelay == 0 && (gameState == PAUSE || (gameState == GAME && gameOver) || gameState == CHARACTER_SELECT || gameState == THEME_SELECT || gameState == SHOP || gameState == ABOUT)) {
		gameState = MENU;
		gameOver = false;
		keyDelay = 15;
	}
	if ((isKeyPressed('p') || isKeyPressed('P')) && keyDelay == 0) {
		if (gameState == GAME && !gameOver) gameState = PAUSE;
		else if (gameState == PAUSE) gameState = GAME;
		keyDelay = 15;
	}
	if ((isKeyPressed('e') || isKeyPressed('E')) && keyDelay == 0) {
		if (gameState == MENU)
			exit(0);
	}
}

//this function for change background indes and if it reach 192 then this function change index to 0
void changeBackground() {
	bgIndex++;
	if (bgIndex >= 191) { // Reset after the 191st image (index 190)
		bgIndex = 0;
	}
}

//this function for change character indes and if it reach 2 then this function change index to 0
void changeCharacter(){
	charIndex++;
	if (charIndex == 2) {
		charIndex = 0;
	}
}

void iDraw()
{
	iClear();

	if (gameState == MENU) drawMenu();

	else if (gameState == LOADING) drawLoading();

	else if (gameState == SHOP) {


		iShowImage(0, 0, 1600, 900, shopBg);
		drawBackButton();
		drawBalanceUI(); // Balance icon and text shown here
	}

	else if (gameState == THEME_SELECT) {
		iShowImage(0, 0, 1600, 900, themeBg);
		drawBackButton();
	}

	else if (gameState == ABOUT) {
		iShowImage(0, 0, 1600, 900, aboutBg);
		drawBackButton();
	}

	else if (gameState == PAUSE) drawPauseMenu();

	else if (gameState == CHARACTER_SELECT) {
		iSetColor(30, 30, 30); iFilledRectangle(0, 0, 1600, 900);
		iSetColor(255, 255, 255);
		iText(700, 500, "Press '1' for MALE  '2' for FEMALE", GLUT_BITMAP_HELVETICA_18);
		iSetColor(200, 200, 200);
		iText(780, 470, "Not Available for now", GLUT_BITMAP_HELVETICA_12);
		iSetColor(255, 255, 255);
		iText(50, 850, "PRESS 'M' TO BACK IN MAIN MENU", GLUT_BITMAP_HELVETICA_18);
	}

	else if (gameState == GAME) {
		//this function show game over text
		if (gameOver) {
			// Draw the Game Over Menu
			iSetColor(255, 0, 0);
			iText(700, 480, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

			iSetColor(255, 255, 255);
			iText(690, 420, "PRESS 'R' TO PLAY AGAIN", GLUT_BITMAP_HELVETICA_18);
			iText(680, 380, "PRESS 'M' FOR MAIN MENU", GLUT_BITMAP_HELVETICA_18);
		}

		else{
			//show background image
			iShowImage(0, 0, 1600, 900, backgroundImages[bgIndex]);

			//show pause button
			iShowImage(1500, 820, 60, 60, pauseBtn);
			
			//show high score
			iSetColor(255, 255, 255);
			char scoreStr[20], hsStr[20];
			sprintf(scoreStr, "SCORE: %d", score);
			sprintf(hsStr, "HI-SCORE: %d", highScore);
			iText(50, 830, scoreStr, GLUT_BITMAP_TIMES_ROMAN_24);
			iText(50, 800, hsStr, GLUT_BITMAP_HELVETICA_12);

			//show coin
			char coinStr[20];
			sprintf(coinStr, "COINS: %d", sessionCoins);
			iSetColor(255, 215, 0); // Gold Color to make it stand out!
			iText(50, 770, coinStr, GLUT_BITMAP_HELVETICA_18);

			// check character jump or not
			if (j > 0){
				//drow character
				iShowImage(x, y, 160, 280, currentImage);
				j--;
			}
			else
			{
				// draw character running animation
				iShowImage(x, y, 160, 240, character[charIndex]);
			}

			//draw Obstacles
			drawObstacles();
			
			//draw coin
			if (gameCoin.active) {
				
				double scale = (1.0 - (gameCoin.y / 575.0)) * 40 + 20;

				iSetColor(255, 215, 0); 
				iFilledCircle(gameCoin.x + scale / 2, gameCoin.y + scale / 2, scale / 2);

				iSetColor(218, 165, 32); 
				iCircle(gameCoin.x + scale / 2, gameCoin.y + scale / 2, scale / 2);

				iSetColor(0, 0, 0); 
				iText(gameCoin.x + scale / 2 - 4, gameCoin.y + scale / 2 - 4, "$", GLUT_BITMAP_HELVETICA_12);
			}
		}
	}
}
int main()
{
	srand((unsigned int)time(NULL));

	loadHighScore();
	loadCoins();
	gameCoin.y = 575 ; // Start first coin a bit delayed
	gameCoin.lane = rand() % 4;
	gameCoin.active = true;

	iInitialize(1600, 900, "Cholo Run");

	//Background pic load code
	iSetTimer(41.67, changeBackground);
	for (int i = 0; i < 191; i++) {
		char filename[100];
		// i starts at 0, so i + 2 = 00002. 
		// When i is 190, i + 2 = 00192.
		sprintf_s(filename, sizeof(filename), "Photo/Background/bg%05d.bmp", i + 2);
		backgroundImages[i] = iLoadImage(filename);
	}

	//load main menu image
	background = iLoadImage("Photo/Bg for Main Menu/Background.png");
	woodPlank = iLoadImage("Photo/Logo/wood_plank.png");
	logoImg = iLoadImage("Photo/Logo/Logo.png");
	loadingImage = iLoadImage("Photo/Loading/Loading.png");
	shopBg = iLoadImage("Photo/Bg for Main Menu/Shop.png");
	themeBg = iLoadImage("Photo/Bg for Main Menu/Theme.png");
	aboutBg = iLoadImage("Photo/Bg for Main Menu/About_us.png");
	pauseBtn = iLoadImage("Photo/Pause Button/Pause_button.png");

	// load image for obstacle
	char filename[100];
	for (int i = 0; i < 8; i++)
	{
		if (i < 4) // indices 0, 1, 2, 3 -> bus
		{
			sprintf(filename, "Photo/Obstacle/Bus/bus%d.png", i + 1);
		}
		else if (i < 6) // indices 4, 5 -> truck
		{
			sprintf(filename, "Photo/Obstacle/Truck/truck%d.png", i - 3);
		}
		else // indices 6, 7 -> rickshaw
		{
			sprintf(filename, "Photo/Obstacle/Rickshaw/rickshaw%d.png", i - 5);
		}
		obstacle[i] = iLoadImage(filename);
	}
	obstacle[8] = iLoadImage("Photo/Obstacle/dog.png");          // indices -> dog
	obstacle[9] = iLoadImage("Photo/Obstacle/crack.png");        // indices -> creck
	obstacle[10] = iLoadImage("Photo/Obstacle/manhole.png");      // indices -> manhole

	//Initialize Obstacles
	for (int i = 0; i <3; i++) {
		obs[i].y = 575; // Spread them out
		obs[i].lane = getUniqueLane();
		obs[i].type = getUniqueobs();
		obs[i].active = true;
		obs[i].speed = 7;
	}

	//load character image
	character[0] = iLoadImage("Photo/Character/Male_Character_Left.png");
	character[1] = iLoadImage("Photo/Character/Male_Character_Right.png");
	currentImage = iLoadImage("Photo/Character/Male_Character_Jump.png");

	//  set timer
	iSetTimer(30, moveObstacles);
	iSetTimer(200, changeCharacter);
	iSetTimer(30, moveLoading);
	iSetTimer(20, fixedUpdate);
	
	//audio play
	PlaySound("Photo/Audio/theme song.wav", NULL, SND_LOOP | SND_ASYNC);

	iStart();
	return 0;
}



//some importent informatin about this game
/*
//variable for Obstacle pic
obstacle[0]= bus[0]
obstacle[1]= bus[1]
obstacle[2]= bus[2]
obstacle[3]=bus[3]
obstacle[4]=truck[0]
obstacle[5]=truck[1]
obstacle[6]=rickshaw[0]
obstacle[7]=rickshaw[1]
obstacle[8]=dog
obstacle[9]=crack
obstacle[10]=manhole

// polygon for road lane left to right
double l1x[4] = { 330, 570, 780,763  };
double l1y[4] = { 0, 0, 575, 575 };
double l2x[4] = { 570, 800, 798, 780 };
double l2y[4] = { 0, 0, 575, 575 };
double l3x[4] = { 800, 1030, 821, 798 };
double l3y[4] = { 0, 0, 575, 575 };
double l4x[4] = { 1030, 1270, 838, 821 };
double l4y[4] = { 0, 0, 575, 575};

//straightline equation for 5 white line
575x-433y-189750=0 equation for 1st white line
575x-210y-327750=0 equation for 2nd white line
575x+2y-460000=0   equation for 3rd white line
575x+209y-592250=0  equation for 4th white line
575x+432y-730250=0  equation for 5th white line
*/
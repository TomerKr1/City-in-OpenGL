// Full Credit! 
/*If a road is over the sea, a small bridge will be created.
Additionally, houses cannot be on a road.

After running the Erode process, the houses that already existed will be deleted, and new ones will be created in their place.
Some things you might not notice happening:

1. When there is a road, and the rain wets the ground around, the road becomes a bridge.

*/
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "glut.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <cstdlib> 
#include <ctime>  
#include <stack>
#include <windows.h>

void showAlert(const char* message) {
	MessageBox(NULL, message, "Alert", MB_OK | MB_ICONEXCLAMATION);
}


const int DROPLET_LIFETIME = 60;
const double EROSION_FACTOR = 0.01;
const double DEPOSITION_FACTOR = 0.001;
const double EVAPORATION_FACTOR = 0.98;

struct Droplet {
	double x, y;
	double volume;
	double sediment;
};
struct Raindrop {
	double x, y, z;
	double speed;
};


typedef struct {
	double x, y, z;
} POINT3;


const int TH = 256; // texture size. must be power of 2
const int TW = 256;
// matrix that stores a texture
unsigned char tx0[TH][TW][3] = { 0 };

bool isEroding = false;

const double PI = 3.14;
const int GSZ = 100;
const int H = 600;
const int W = 600;


int numberOfHousesTotal = 0;
bool terrainIsForming = true;
bool showRain = false;
double angle = 0;
double sight_angle = PI;
POINT3 direction = { sin(sight_angle),-0.3,cos(sight_angle) };
POINT3 eye = { 2,15,10 };
POINT3 sight_dir = { sin(sight_angle),0,cos(sight_angle) }; // in plane X-Z
double speed = 0;
double angular_speed = 0;
const int MaxHousesOnMap = 5000;
double ground[GSZ][GSZ] = { 0 };
double river[GSZ][GSZ] = { 0 };

int riverX[15];
int riverY[15];

int housesX[MaxHousesOnMap];
int housesY[MaxHousesOnMap];
int houseColor[MaxHousesOnMap];

double tmp[GSZ][GSZ];
bool errorMess = false;
const int MAP_SIZE = 100;
bool roadPositions[MAP_SIZE][MAP_SIZE] = { false };
bool isCaptured = false;
const int NUM_RAINDROPS = 5000;

Raindrop raindrops[NUM_RAINDROPS];

void initRain() {
	for (int i = 0; i < NUM_RAINDROPS; i++) {
		raindrops[i].x = (rand() % GSZ) - GSZ / 2.0;
		raindrops[i].y = 20 + (rand() % 10); // Start above the scene
		raindrops[i].z = (rand() % GSZ) - GSZ / 2.0;
		raindrops[i].speed = 0.1 + (rand() % 10) / 100.0; // Random speed
	}
}

void updateRain() {
	for (int i = 0; i < NUM_RAINDROPS; i++) {
		raindrops[i].y -= raindrops[i].speed;
		if (raindrops[i].y < -10) { // Reset raindrop if it's below the ground
			raindrops[i].y = 20 + (rand() % 10);
			raindrops[i].x = (rand() % GSZ) - GSZ / 2.0;
			raindrops[i].z = (rand() % GSZ) - GSZ / 2.0;
		}
	}
}

void drawRain() {

	glColor3f(0.7, 0.7, 1.0); // Light blue color for rain
	glBegin(GL_LINES);
	for (int i = 0; i < NUM_RAINDROPS; i++) {
		glVertex3d(raindrops[i].x, raindrops[i].y, raindrops[i].z);
		glVertex3d(raindrops[i].x, raindrops[i].y - 0.3, raindrops[i].z);
	}
	glEnd();
	
}
void DrawCloud(double x, double y, double z);
void UpdateTerrain2();
void UpdateTerrain3();
void Smooth();
void erode();
void DrawRoad();
void initRain();
void updateRain();
void drawRain();


// Function to generate a random double in the range [0, GSZ - 1)
double generateRandomDouble(int GSZ) {
	return static_cast<double>(rand()) / RAND_MAX * (GSZ - 1);
}

void DrawCloud(double x, double y, double z) {
	glColor3d(1.0, 1.0, 1.0); // Set color to white

	// Create a cloud using multiple spheres
	glPushMatrix();
	glTranslated(x, y, z);
	glutSolidIcosahedron();
	glutSolidSphere(1, 20, 20); // Central sphere

	glTranslated(-1.5, 0.5, 0);
	glutSolidSphere(0.8, 20, 20); // Left sphere

	glTranslated(3.0, 0, 0);
	glutSolidSphere(0.8, 20, 20); // Right sphere

	glTranslated(-1.5, 0.5, -1.0);
	glutSolidSphere(0.7, 20, 20); // Back sphere

	glTranslated(0, 0, 2.0);
	glutSolidSphere(0.7, 20, 20); // Front sphere

	glPopMatrix();
}
void SetTexture(int numTxture)
{
	int i, j;
	int rnd;
	switch (numTxture)
	{
	case 0: // bricks
		for (i = 0;i < TH;i++)
			for (j = 0;j < TW;j++)
			{// gray
				rnd = rand() % 30;
				if (i % (TH / 2) <= 15 ||
					(i < TH / 2 && j % (TW / 2) <= 15) ||// vertical gray line on bottom half
					(i >= TH / 2 && (j > 15 && j % (TW / 4) <= 15) && (j < TW / 2 - 15) ||
						i >= TH / 2 && (j % (TW / 4) <= 15) && (j > 3 * TW / 4 - 15))) // vertical gray lines on top half
				{
					tx0[i][j][0] = 190 + rnd;
					tx0[i][j][1] = 190 + rnd;
					tx0[i][j][2] = 190 + rnd;
				}
				else  // bricks
				{
					tx0[i][j][0] = 180 + rnd;
					tx0[i][j][1] = 70 + rnd;
					tx0[i][j][2] = 0 + rnd;
				}
			}
		break;
	case 1: // road
		for (i = 0;i < TH;i++)
			for (j = 0;j < TW;j++)
			{// white lines
				rnd = rand() % 30;
				if (i > TH - 15 || i < 15 ||
					i < TH / 2 && i >= TH / 2 - 15 && j < TW / 2)
				{
					tx0[i][j][0] = 255 - rnd;
					tx0[i][j][1] = 255 - rnd;
					tx0[i][j][2] = 255 - rnd;
				}
				else  // road
				{
					tx0[i][j][0] = 140 + rnd;
					tx0[i][j][1] = 140 + rnd;
					tx0[i][j][2] = 140 + rnd;
				}
			}
		break;
	}
}

double getHeight(int x, int y) {
	// Constrain x to be within [0, GSZ - 1]
	if (x < GSZ - 1) {
		if (x < 0) {
			x = 0;
		}
	}
	else {
		x = GSZ - 1; // Ensure x is not greater than GSZ - 1
	}
	// Constrain y to be within [0, GSZ - 1]
	if (y < GSZ - 1) {
		if (y < 0) {
			y = 0;
		}
	}
	else {
		y = GSZ - 1; // Ensure y is not greater than GSZ - 1
	}

	return ground[y][x];
}

void setHeight(int x, int y, double height) {
	// Constrain x to be within [0, GSZ - 1]
	if (x < GSZ - 1) {
		if (x < 0) {
			x = 0;
		}
	}
	else {
		x = GSZ - 1; // Ensure x is not greater than GSZ - 1
	}

	// Constrain y to be within [0, GSZ - 1]
	if (y < GSZ - 1) {
		if (y < 0) {
			y = 0;
		}

	}
	else {
		y = GSZ - 1; // Ensure y is not greater than GSZ - 1
	}


	ground[y][x] = height;
}

void RainDropAlgo() {
	Droplet droplet;
	droplet.x = generateRandomDouble(GSZ);
	droplet.y = generateRandomDouble(GSZ);
	droplet.volume = 1.0;
	droplet.sediment = 0.0;

	for (int lifetime = 0; lifetime < DROPLET_LIFETIME; lifetime++) {
		int cellX = static_cast<int>(droplet.x);
		int cellY = static_cast<int>(droplet.y);

		// Find the steepest downhill slope
		double lowheight = getHeight(cellX, cellY);
		int newX = cellX, newY = cellY;
		if (lowheight < 0)continue; // this fixed the problem with the water.
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (j <= 0 && i <= 0) continue;
				double height = getHeight(cellX + j, cellY + i);
				if (height < lowheight) {
					lowheight = height;
					newX = cellX + j;
					newY = cellY + i;
				}
			}
		}

		// flow down
		if (newX == cellX && newY == cellY) {
			setHeight(cellX, cellY, getHeight(cellX, cellY) + droplet.sediment);
			break;
		}
		double heightDiff = getHeight(cellX, cellY) - getHeight(newX, newY);

		// Erode
		double erosionAmount = EROSION_FACTOR * heightDiff * droplet.volume;
		droplet.sediment = erosionAmount;

		setHeight(cellX, cellY, getHeight(cellX, cellY) - erosionAmount);

		// Move the droplet
		droplet.x = newX;
		droplet.y = newY;

		double depositAmount = DEPOSITION_FACTOR * droplet.sediment;
		droplet.sediment -= depositAmount;
		setHeight(newX, newY, getHeight(newX, newY) + depositAmount);

		// Evaporate some water
		droplet.volume *= EVAPORATION_FACTOR;

		// Update river
		river[newY][newX] = (river[newY][newX] + droplet.volume * 0.0001 > 1.0)
			? 1.0
			: (river[newY][newX] + droplet.volume * 0.0001);

	}
	
}

void hydraulicErosion(int countOfDrop) {
	for (int i = 0; i < countOfDrop; i++) {
		RainDropAlgo();
	}
}

void init() {
	// Set the color of the window background
	glClearColor(0.5, 0.7, 1, 0);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Initialize the ground array to 0
	memset(ground, 0, sizeof(ground));

	// Update terrain
	for (int i = 0; i < 3000; i++)
		UpdateTerrain2();
	Smooth();
	for (int i = 0; i < 2000; i++)
		UpdateTerrain2();

	// Ensure ground is consistent along the middle row
	for (int j = 0; j < GSZ; j++) {
		ground[GSZ / 2 - 1][j] = ground[GSZ / 2 + 1][j] = ground[GSZ / 2][j];
	}

	// Initialize housesX, housesY, road, riverX, riverY, houseColor to -1
	for (int i = 0; i < MaxHousesOnMap; ++i) {
		housesX[i] = -1;
		housesY[i] = -1;
	
		houseColor[i] = -1;
	}


	memset(river, 0, sizeof(river));
	initRain();
	// Enable normalization
	glEnable(GL_NORMALIZE);

	// Set texture parameters
	SetTexture(1); // road
	glBindTexture(GL_TEXTURE_2D, 1); // texture number is 1
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TW, TH, 0, GL_RGB, GL_UNSIGNED_BYTE, tx0);


}

void setColor(int chosenColor) {

	switch (chosenColor) {
	case 1:
		glColor3d(1, 0.0, 0.0); //  Red
		break;
	case 2:
		glColor3d(0.05, 0.01, 1); // Blue
		break;
	case 3:
		glColor3d(0.32, 0.19, 0.44); // Purple
		break;
	case 4:
		glColor3d(0.07, 0.73, 0.86);
		break;
	case 5:
		glColor3d(0.6, 0.6, 0.0); // Dark Yellow
		break;
	}
}

// Function to draw a single polygon with given vertices
void drawPolygon(const double vertices[3][3], int chosenColor) {
	setColor(chosenColor);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 3; ++i) {
		glVertex3d(vertices[i][0], vertices[i][1], vertices[i][2]);
	}
	glEnd();
}

// Function to draw the roof
void DrawRoof(int numberOfStairs, int chosenColor) {
	numberOfStairs += 0.5;

	// Increase the height of the peak to make the roof more pointed
	double peakHeight = 3 + numberOfStairs; // Adjust this value as needed

	// Define the vertices for each of the four polygons with a higher peak
	const double vertices[4][3][3] = {
		{ {1 * numberOfStairs, 1 * numberOfStairs, 0}, {0, 1 * numberOfStairs, 0}, {0.5 * numberOfStairs, peakHeight, 0.5 * numberOfStairs} },
		{ {1 * numberOfStairs, 1 * numberOfStairs, 0}, {1 * numberOfStairs, 1 * numberOfStairs, 1 * numberOfStairs}, {0.5 * numberOfStairs, peakHeight, 0.5 * numberOfStairs} },
		{ {1 * numberOfStairs, 1 * numberOfStairs, 1 * numberOfStairs}, {0, 1 * numberOfStairs, 1 * numberOfStairs}, {0.5 * numberOfStairs, peakHeight, 0.5 * numberOfStairs} },
		{ {0, 1 * numberOfStairs, 1 * numberOfStairs}, {0, 1 * numberOfStairs, 0}, {0.5 * numberOfStairs, peakHeight, 0.5 * numberOfStairs} }
	};

	// Draw each polygon
	for (int i = 0; i < 4; ++i) {
		drawPolygon(vertices[i], chosenColor);
	}
}


void DrawBase(int numberOfFloors)
{
	// Define the colors for each wall
	double colors[4][3] = {
	  {0.96, 0.67, 0.4}, // Cream color
	  {0.96, 0.67, 0.4}, // Cream color
	  {0.96, 0.67, 0.4}, // Cream color
	  {0.96, 0.67, 0.4}  // Cream color
	};

	// Define the vertices for each wall
	double vertices[4][4][3] = {
		{ {0, 0, 0}, {1 * numberOfFloors, 0, 0},
		{1 * numberOfFloors, 1 * numberOfFloors, 0}, {0, 1 * numberOfFloors, 0} },
		{ {1 * numberOfFloors, 0, 0}, {1 * numberOfFloors,
		1 * numberOfFloors, 0}, {1 * numberOfFloors, 1 * numberOfFloors, 1 * numberOfFloors}, {1 * numberOfFloors, 0, 1 * numberOfFloors} },
		{ {1 * numberOfFloors, 1 * numberOfFloors, 1 * numberOfFloors}, {1 * numberOfFloors, 0, 1 * numberOfFloors}, {0, 0, 1 * numberOfFloors}, {0, 1 * numberOfFloors, 1 * numberOfFloors} },
		{ {0, 1 * numberOfFloors, 1 * numberOfFloors}, {0, 0, 1 * numberOfFloors}, {0, 0, 0}, {0, 1 * numberOfFloors, 0} }
	};

	// Draw each wall
	for (int i = 0; i < 4; ++i)
	{
		glBegin(GL_POLYGON);
		glColor3d(colors[i][0], colors[i][1], colors[i][2]);
		for (int j = 0; j < 4; ++j)
		{
			glVertex3d(vertices[i][j][0], vertices[i][j][1], vertices[i][j][2]);
		}
		glEnd();
	}
}


void DrawWindowOnWall(double startX, double endX, double wallPosition, double heightOfWindow, double offsetZ, bool shine)
{
	// Set color
	if (shine)
		glColor3d(1.0, 1.0, 0.0); // Yellow color for shining window
	else
		glColor3d(0, 0, 0); // Dark blue for normal window

	// Draw window on wall
	glBegin(GL_POLYGON);
	glVertex3d(startX, wallPosition, offsetZ);
	glVertex3d(endX, wallPosition, offsetZ);
	glVertex3d(endX, wallPosition + heightOfWindow, offsetZ);
	glVertex3d(startX, wallPosition + heightOfWindow, offsetZ);
	glEnd();
}

void DrawWindows(int numWindows, int numFloors)
{

	double height = 0.4, sizeWall = numFloors, sizeWindow = 0.8 / (numWindows + 1) + 0.5,
		Winspacing = (sizeWall - (numWindows * sizeWindow)) / (numWindows + 1), startXPosition = Winspacing;

	// Loop through each floor
	for (int floor = 0; floor < numFloors; floor++)
	{
		if (floor % 2 == 1)
		{
			// Loop through each window on the current floor
			for (int i = 0; i < numWindows; ++i)
			{
				double currentStartXPosition = startXPosition + i * (sizeWindow + Winspacing);
				double currentEndXPosition = currentStartXPosition + sizeWindow;

				// Determine if this window should shine
				bool shine = std::rand() % 5 == 0;  // 20% chance to shine

				// Draw windows

				DrawWindowOnWall(currentStartXPosition, currentEndXPosition, 0.3 + floor, height, sizeWall + 0.01, shine);
				DrawWindowOnWall(currentEndXPosition, currentStartXPosition, 0.3 + floor + height, height, -0.01, shine);

			}
		}
	}
}


void UpdateTerrain2()
{
	double delta = 0.04;
	if (rand() % 2 == 0)
		delta = -delta;
	int x1, y1, x2, y2;
	x1 = rand() % GSZ;
	y1 = rand() % GSZ;
	x2 = rand() % GSZ;
	y2 = rand() % GSZ;
	double a, b;
	if (x1 != x2)
	{
		a = (y2 - y1) / ((double)x2 - x1);
		b = y1 - a * x1;
		for (int i = 0;i < GSZ;i++)
			for (int j = 0;j < GSZ;j++)
			{
				if (i < a * j + b) ground[i][j] += delta;
				else ground[i][j] -= delta;
			}
	}

}

void UpdateTerrain3()
{
	double delta = 0.02;
	int x, z, count;
	int numPoints = 800;

	x = rand() % GSZ;
	z = rand() % GSZ;

	if (rand() % 2 == 0)
		delta = -delta;

	for (count = 1;count <= numPoints;count++)
	{
		ground[z][x] += delta;
		switch (rand() % 4)
		{
		case 0: // right
			x++;
			break;
		case 1: // up
			z++;
			break;
		case 2: // down
			z--;
			break;
		case 3: // left
			x--;
			break;
		}
		x += GSZ;
		x = x % GSZ;
		z += GSZ;
		z = z % GSZ;
	}


}

void Smooth()
{
	int i, j;
	for (i = 1;i < GSZ - 1;i++)
		for (j = 1;j < GSZ - 1;j++)
		{
			tmp[i][j] = (ground[i + 1][j - 1] + 2 * ground[i + 1][j] + ground[i + 1][j + 1] +
				2 * ground[i][j - 1] + 4 * ground[i][j] + 2 * ground[i][j + 1] +
				ground[i - 1][j - 1] + 2 * ground[i - 1][j] + ground[i - 1][j + 1]) / 16.0;
		}

	for (i = 1;i < GSZ - 1;i++)
		for (j = 1;j < GSZ - 1;j++)
			ground[i][j] = tmp[i][j];

}




void SetColor(double h)
{
	h = fabs(h) / 10.0;

	if (h < 0.03) // sand
		glColor3d(0.9, 0.8, 0.7);
	else if (h < 0.5)// grass
		glColor3d(0.2 + h / 3, 0.5 - h / 2, 0);
	else
		glColor3d(1.2 * h, 1.2 * h, 1.3 * h);


}


void SetColorRiver(double h)
{
	h /= 6; // Normalize h

	// Set color based on normalized height
	if (h < 0.0002)
	{
		glColor4d(0, 0.4, 0.7, 0.7);  // Dark blue
	}
	else if (h < 0.0006)
	{
		glColor4d(0.15, 0.5, 0.75, 0.7); // Medium blue
	}
	else
	{
		glColor4d(0.25, 0.6, 0.81, 0.7); // Light blue
	}
}


void DrawFloor()
{

	int i, j;

	for (i = 2;i < GSZ;i++)
		for (j = 2;j < GSZ;j++)
		{
			glBegin(GL_POLYGON);
			SetColor(ground[i][j]);
			glVertex3d(j - GSZ / 2, ground[i][j], i - GSZ / 2);
			SetColor(ground[i - 1][j]);
			glVertex3d(j - GSZ / 2, ground[i - 1][j], i - 1 - GSZ / 2);
			SetColor(ground[i - 1][j - 1]);
			glVertex3d(j - 1 - GSZ / 2, ground[i - 1][j - 1], i - 1 - GSZ / 2);
			SetColor(ground[i][j - 1]);
			glVertex3d(j - 1 - GSZ / 2, ground[i][j - 1], i - GSZ / 2);
			glEnd();
		}

	
	DrawRoad();


	glDisable(GL_BLEND);

}
void DrawRoadBetween(float x1, float y1, float x2, float y2) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1); // use texture number 1 - road
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE to count on color

	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0); glVertex3d(x1, 0.1, y1 - 0.5);
	glTexCoord2d(0, 2);glVertex3d(x2, 0.1, y2 - 0.5);
	glTexCoord2d(2, 2); glVertex3d(x2, 0.1, y2 + 0.5);
	glTexCoord2d(2, 0); glVertex3d(x1, 0.1, y1 + 0.5);
	glEnd();
}

void drawRiver()
{
	int i, j;

	for (i = 1; i < GSZ - 1; i++)
	{
		for (j = 1; j < GSZ - 1; j++)
		{
			{
				glBegin(GL_POLYGON);
				SetColorRiver(river[i][j]);
				glVertex3d(j - GSZ / 2, river[i][j], i - GSZ / 2);
				SetColorRiver(river[i - 1][j]);
				glVertex3d(j - GSZ / 2, river[i - 1][j], i - 1 - GSZ / 2);
				SetColorRiver(river[i - 1][j - 1]);
				glVertex3d(j - 1 - GSZ / 2, river[i - 1][j - 1], i - 1 - GSZ / 2);
				SetColorRiver(river[i][j - 1]);
				glVertex3d(j - 1 - GSZ / 2, river[i][j - 1], i - GSZ / 2);
				glEnd();

			}
		}
	}
}

void drawHouses()
{
	// Iterate over all possible house positions
	for (int i = 0; i < numberOfHousesTotal; i++)
	{
		int houseX = housesX[i];
		int houseY = housesY[i];

		// Validate house coordinates
		if (houseX > 0 && houseY > 0 && houseX < GSZ && houseY < GSZ)
		{
			// Set a random color or pattern for the house
			int random = i % 10;

			glPushMatrix();

			// Translate and scale the house
			glTranslated(houseY - GSZ / 2, ground[houseX][houseY], houseX - GSZ / 2);
			glScaled(0.1, 0.1, 0.05);

			// Draw house components
			DrawBase(random);
			DrawRoof(random, houseColor[i]);
			DrawWindows(random, random);

			glPopMatrix();
		}
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clean frame buffer and Z-buffer
	glViewport(0, 0, W, H);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // unity matrix of projection
	glFrustum(-1, 1, -1, 1, 0.7, 300);
	gluLookAt(eye.x, eye.y, eye.z, eye.x + direction.x, eye.y + direction.y, eye.z + direction.z, 0, 1, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // unity matrix of model
	srand(time(0));
	for (int i = 0; i < 200; ++i) {
		double x = static_cast<double>(rand() % GSZ - GSZ / 2);
		double y = static_cast<double>(rand() % 50 + 20); //  Clouds height range
		double z = static_cast<double>(rand() % GSZ - GSZ / 2);
		DrawCloud(x, y, z);
		
	}
	if (showRain) {
		drawRain();
	}
	DrawFloor();
	drawRiver();
	drawHouses();
	glutSwapBuffers();
}

void idle()
{
	/*if (terrainIsForming)
			UpdateTerrain3();*/
	int i, j;
	double dist;
	angle += 0.1;
	sight_angle += angular_speed; // if angular speed >0 we turn left, oterwise we turn right
	sight_angle += angular_speed; // if angular speed >0 we turn left, oterwise we turn right
	direction.x = sin(sight_angle);
	direction.z = cos(sight_angle);

	eye.x += speed * direction.x;
	eye.z += speed * direction.z;
	if (isEroding)
	{
		// Initialize housesX, housesY, road, riverX, riverY, houseColor to -1
		for (int i = 0; i < numberOfHousesTotal; ++i) {
			housesX[i] = -1;
			housesY[i] = -1;
			houseColor[i] = -1;
			drawHouses();
		}
		numberOfHousesTotal = 0;
		erode();
	}
	updateRain();
	glutPostRedisplay();
}

void erode() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	hydraulicErosion(1000);  // Simulate droplets
	DrawFloor();
	drawRiver();
	glutSwapBuffers();
	DrawRoad();
}


void DrawRoad2()
{
	int j;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1); // use texture number 1 - road
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // GL_MODULATE to count on color


	for (j = 1;j < GSZ;j++)
	{
		if (ground[GSZ / 2][j - 1] > 0)
		{
			glBegin(GL_POLYGON);
			glTexCoord2d(0, 0);	glVertex3d(j - GSZ / 2 - 1, ground[GSZ / 2][j - 1] + 0.2, 1); // 1
			glTexCoord2d(0, 1);	 glVertex3d(j - GSZ / 2 - 1, ground[GSZ / 2][j - 1] + 0.2, -1); //2
			glTexCoord2d(1, 1);	  glVertex3d(j - GSZ / 2, ground[GSZ / 2][j] + 0.2, -1);  //3
			glTexCoord2d(1, 0);	  glVertex3d(j - GSZ / 2, ground[GSZ / 2][j] + 0.2, 1);  // 4
			glEnd();
		}

	}
	for (j = 1; j < GSZ; j++)
	{
		if (ground[j - 1][GSZ / 2] > 0)
		{
			glBegin(GL_POLYGON);
			glTexCoord2d(0, 0); glVertex3d(-1, ground[j - 1][GSZ / 2] + 0.2, j - GSZ / 2 - 1); // 1
			glTexCoord2d(0, 1); glVertex3d(1, ground[j - 1][GSZ / 2] + 0.2, j - GSZ / 2 - 1); //2
			glTexCoord2d(1, 1); glVertex3d(1, ground[j][GSZ / 2] + 0.2, j - GSZ / 2);  //3
			glTexCoord2d(1, 0); glVertex3d(-1, ground[j][GSZ / 2] + 0.2, j - GSZ / 2);  // 4
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);
}
void DrawRoadv2()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1); // use texture number 1 - road
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	
	// 
	// Function to check if a road segment is valid (not crossing large water bodies)
	auto isValidRoadSegment = [](int x1, int y1, int x2, int y2) {
		int steps = std::max(abs(x2 - x1), abs(y2 - y1));
		int waterCount = 0;
		for (int i = 0; i <= steps; i++) {
			int x = x1 + (x2 - x1) * i / steps;
			int y = y1 + (y2 - y1) * i / steps;
			if (ground[y][x] <= 0) {
				waterCount++;
				if (waterCount > steps / 1) // Allow small water crossings
					return false;
			}
			else {
				waterCount = 0;
			}
		}
		return true;
	};

	// Draw main roads
	if (isValidRoadSegment(0, GSZ / 2, GSZ - 1, GSZ / 2)) {
		for (int j = 1; j < GSZ; j++) {
			if (ground[GSZ / 2][j - 1] > 0) {
				glBegin(GL_POLYGON);
				glTexCoord2d(0, 0); glVertex3d(j - GSZ / 2 - 1, ground[GSZ / 2][j - 1] + 0.2, 1);
				glTexCoord2d(0, 1); glVertex3d(j - GSZ / 2 - 1, ground[GSZ / 2][j - 1] + 0.2, -1);
				glTexCoord2d(1, 1); glVertex3d(j - GSZ / 2, ground[GSZ / 2][j] + 0.2, -1);
				glTexCoord2d(1, 0); glVertex3d(j - GSZ / 2, ground[GSZ / 2][j] + 0.2, 1);
				glEnd();
			}
		}
	}

	if (isValidRoadSegment(GSZ / 2, 0, GSZ / 2, GSZ - 1)) {
		for (int j = 1; j < GSZ; j++) {
			if (ground[j - 1][GSZ / 2] > 0) {
				glBegin(GL_POLYGON);
				glTexCoord2d(0, 0); glVertex3d(-1, ground[j - 1][GSZ / 2] + 0.2, j - GSZ / 2 - 1);
				glTexCoord2d(0, 1); glVertex3d(1, ground[j - 1][GSZ / 2] + 0.2, j - GSZ / 2 - 1);
				glTexCoord2d(1, 1); glVertex3d(1, ground[j][GSZ / 2] + 0.2, j - GSZ / 2);
				glTexCoord2d(1, 0); glVertex3d(-1, ground[j][GSZ / 2] + 0.2, j - GSZ / 2);
				glEnd();
			}
		}
	}
	int numAdditionalRoads = 2; // You can adjust this number
	for (int i = 1; i <= numAdditionalRoads; i++) {
		int pos = (i * GSZ) / (numAdditionalRoads + 1);

		// Horizontal additional road
		if (isValidRoadSegment(0, pos, GSZ - 1, pos)) {
			for (int j = 1; j < GSZ; j++) {
				if (ground[pos][j - 1] > 0) {
					glBegin(GL_POLYGON);
					glTexCoord2d(0, 0); glVertex3d(j - GSZ / 2 - 1, ground[pos][j - 1] + 0.2, pos - GSZ / 2 + 0.5);
					glTexCoord2d(0, 1); glVertex3d(j - GSZ / 2 - 1, ground[pos][j - 1] + 0.2, pos - GSZ / 2 - 0.5);
					glTexCoord2d(1, 1); glVertex3d(j - GSZ / 2, ground[pos][j] + 0.2, pos - GSZ / 2 - 0.5);
					glTexCoord2d(1, 0); glVertex3d(j - GSZ / 2, ground[pos][j] + 0.2, pos - GSZ / 2 + 0.5);
					glEnd();
				}
			}
		}

		// Vertical additional road
		if (isValidRoadSegment(pos, 0, pos, GSZ - 1)) {
			for (int j = 1; j < GSZ; j++) {
				if (ground[j - 1][pos] > 0) {
					glBegin(GL_POLYGON);
					glTexCoord2d(0, 0); glVertex3d(pos - GSZ / 2 - 0.5, ground[j - 1][pos] + 0.2, j - GSZ / 2 - 1);
					glTexCoord2d(0, 1); glVertex3d(pos - GSZ / 2 + 0.5, ground[j - 1][pos] + 0.2, j - GSZ / 2 - 1);
					glTexCoord2d(1, 1); glVertex3d(pos - GSZ / 2 + 0.5, ground[j][pos] + 0.2, j - GSZ / 2);
					glTexCoord2d(1, 0); glVertex3d(pos - GSZ / 2 - 0.5, ground[j][pos] + 0.2, j - GSZ / 2);
					glEnd();
				}
			}
		}
	}
	glDisable(GL_TEXTURE_2D);
}
void DrawRoadV3()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	auto isValidRoadSegment = [](int x1, int y1, int x2, int y2) -> bool {
		int steps = std::max(abs(x2 - x1), abs(y2 - y1));
		int waterCount = 0;
		for (int i = 0; i <= steps; i++) {
			int x = x1 + (x2 - x1) * i / steps;
			int y = y1 + (y2 - y1) * i / steps;
			if (x < 0 || x >= GSZ || y < 0 || y >= GSZ)
				return false;
			if (ground[y][x] <= 0) {
				waterCount++;
				if (waterCount > steps / 1) // Allow small water crossings
					return false;
			}
			else {
				waterCount = 0;
			}
		}
		return true;
	};

	auto drawRoadSegment = [](int x1, int y1, int x2, int y2, double height1, double height2, bool isBridge) {
		double bridgeHeight = std::max(0.5, std::max(height1, height2) + 0.5);

		glBegin(GL_POLYGON);
		glTexCoord2d(0, 0); glVertex3d(x1 - GSZ / 2.0, isBridge ? bridgeHeight : height1 + 0.2, y1 - GSZ / 2.0 + 0.5);
		glTexCoord2d(0, 1); glVertex3d(x1 - GSZ / 2.0, isBridge ? bridgeHeight : height1 + 0.2, y1 - GSZ / 2.0 - 0.5);
		glTexCoord2d(1, 1); glVertex3d(x2 - GSZ / 2.0, isBridge ? bridgeHeight : height2 + 0.2, y2 - GSZ / 2.0 - 0.5);
		glTexCoord2d(1, 0); glVertex3d(x2 - GSZ / 2.0, isBridge ? bridgeHeight : height2 + 0.2, y2 - GSZ / 2.0 + 0.5);
		glEnd();

		if (isBridge) {
			// Draw bridge supports
			glDisable(GL_TEXTURE_2D);
			glColor3f(0.5, 0.5, 0.5); // Gray color for bridge supports

			// Function to draw a single support
			auto drawSupport = [](double x, double y, double height) {
				glBegin(GL_QUADS);
				glVertex3d(x - 0.1, 0, y + 0.6);
				glVertex3d(x + 0.1, 0, y + 0.6);
				glVertex3d(x + 0.1, height, y + 0.6);
				glVertex3d(x - 0.1, height, y + 0.6);

				glVertex3d(x - 0.1, 0, y - 0.6);
				glVertex3d(x + 0.1, 0, y - 0.6);
				glVertex3d(x + 0.1, height, y - 0.6);
				glVertex3d(x - 0.1, height, y - 0.6);
				glEnd();
			};

			// Draw supports at both ends of the bridge
			drawSupport(x1 - GSZ / 2.0, y1 - GSZ / 2.0, bridgeHeight);
			drawSupport(x2 - GSZ / 2.0, y2 - GSZ / 2.0, bridgeHeight);

			glEnable(GL_TEXTURE_2D);
		}
	};

	// Draw main roads
	for (int j = 1; j < GSZ; j++) {
		bool isBridge = ground[GSZ / 2][j - 1] <= 0 || ground[GSZ / 2][j] <= 0;
		drawRoadSegment(j - 1, GSZ / 2, j, GSZ / 2,
			std::max(0.0, ground[GSZ / 2][j - 1]),
			std::max(0.0, ground[GSZ / 2][j]),
			isBridge);
	}

	

	// Draw additional direct roads
	int numAdditionalRoads = 2;
	for (int i = 1; i <= numAdditionalRoads; i++) {
		int pos = (i * GSZ) / (numAdditionalRoads + 1);

		// Horizontal additional road
		if (isValidRoadSegment(0, pos, GSZ - 1, pos)) {
			for (int j = 1; j < GSZ; j++) {
				bool isBridge = ground[pos][j - 1] <= 0 || ground[pos][j] <= 0;
			//	bool isBridge = false;
				drawRoadSegment(j - 1, pos, j, pos,
					std::max(0.0, ground[pos][j - 1]),
					std::max(0.0, ground[pos][j]),
					isBridge);
			}
		}
	}

	glDisable(GL_TEXTURE_2D);
}

bool isValidRoadSegment(int x1, int y1, int x2, int y2) {
	int steps = std::max(abs(x2 - x1), abs(y2 - y1));
	int waterCount = 0;
	for (int i = 0; i <= steps; i++) {
		int x = x1 + (x2 - x1) * i / steps;
		int y = y1 + (y2 - y1) * i / steps;
		if (x < 0 || x >= GSZ || y < 0 || y >= GSZ)
			return false;
		if (ground[y][x] <= 0) {
			waterCount++;
			if (waterCount > steps / 3) // Allow small water crossings
				return false;
		}
		else {
			waterCount = 0;
		}
	}
	return true;
}

void drawSupport(double x, double y, double height) {
	glBegin(GL_QUADS);
	glVertex3d(x - 0.1, 0, y + 0.5);
	glVertex3d(x + 0.1, 0, y + 0.5);
	glVertex3d(x + 0.1, height, y + 0.5);
	glVertex3d(x - 0.1, height, y + 0.5);

	glVertex3d(x - 0.1, 0, y - 0.5);
	glVertex3d(x + 0.1, 0, y - 0.5);
	glVertex3d(x + 0.1, height, y - 0.5);
	glVertex3d(x - 0.1, height, y - 0.5);
	glEnd();
}

void drawRoadSegment(int x1, int y1, int x2, int y2, double height1, double height2, bool isBridge) {
	double bridgeHeight = std::max(0.5, std::max(height1, height2) + 0.5);

	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0); glVertex3d(x1 - GSZ / 2.0, isBridge ? bridgeHeight : height1 + 0.2, y1 - GSZ / 2.0 + 0.5);
	glTexCoord2d(0, 1); glVertex3d(x1 - GSZ / 2.0, isBridge ? bridgeHeight : height1 + 0.2, y1 - GSZ / 2.0 - 0.5);
	glTexCoord2d(1, 1); glVertex3d(x2 - GSZ / 2.0, isBridge ? bridgeHeight : height2 + 0.2, y2 - GSZ / 2.0 - 0.5);
	glTexCoord2d(1, 0); glVertex3d(x2 - GSZ / 2.0, isBridge ? bridgeHeight : height2 + 0.2, y2 - GSZ / 2.0 + 0.5);
	glEnd();

	if (isBridge) {
		// Draw bridge supports
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.5, 0.5, 0.5); // Gray color for bridge supports

		// Draw supports at both ends of the bridge
		drawSupport(x1 - GSZ / 2.0, y1 - GSZ / 2.0, bridgeHeight);
		drawSupport(x2 - GSZ / 2.0, y2 - GSZ / 2.0, bridgeHeight);

		glEnable(GL_TEXTURE_2D);
	}
}

void DrawRoad() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Draw main roads
	for (int j = 1; j < GSZ; j++) {
		bool isBridge = ground[GSZ / 2][j - 1] <= 0 || ground[GSZ / 2][j] <= 0;
		drawRoadSegment(j - 1, GSZ / 2, j, GSZ / 2,
			std::max(0.0, ground[GSZ / 2][j - 1]),
			std::max(0.0, ground[GSZ / 2][j]),
			isBridge);
	}

	// Draw additional direct roads
	int numAdditionalRoads = 2;
	for (int i = 1; i <= numAdditionalRoads; i++) {
		int pos = (i * GSZ) / (numAdditionalRoads + 1);

		// Horizontal additional road
		if (isValidRoadSegment(0, pos, GSZ - 1, pos)) {
			for (int j = 1; j < GSZ; j++) {
				bool isBridge = ground[pos][j - 1] <= 0 || ground[pos][j] <= 0;
				drawRoadSegment(j - 1, pos, j, pos,
					std::max(0.0, ground[pos][j - 1]),
					std::max(0.0, ground[pos][j]),
					isBridge);
			}
		}
	}
	
	glDisable(GL_TEXTURE_2D);
}
bool AlreadyExistHouse(int x, int z)
{
	for (int i = 0; i < numberOfHousesTotal; ++i)
	{
		if (housesX[i] == x && housesY[i] == z)
			return true;
	}
	return false;
}



bool isNearWater(int row, int col, int size, int minDistance, int maxDistance)
{
	bool nearSea = false;
	bool nearRiver = false;

	for (int i = row - maxDistance; i < row + size + maxDistance; i++)
	{
		for (int j = col - maxDistance; j < col + size + maxDistance; j++)
		{
			if (i < 0 || i >= GSZ || j < 0 || j >= GSZ)
				continue;

			int distance = std::max(std::abs(i - row), std::abs(j - col));

			// Check for sea
			if (ground[i][j] <= 0)
			{
				if (distance >= minDistance && distance <= maxDistance)
					nearSea = true;
				else if (distance < minDistance) {
					return false; // Too close to sea
				}
			}

			// Check for river
			if (river[i][j] > 0 && river[i][j] >= ground[i][j] + 0.0001)
			{
				if (distance >= minDistance && distance <= maxDistance)
					nearRiver = true;
				else if (distance < minDistance)
					return false; // Too close to river
			}
		}
	}

	return nearSea && nearRiver;
}
bool isTooCloseToRoad(int i, int j)
{

	int numAdditionalRoads = 2; // Make sure this matches the number in DrawRoad

	// Check main roads
	if (abs(i - GSZ / 2) <= 2 || abs(j - GSZ / 2) <= 2)
		return true;

//	 Check additional roads
	for (int k = 1; k <= numAdditionalRoads; k++) {
		int pos = (k * GSZ) / (numAdditionalRoads + 1);

		// Check horizontal additional road
		if (abs(i - pos) <= 2)
			return true;

	//	 Check vertical additional road
		if (abs(j - pos) <= 2)
			return true;
	}

	return false;
}
void BuildCity(int minDistance, int maxDistance)
{
	int maxHousesForNeig =25; // You can change the maximum number of houses "in the neighborhood"
	int counter = 0;
	int numberOfHouses = 0;
	int houseColors = 1;

	if (numberOfHousesTotal == MaxHousesOnMap)return;
	
	for (int i = 0; i < GSZ ; i++)
	{
		for (int j = 0; j < GSZ ; j++)
		{

			//bool isTooCloseToRoad = (abs(i - GSZ / 2) <= 2) || (abs(j - GSZ / 2) <= 2);
			if (!AlreadyExistHouse(i, j) && isNearWater(i, j, 2, minDistance, maxDistance) && !isTooCloseToRoad(i, j))
			{
				if (numberOfHousesTotal == MaxHousesOnMap)return;
				for (int x = i; x < i + 2; x++)
				{
					for (int z = j; z < j + 2; z++)
					{
						if (!AlreadyExistHouse(x, z))
						{
							if (ground[x][z] <= 0)break;
							if (counter == maxHousesForNeig)break;
							housesX[numberOfHouses] = x;
							housesY[numberOfHouses] = z;
							houseColor[numberOfHouses] = rand() % 5 + 1;
							numberOfHouses++;
							numberOfHousesTotal++;
						}
					}
				}
			}
		}

	}
	if (numberOfHousesTotal == 0)
	{
		if (!errorMess)	printf("Error Finding Land. \n\n");
		errorMess = true;
	}
	drawHouses();
}



bool isValidPoint(int x, int y) {
	return x >= 0 && x < GSZ&& y >= 0 && y < GSZ;
}

void menu(int choice)
{
	switch (choice)
	{
	case 1:
		if (!isEroding)
		{
			printf("Old Houses will be deleted \n\n");
			showAlert("Old Houses will be deleted, please Wait Couple of second for the proccess.");
			isEroding = true; // Start erosion
			showRain = !showRain;
		}
		else
		{
			isEroding = false; // Stop erosion
			showRain = !showRain;
		}
		break;
	case 2:

		BuildCity(3, 5);
		printf("Currenley Houses on the map %d, Max Houses: %d \n", numberOfHousesTotal, MaxHousesOnMap);
		break;

	case 3:
		showRain = !showRain;
		break;
	}
}
void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		angular_speed += 0.001;
		break;
	case GLUT_KEY_RIGHT:
		angular_speed -= 0.001;
		break;

	case GLUT_KEY_UP:
		speed += 0.01;
		break;
	case GLUT_KEY_DOWN:
		speed -= 0.01;
		break;

	case GLUT_KEY_PAGE_UP:
		eye.y += 0.1;
		break;
	case GLUT_KEY_PAGE_DOWN:
		eye.y -= 0.1;
		break;
	}
}


void main(int argc, char* argv[])
{
	showAlert("If you received terrain without mountains, please refresh");
	srand(static_cast<unsigned int>(time(0)));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(400, 100);
	glutCreateWindow("Project");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutSpecialFunc(SpecialKeys);
	glutCreateMenu(menu);
	glutAddMenuEntry("Hydraulic Errosion On/Off", 1);
	glutAddMenuEntry("Build City", 2);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
	init();
	glutMainLoop();

}
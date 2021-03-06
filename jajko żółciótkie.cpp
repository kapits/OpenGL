// jajko żółciótkie.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <windows.h>
#include <gl/gl.h>
#include <glut.h>
#include <cmath>
#include <iostream>

//chyba można po prostu użyć math.h tutaj
#define M_PI 3.14159265358979323846

static GLfloat thetaY = 0.0;   // kąt obrotu obiektu
static GLfloat thetaX = 0.0;

static GLfloat pix2angle;     // przelicznik pikseli na stopnie

static GLint status = 0;       // stan klawiszy myszy 
							   // 0 - nie naciśnięto żadnego klawisza
							   // 1 - naciśnięty zostać lewy klawisz

struct Point {
	float x, y, z;
};
static Point** colors;
static Point** normals;

float getRandom() {
	return float(rand() % 1000) / 1000;
}

Point getBezierCurve(float u, float v) {
	float x, y, z;

	x = (-90 * pow(u, 5) + 225 * pow(u, 4) - 270 * pow(u, 3) + 180 * pow(u, 2) - 45 * u) * cos(M_PI * v);
	y = 160 * pow(u, 4) - 320 * pow(u, 3) + 160 * pow(u, 2);
	z = (-90 * pow(u, 5) + 225 * pow(u, 4) - 270 * pow(u, 3) + 180 * pow(u, 2) - 45 * u) * sin(M_PI * v);

	return { x, y, z }; //coordinates
}

Point getPhongVectors(float u, float v) {
	float x, y, z, x_u, x_v, y_u, y_v, z_u, z_v;

	x_u = (-450 * pow(u, 4) + 900 * pow(u, 3) - 810 * pow(u, 2) + 360 * u - 45) * cos(M_PI * v);
	x_v = M_PI * (90 * pow(u, 5) - 225 * pow(u, 4) + 270 * pow(u, 3) - 180 * pow(u, 2) + 45 * u) * sin(M_PI * v);
	y_u = 640 * pow(u, 3) - 960 * pow(u, 2) + 320 * u;
	y_v = 0;
	z_u = (-450 * pow(u, 4) + 900 * pow(u, 3) - 810 * pow(u, 2) + 360 * u - 45) * sin(M_PI * v);
	z_v = (-M_PI) * (90 * pow(u, 5) - 225 * pow(u, 4) + 270 * pow(u, 3) - 180 * pow(u, 2) + 45 * u) * cos(M_PI * v);

	x = y_u * z_v - z_u * y_v;
	y = z_u * x_v - x_u * z_v;
	z = x_u * y_v - y_u * x_v;

	float length = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

	x = x / length;
	y = y / length;
	z = z / length;

	return { x, y, z };
}

Point** getBezierValues(int n) {
	float interval = 1.0f / n;
	Point** bezierValues = new Point*[n + 1];

	for (size_t i = 0; i < n + 1; i++) {
		bezierValues[i] = new Point[n + 1];
	}

	for (size_t i = 0; i <= n; i++) {
		for (size_t j = 0; j <= n; j++) {
			float u = ((float)i) / n;
			float v = ((float)j) / n;

			bezierValues[i][j] = getBezierCurve(u, v);
		}
	}

	return bezierValues;
}

Point** getColors(int n) {
	Point** colors = new Point*[n + 1];

	for (size_t i = 0; i < n + 1; i++) {
		colors[i] = new Point[n + 1];
	}

	for (size_t i = 0; i <= n; i++) {
		for (size_t j = 0; j <= n; j++) {
			colors[i][j] = { getRandom(), getRandom(), getRandom() };
		}
	}

	for (size_t i = 0; i < n / 2; i++) {
		colors[i][n] = colors[n - i][0];
		colors[i][0] = colors[n - i][n];
	}

	return colors;
}

Point** getNormals(int n) {
	float interval = 1.0f / n;
	Point** vectorValues = new Point*[n + 1];

	for (size_t i = 0; i < n + 1; i++) {
		vectorValues[i] = new Point[n + 1];
	}

	for (size_t i = 0; i <= n; i++) {
		for (size_t j = 0; j <= n; j++) {
			float u = ((float)i) / n;
			float v = ((float)j) / n;

			vectorValues[i][j] = getPhongVectors(u, v);

			if (i < n / 2) {
				vectorValues[i][j].x = (-1) * vectorValues[i][j].x;
				vectorValues[i][j].y = (-1) * vectorValues[i][j].y;
				vectorValues[i][j].z = (-1) * vectorValues[i][j].z;
			}
		}
	}

	return vectorValues;
}

static int x_pos_old = 0;       // poprzednia pozycja kursora myszy
static int y_pos_old = 0;
static int z_pos_old = 1;
static int righty = 0;

static int delta_x = 0;        // różnica pomiędzy pozycją bieżącą
									  // i poprzednią kursora myszy 
static int delta_y = 0;
static float delta_z = 1.0;

float r = 10.0;

typedef float point3[3];
static GLfloat viewer[] = { 0.0, 0.0, 10.0 };
using namespace std;

void Mouse(int btn, int state, int x, int y) {
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		x_pos_old = x;         // przypisanie aktualnie odczytanej pozycji kursora 
		y_pos_old = y;			// jako pozycji poprzedniej

		status = 1;          // wcięnięty został lewy klawisz myszy
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		y_pos_old = y;
		status = 2;
	}
	else {
		status = 0;
	}
}


void Motion(GLsizei x, GLsizei y) {

	delta_x = x - x_pos_old;     // obliczenie różnicy położenia kursora myszy
	delta_y = y - y_pos_old;

	x_pos_old = x;            // podstawienie bieżącego położenia jako poprzednie
	y_pos_old = y;

	glutPostRedisplay();     // przerysowanie obrazu sceny
}

void Axes(void) {
	point3  x_min = { -5.0, 0.0, 0.0 };
	point3  x_max = { 5.0, 0.0, 0.0 };
	point3  y_min = { 0.0, -5.0, 0.0 };
	point3  y_max = { 0.0, 5.0, 0.0 };
	point3  z_min = { 0.0, 0.0, -5.0 };
	point3  z_max = { 0.0, 0.0, 5.0 };
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3fv(x_min);
	glVertex3fv(x_max);
	glEnd();

	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3fv(y_min);
	glVertex3fv(y_max);
	glEnd();

	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3fv(z_min);
	glVertex3fv(z_max);
	glEnd();
}

float getX(float u, float v) {
	float PIV = M_PI * v;
	return ((-90 * pow(u, 5) + 225 * pow(u, 4) - 270 * pow(u, 3) + 180 * pow(u, 2) - 45 * u) * cos(PIV));
}
float getY(float u, float v) {
	return (160 * pow(u, 4) - 320 * pow(u, 3) + 160 * pow(u, 2));
}
float getZ(float u, float v) {
	float PIV = M_PI * v;
	return ((-90 * pow(u, 5) + 225 * pow(u, 4) - 270 * pow(u, 3) + 180 * pow(u, 2) - 45 * u) * sin(PIV));
}

struct Punkt {
	float x;
	float y;
	float z;
	float xRGB;
	float yRGB;
	float zRGB;
};

//1 - punkty
//2 - siatka
//3 - wypełnione
int model = 3;
int movement = 1;
float KAT = 90.0;

//ilość podzialow boku kwadratu jednostkowego
const int N = 50;

struct Punkt eggPoints[N][N];

float krok = 1.0 / N;
float theta = 0.0;
float fi = 0.0;

void drawEggFromPoints(int n, Point** bezierValues) {
	glBegin(GL_POINTS);

	for (size_t i = 0; i <= n; i++) {
		for (size_t j = 0; j <= n; j++) {
			glVertex3f(bezierValues[i][j].x, bezierValues[i][j].y - 5, bezierValues[i][j].z);
		}
	}

	glEnd();
}


void drawEggFromLines(int n, Point** bezierValues, Point** normals) {
	glBegin(GL_LINES);

	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			glVertex3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
			glVertex3f(bezierValues[i][j].x, bezierValues[i][j].y - 5, bezierValues[i][j].z);
			glVertex3f(normals[i + 1][j].x, normals[i + 1][j].y, normals[i + 1][j].z);
			glVertex3f(bezierValues[i + 1][j].x, bezierValues[i + 1][j].y - 5, bezierValues[i + 1][j].z);
			glVertex3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
			glVertex3f(bezierValues[i][j].x, bezierValues[i][j].y - 5, bezierValues[i][j].z);
			glVertex3f(normals[i][j + 1].x, normals[i][j + 1].y, normals[i][j + 1].z);
			glVertex3f(bezierValues[i][j + 1].x, bezierValues[i][j + 1].y - 5, bezierValues[i][j + 1].z);
			glVertex3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
			glVertex3f(bezierValues[i][j].x, bezierValues[i][j].y - 5, bezierValues[i][j].z);
			glVertex3f(normals[i + 1][j + 1].x, normals[i + 1][j + 1].y, normals[i + 1][j + 1].z);
			glVertex3f(bezierValues[i + 1][j + 1].x, bezierValues[i + 1][j + 1].y - 5, bezierValues[i + 1][j + 1].z);
		}
	}

	glEnd();
}

void drawEggFromTriangles(int n, Point** bezierValues, Point** normals) {
	glBegin(GL_TRIANGLES);

	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
			glVertex3f(bezierValues[i][j].x, bezierValues[i][j].y - 5, bezierValues[i][j].z);
			glNormal3f(normals[i + 1][j].x, normals[i + 1][j].y, normals[i + 1][j].z);
			glVertex3f(bezierValues[i + 1][j].x, bezierValues[i + 1][j].y - 5, bezierValues[i + 1][j].z);
			glNormal3f(normals[i + 1][j + 1].x, normals[i + 1][j + 1].y, normals[i + 1][j + 1].z);
			glVertex3f(bezierValues[i + 1][j + 1].x, bezierValues[i + 1][j + 1].y - 5, bezierValues[i + 1][j + 1].z);

			glNormal3f(normals[i][j + 1].x, normals[i][j + 1].y, normals[i][j + 1].z);
			glVertex3f(bezierValues[i][j + 1].x, bezierValues[i][j + 1].y - 5, bezierValues[i][j + 1].z);
			glNormal3f(normals[i][j].x, normals[i][j].y, normals[i][j].z);
			glVertex3f(bezierValues[i][j].x, bezierValues[i][j].y - 5, bezierValues[i][j].z);
			glNormal3f(normals[i + 1][j + 1].x, normals[i + 1][j + 1].y, normals[i + 1][j + 1].z);
			glVertex3f(bezierValues[i + 1][j + 1].x, bezierValues[i + 1][j + 1].y - 5, bezierValues[i + 1][j + 1].z);
		}
	}

	glEnd();
}


void drawEgg(int n) {
	Point** bezierValues = getBezierValues(n);
	Point** normals = getNormals(n);

	//glRotatef(theta[0], 1.0, 0.0, 0.0);
	//glRotatef(theta[1], 0.0, 1.0, 0.0);
	//glRotatef(theta[2], 0.0, 0.0, 1.0);


	drawEggFromTriangles(n, bezierValues, normals);

}





void EggPtCount() {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {

			//wyliczanie odpowienich koordynat punktów jajka
			eggPoints[i][j].x = getX(i * krok, j * krok);
			eggPoints[i][j].y = getY(i * krok, j * krok);
			eggPoints[i][j].z = getZ(i * krok, j * krok);

			//losowanie kolorów
			eggPoints[i][j].xRGB = ((double)rand() / (RAND_MAX));
			eggPoints[i][j].yRGB = ((double)rand() / (RAND_MAX));
			eggPoints[i][j].zRGB = ((double)rand() / (RAND_MAX));
		}
	}
}

void Egg() {
	switch (model) {

		//punkty
		case 1:
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					glBegin(GL_POINTS);
					glColor3f(1.0f, 0.0f, 0.0f);
					glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
					glEnd();
				}
			}
			break;

			//linie
		case 2:
			glBegin(GL_LINES);
			glColor3f(1.0f, 1.0f, 1.0f);

			//poziome
			for (int i = 0; i < N; i++) {
				int pom = N - i;
				for (int j = 0; j < N - 1; j++) {
					glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
					glVertex3f(eggPoints[i][j + 1].x, eggPoints[i][j + 1].y - 5, eggPoints[i][j + 1].z);
				}
				if (pom != N) {
					glVertex3f(eggPoints[i][0].x, eggPoints[i][0].y - 5, eggPoints[i][0].z);
					glVertex3f(eggPoints[pom][N - 1].x, eggPoints[pom][N - 1].y - 5, eggPoints[pom][N - 1].z);
				}
			}

			//pionowe
			glColor3f(1.0f, 1.0f, 0.0f);
			for (int i = 0; i < N; i++) {
				int pom = N - i;
				for (int j = 0; j < N; j++) {
					if (i == N - 1) {
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glVertex3f(eggPoints[0][0].x, eggPoints[0][0].y - 5, eggPoints[0][0].z);
					}
					else {
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glVertex3f(eggPoints[i + 1][j].x, eggPoints[i + 1][j].y - 5, eggPoints[i + 1][j].z);
					}
				}
			}

			//na skos
			glColor3f(0.3f, 0.0f, 0.40f);
			for (int i = N / 2; i < N; i++) {
				int pom = N - i;
				for (int j = 0; j < N - 1; j++) {
					if (i == N - 1) {
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glVertex3f(eggPoints[0][0].x, eggPoints[0][0].y - 5, eggPoints[0][0].z);
					}
					else {
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glVertex3f(eggPoints[i + 1][j + 1].x, eggPoints[i + 1][j + 1].y - 5, eggPoints[i + 1][j + 1].z);
					}
					if (pom != N) {
						glVertex3f(eggPoints[i][0].x, eggPoints[i][0].y - 5, eggPoints[i][0].z);
						glVertex3f(eggPoints[pom + 1][N - 1].x, eggPoints[pom + 1][N - 1].y - 5, eggPoints[pom + 1][N - 1].z);
					}
				}
			}
			glVertex3f(eggPoints[0][0].x, eggPoints[0][0].y - 5, eggPoints[0][0].z);
			glVertex3f(eggPoints[1][N - 1].x, eggPoints[1][N - 1].y - 5, eggPoints[1][N - 1].z);

			glVertex3f(eggPoints[0][0].x, eggPoints[0][0].y - 5, eggPoints[0][0].z);
			glVertex3f(eggPoints[N - 1][N - 1].x, eggPoints[N - 1][N - 1].y - 5, eggPoints[N - 1][N - 1].z);
			for (int i = 1; i < N / 2 + 1; i++) {
				int pom = N - i;
				for (int j = 0; j < N - 1; j++) {
					glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
					glVertex3f(eggPoints[i - 1][j + 1].x, eggPoints[i - 1][j + 1].y - 5, eggPoints[i - 1][j + 1].z);
					if (pom != N) {
						glVertex3f(eggPoints[i][0].x, eggPoints[i][0].y - 5, eggPoints[i][0].z);
						glVertex3f(eggPoints[pom - 1][N - 1].x, eggPoints[pom - 1][N - 1].y - 5, eggPoints[pom - 1][N - 1].z);
					}
				}
			}

			glEnd();
			break;

			//kolor
		case 3:
			glBegin(GL_TRIANGLES);
			for (int i = N / 2; i < N; i++) {
				int pom = N - i;
				for (int j = 0; j < N - 1; j++) {
					if (i == N - 1) {
						glColor3f(eggPoints[i][j].xRGB, eggPoints[i][j].yRGB, eggPoints[i][j].zRGB);
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glColor3f(eggPoints[i][j + 1].xRGB, eggPoints[i][j + 1].yRGB, eggPoints[i][j + 1].zRGB);
						glVertex3f(eggPoints[i][j + 1].x, eggPoints[i][j + 1].y - 5, eggPoints[i][j + 1].z);
						glColor3f(eggPoints[0][0].xRGB, eggPoints[0][0].yRGB, eggPoints[0][0].zRGB);
						glVertex3f(eggPoints[0][0].x, eggPoints[0][0].y - 5, eggPoints[0][0].z);
					}
					else {
						glColor3f(eggPoints[i][j].xRGB, eggPoints[i][j].yRGB, eggPoints[i][j].zRGB);
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glColor3f(eggPoints[i + 1][j].xRGB, eggPoints[i + 1][j].yRGB, eggPoints[i + 1][j].zRGB);
						glVertex3f(eggPoints[i + 1][j].x, eggPoints[i + 1][j].y - 5, eggPoints[i + 1][j].z);
						glColor3f(eggPoints[i + 1][j + 1].xRGB, eggPoints[i + 1][j + 1].yRGB, eggPoints[i + 1][j + 1].zRGB);
						glVertex3f(eggPoints[i + 1][j + 1].x, eggPoints[i + 1][j + 1].y - 5, eggPoints[i + 1][j + 1].z);

						glColor3f(eggPoints[i][j].xRGB, eggPoints[i][j].yRGB, eggPoints[i][j].zRGB);
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glColor3f(eggPoints[i][j + 1].xRGB, eggPoints[i][j + 1].yRGB, eggPoints[i][j + 1].zRGB);
						glVertex3f(eggPoints[i][j + 1].x, eggPoints[i][j + 1].y - 5, eggPoints[i][j + 1].z);
						glColor3f(eggPoints[i + 1][j + 1].xRGB, eggPoints[i + 1][j + 1].yRGB, eggPoints[i + 1][j + 1].zRGB);
						glVertex3f(eggPoints[i + 1][j + 1].x, eggPoints[i + 1][j + 1].y - 5, eggPoints[i + 1][j + 1].z);
					}
					if (pom != N) {
						glColor3f(eggPoints[i][0].xRGB, eggPoints[i][0].yRGB, eggPoints[i][0].zRGB);
						glVertex3f(eggPoints[i][0].x, eggPoints[i][0].y - 5, eggPoints[i][0].z);
						glColor3f(eggPoints[pom][N - 1].xRGB, eggPoints[pom][N - 1].yRGB, eggPoints[pom][N - 1].zRGB);
						glVertex3f(eggPoints[pom][N - 1].x, eggPoints[pom][N - 1].y - 5, eggPoints[pom][N - 1].z);
						glColor3f(eggPoints[pom + 1][N - 1].xRGB, eggPoints[pom + 1][N - 1].yRGB, eggPoints[pom + 1][N - 1].zRGB);
						glVertex3f(eggPoints[pom + 1][N - 1].x, eggPoints[pom + 1][N - 1].y - 5, eggPoints[pom + 1][N - 1].z);

						if (i != N / 2) {
							glColor3f(eggPoints[i][0].xRGB, eggPoints[i][0].yRGB, eggPoints[i][0].zRGB);
							glVertex3f(eggPoints[i][0].x, eggPoints[i][0].y - 5, eggPoints[i][0].z);
							glColor3f(eggPoints[i - 1][0].xRGB, eggPoints[i - 1][0].yRGB, eggPoints[i - 1][0].zRGB);
							glVertex3f(eggPoints[i - 1][0].x, eggPoints[i - 1][0].y - 5, eggPoints[i - 1][0].z);
							glColor3f(eggPoints[pom + 1][N - 1].xRGB, eggPoints[pom + 1][N - 1].yRGB, eggPoints[pom + 1][N - 1].zRGB);
							glVertex3f(eggPoints[pom + 1][N - 1].x, eggPoints[pom + 1][N - 1].y - 5, eggPoints[pom + 1][N - 1].z);
						}
					}
				}
			}
			glColor3f(eggPoints[0][0].xRGB, eggPoints[0][0].yRGB, eggPoints[0][0].zRGB);
			glVertex3f(eggPoints[0][0].x, eggPoints[0][0].y - 5, eggPoints[0][0].z);
			glColor3f(eggPoints[N - 1][0].xRGB, eggPoints[N - 1][0].yRGB, eggPoints[N - 1][0].zRGB);
			glVertex3f(eggPoints[N - 1][0].x, eggPoints[N - 1][0].y - 5, eggPoints[N - 1][0].z);
			glColor3f(eggPoints[1][N - 1].xRGB, eggPoints[1][N - 1].yRGB, eggPoints[1][N - 1].zRGB);
			glVertex3f(eggPoints[1][N - 1].x, eggPoints[1][N - 1].y - 5, eggPoints[1][N - 1].z);


			glColor3f(eggPoints[0][0].xRGB, eggPoints[0][0].yRGB, eggPoints[0][0].zRGB);
			glVertex3f(eggPoints[0][0].x, eggPoints[0][0].y - 5, eggPoints[0][0].z);
			glColor3f(eggPoints[1][0].xRGB, eggPoints[1][0].yRGB, eggPoints[1][0].zRGB);
			glVertex3f(eggPoints[1][0].x, eggPoints[1][0].y - 5, eggPoints[1][0].z);
			glColor3f(eggPoints[N - 1][N - 1].xRGB, eggPoints[N - 1][N - 1].yRGB, eggPoints[N - 1][N - 1].zRGB);
			glVertex3f(eggPoints[N - 1][N - 1].x, eggPoints[N - 1][N - 1].y - 5, eggPoints[N - 1][N - 1].z);


			for (int i = 0; i < N / 2 + 1; i++) {
				int pom = N - i;
				for (int j = 0; j < N - 1; j++) {
					if (i == 1) {
						glColor3f(eggPoints[i][j].xRGB, eggPoints[i][j].yRGB, eggPoints[i][j].zRGB);
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glColor3f(eggPoints[i][j + 1].xRGB, eggPoints[i][j + 1].yRGB, eggPoints[i][j + 1].zRGB);
						glVertex3f(eggPoints[i][j + 1].x, eggPoints[i][j + 1].y - 5, eggPoints[i][j + 1].z);
						glColor3f(eggPoints[0][0].xRGB, eggPoints[0][0].yRGB, eggPoints[0][0].zRGB);
						glVertex3f(eggPoints[0][0].x, eggPoints[0][0].y - 5, eggPoints[0][0].z);
					}
					else {
						if (i != 0) {
							glColor3f(eggPoints[i][j].xRGB, eggPoints[i][j].yRGB, eggPoints[i][j].zRGB);
							glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
							glColor3f(eggPoints[i - 1][j].xRGB, eggPoints[i - 1][j].yRGB, eggPoints[i - 1][j].zRGB);
							glVertex3f(eggPoints[i - 1][j].x, eggPoints[i - 1][j].y - 5, eggPoints[i - 1][j].z);
							glColor3f(eggPoints[i - 1][j + 1].xRGB, eggPoints[i - 1][j + 1].yRGB, eggPoints[i - 1][j + 1].zRGB);
							glVertex3f(eggPoints[i - 1][j + 1].x, eggPoints[i - 1][j + 1].y - 5, eggPoints[i - 1][j + 1].z);
						}


						glColor3f(eggPoints[i][j].xRGB, eggPoints[i][j].yRGB, eggPoints[i][j].zRGB);
						glVertex3f(eggPoints[i][j].x, eggPoints[i][j].y - 5, eggPoints[i][j].z);
						glColor3f(eggPoints[i][j + 1].xRGB, eggPoints[i][j + 1].yRGB, eggPoints[i][j + 1].zRGB);
						glVertex3f(eggPoints[i][j + 1].x, eggPoints[i][j + 1].y - 5, eggPoints[i][j + 1].z);
						glColor3f(eggPoints[i - 1][j + 1].xRGB, eggPoints[i - 1][j + 1].yRGB, eggPoints[i - 1][j + 1].zRGB);
						glVertex3f(eggPoints[i - 1][j + 1].x, eggPoints[i - 1][j + 1].y - 5, eggPoints[i - 1][j + 1].z);
					}
					if (pom != N) {
						glColor3f(eggPoints[i][0].xRGB, eggPoints[i][0].yRGB, eggPoints[i][0].zRGB);
						glVertex3f(eggPoints[i][0].x, eggPoints[i][0].y - 5, eggPoints[i][0].z);
						glColor3f(eggPoints[pom][N - 1].xRGB, eggPoints[pom][N - 1].yRGB, eggPoints[pom][N - 1].zRGB);
						glVertex3f(eggPoints[pom][N - 1].x, eggPoints[pom][N - 1].y - 5, eggPoints[pom][N - 1].z);
						glColor3f(eggPoints[pom - 1][N - 1].xRGB, eggPoints[pom - 1][N - 1].yRGB, eggPoints[pom - 1][N - 1].zRGB);
						glVertex3f(eggPoints[pom - 1][N - 1].x, eggPoints[pom - 1][N - 1].y - 5, eggPoints[pom - 1][N - 1].z);


						glColor3f(eggPoints[i][0].xRGB, eggPoints[i][0].yRGB, eggPoints[i][0].zRGB);
						glVertex3f(eggPoints[i][0].x, eggPoints[i][0].y - 5, eggPoints[i][0].z);
						glColor3f(eggPoints[i + 1][0].xRGB, eggPoints[i + 1][0].yRGB, eggPoints[i + 1][0].zRGB);
						glVertex3f(eggPoints[i + 1][0].x, eggPoints[i + 1][0].y - 5, eggPoints[i + 1][0].z);
						glColor3f(eggPoints[pom - 1][N - 1].xRGB, eggPoints[pom - 1][N - 1].yRGB, eggPoints[pom - 1][N - 1].zRGB);
						glVertex3f(eggPoints[pom - 1][N - 1].x, eggPoints[pom - 1][N - 1].y - 5, eggPoints[pom - 1][N - 1].z);
					}
				}
			}
			glEnd();
			break;
	}

}

void Sierpinski(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLint level) {
	if (level > 0) {
		Sierpinski(x, y + width / 2, z, width / 2, level - 1);
		Sierpinski(x - width / 4, y, z + width / 4, width / 2, level - 1);
		Sierpinski(x + width / 4, y, z + width / 4, width / 2, level - 1);
		Sierpinski(x - width / 4, y, z - width / 4, width / 2, level - 1);
		Sierpinski(x + width / 4, y, z - width / 4, width / 2, level - 1);
	}
	else {
		glBegin(GL_TRIANGLES);
		glColor3f(0.72f, 0.53f, 0.04f);

		//glColor3f(((rand() % 100)*0.01), ((rand() % 100)*0.01), ((rand() % 100)*0.01));
		//face 1
		glVertex3f(x - width / 2, y, z + width / 2);
		glVertex3f(x + width / 2, y, z + width / 2);
		glVertex3f(x, y + width, z);

		glColor3f(0.8f, 0.68f, 0.0f);
		//glColor3f(((rand() % 100)*0.01), ((rand() % 100)*0.01), ((rand() % 100)*0.01));
		//face 2
		glVertex3f(x - width / 2, y, z - width / 2);
		glVertex3f(x + width / 2, y, z - width / 2);
		glVertex3f(x, y + width, z);

		glColor3f(0.85f, 0.85f, 0.1f);
		//glColor3f(((rand() % 100)*0.01), ((rand() % 100)*0.01), ((rand() % 100)*0.01));
		//face 3
		glVertex3f(x + width / 2, y, z + width / 2);
		glVertex3f(x + width / 2, y, z - width / 2);
		glVertex3f(x, y + width, z);

		glColor3f(0.88f, 0.85f, 0.45f);
		//glColor3f(((rand() % 100)*0.01), ((rand() % 100)*0.01), ((rand() % 100)*0.01));
		//face 4
		glVertex3f(x - width / 2, y, z - width / 2);
		glVertex3f(x - width / 2, y, z + width / 2);
		glVertex3f(x, y + width, z);

		glColor3f(0.93f, 0.60f, 0.0f);
		//glColor3f(((rand() % 100)*0.01), ((rand() % 100)*0.01), ((rand() % 100)*0.01));
		glVertex3f(x - width / 2, y, z - width / 2);
		glVertex3f(x + width / 2, y, z - width / 2);
		glVertex3f(x - width / 2, y, z + width / 2);


		glVertex3f(x + width / 2, y, z - width / 2);
		glVertex3f(x - width / 2, y, z + width / 2);
		glVertex3f(x + width / 2, y, z + width / 2);
		glEnd();
	}
}

// angle of rotation for the camera direction
float angle = 0.0;
// actual vector representing the camera's direction
float lx = 0.0f, lz = -1.0f;
// XZ position of the camera
float x2 = 0.0f, z2 = 5.0f;

static float centerX = 0.0, centerY = 0.0, centerZ = 0.0;

static bool zoombarrier = true;
static int modelek = 1;

void RenderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(viewer[0], viewer[1], viewer[2], centerX, centerY, centerZ, 0.0, 1.0, 0.0);
	//gluLookAt(x2, 1.0f, z2, x2 + lx, 1.0f, z2 + lz, 0.0f, 1.0f, 0.0f);
	Axes();
	viewer[0] = r * cos(theta)*cos(fi);
	viewer[1] = r * sin(fi);
	viewer[2] = r * sin(theta)*cos(fi);

	if (status == 1) {
		//thetaY += delta_x * pix2angle;    // modyfikacja kąta obrotu o kat proporcjonalny
		//thetaX += delta_y * pix2angle;

		theta += delta_x * 0.05 * pix2angle;
		fi += delta_y * 0.05 * pix2angle;

		GLfloat light_position1[] = { viewer[2], viewer[1], viewer[0], 1.0 };
		glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
	}
	else if (status == 2) {
		//delta_z += delta_y * 0.01;
		if (zoombarrier == true) {
			if (r < 7) {
				r = 7;
			}
			else {
				r += delta_y * 0.01;
			}
		}
		else {
			r += delta_y * 0.01;
		}

	}
	// do różnicy położeń kursora myszy

//glRotatef(thetaY, 0.0, 1.0, 0.0);  //obrót obiektu o nowy kąt
//glRotatef(thetaX, 1.0, 0.0, 0.0);  // do różnicy położeń kursora myszy
//glScalef(delta_z, delta_z, delta_z);

//zakomentować niepotrzebne:
	if (modelek == 1) {
		drawEgg(N);
		zoombarrier = true;
	}
	else {
		Sierpinski(0.0, -3.0, 0.0, 8, 4);
		zoombarrier = false;
	}

	//koordynaty początkowe x y z, szerokość, poziom

	//glColor3f(1.0f, 1.0f, 1.0f);
	// Ustawienie koloru rysowania na biały

	//glutWireTeapot(3.0);
	// Narysowanie czajnika

	glFlush();
	glutSwapBuffers();
}


//sterowanie wyswietlanym modelem
void keys(unsigned char key, int x, int y) {
	if (key == '1') model = 1;
	if (key == '2') model = 2;
	if (key == '3') model = 3;
	if (key == '4') {
		if (modelek == 1) modelek = 2;
		else modelek = 1;
	}
	if (key == 'w') {
		centerY += 0.1;
	}
	if (key == 's') {
		centerY -= 0.1;
	}
	if (key == 'a') {
		centerZ += 0.1;
	}
	if (key == 'd') {
		centerZ -= 0.1;
	}
	RenderScene();
}

void processSpecialKeys(int key, int xx, int yy) {

	float fraction = 0.1f;

	switch (key) {
		case GLUT_KEY_LEFT:
			angle -= 0.01f;
			lx = sin(angle);
			lz = -cos(angle);
			break;
		case GLUT_KEY_RIGHT:
			angle += 0.01f;
			lx = sin(angle);
			lz = -cos(angle);
			break;
		case GLUT_KEY_UP:
			x2 += lx * fraction;
			z2 += lz * fraction;
			break;
		case GLUT_KEY_DOWN:
			x2 -= lx * fraction;
			z2 -= lz * fraction;
			break;
	}
}

void MyInit(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Kolor czyszc¹cy (wype³nienia okna) ustawiono na czarny

	/*************************************************************************************/

  //  Definicja materia³u z jakiego zrobiony jest czajnik 
  //  i definicja Ÿród³a œwiat³a

  /*************************************************************************************/

	GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	// wspó³czynniki ka =[kar,kag,kab] dla œwiat³a otoczenia

	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	// wspó³czynniki kd =[kdr,kdg,kdb] œwiat³a rozproszonego

	GLfloat mat_specular[] = { 0.7, 0.7, 0.7, 1.0 };
	// wspó³czynniki ks =[ksr,ksg,ksb] dla œwiat³a odbitego                

	GLfloat mat_shininess = { 50.0 };
	// wspó³czynnik n opisuj¹cy po³ysk powierzchni


	/*************************************************************************************/
	// Definicja Ÿród³a œwiat³a

	GLfloat light_position1[] = { 4.0, 4.0, 0.0, 1.0 };
	
	// po³o¿enie Ÿród³a

	GLfloat light_ambient[] = { 0.1,  0.1,  0.1, 0.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a otoczenia
	// Ia = [Iar,Iag,Iab]

	//GLfloat light_diffuse[] = { 1.0, 0.0, 0.0, 1.0 };
	const GLfloat COLOR_YELLOW[] = { 1.0, 1.0, 0.0, 1.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a powoduj¹cego
	// odbicie dyfuzyjne Id = [Idr,Idg,Idb]

	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a powoduj¹cego
	// odbicie kierunkowe Is = [Isr,Isg,Isb]

	GLfloat att_constant = { 1.0 };
	// sk³adowa sta³a ds dla modelu zmian oœwietlenia w funkcji 
	// odleg³oœci od Ÿród³a

	GLfloat att_linear = { 0.05 };
	// sk³adowa liniowa dl dla modelu zmian oœwietlenia w funkcji 
	// odleg³oœci od Ÿród³a

	GLfloat att_quadratic = { 0.001 };
	// sk³adowa kwadratowa dq dla modelu zmian oœwietlenia w funkcji
	// odleg³oœci od Ÿród³a

	/*************************************************************************************/

	// Ustawienie parametrów materia³u i Ÿród³a œwiat³a

	/*************************************************************************************/
	// Ustawienie patrametrów materia³u


	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);


	/*************************************************************************************/
	// Ustawienie parametrów Ÿród³a

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, COLOR_YELLOW);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position1);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);



	/*************************************************************************************/
	// Ustawienie opcji systemu oœwietlania sceny 

	glShadeModel(GL_SMOOTH); // w³aczenie ³agodnego cieniowania
	glEnable(GL_LIGHTING);   // w³aczenie systemu oœwietlenia sceny 
	glEnable(GL_LIGHT0);     // w³¹czenie Ÿród³a o numerze 0
	glEnable(GL_LIGHT1);     // w³¹czenie Ÿród³a o numerze 0
	glEnable(GL_DEPTH_TEST); // w³¹czenie mechanizmu z-bufora 

	/*************************************************************************************/

}

/*************************************************************************************/

// Funkcja ma za zadanie utrzymanie sta³ych proporcji rysowanych
// w przypadku zmiany rozmiarów okna.
// Parametry vertical i horizontal (wysokoœæ i szerokoœæ okna) s¹
// przekazywane do funkcji za ka¿dym razem gdy zmieni siê rozmiar okna.


void ChangeSize(GLsizei horizontal, GLsizei vertical) {

	pix2angle = 360.0 / (float)horizontal;  // przeliczenie pikseli na stopnie

	GLfloat AspectRatio;
	if (vertical == 0)
		vertical = 1;
	glViewport(0, 0, horizontal, vertical);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(70, 1.0, 1.0, 30.0);

	AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;
	if (horizontal <= vertical)
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

	else
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void main(void) {
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	EggPtCount();
	glutCreateWindow("Ćwiczenie 4");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);

	MyInit();
	glEnable(GL_DEPTH_TEST);
	glutKeyboardFunc(keys);
	glutMouseFunc(Mouse);
	// Ustala funkcję zwrotną odpowiedzialną za badanie stanu myszy

	glutMotionFunc(Motion);
	//glutKeyboardFunc(processNormalKeys);
	//glutSpecialFunc(processSpecialKeys);
	// Ustala funkcję zwrotną odpowiedzialną za badanie ruchu myszy

	glutMainLoop();
}
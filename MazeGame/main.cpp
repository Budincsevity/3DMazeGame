/*  The Maze Game
*
*
*	Created by Budincsevity Norbert, 2014
*/


#pragma comment (lib,"glut32.lib")
#pragma comment (lib,"glaux.lib")

#include "glaux.h"
#include "glut.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include "MilkshapeModel.h"		

const int sMax=6, m=40, mm=m+1, direction_parts=36;
int cur_direction=0;
int glWin, pathLen, myLen = 0;

const int move[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
const int move_key[4] = { GLUT_KEY_UP, GLUT_KEY_LEFT, GLUT_KEY_DOWN, GLUT_KEY_RIGHT };

unsigned textureId = -1, texFloor = -1;

double distance=4.;

float	lightAmb[] = { 0.03, 0.03, 0.03 };
float	lightDif[] = { 0.95, 0.95, 0.95 };
float	lightPos[] = { (int)m / 2, 7, (int)m / 2 };

GLfloat density = 0.3;
GLfloat translate = 0;
GLfloat fogColor[4] = { 0.5, 0.5, 0.5, 1.0 };
GLfloat	yrot = 0.0f;													// Y Rotation

AUX_RGBImageRec *localTexture = NULL, *localFloor = NULL;

char data[m + 2][m + 2], cp[m + 2][m + 2];

struct Tpos
{char x,y;};

struct Player
{
    int x,y,z;
    int dx,dz;
    bool isGo;
	int positionX;
	int positionZ;
	int lastPressed;
}player,ex, botOne, botTwo, botThree;

struct Maps
{
	int x, z;
} map;

void genMap(int, int);
void display(void);
void halt(bool f=false);
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HDC			hDC = NULL;												// Private GDI Device Context
HGLRC		hRC = NULL;												// Permanent Rendering Context
HWND		hWnd = NULL;												// Holds Our Window Handle
HINSTANCE	hInstance;												// Holds The Instance Of The Application

Model *pModel = NULL;												// Holds The Model Data
Model *pBotOne = NULL;
Model *pBotTwo = NULL;
Model *pBotThree = NULL;

bool	keysArrayinit[256];											// Array Used For The Keyboard Routine
bool	active = TRUE;												// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;											// Fullscreen Flag Set To Fullscreen Mode By Default

AUX_RGBImageRec *LoadBMP(const char *Filename)						// Loads A Bitmap Image
{
	FILE *File = NULL;												// File Handle

	if (!Filename)													// Make Sure A Filename Was Given
	{
		return NULL;												// If Not Return NULL
	}

	File = fopen(Filename, "r");										// Check To See If The File Exists

	if (File)														// Does The File Exist?
	{
		fclose(File);												// Close The Handle
		return auxDIBImageLoadA(Filename);							// Load The Bitmap And Return A Pointer
	}

	return NULL;													// If Load Failed Return NULL
}

GLuint LoadGLTexture(const char *filename)						// Load Bitmaps And Convert To Textures
{
	AUX_RGBImageRec *pImage;										// Create Storage Space For The Texture
	GLuint texture = 0;												// Texture ID

	pImage = LoadBMP(filename);									// Loads The Bitmap Specified By filename

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if (pImage != NULL && pImage->data != NULL)					// If Texture Image Exists
	{
		glGenTextures(1, &texture);									// Create The Texture

		// Typical Texture Generation Using Data From The Bitmap
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, pImage->sizeX, pImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, pImage->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		free(pImage->data);											// Free The Texture Image Memory
		free(pImage);												// Free The Image Structure
	}

	return texture;													// Return The Status
}

void drawFloor(GLfloat x1, GLfloat x2, GLfloat z1, GLfloat z2, unsigned texture=texFloor)
{
    glBindTexture ( GL_TEXTURE_2D, texture );
    glBegin(GL_POLYGON);
        glNormal3f( 0.0, 1.0, 0.0);
        glTexCoord2f(0,0);
        glVertex3f( x1, 0, z2 );
        glTexCoord2f(1,0);
        glVertex3f( x2, 0, z2 );
        glTexCoord2f(1,1);
        glVertex3f( x2, 0, z1 );
        glTexCoord2f(0,1);
        glVertex3f( x1, 0, z1 );
    glEnd(); 
}

void drawBox (GLint j, GLint i, unsigned texture=textureId)
{
    GLfloat x1=i, x2=i+1, y1=0, y2=1, z1=j, z2=j+1;
    glBindTexture ( GL_TEXTURE_2D, texture );
    

    glBegin(GL_POLYGON); // Back
        glNormal3f( 0.0, 0.0, -1.0);
        glTexCoord2f(0,0);
        glVertex3f( x2, y1, z1 );
        glTexCoord2f(1,0);
        glVertex3f( x1, y1, z1 );
        glTexCoord2f(1,1);
        glVertex3f( x1, y2, z1 );
        glTexCoord2f(0,1);
        glVertex3f( x2, y2, z1 );
    glEnd();

    glBegin(GL_POLYGON); // Front
        glNormal3f( 0.0, 0.0, 1.0);
        glTexCoord2f(0,0);
        glVertex3f( x1, y1, z2 );
        glTexCoord2f(1,0);
        glVertex3f( x2, y1, z2 );
        glTexCoord2f(1,1);
        glVertex3f( x2, y2, z2 );
        glTexCoord2f(0,1);
        glVertex3f( x1, y2, z2 );
    glEnd();

    glBegin(GL_POLYGON); // Left
        glNormal3f( -1.0, 0.0, 0.0);
        glTexCoord2f(0,0);
        glVertex3f( x1, y1, z1 );
        glTexCoord2f(1,0);
        glVertex3f( x1, y1, z2 );
        glTexCoord2f(1,1);
        glVertex3f( x1, y2, z2 );
        glTexCoord2f(0,1);
        glVertex3f( x1, y2, z1 );
    glEnd();

    glBegin(GL_POLYGON); // Right
        glNormal3f( 1.0, 0.0, 0.0);
        glTexCoord2f(0,0);
        glVertex3f( x2, y1, z2 );
        glTexCoord2f(1,0);
        glVertex3f( x2, y1, z1 );
        glTexCoord2f(1,1);
        glVertex3f( x2, y2, z1 );
        glTexCoord2f(0,1);
        glVertex3f( x2, y2, z2 );
    glEnd();

    glBegin(GL_POLYGON); // Top
        glNormal3f( 0.0, 1.0, 0.0);
        glTexCoord2f(0,0);
        glVertex3f( x1, y2, z2 );
        glTexCoord2f(1,0);
        glVertex3f( x2, y2, z2 );
        glTexCoord2f(1,1);
        glVertex3f( x2, y2, z1 );
        glTexCoord2f(0,1);
        glVertex3f( x1, y2, z1 );
    glEnd();
}

void drawGamer(int angle, int x, int z)
{
	glPushMatrix();
		glTranslatef(player.x + (1.0*player.dx / sMax) + 0.5f, player.y + 0.5f, player.z + (1.0*player.dz / sMax) + 0.5f);
		glRotatef(angle, 0, 1, 0);
		glScalef(0.005, 0.005, 0.005);
		pModel->draw();
		glColor3d(1, 1, 1);
	glPopMatrix();
	player.positionZ = z;
	player.positionX = x;
}

void animate()
{
        if (player.dx>0)	player.dx+=1; else
	    if (player.dz>0)	player.dz+=1; else
        if (player.dx<0)	player.dx-=1; else
        if (player.dz<0)	player.dz-=1;
        if ((player.dx>=sMax)||(player.dz>=sMax))
        {
            player.isGo=false;
            if (player.dx>0)	player.x+=1;
            if (player.dz>0)	player.z+=1;
            player.dx=0; player.dz=0;
        }else
        if ((player.dx<=-sMax)||(player.dz<=-sMax))
        {
            player.isGo=false;
            if (player.dx<0)	player.x-=1;
            if (player.dz<0)	player.z-=1;
            player.dx=0; player.dz=0;
        }
 
    glutPostRedisplay();
}

void init()
{
	pModel->reloadTextures();
	pBotOne->reloadTextures();
	pBotTwo->reloadTextures();
	pBotThree->reloadTextures();

    glClearColor( 0.5, 0.5, 0.5, 0.5 );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, density);
	glHint(GL_FOG_HINT, GL_NICEST);
    glEnable( GL_CULL_FACE );
	glDepthFunc(GL_LEQUAL);											// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);				// Really Nice Perspective Calculations
    glPixelStorei ( GL_PACK_ALIGNMENT, 1 );
    glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );
    glShadeModel (GL_SMOOTH);
    glLightfv    ( GL_LIGHT0, GL_AMBIENT,  lightAmb );
    glLightfv    ( GL_LIGHT0, GL_DIFFUSE,  lightDif );
    glEnable ( GL_LIGHT0 );
    glEnable ( GL_LIGHTING );
	glEnable(GL_COLOR_MATERIAL);
	player.dx = 0; player.dz = 0; player.isGo = false; player.lastPressed = 0;
}

void display()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt(player.x+(1.0*player.dx/sMax)+0.5f+3*cos(M_PI_2+cur_direction/double(direction_parts)*2.*M_PI),player.y+distance,player.z+(1.0*player.dz/sMax)+0.5f+3*sin(M_PI_2+cur_direction/double(direction_parts)*2.*M_PI),
              player.x+(1.0*player.dx/sMax)+0.5f,player.y+0.5f,player.z+(1.0*player.dz/sMax)-0.5f,
              0,1,0);
    
    for (int i=0;i<map.x;i++)
        for (int j=0;j<map.z;j++)
            if (data[j][i] == 'x')
            {
                drawBox(j,i);
            } else {
                drawFloor(i,i+1,j,j+1);
            }

		if (player.lastPressed == 1) {
			drawGamer(180, 1, 0);
		}
		else if (player.lastPressed == 2) {
			drawGamer(90, 0, 1);
		}
		else if (player.lastPressed == 3) {
			drawGamer(0, -1, 0);
		}
		else {
			drawGamer(-90, 0, -1);
		}
	    
    glPushMatrix(); 
		glTranslatef ( ex.x +0.5f, ex.y+0.5f, ex.z+0.5f);
		glScalef(0.005, 0.005, 0.005);
		glColor4d(1,1,0.,0.4);
		pModel->draw();
		glColor3d(1,1,1);
    glPopMatrix(); 

	glPushMatrix();
		glTranslatef(botOne.x + 0.5f, botOne.y + 0.5f, botOne.z + 0.5f);
		glScalef(0.005, 0.005, 0.005);
		glColor4d(1, 0, 1., 0.4);
		pBotOne->draw();
		glColor3d(1, 1, 1);
	glPopMatrix();
	
	glPushMatrix();
		glTranslatef(botTwo.x + 0.5f, botTwo.y + 0.5f + translate, botTwo.z + 0.5f);
		glScalef(0.01, 0.01, 0.01);
		glColor4d(0, 1, 0., 0.4);
		glRotatef(180, 0, 1, 0);
		pBotTwo->draw();
		glColor3d(1, 1, 1);
	glPopMatrix();

	glPushMatrix();
		glTranslatef(botThree.x + 0.5f, botThree.y + 0.5f, botThree.z + 0.5f);
		glScalef(0.03, 0.03, 0.03);
		glColor4d(1, 1, 0., 0.4);
		pBotThree->draw();
		glColor3d(1, 1, 1);
	glPopMatrix();
	
	// Draw The Model
	pModel->reloadTextures();
	pBotOne->reloadTextures();
	pBotTwo->reloadTextures();	
	pBotThree->reloadTextures();
    glutSwapBuffers();
}

void reshape ( int w, int h )
{
    glViewport( 0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 60.0, (GLfloat)w/(GLfloat)h, 1.0, 60.0);
    
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt(0,0,25,0,0,0,0,1,0);
}

bool canMoveOnField(int z, int x)
{
	if (x >= 0 && z >= 0)
	{
		if (x <= map.x && z <= map.z)
		{
			if (data[x][z] == 'G')
			{
				halt(false);
				return false;
			}

			else if (data[x][z] == 'B')
			{
				PlaySound(L"box.wav", NULL, SND_ASYNC | SND_FILENAME);
				return false;
			}

			else if (data[z][x] == '.')
			{
				return true;
			}

			else return false;
		}
		else return false;
	}
	else return false;
}

int step(int y, int x)
{
	int res = 0;
	if ((data[y][x] == 'x')) res++;
	if ((y<mm) && (data[y + 1][x] == 'x')) res++;
	if ((y>0) && (data[y - 1][x] == 'x')) res++;
	if ((x<mm) && (data[y][x + 1] == 'x')) res++;
	if ((x>0) && (data[y][x - 1] == 'x')) res++;
	return res;
}	

int stepC(int y, int x)
{
	int res = 0;
	if ((cp[y][x] == 'x')) res++;
	if ((y<mm) && (cp[y + 1][x] == 'x')) res++;
	if ((y>0) && (cp[y - 1][x] == 'x')) res++;
	if ((x<mm) && (cp[y][x + 1] == 'x')) res++;
	if ((x>0) && (cp[y][x - 1] == 'x')) res++;
	return res;
}

void DataToCp(void)
{
	for (int j = 1; j <= mm; j++)
		for (int i = 1; i <= mm; i++)
			cp[j][i] = data[j][i];
}

void fill(int y, int x, Tpos *v, int *l)
{
	int st = 1, en = 0;
	int G[mm*mm], Ll[mm*mm];
	G[0] = y*mm + x; Ll[0] = 0;
	cp[y][x] = 'x';
	while (st != en)
	{
		if ((G[en] % mm + 1<mm) && (cp[(int)G[en] / mm][G[en] % mm + 1] == '.'))
		{
			G[st] = G[en] + 1;
			Ll[st] = Ll[en] + 1;
			cp[(int)G[st] / mm][G[st] % mm] = 'x';
			st++;
		}
		if ((G[en] % mm - 1>0) && (cp[(int)G[en] / mm][G[en] % mm - 1] == '.'))
		{
			G[st] = G[en] - 1;
			Ll[st] = Ll[en] + 1;
			cp[(int)G[st] / mm][G[st] % mm] = 'x';
			st++;
		}
		if ((((int)G[en] / mm) + 1<mm) && (cp[((int)G[en] / mm) + 1][G[en] % mm] == '.'))
		{
			G[st] = G[en] + mm;
			Ll[st] = Ll[en] + 1;
			cp[(int)G[st] / mm][G[st] % mm] = 'x';
			st++;
		}
		if ((((int)G[en] / mm) - 1>0) && (cp[((int)G[en] / mm) - 1][G[en] % mm] == '.'))
		{
			G[st] = G[en] - mm;
			Ll[st] = Ll[en] + 1;
			cp[(int)G[st] / mm][G[st] % mm] = 'x';
			st++;
		}
		en++;
	}
	int rnd = rand() % 5 + 1;
	(*v).x = (int)G[en - rnd] % mm;
	(*v).y = (int)G[en - rnd] / mm;
	*l = Ll[en - rnd];
}

bool isGood(Tpos a)
{
	DataToCp();
	int k = 0, i, j, lt;
	Tpos temp;
	cp[a.y][a.x] = 'x';
	if ((step(a.y, a.x)>3) || (step(a.y + 1, a.x)>3) || (step(a.y - 1, a.x)>3) || (step(a.y, a.x + 1)>3) || (step(a.y, a.x - 1)>3))
		return false;
	for (j = 1; j <= m; j++)
		for (i = 1; i <= m; i++)
			if (cp[j][i] == '.')
			{
				if (k>0)
					return false;
				fill(j, i, &temp, &lt);
				k++;
			}
	return true;
}

void genMap(int posX, int posY)
{
	int i, j;
	map.x = m + 2; map.z = m + 2;
	printf("Generating map... Please Wait.\n");
	srand((unsigned)time(NULL));
	for (j = 0; j<map.z; j++)
		for (i = 0; i<map.x; i++)
		{
			if ((i == 0) || (j == 0) || (i == map.x - 1) || (j == map.z - 1) || i == posX || j == posY)
				data[j][i] = 'x';
			else
				data[j][i] = '.';
		}

	int k = 0;
	Tpos t;
	while (k<(int)m*m / 2.5)
	{
		t.x = rand() % mm + 1;
		t.y = rand() % mm + 1;
		if ((data[t.y][t.x] == '.') && (isGood(t)))
		{
			data[t.y][t.x] = 'x';
			k++;
		}
	}
	for (j = 1; j <= m; j++)
		for (i = 1; i <= m; i++)
			if ((data[j][i] == 'x') && (step(j, i)>3))
			{
				data[j][i] = '.';
			}
	for (j = 1; j <= m; j++)
		for (i = 1; i <= m; i++)
			if ((data[j][i] == '.') && (step(j, i)<2))
			{
				t.x = i; t.y = j;
				if (isGood(t))
					data[j][i] = 'x';
			}

	Tpos ps[11]; k = 0;
	while (k <= 10)
	{
		t.x = rand() % mm + 1;
		t.y = rand() % mm + 1;

		if (data[t.y][t.x] == '.')
		{
			if (k == 3)
			{
				botOne.x = t.x;
				botOne.z = t.y;
				data[t.x][t.y] = 'B';
			}

			if (k == 8)
			{
				botTwo.x = t.x;
				botTwo.z = t.y;
				data[t.x][t.y] = 'B';
			}

			if (k == 9)
			{
				botThree.x = t.x;
				botThree.z = t.y;
				data[t.x][t.y] = 'B';
			}
			ps[k] = t;
			k++;
		}
	}
	k = rand() % 11;
	player.x = ps[k].x;
	player.z = ps[k].y;
	k = rand() % 11;
	ex.x = ps[k].x;
	ex.z = ps[k].y;

	data[ex.x][ex.z] = 'G';

	DataToCp();
	fill(ps[k].y, ps[k].x, &t, &pathLen);

	printf("Generating complete.\n");
}

void key(unsigned char key, int x, int y)
{
	if (key == 'q' || key == 'Q' || key == 27)
		halt(false);

	if (key == 'a' || key == 'A')
	{
		cur_direction--;
		if (cur_direction<0)
			cur_direction += direction_parts;
	}

	if (key == 's' || key == 'S')
	{
		cur_direction++;
		if (cur_direction == direction_parts)
			cur_direction = 0;
	}

	if ((key == 'z' || key == 'Z') && (distance < 58.))
	{
		distance += 0.25;
		if (distance < 23)
		{
			fogColor[3] -= 0.2;
			density -= 0.015;
			glFogfv(GL_FOG_COLOR, fogColor);
			glFogf(GL_FOG_DENSITY, density);
		}
	}

	if ((key == 'x' || key == 'X') && (distance>0))
	{
		distance -= 0.25;
		if (distance < 23)
		{
			fogColor[3] += 0.2;
			density += 0.015;
			glFogfv(GL_FOG_COLOR, fogColor);
			glFogf(GL_FOG_DENSITY, density);
		}
	}
	if (key == 'r' || key == 'R')
	{
		cur_direction = 0; distance = 4.;
	}

	if (key == 'M' || key == 'm') {

		try {
			data[player.z - player.positionZ][player.x - player.positionX] = '.';
			PlaySound(L"move.wav", NULL, SND_ASYNC | SND_FILENAME);
		}
		catch (int e){}
	}
}

void keys( int key, int x, int y)
{
    if (player.isGo) return;
    int dir=int(((direction_parts)/double(direction_parts))*4.+0.5);
    for (int i=0; i<4; i++)
        if ( key == move_key[i] ) {
			//Up key
			if (i == 0) {
				player.lastPressed = 2;
			}
			//Left key
			else if (i == 1) {
				player.lastPressed = 1;
			}
			//Down key
			else if (i == 2) {
				player.lastPressed = 4;
			}
			//Right key
			else if (i == 3) {
				player.lastPressed = 3;
			}
            int newz=player.z+move[(dir+i)%4][0];
            int newx=player.x+move[(dir+i)%4][1];
            if (canMoveOnField(newz,newx)) {
                player.isGo=true;
                player.dz+=move[(dir+i)%4][0];
                player.dx+=move[(dir+i)%4][1];
                myLen++;
            }
        }
}

void Timer(int iUnused)
{
	translate += 0.1;
	if (translate > 1.5)
		translate -= 1.5;
	glutTimerFunc(100, Timer, 0);
}

void halt(bool f)
{
	glutDestroyWindow(glWin);

	exit(0);
}

int main(int argc, char** argv)
{
	pModel = new MilkshapeModel();					// Memory To Hold The Model
	pModel->loadModelData("data/BattleDroid.ms3d");	// Loads The Model And Checks For Errors

	pBotOne = new MilkshapeModel();
	pBotOne->loadModelData("data/model.ms3d");

	pBotTwo = new MilkshapeModel();
	pBotTwo->loadModelData("data/dwarf2.ms3d");

	pBotThree = new MilkshapeModel();
	pBotThree->loadModelData("data/turtle1.ms3d");

	genMap(0, 0);
	printf("Loading models...");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1024, 768);

	glWin = glutCreateWindow("Maze game");
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutSpecialFunc(keys);
	glutIdleFunc(animate);
	glutTimerFunc(100, Timer, 0);

	//Load textures
	localTexture = auxDIBImageLoad(L"data/ground.bmp");
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, localTexture->sizeX, localTexture->sizeY, GL_RGB, GL_UNSIGNED_BYTE, localTexture->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	localFloor = auxDIBImageLoad(L"data/grass.bmp");
	glGenTextures(1, &texFloor);
	glBindTexture(GL_TEXTURE_2D, texFloor);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, localFloor->sizeX, localFloor->sizeY, GL_RGB, GL_UNSIGNED_BYTE, localFloor->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	printf("Loading complete.");
	glutFullScreen();
	glutMainLoop();
	return 0;
}
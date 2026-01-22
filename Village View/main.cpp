#include <windows.h>
#include <GL/glut.h>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <cstdio>

using namespace std;

int winW=900, winH=600;
int refreshMs=16;
int sceneIndex=1;

// --------------------------
// Scenario-1
// --------------------------
bool  s1_isDay=true;

float s1_cloudX1=-200.0f;
float s1_cloudX2=250.0f;
float s1_birdX=-150.0f;

float s1_sunX=650.0f;
float s1_sunY=520.0f;
float s1_sunY_day=520.0f;
float s1_sunY_night=120.0f;
float s1_sunY_target=520.0f;

float s1_wavePhase=0.0f;

// --------------------------
// Scenario-2
// --------------------------
bool  s2_isDay=true;
bool  s2_isRaining=false;

float s2_bladeAngle=0.0f;

bool  s2_tractorMove=true;
float s2_tractorX=520.0f;
int   s2_tractorDir=1;

float s2_tractorSpeed=1.2f;
const float s2_DEFAULT_SPEED=1.2f;

float s2_SPEED_STEP=0.3f;
float s2_MAX_SPEED=8.0f;

float s2_cloudX1=140.0f;
float s2_cloudX2=620.0f;
float s2_cloudSpeed1=0.25f;
float s2_cloudSpeed2=0.18f;

float s2_birdX[5]={-50,-220,-380,-140,-300};
float s2_birdY[5] = {500,540,515,560,535};
float s2_birdSpeed[5]={0.90f,0.65f,0.75f,0.55f,0.70f};

const int s2_RAIN_COUNT=450;
float s2_rainX[s2_RAIN_COUNT];
float s2_rainY[s2_RAIN_COUNT];
float s2_rainV[s2_RAIN_COUNT];
float s2_rainWind=1.6f;

// --------------------------
// Scenario-3
// --------------------------
float s3_boatPosition=0.0f;
float s3_boatSpeed=0.02f;

float s3_cloudPosition=-10.0f;
float s3_cloudSpeed=0.03f;

bool  s3_isPaused=false;
bool  s3_isNightMode=false;

const float S3_PI=3.14159265358979323846f;

//function
float clamp01(float v)
{
    return max(0.0f, min(1.0f,v));
}

void setOrtho2D()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0,(double)winW,0.0,(double)winH);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void s3_applyTransformBegin()
{
    glPushMatrix();
    glTranslatef((float)winW*0.5f, (float)winH*0.5f, 0.0f);

    float sx=(float)winW/40.0f;
    float sy=(float)winH/40.0f;

    glScalef(sx, sy, 1.0f);
}

void s3_applyTransformEnd()
{
    glPopMatrix();
}


void drawRect(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void filledCircle(float cx, float cy, float r, int seg=80)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i=0; i<=seg; i++)
    {
        float a=(float)i*2.0f*3.14159265f/(float)seg;
        glVertex2f(cx+cos(a)*r, cy+sin(a)*r);
    }
    glEnd();
}

void drawStar(float x, float y)
{
    glBegin(GL_LINES);
    glVertex2f(x-3, y);
    glVertex2f(x+3, y);

    glVertex2f(x, y-3);
    glVertex2f(x, y+3);
    glEnd();
}

// ============================================================
//  ----------------------- SCENARIO 1 ------------------------
// ============================================================
void s1_setSceneColor(float r, float g, float b, bool isLightObject = false)
{
    if (s1_isDay||isLightObject)
    {
        glColor3f(r,g,b);
        return;
    }

    float dark=0.35f;
    float blueBoost=0.10f;

    float nr=r*dark;
    float ng=g*(dark+0.02f);
    float nb=b*(dark+0.10f);

    nb=clamp01(nb+blueBoost);
    glColor3f(clamp01(nr), clamp01(ng), clamp01(nb));
}

void s1_drawFilledCircle(float cx, float cy, float r, int segments=90)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i=0; i<=segments; i++)
    {
        float a=(float)i*2.0f*3.14159265f/(float)segments;
        glVertex2f(cx+cos(a)*r, cy+sin(a)*r);
    }
    glEnd();
}

void s1_drawFilledEllipse(float cx, float cy, float rx, float ry, int segments=90)
{
    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glScalef(rx, ry, 1);
    s1_drawFilledCircle(0, 0, 1.0f, segments);
    glPopMatrix();
}

void s1_drawWindowGlow(float x1, float y1, float x2, float y2, float spreadX, float spreadY)
{
    if (s1_isDay)
    {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.95f, 0.45f, 0.25f);
    glBegin(GL_QUADS);
    glVertex2f(x1-spreadX, y1-spreadY);
    glVertex2f(x2+spreadX, y1-spreadY);
    glVertex2f(x2+spreadX, y2+spreadY);
    glVertex2f(x1-spreadX, y2+spreadY);
    glEnd();

    glColor4f(1.0f, 0.95f, 0.45f, 0.12f);
    float cx=(x1+x2)*0.5f;
    glBegin(GL_TRIANGLES);
    glVertex2f(cx, y1);
    glVertex2f(x1-spreadX*2, y1-55);
    glVertex2f(x2+spreadX*2, y1-55);
    glEnd();

    glDisable(GL_BLEND);
}

void s1_drawCloud(float x, float y, float s)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    if (s1_isDay)
    {
        s1_setSceneColor(1, 1, 1, false);
    }
    else
    {
        s1_setSceneColor(0.65f, 0.65f, 0.70f, false);
    }

    s1_drawFilledCircle(0, 0, 26);
    s1_drawFilledCircle(26, 8, 22);
    s1_drawFilledCircle(52, 0, 26);
    s1_drawFilledCircle(26, -8, 22);

    glPopMatrix();
}

void s1_drawBird(float x, float y, float s)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    if (s1_isDay)
    {
        glColor3f(0, 0, 0);
    }
    else
    {
        glColor3f(0.85f, 0.85f, 0.90f);
    }

    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(0, 0);
    glVertex2f(12, 8);
    glVertex2f(24, 0);
    glVertex2f(36, 8);
    glVertex2f(48, 0);
    glEnd();

    glPopMatrix();
}

void s1_drawFlower(float x, float y, float s, float pr, float pg, float pb)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    s1_setSceneColor(0.10f, 0.55f, 0.18f, false);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(0, 18);
    glEnd();

    s1_setSceneColor(pr, pg, pb, false);
    s1_drawFilledCircle(-5, 18, 4);
    s1_drawFilledCircle(5, 18, 4);
    s1_drawFilledCircle(0, 23, 4);
    s1_drawFilledCircle(-3, 26, 4);
    s1_drawFilledCircle(3, 26, 4);

    s1_setSceneColor(1.0f, 0.92f, 0.20f, false);
    s1_drawFilledCircle(0, 21, 3);

    glPopMatrix();
}

void s1_drawFlowerPatches()
{
    s1_drawFlower(110, 70, 1.00f, 1.0f, 0.30f, 0.30f);
    s1_drawFlower(150, 60, 0.90f, 1.0f, 0.55f, 0.20f);

    s1_drawFlower(225, 40, 0.80f, 0.95f, 0.55f, 0.20f);

    s1_drawFlower(680, 60, 1.00f, 1.0f, 0.30f, 0.30f);
    s1_drawFlower(720, 45, 0.90f, 1.0f, 0.55f, 0.20f);
    s1_drawFlower(760, 70, 0.85f, 0.90f, 0.40f, 1.0f);

    s1_drawFlower(880, 65, 0.90f, 1.0f, 0.35f, 0.45f);
    s1_drawFlower(865, 40, 0.80f, 0.40f, 0.80f, 1.0f);

    s1_drawFlower(660, 85, 0.80f, 0.95f, 0.55f, 0.20f);
}

void s1_drawRiver()
{
    s1_setSceneColor(0.15f, 0.55f, 0.85f, false);
    drawRect(0, 250, (float)winW, 290);

    float amp=4.0f;
    float freq=0.020f;
    float sp=s1_wavePhase;

    if (s1_isDay)
    {
        glColor3f(0.80f, 0.92f, 1.0f);
    }
    else
    {
        glColor3f(0.45f, 0.55f, 0.70f);
    }

    glLineWidth(1.5f);

    for (int k=0; k<4; k++)
    {
        float yBase=258+k*8.0f;
        glBegin(GL_LINE_STRIP);
        for (int x=0; x<=winW; x+=6)
        {
            float y=yBase+sin(x*freq+sp+k*0.8f)*amp;
            glVertex2f((float)x, y);
        }
        glEnd();
    }
}

void s1_drawSkyMountains()
{
    if (s1_isDay)
    {
        glBegin(GL_QUADS);
        glColor3f(0.80f, 0.92f, 1.00f);
        glVertex2f(0, (float)winH);
        glVertex2f((float)winW, (float)winH);

        glColor3f(0.88f, 0.96f, 1.00f);
        glVertex2f((float)winW, 280);
        glVertex2f(0, 280);
        glEnd();
    }
    else
    {
        glBegin(GL_QUADS);
        glColor3f(0.02f, 0.03f, 0.10f);
        glVertex2f(0, (float)winH);
        glVertex2f((float)winW, (float)winH);

        glColor3f(0.06f, 0.05f, 0.16f);
        glVertex2f((float)winW, 280);
        glVertex2f(0, 280);
        glEnd();

        glColor3f(1, 1, 1);
        glLineWidth(1.0f);

        drawStar(90, 530);
        drawStar(180, 470);
        drawStar(270, 540);
        drawStar(380, 490);
        drawStar(520, 520);
        drawStar(700, 540);
        drawStar(820, 490);
        drawStar(760, 450);
        drawStar(120, 500);
        drawStar(320, 460);
        drawStar(610, 470);
        drawStar(860, 540);
        drawStar(450, 560);
    }

    s1_setSceneColor(0.55f, 0.75f, 0.86f, false);
    s1_drawFilledCircle(250, 300, 130);
    s1_drawFilledCircle(450, 305, 160);
    s1_drawFilledCircle(680, 295, 140);

    s1_drawRiver();
}

void s1_drawSunOrMoon()
{
    if (s1_isDay)
    {
        glColor3f(1.0f, 0.92f, 0.25f);
        s1_drawFilledCircle(s1_sunX, s1_sunY, 26);

        glColor3f(1.0f, 0.88f, 0.22f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(s1_sunX-50, s1_sunY);
        glVertex2f(s1_sunX-32, s1_sunY);

        glVertex2f(s1_sunX+32, s1_sunY);
        glVertex2f(s1_sunX+50, s1_sunY);

        glVertex2f(s1_sunX, s1_sunY+50);
        glVertex2f(s1_sunX, s1_sunY+32);

        glVertex2f(s1_sunX, s1_sunY-32);
        glVertex2f(s1_sunX, s1_sunY-50);
        glEnd();
    }
    else
    {
        glColor3f(0.95f, 0.95f, 1.0f);
        s1_drawFilledCircle(700, 520, 22);

        glColor3f(0.02f, 0.03f, 0.10f);
        s1_drawFilledCircle(710, 526, 18);
    }
}

void s1_drawGroundBase()
{
    s1_setSceneColor(0.25f, 0.70f, 0.30f, false);
    drawRect(0, 0, (float)winW, 250);

    s1_setSceneColor(0.20f, 0.62f, 0.25f, false);
    drawRect(0, 290, (float)winW, 320);
}

void s1_drawPerspectiveRoad()
{
    float roadBottomL=260;
    float roadBottomR=640;
    float roadTopL=430;
    float roadTopR=470;
    float topY=250;

    // Left field
    s1_setSceneColor(0.65f, 0.85f, 0.60f, false);
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(roadBottomL, 0);
    glVertex2f(roadTopL, topY);
    glVertex2f(0, topY);
    glEnd();

    // Right field
    s1_setSceneColor(0.65f, 0.85f, 0.60f, false);
    glBegin(GL_POLYGON);
    glVertex2f(roadBottomR, 0);
    glVertex2f((float)winW, 0);
    glVertex2f((float)winW, topY);
    glVertex2f(roadTopR, topY);
    glEnd();

    // Road
    s1_setSceneColor(0.78f, 0.63f, 0.30f, false);
    glBegin(GL_POLYGON);
    glVertex2f(roadBottomL, 0);
    glVertex2f(roadBottomR, 0);
    glVertex2f(roadTopR, topY);
    glVertex2f(roadTopL, topY);
    glEnd();

    // Strips
    s1_setSceneColor(0.95f, 0.80f, 0.30f, false);

    glBegin(GL_POLYGON);
    glVertex2f(430, 0);
    glVertex2f(455, 0);
    glVertex2f(452, topY);
    glVertex2f(440, topY);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(470, 0);
    glVertex2f(495, 0);
    glVertex2f(460, topY);
    glVertex2f(455, topY);
    glEnd();
}

void s1_drawTreeBig(float x, float baseY, float s)
{
    glPushMatrix();
    glTranslatef(x, baseY, 0);
    glScalef(s, s, 1);

    s1_setSceneColor(0.45f, 0.22f, 0.10f, false);
    drawRect(-12, 0, 12, 95);

    glBegin(GL_TRIANGLES);
    glVertex2f(-10, 70);
    glVertex2f(-55, 95);
    glVertex2f(-35, 105);

    glVertex2f(10, 70);
    glVertex2f(55, 95);
    glVertex2f(35, 105);
    glEnd();

    s1_setSceneColor(0.20f, 0.65f, 0.25f, false);
    s1_drawFilledCircle(0, 130, 55);
    s1_drawFilledCircle(-45, 120, 40);
    s1_drawFilledCircle(45, 120, 40);
    s1_drawFilledCircle(0, 95, 35);

    glPopMatrix();
}

void s1_drawHouseLeft()
{
    s1_setSceneColor(0.82f, 0.92f, 1.0f, false);
    drawRect(90, 110, 240, 220);

    glBegin(GL_POLYGON);
    s1_setSceneColor(0.98f, 0.55f, 0.20f, false);
    glVertex2f(70, 220);
    glVertex2f(260, 220);
    glVertex2f(230, 285);
    glVertex2f(100, 285);
    glEnd();

    s1_setSceneColor(0.08f, 0.08f, 0.08f, false);
    drawRect(190, 110, 225, 190);

    if (!s1_isDay)
    {
        s1_setSceneColor(1.0f, 0.95f, 0.35f, true);
        drawRect(115, 145, 155, 175);
        s1_drawWindowGlow(115, 145, 155, 175, 10, 6);
    }
    else
    {
        s1_setSceneColor(0.12f, 0.12f, 0.12f, false);
        drawRect(115, 145, 155, 175);
    }
}

void s1_drawHouseRight()
{
    s1_setSceneColor(0.92f, 0.92f, 0.92f, false);
    drawRect(610, 90, 870, 210);

    glBegin(GL_POLYGON);
    s1_setSceneColor(0.40f, 0.78f, 0.35f, false);
    glVertex2f(580, 210);
    glVertex2f(900, 210);
    glVertex2f(850, 295);
    glVertex2f(630, 295);
    glEnd();

    s1_setSceneColor(0.06f, 0.06f, 0.06f, false);
    drawRect(650, 90, 710, 185);

    if (!s1_isDay)
    {
        s1_setSceneColor(1.0f, 0.95f, 0.35f, true);
        drawRect(780, 125, 845, 165);
        s1_drawWindowGlow(780, 125, 845, 165, 12, 7);
    }
    else
    {
        s1_setSceneColor(0.12f, 0.12f, 0.12f, false);
        drawRect(780, 125, 845, 165);
    }
}

void s1_drawCenterHut()
{
    s1_setSceneColor(0.94f, 0.88f, 0.75f, false);
    drawRect(435, 250, 465, 295);

    glBegin(GL_TRIANGLES);
    s1_setSceneColor(0.78f, 0.66f, 0.30f, false);
    glVertex2f(425, 295);
    glVertex2f(475, 295);
    glVertex2f(450, 335);
    glEnd();

    s1_setSceneColor(0.08f, 0.08f, 0.08f, false);
    drawRect(447, 250, 453, 280);

    if (!s1_isDay)
    {
        s1_setSceneColor(1.0f, 0.95f, 0.35f, true);

        drawRect(438, 270, 444, 282);
        drawRect(456, 270, 462, 282);

        s1_drawWindowGlow(438, 270, 444, 282, 6, 4);
        s1_drawWindowGlow(456, 270, 462, 282, 6, 4);
    }
    else
    {
        s1_setSceneColor(0.12f, 0.12f, 0.12f, false);
        drawRect(438, 270, 444, 282);
        drawRect(456, 270, 462, 282);
    }
}

void s1_drawCow(float x, float y, float s)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    s1_setSceneColor(0.95f, 0.95f, 0.95f, false);
    s1_drawFilledEllipse(0, 0, 42, 24);

    s1_setSceneColor(0.10f, 0.10f, 0.10f, false);
    s1_drawFilledEllipse(-12, 4, 10, 7);
    s1_drawFilledEllipse(10, -3, 12, 8);

    s1_setSceneColor(0.95f, 0.95f, 0.95f, false);
    s1_drawFilledEllipse(50, 8, 16, 12);

    s1_setSceneColor(0.85f, 0.85f, 0.85f, false);
    s1_drawFilledEllipse(42, 18, 6, 4);
    s1_drawFilledEllipse(58, 18, 6, 4);

    s1_setSceneColor(0.25f, 0.15f, 0.10f, false);
    drawRect(-22, -24, -14, -45);
    drawRect(-6, -24, 2, -45);
    drawRect(12, -24, 20, -45);
    drawRect(28, -24, 36, -45);

    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(-40, 8);
    glVertex2f(-58, 18);
    glEnd();

    s1_setSceneColor(0.10f, 0.10f, 0.10f, false);
    s1_drawFilledCircle(-60, 18, 4);

    glPopMatrix();
}

void s1_drawLampPost(float x, float yBase, float s, bool armToRight)
{
    glPushMatrix();
    glTranslatef(x, yBase, 0);
    glScalef(s, s, 1);

    float dir = 1.0f;
    if (!armToRight)
    {
        dir = -1.0f;
    }

    s1_setSceneColor(0.25f, 0.25f, 0.25f, false);
    drawRect(-4, 0, 4, 120);

    drawRect(4 * dir, 105, 26 * dir, 112);

    s1_setSceneColor(0.20f, 0.20f, 0.20f, false);
    float hx1 = 22 * dir;
    float hx2 = 34 * dir;
    drawRect(min(hx1, hx2), 95, max(hx1, hx2), 115);

    if (!s1_isDay)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float bx=28*dir;

        glColor4f(1.0f, 0.95f, 0.40f, 0.95f);
        s1_drawFilledCircle(bx, 100, 7);

        glColor4f(1.0f, 0.95f, 0.40f, 0.35f);
        s1_drawFilledCircle(bx, 100, 14);

        glColor4f(1.0f, 0.95f, 0.40f, 0.16f);
        glBegin(GL_TRIANGLES);
        glVertex2f(bx, 98);
        glVertex2f((bx-73*dir), 0);
        glVertex2f((bx+77*dir), 0);
        glEnd();

        glDisable(GL_BLEND);
    }

    glPopMatrix();
}

void s1_displayScene()
{
    s1_drawSkyMountains();
    s1_drawSunOrMoon();

    // clouds
    s1_drawCloud(s1_cloudX1, 520, 1.25f);
    s1_drawCloud(s1_cloudX2, 485, 1.0f);

    // birds
    s1_drawBird(s1_birdX, 540, 1.0f);
    s1_drawBird(s1_birdX + 140, 560, 0.85f);
    s1_drawBird(s1_birdX + 260, 525, 0.95f);

    s1_drawGroundBase();
    s1_drawPerspectiveRoad();

    s1_drawTreeBig(120, 250, 1.1f);
    s1_drawTreeBig(310, 245, 1.0f);
    s1_drawTreeBig(760, 248, 1.1f);

    // Lamp posts (left arm right, right arm left)
    s1_drawLampPost(330, 90, 1.0f, true);
    s1_drawLampPost(560, 90, 1.0f, false);

    s1_drawHouseLeft();
    s1_drawHouseRight();
    s1_drawCenterHut();

    // Cow only in day
    if (s1_isDay)
    {
        s1_drawCow(220, 105, 0.9f);
    }

    s1_drawFlowerPatches();
}


// ============================================================
//  -------------------- SCENARIO 2 (Farhan's Code) --------------
// ============================================================
void s2_drawPoint(int x, int y)
{
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

// DDA line
void s2_ddaLine(int x1, int y1, int x2, int y2) {
    int dx=x2-x1;
    int dy=y2-y1;

    int steps=max(abs(dx), abs(dy));
    if (steps==0)
    {
        s2_drawPoint(x1, y1);
        return;
    }

    float xInc=dx/(float)steps;
    float yInc=dy/(float)steps;

    float x=(float)x1;
    float y=(float)y1;

    for (int i=0; i<=steps; i++)
    {
        s2_drawPoint((int)(x+0.5f), (int)(y+0.5f));
        x+=xInc;
        y+=yInc;
    }
}

// Midpoint circle
void s2_plot8(int xc, int yc, int x, int y)
{
    s2_drawPoint(xc+x, yc+y);
    s2_drawPoint(xc-x, yc+y);
    s2_drawPoint(xc+x, yc-y);
    s2_drawPoint(xc-x, yc-y);
    s2_drawPoint(xc+y, yc+x);
    s2_drawPoint(xc-y, yc+x);
    s2_drawPoint(xc+y, yc-x);
    s2_drawPoint(xc-y, yc-x);
}

void s2_midpointCircle(int xc, int yc, int r)
{
    int x=0;
    int y=r;
    int d=1-r;

    s2_plot8(xc, yc, x, y);

    while (x<y)
    {
        x++;
        if (d<0)
        {
            d=d+2*x+1;
        }
        else
        {
            y--;
            d=d+2*(x-y)+1;
        }
        s2_plot8(xc, yc, x, y);
    }
}

void s2_rectf(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void s2_drawMoon()
{
    glColor3f(0.95f, 0.95f, 1.0f);
    filledCircle(760, 520, 22);

    glColor3f(0.08f, 0.08f, 0.15f);
    filledCircle(770, 528, 18);
}

void s2_drawHeadlightBeamLocal()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.95f, 0.25f, 0.35f);
    glBegin(GL_TRIANGLES);
    glVertex2f(115, 50);
    glVertex2f(200, 72);
    glVertex2f(200, 30);
    glEnd();

    glColor4f(1.0f, 0.95f, 0.25f, 0.90f);
    filledCircle(112, 50, 4);

    glDisable(GL_BLEND);
}

void s2_drawBird(float x, float y, float s)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    if (s2_isDay)
    {
        glColor3f(0.10f, 0.10f, 0.10f);
    }
    else
    {
        glColor3f(0.90f, 0.90f, 0.90f);
    }

    glPointSize(2.0f);

    s2_ddaLine(-12, 0, 0, 8);
    s2_ddaLine(0, 8, 12, 0);

    glPopMatrix();
}

void s2_drawBirds() {
    s2_drawBird(s2_birdX[0], s2_birdY[0], 1.2f);
    s2_drawBird(s2_birdX[1], s2_birdY[1], 1.0f);
    s2_drawBird(s2_birdX[2], s2_birdY[2], 1.1f);
    s2_drawBird(s2_birdX[3], s2_birdY[3], 0.9f);
    s2_drawBird(s2_birdX[4], s2_birdY[4], 1.0f);
}

void s2_drawCloud(float x, float y, float s)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    if (s2_isRaining)
    {
        if (s2_isDay)
        {
            glColor3f(0.78f, 0.80f, 0.82f);
        }
        else
        {
            glColor3f(0.45f, 0.46f, 0.50f);
        }
    }
    else
    {
        if (s2_isDay)
        {
            glColor3f(1, 1, 1);
        }
        else
        {
            glColor3f(0.75f, 0.75f, 0.80f);
        }
    }

    filledCircle(0, 0, 22);
    filledCircle(22, 6, 18);
    filledCircle(44, 0, 22);
    filledCircle(22, -6, 18);

    glPopMatrix();
}

void s2_drawSkyAndHills()
{
    if (s2_isDay)
        {
        if (s2_isRaining)
        {
            glColor3f(0.74f, 0.78f, 0.82f);
            s2_rectf(0, 0, (float)winW, (float)winH);
        }
        else
        {
            glColor3f(0.70f, 0.88f, 0.98f);
            s2_rectf(0, 0, (float)winW, (float)winH);
        }

        glColor3f(s2_isRaining?0.70f:0.78f, s2_isRaining?0.80f:0.88f, s2_isRaining?0.86f:0.94f);
        filledCircle(260, 330, 240);
        filledCircle(640, 320, 260);

    }
    else
    {
        if (s2_isRaining)
        {
            glColor3f(0.10f, 0.11f, 0.14f);
            s2_rectf(0, 0, (float)winW, (float)winH);

            glColor3f(0.70f, 0.70f, 0.78f);
            filledCircle(760, 520, 18);
        }
        else
        {
            glColor3f(0.08f, 0.08f, 0.15f);
            s2_rectf(0, 0, (float)winW, (float)winH);

            glColor3f(1, 1, 1);
            glLineWidth(1.0f);

            drawStar(90, 530);
            drawStar(180, 470);
            drawStar(270, 540);
            drawStar(380, 490);
            drawStar(520, 520);
            drawStar(700, 540);
            drawStar(820, 490);
            drawStar(760, 450);

            s2_drawMoon();
        }

        glColor3f(s2_isRaining?0.28f:0.25f, s2_isRaining ? 0.28f:0.22f, s2_isRaining ? 0.32f:0.30f);
        filledCircle(260, 330, 240);
        filledCircle(640, 320, 260);
    }

    s2_drawCloud(s2_cloudX1, 525, 1.4f);
    s2_drawCloud(s2_cloudX2, 540, 1.2f);

    if (s2_isRaining)
    {
        if (s2_isDay)
        {
            glColor3f(0.18f, 0.18f, 0.18f);
        }
        else
        {
            glColor3f(0.75f, 0.75f, 0.78f);
        }
    }
    s2_drawBirds();
}

void s2_drawFieldAndRoad()
{
    if (s2_isDay)
        {
        glColor3f(s2_isRaining ? 0.52f : 0.62f, s2_isRaining ? 0.70f : 0.78f, s2_isRaining ? 0.32f : 0.38f);
        s2_rectf(0, 0, (float)winW, 270);

        glColor3f(s2_isRaining ? 0.50f : 0.58f, s2_isRaining ? 0.66f : 0.74f, s2_isRaining ? 0.30f : 0.34f);
        s2_rectf(0, 270, (float)winW, 330);

        glColor3f(0.92f, 0.88f, 0.55f);
        s2_rectf(0, 170, (float)winW, 220);
    }
    else
    {
        glColor3f(0.20f, 0.30f, 0.18f);
        s2_rectf(0, 0, (float)winW, 270);

        glColor3f(0.18f, 0.28f, 0.16f);
        s2_rectf(0, 270, (float)winW, 330);

        glColor3f(0.55f, 0.52f, 0.35f);
        s2_rectf(0, 170, (float)winW, 220);
    }
}

void s2_drawTree(float x, float y, float s)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(s, s, 1);

    if (s2_isDay)
    {
        glColor3f(0.45f, 0.30f, 0.15f);
    }
    else
    {
        glColor3f(0.25f, 0.18f, 0.10f);
    }
    s2_rectf(-6, 0, 6, 55);

    if (s2_isDay)
    {
        glColor3f(s2_isRaining ? 0.30f : 0.35f, s2_isRaining ? 0.48f : 0.55f, s2_isRaining ? 0.22f : 0.25f);
    }
    else
    {
        glColor3f(0.18f, 0.30f, 0.18f);
    }
    filledCircle(0, 70, 22);
    filledCircle(-12, 60, 18);
    filledCircle(12, 60, 18);

    glPopMatrix();
}

void s2_drawBarn()
{
    if (s2_isDay)
    {
        glColor3f(0.92f, 0.35f, 0.22f);
    }
    else
    {
        glColor3f(0.45f, 0.18f, 0.14f);
    }
    s2_rectf(140, 240, 320, 330);

    if (s2_isDay)
    {
        glColor3f(0.95f, 0.95f, 0.95f);
    }
    else
    {
        glColor3f(0.70f, 0.70f, 0.70f);
    }

    glBegin(GL_POLYGON);
    glVertex2f(120, 330);
    glVertex2f(340, 330);
    glVertex2f(300, 380);
    glVertex2f(160, 380);
    glEnd();

    if (s2_isDay)
    {
        glColor3f(0.45f, 0.28f, 0.12f);
    }
    else
    {
        glColor3f(0.20f, 0.14f, 0.08f);
    }
    s2_rectf(230, 240, 300, 315);
}

void s2_drawWindmillTower()
{
    if (s2_isDay)
    {
        glColor3f(0.72f, 0.55f, 0.30f);
    }
    else
    {
        glColor3f(0.40f, 0.32f, 0.20f);
    }

    glPointSize(2.0f);

    s2_ddaLine(560, 230, 590, 380);
    s2_ddaLine(620, 230, 600, 380);

    s2_ddaLine(565, 260, 615, 260);
    s2_ddaLine(570, 290, 610, 290);
    s2_ddaLine(575, 320, 605, 320);
    s2_ddaLine(580, 350, 600, 350);

    s2_ddaLine(565, 260, 610, 320);
    s2_ddaLine(615, 260, 575, 320);
    s2_ddaLine(570, 290, 605, 350);
    s2_ddaLine(610, 290, 580, 350);
}

void s2_drawWindmillBlades(float cx, float cy)
{
    if (s2_isDay)
    {
        glColor3f(0.50f, 0.32f, 0.15f);
    }
    else
    {
        glColor3f(0.30f, 0.22f, 0.14f);
    }
    filledCircle(cx, cy, 8);

    glPushMatrix();
    glTranslatef(cx, cy, 0);
    glRotatef(s2_bladeAngle, 0, 0, 1);

    if (s2_isDay) {
        glColor3f(0.45f, 0.28f, 0.12f);
    } else {
        glColor3f(0.25f, 0.18f, 0.10f);
    }

    for (int i = 0; i < 8; i++) {
        glRotatef(45.0f, 0, 0, 1);
        glBegin(GL_POLYGON);
        glVertex2f(10, -4);
        glVertex2f(60, -10);
        glVertex2f(62, 10);
        glVertex2f(10, 4);
        glEnd();
    }

    glPopMatrix();
}

void s2_drawWindmill()
{
    s2_drawWindmillTower();
    s2_drawWindmillBlades(600, 400);
}

void s2_drawWheel(int cx, int cy, int rOuter, int rInner)
{
    glColor3f(0.08f, 0.08f, 0.08f);
    filledCircle((float)cx, (float)cy, (float)rOuter);

    glColor3f(0.95f, 0.95f, 0.95f);
    filledCircle((float)cx, (float)cy, (float)rInner);

    glColor3f(0.0f, 0.0f, 0.0f);
    glPointSize(2.0f);
    s2_midpointCircle(cx, cy, rOuter);
}

void s2_drawTractorShapeLocal()
{
    glColor3f(s2_isDay ? 0.85f : 0.45f, s2_isDay ? 0.25f : 0.15f, s2_isDay ? 0.18f : 0.15f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 25);
    glVertex2f(85, 25);
    glVertex2f(95, 40);
    glVertex2f(40, 55);
    glVertex2f(0, 50);
    glEnd();

    glColor3f(s2_isDay ? 0.78f : 0.38f, s2_isDay ? 0.20f : 0.12f, s2_isDay ? 0.14f : 0.12f);
    s2_rectf(70, 40, 115, 58);

    glColor3f(s2_isDay ? 0.55f : 0.35f, s2_isDay ? 0.55f : 0.35f, s2_isDay ? 0.55f : 0.35f);
    glBegin(GL_POLYGON);
    glVertex2f(35, 55);
    glVertex2f(70, 55);
    glVertex2f(78, 92);
    glVertex2f(40, 92);
    glEnd();

    glColor3f(0.75f, 0.90f, 0.95f);
    s2_rectf(45, 62, 68, 85);

    s2_drawWheel(30, 25, 22, 12);
    s2_drawWheel(105, 22, 16, 9);
}

void s2_drawTractor(float x, float y)
{
    glPushMatrix();
    glTranslatef(x, y, 0);

    if (s2_tractorDir == -1)
    {
        glTranslatef(120, 0, 0);
        glScalef(-1, 1, 1);
    }

    bool headlightOn = (!s2_isDay) && s2_tractorMove && (s2_tractorSpeed > 0.0f);
    if (headlightOn)
    {
        s2_drawHeadlightBeamLocal();
    }

    s2_drawTractorShapeLocal();
    glPopMatrix();
}

void s2_initRain()
{
    for (int i=0; i<s2_RAIN_COUNT; i++)
        {
        s2_rainX[i]=(float)(rand()%(winW+1));
        s2_rainY[i]=(float)(rand()%(winH+1));
        s2_rainV[i]=6.0f+(float)(rand()%70)/10.0f;
    }
}

void s2_drawRain()
{
    if (!s2_isRaining)
    {
        return;
    }

    if (s2_isDay)
    {
        glColor3f(0.70f, 0.85f, 1.0f);
    }
    else
    {
        glColor3f(0.55f, 0.70f, 0.85f);
    }

    glPointSize(1.0f);

    for (int i=0; i<s2_RAIN_COUNT; i++)
    {
        int x1=(int)s2_rainX[i];
        int y1=(int)s2_rainY[i];
        int x2=(int)(s2_rainX[i]+s2_rainWind);
        int y2=(int)(s2_rainY[i]-(10.0f+s2_rainV[i]*0.25f));
        s2_ddaLine(x1, y1, x2, y2);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.18f, 0.18f, 0.18f, s2_isDay ? 0.08f : 0.12f);
    s2_rectf(0, 0, (float)winW, (float)winH);

    glDisable(GL_BLEND);
}

void s2_displayScene()
{
    s2_drawSkyAndHills();
    s2_drawFieldAndRoad();

    s2_drawBarn();
    s2_drawWindmill();

    s2_drawTree(760, 210, 1.35f);
    s2_drawTree(780, 55, 1.2f);

    s2_drawTractor(s2_tractorX, 170);

    s2_drawTree(300, 30, 1.8f);
    s2_drawTree(140, 40, 1.4f);

    s2_drawRain();
}

// ============================================================
//  -------------------- SCENARIO 3 ---------------------------
// ============================================================

// Sun
void s3_sun()
{
    int triangleAmount=20;
    float twicePi=2.0f*S3_PI;

    float x=5.0f;
    float y=14.0f;
    float radius=2.0f;

    glBegin(GL_TRIANGLE_FAN);
    glColor3ub(255, 204, 0);
    glVertex2f(x, y);

    for (int i=0; i<=triangleAmount; i++)
        {
        glVertex2f
        (
            x+(radius*cos(i*twicePi/triangleAmount)),
            y+(radius*sin(i*twicePi/triangleAmount))
        );
    }
    glEnd();
}

// Moon
void s3_moon()
{
    int triangleAmount=20;
    float twicePi=2.0f*S3_PI;

    float xx=5.0f;
    float yx=14.0f;
    float radiusx=2.0f;

    glBegin(GL_TRIANGLE_FAN);
    glColor3ub(255, 255, 255);
    glVertex2f(xx, yx);

    for (int i=0; i<=triangleAmount; i++)
    {
        glVertex2f (xx+(radiusx*cos(i*twicePi/triangleAmount)), yx+(radiusx*sin(i*twicePi/triangleAmount)));
    }
    glEnd();

    float x=6.0f;
    float y=14.0f;
    float radius=1.5f;

    glBegin(GL_TRIANGLE_FAN);

    if (s3_isNightMode)
    {
        glColor3ub(10, 20, 40);
    }
    else
    {
        glColor3ub(255, 255, 255);
    }

    glVertex2f(x, y);

    for (int i=0; i<=triangleAmount; i++)
    {
        glVertex2f (x+(radius*cos(i*twicePi/triangleAmount)), y+(radius*sin(i*twicePi/triangleAmount)));
    }
    glEnd();
}

// River
void s3_river()
{
    glBegin(GL_QUADS);

    if (s3_isNightMode)
    {
        glColor3ub(20, 60, 100);
    }
    else
    {
        glColor3ub(38, 154, 214);
    }

    glVertex2f(-20.0f, -20.0f);
    glVertex2f(-20.0f, 6.0f);
    glVertex2f(20.0f, 6.0f);
    glVertex2f(20.0f, -20.0f);
    glEnd();
}

// House
void s3_house()
{
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex2f(-4.0f, 0.0f);
    glVertex2f(-4.0f, 4.0f);
    glVertex2f(-11.0f, 4.0f);
    glVertex2f(-11.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(-11.0f, 4.0f);
    glVertex2f(-4.0f, 4.0f);
    glVertex2f(-6.0f, 6.0f);
    glVertex2f(-13.0f, 6.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(210, 105, 30);
    glVertex2f(-9.0f, 2.0f);
    glVertex2f(-9.0f, 0.0f);
    glVertex2f(-6.0f, 0.0f);
    glVertex2f(-6.0f, 2.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(210, 180, 140);
    glVertex2f(-7.5f, 2.0f);
    glVertex2f(-7.5f, 0.0f);
    glVertex2f(-7.25f, 0.0f);
    glVertex2f(-7.25f, 2.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(-11.0f, 4.0f);
    glVertex2f(-15.0f, 4.0f);
    glVertex2f(-15.0f, 0.0f);
    glVertex2f(-11.0f, 0.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(210, 105, 30);
    glVertex2f(-14.0f, 3.0f);
    glVertex2f(-14.0f, 1.0f);
    glVertex2f(-12.0f, 1.0f);
    glVertex2f(-12.0f, 3.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(-15.0f, 4.0f);
    glVertex2f(-13.0f, 6.0f);
    glVertex2f(-11.0f, 4.0f);
    glEnd();
}

// Grass
void s3_grass()
{
    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-1.5f, -18.0f);
    glVertex2f(-0.5f, -18.0f);
    glVertex2f(-1.0f, -17.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-7.0f, -7.0f);
    glVertex2f(-6.5f, -8.0f);
    glVertex2f(-7.5f, -8.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(2.0f, -15.0f);
    glVertex2f(1.5f, -16.0f);
    glVertex2f(2.5f, -16.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-2.0f, -14.0f);
    glVertex2f(-2.5f, -13.0f);
    glVertex2f(-3.0f, -14.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(1.0f, -4.0f);
    glVertex2f(0.5f, -5.0f);
    glVertex2f(1.5f, -5.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-9.5f, -3.0f);
    glVertex2f(-9.0f, -4.0f);
    glVertex2f(-10.0f, -4.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-14.5f, -10.0f);
    glVertex2f(-14.0f, -11.0f);
    glVertex2f(-15.0f, -11.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-17.0f, -5.0f);
    glVertex2f(-16.5f, -6.0f);
    glVertex2f(-17.5f, -6.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-18.5f, -17.0f);
    glVertex2f(-18.0f, -18.0f);
    glVertex2f(-19.0f, -18.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-13.5f, -5.0f);
    glVertex2f(-13.0f, -6.0f);
    glVertex2f(-14.0f, -6.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-17.5f, -8.0f);
    glVertex2f(-17.0f, -9.0f);
    glVertex2f(-18.0f, -9.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-10.5f, -10.0f);
    glVertex2f(-10.0f, -11.0f);
    glVertex2f(-11.0f, -11.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-14.5f, -17.0f);
    glVertex2f(-14.0f, -18.0f);
    glVertex2f(-15.0f, -18.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-1.5f, -1.0f);
    glVertex2f(-1.0f, -2.0f);
    glVertex2f(-2.0f, -2.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-4.5f, -18.0f);
    glVertex2f(-4.0f, -19.0f);
    glVertex2f(-5.0f, -19.0f);
    glEnd();
}

// Fence
void s3_fence()
{
    glLineWidth(4);
    glBegin(GL_LINES);
    glColor3ub(255, 255, 102);

    glVertex2f(-15.0f, 4.0f);
    glVertex2f(-20.0f, 4.0f);
    glVertex2f(-15.0f, 3.5f);
    glVertex2f(-20.0f, 3.5f);
    glVertex2f(-15.0f, 3.0f);
    glVertex2f(-20.0f, 3.0f);
    glVertex2f(-15.0f, 2.5f);
    glVertex2f(-20.0f, 2.5f);
    glVertex2f(-15.0f, 2.0f);
    glVertex2f(-20.0f, 2.0f);
    glVertex2f(-15.0f, 1.5f);
    glVertex2f(-20.0f, 1.5f);
    glVertex2f(-15.0f, 1.0f);
    glVertex2f(-20.0f, 1.0f);
    glVertex2f(-15.0f, 0.5f);
    glVertex2f(-20.0f, 0.5f);

    glVertex2f(-15.5f, 4.5f);
    glVertex2f(-15.5f, 0.0f);
    glVertex2f(-16.0f, 4.5f);
    glVertex2f(-16.0f, 0.0f);
    glVertex2f(-16.5f, 4.5f);
    glVertex2f(-16.5f, 0.0f);
    glVertex2f(-17.0f, 4.5f);
    glVertex2f(-17.0f, 0.0f);
    glVertex2f(-17.5f, 4.5f);
    glVertex2f(-17.5f, 0.0f);
    glVertex2f(-18.0f, 4.5f);
    glVertex2f(-18.0f, 0.0f);
    glVertex2f(-18.5f, 4.5f);
    glVertex2f(-18.5f, 0.0f);
    glVertex2f(-19.0f, 4.5f);
    glVertex2f(-19.0f, 0.0f);
    glVertex2f(-19.5f, 4.5f);
    glVertex2f(-19.5f, 0.0f);

    glVertex2f(0.0f, 4.0f);
    glVertex2f(-4.0f, 4.0f);
    glVertex2f(0.0f, 3.5f);
    glVertex2f(-4.0f, 3.5f);
    glVertex2f(0.0f, 3.0f);
    glVertex2f(-4.0f, 3.0f);
    glVertex2f(0.0f, 2.5f);
    glVertex2f(-4.0f, 2.5f);
    glVertex2f(0.0f, 2.0f);
    glVertex2f(-4.0f, 2.0f);
    glVertex2f(0.0f, 1.5f);
    glVertex2f(-4.0f, 1.5f);
    glVertex2f(0.0f, 1.0f);
    glVertex2f(-4.0f, 1.0f);
    glVertex2f(0.0f, 0.5f);
    glVertex2f(-4.0f, 0.5f);

    glVertex2f(-3.5f, 4.5f);
    glVertex2f(-3.5f, 0.0f);
    glVertex2f(-3.0f, 4.5f);
    glVertex2f(-3.0f, 0.0f);
    glVertex2f(-2.5f, 4.5f);
    glVertex2f(-2.5f, 0.0f);
    glVertex2f(-2.0f, 4.5f);
    glVertex2f(-2.0f, 0.0f);
    glVertex2f(-1.5f, 4.5f);
    glVertex2f(-1.5f, 0.0f);
    glVertex2f(-1.0f, 4.5f);
    glVertex2f(-1.0f, 0.0f);
    glVertex2f(-0.5f, 4.5f);
    glVertex2f(-0.5f, 0.0f);
    glVertex2f(0.0f, 4.5f);
    glVertex2f(0.0f, 0.0f);

    glEnd();
}

// Straw hut
void s3_straw()
{
    glBegin(GL_POLYGON);
    glColor3ub(255, 219, 77);
    glVertex2f(-16.0f, 0.0f);
    glVertex2f(-16.0f, 2.5f);
    glVertex2f(-19.0f, 2.5f);
    glVertex2f(-19.0f, 0.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3ub(255, 219, 77);
    glVertex2f(-18.5f, 3.5f);
    glVertex2f(-16.5f, 3.5f);
    glVertex2f(-16.0f, 2.5f);
    glVertex2f(-19.0f, 2.5f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3ub(255, 219, 77);
    glVertex2f(-18.5f, 3.5f);
    glVertex2f(-16.5f, 3.5f);
    glVertex2f(-17.5f, 5.0f);
    glEnd();

    glLineWidth(4);
    glBegin(GL_LINES);
    glColor3ub(255, 219, 77);
    glVertex2f(-17.5f, 5.5f);
    glVertex2f(-17.5f, 0.0f);
    glEnd();
}

// Road
void s3_way()
{
    glBegin(GL_QUADS);
    glColor3ub(153, 153, 102);
    glVertex2f(-9.5f, 0.0f);
    glVertex2f(-5.5f, 0.0f);
    glVertex2f(-1.0f, -5.0f);
    glVertex2f(-3.5f, -7.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(153, 153, 102);
    glVertex2f(-3.5f, -7.0f);
    glVertex2f(-3.5f, -8.5f);
    glVertex2f(-0.75f, -9.0f);
    glVertex2f(-1.0f, -5.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(153, 153, 102);
    glVertex2f(-3.5f, -7.0f);
    glVertex2f(-0.75f, -9.0f);
    glVertex2f(-9.0f, -20.0f);
    glVertex2f(-14.0f, -20.5f);
    glEnd();
}

// Sky
void s3_sky()
{
    glBegin(GL_QUADS);

    if (s3_isNightMode)
    {
        glColor3ub(10, 20, 40);
    }
    else
    {
        glColor3ub(51, 204, 255);
    }

    glVertex2f(-20.0f, 20.0f);
    glVertex2f(-20.0f, 6.0f);
    glVertex2f(20.0f, 6.0f);
    glVertex2f(20.0f, 20.0f);
    glEnd();
}

// Stars
void s3_stars()
{
    if (!s3_isNightMode) return;

    glPointSize(7.5f);
    glBegin(GL_POINTS);
    glColor3ub(255, 255, 255);

    glVertex2f(-18.0f, 14.0f);
    glVertex2f(-16.0f, 16.0f);
    glVertex2f(-14.0f, 17.0f);
    glVertex2f(-12.0f, 14.7f);
    glVertex2f(-10.0f, 12.7f);
    glVertex2f(-8.0f, 16.7f);
    glVertex2f(-6.0f, 14.7f);
    glVertex2f(-4.0f, 18.7f);
    glVertex2f(-2.2f, 15.7f);
    glVertex2f(0.0f, 12.7f);
    glVertex2f(2.0f, 16.7f);
    glVertex2f(8.0f, 15.7f);
    glVertex2f(10.0f, 12.7f);
    glVertex2f(12.0f, 16.7f);
    glVertex2f(12.0f, 13.7f);
    glVertex2f(14.0f, 10.7f);
    glVertex2f(16.0f, 12.7f);
    glVertex2f(18.0f, 15.7f);

    glEnd();
}

// Tree
void s3_tree()
{
    glBegin(GL_QUADS);
    glColor3ub(128, 0, 0);
    glVertex2f(-3.0f, 0.0f);
    glVertex2f(-1.0f, 0.0f);
    glVertex2f(-1.0f, 7.0f);
    glVertex2f(-3.0f, 7.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-7.0f, 9.0f);
    glVertex2f(3.0f, 9.0f);
    glVertex2f(-2.0f, 12.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(-6.0f, 7.0f);
    glVertex2f(-2.0f, 10.0f);
    glVertex2f(2.0f, 7.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(128, 0, 0);
    glVertex2f(9.0f, 6.0f);
    glVertex2f(9.5f, 6.0f);
    glVertex2f(9.5f, 9.0f);
    glVertex2f(9.0f, 9.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(8.5f, 9.0f);
    glVertex2f(10.0f, 9.0f);
    glVertex2f(9.25f, 11.0f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.5f, 0.0f);
    glVertex2f(8.0f, 9.5f);
    glVertex2f(10.5f, 9.5f);
    glVertex2f(9.25f, 11.0f);
    glEnd();
}

// Ground
void s3_ground()
{
    glBegin(GL_TRIANGLES);

    if (s3_isNightMode)
    {
        glColor3ub(50, 100, 30);
    }
    else
    {
        glColor3ub(102, 255, 51);
    }

    glVertex2f(0.0f, 0.0f);
    glVertex2f(4.0f, -5.0f);
    glVertex2f(0.0f, -10.0f);
    glEnd();

    glBegin(GL_QUADS);

    if (s3_isNightMode)
    {
        glColor3ub(50, 100, 30);
    }
    else
    {
        glColor3ub(102, 255, 51);
    }

    glVertex2f(-20.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(0.0f, -20.0f);
    glVertex2f(-20.0f, -20.0f);
    glEnd();

    glBegin(GL_TRIANGLES);

    if (s3_isNightMode)
    {
        glColor3ub(50, 100, 30);
    }
    else
    {
        glColor3ub(102, 255, 51);
    }

    glVertex2f(0.0f, -20.0f);
    glVertex2f(4.0f, -15.0f);
    glVertex2f(0.0f, -10.0f);
    glEnd();
}

// Static boat
void s3_boatStatic()
{
    glBegin(GL_QUADS);
    glColor3ub(165, 42, 42);
    glVertex2f(3.0f, -12.0f);
    glVertex2f(5.0f, -14.0f);
    glVertex2f(8.0f, -14.0f);
    glVertex2f(10.0f, -12.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(192, 192, 192);
    glVertex2f(3.5f, -11.0f);
    glVertex2f(3.75f, -11.0f);
    glVertex2f(3.75f, -16.0f);
    glVertex2f(3.5f, -16.0f);
    glEnd();
}

// Moving boat
void s3_boatMoving()
{
    glPushMatrix();
    glTranslatef(s3_boatPosition, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    glColor3ub(139, 0, 0);
    glVertex2f(5.0f, 3.0f);
    glVertex2f(7.0f, 1.0f);
    glVertex2f(10.0f, 1.0f);
    glVertex2f(12.0f, 3.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3ub(255, 215, 0);
    glVertex2f(7.0f, 5.0f);
    glVertex2f(7.0f, 3.0f);
    glVertex2f(10.0f, 3.0f);
    glVertex2f(10.0f, 5.0f);
    glEnd();

    glPopMatrix();
}

// Hills
void s3_hill()
{
    glBegin(GL_TRIANGLES);

    if (s3_isNightMode)
    {
        glColor3ub(60, 60, 20);
    }
    else
    {
        glColor3ub(128, 128, 0);
    }

    glVertex2f(-20.0f, 4.0f);
    glVertex2f(-12.0f, 10.0f);
    glVertex2f(-4.0f, 4.0f);
    glEnd();

    glBegin(GL_TRIANGLES);

    if (s3_isNightMode)
    {
        glColor3ub(60, 60, 20);
    }
    else
    {
        glColor3ub(128, 128, 0);
    }

    glVertex2f(-4.0f, 4.0f);
    glVertex2f(2.0f, 10.0f);
    glVertex2f(9.0f, 4.0f);
    glEnd();

    glBegin(GL_TRIANGLES);

    if (s3_isNightMode)
    {
        glColor3ub(60, 60, 20);
    }
    else
    {
        glColor3ub(128, 128, 0);
    }

    glVertex2f(20.0f, 4.0f);
    glVertex2f(15.0f, 10.0f);
    glVertex2f(9.0f, 4.0f);
    glEnd();
}

// Cloud
void s3_cloud()
{
    glPushMatrix();
    glTranslatef(s3_cloudPosition, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(-10.0f, 18.0f);
    glVertex2f(-9.0f, 19.0f);
    glVertex2f(-8.0f, 18.0f);
    glVertex2f(-7.0f, 19.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(-13.0f, 15.0f);
    glVertex2f(-12.0f, 16.0f);
    glVertex2f(-11.0f, 15.0f);
    glVertex2f(-10.0f, 16.0f);
    glEnd();

    glPopMatrix();
}


void s3_displayScene()
{
    s3_sky();

    if (s3_isNightMode)
    {
        s3_moon();
        s3_stars();
    }
    else
    {
        s3_sun();
    }

    s3_cloud();
    s3_hill();

    s3_river();

    s3_boatStatic();
    s3_boatMoving();

    s3_ground();
    s3_way();
    s3_grass();
    s3_fence();
    s3_straw();
    s3_house();
    s3_tree();
}

//display function
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    setOrtho2D();

    if (sceneIndex == 1)
    {
        s1_displayScene();
    }
    else if (sceneIndex==2)
    {
        s2_displayScene();
    }
    else
    {
        s3_applyTransformBegin();
        s3_displayScene();
        s3_applyTransformEnd();
    }

    glutSwapBuffers();
}


void Timer(int v)
{
    if (sceneIndex==1)
        {
        s1_cloudX1+=0.35f;
        s1_cloudX2+=0.22f;

        if (s1_cloudX1>winW+250)
        {
            s1_cloudX1=-300;
        }
        if (s1_cloudX2>winW+250)
        {
            s1_cloudX2=-380;
        }

        s1_birdX+=0.55f;
        if (s1_birdX>winW+200)
        {
            s1_birdX=-260;
        }

        float step=1.2f;
        if (s1_sunY<s1_sunY_target)
        {
            s1_sunY=min(s1_sunY+step, s1_sunY_target);
        }
        else if (s1_sunY>s1_sunY_target)
        {
            s1_sunY=max(s1_sunY-step, s1_sunY_target);
        }

        s1_wavePhase+=0.08f;
    }

    if (sceneIndex==2)
    {
        s2_bladeAngle-=3.0f;
        if (s2_bladeAngle<-360.0f)
        {
            s2_bladeAngle+=360.0f;
        }

        s2_cloudX1+= s2_cloudSpeed1;
        s2_cloudX2+= s2_cloudSpeed2;

        if (s2_cloudX1>winW+150)
        {
            s2_cloudX1=-180;
        }
        if (s2_cloudX2>winW+150)
        {
            s2_cloudX2=-260;
        }

        for (int i=0; i<5; i++)
        {
            s2_birdX[i] += s2_birdSpeed[i];
            if (s2_birdX[i]>winW+50)
            {
                s2_birdX[i]=-420;
            }
        }

        if (s2_tractorMove)
        {
            s2_tractorX+=(s2_tractorSpeed*s2_tractorDir);

            if (s2_tractorDir == 1)
            {
                if (s2_tractorX>winW+140)
                {
                    s2_tractorX=-140;
                }
            }
            else
            {
                if (s2_tractorX<-160)
                {
                    s2_tractorX=winW+140;
                }
            }
        }

        if (s2_isRaining)
        {
            for (int i=0; i<s2_RAIN_COUNT; i++)
            {
                s2_rainY[i]-=s2_rainV[i];
                s2_rainX[i]+=s2_rainWind;

                if (s2_rainY[i]<-20)
                {
                    s2_rainY[i]=(float)(winH+(rand()%80));
                    s2_rainX[i]=(float)(rand()%(winW+1));
                }
                if (s2_rainX[i]>winW+20)
                {
                    s2_rainX[i]=-20;
                }
            }
        }
    }

        if (sceneIndex == 3)
        {
        float dt=(float)refreshMs/50.0f;

        if (s3_isPaused==false)
        {
            s3_boatPosition=s3_boatPosition+(s3_boatSpeed*dt);

            if (s3_boatPosition>20.0f)
            {
                s3_boatPosition=-20.0f;
            }
            if (s3_boatPosition<-20.0f)
            {
                s3_boatPosition=20.0f;
            }

            s3_cloudPosition = s3_cloudPosition+(s3_cloudSpeed*dt);

            if (s3_cloudPosition>30.0f)
            {
                s3_cloudPosition=-20.0f;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(refreshMs, Timer, 0);
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        exit(0);
    }

    if (key=='n'||key=='N')
    {
    sceneIndex=sceneIndex+1;

    if (sceneIndex>3)
    {
        sceneIndex=1;
    }

    glutPostRedisplay();
    return;
}

    if (sceneIndex==1)
    {
        if (key=='s'|| key=='S')
        {
            s1_isDay=!s1_isDay;

            if (s1_isDay)
            {
                s1_sunY_target = s1_sunY_day;
            }
            else
            {
                s1_sunY_target = s1_sunY_night;
            }
        }
        return;
    }

    if (sceneIndex==2)
        {
        if (key=='s'||key=='S')
        {
            s2_tractorMove=!s2_tractorMove;
            return;
        }

        if (key=='q'||key=='Q')
        {
            s2_tractorSpeed+=s2_SPEED_STEP;
            if (s2_tractorSpeed>s2_MAX_SPEED)
            {
                s2_tractorSpeed=s2_MAX_SPEED;
            }
            return;
        }

        if (key=='w'||key=='W')
        {
            s2_tractorSpeed-=s2_SPEED_STEP;
            if (s2_tractorSpeed<0.0f)
            {
                s2_tractorSpeed = 0.0f;
            }
            return;
        }

        if (key=='r'||key=='R')
        {
            s2_tractorSpeed=s2_DEFAULT_SPEED;
            return;
        }

        if (key=='e'||key=='E')
        {
            s2_tractorDir*=-1;
            return;
        }

        if (key=='d'||key=='D')
        {
            s2_isDay=!s2_isDay;
            return;
        }

        if (key=='b'||key=='B')
        {
            s2_isRaining=!s2_isRaining;
            return;
        }
    }
        if (sceneIndex==3)
        {
        if (key=='a'||key=='A')
        {
            s3_isPaused=true;
            return;
        }

        if (key=='s'||key=='S')
        {
            s3_isPaused=false;
            return;
        }

        if (key=='d'||key=='D')
        {
            s3_isNightMode=false;
            return;
        }

        if (key=='m'||key=='M')
        {
            s3_isNightMode=true;
            return;
        }
    }
}

void specialKeys(int key, int x, int y)
{
    if (sceneIndex!=3)
    {
        return;
    }

    if (key==GLUT_KEY_LEFT)
    {
        s3_boatSpeed=s3_boatSpeed-0.01f;

        if (s3_boatSpeed<-0.2f)
        {
            s3_boatSpeed=-0.2f;
        }
    }
    else if (key==GLUT_KEY_RIGHT)
    {
        s3_boatSpeed=s3_boatSpeed+0.01f;

        if (s3_boatSpeed>0.2f)
        {
            s3_boatSpeed=0.2f;
        }
    }
    else if (key==GLUT_KEY_UP)
    {
        s3_cloudSpeed=s3_cloudSpeed+0.005f;
    }
    else if (key==GLUT_KEY_DOWN)
        {
        s3_cloudSpeed=s3_cloudSpeed-0.005f;

        if (s3_cloudSpeed<0.01f)
        {
            s3_cloudSpeed=0.01f;
        }
    }

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    if (sceneIndex!=3)
    {
        return;
    }

    if (state==GLUT_DOWN)
    {
        if (button==GLUT_LEFT_BUTTON)
        {
            s3_boatSpeed=s3_boatSpeed+0.02f;

            if (s3_boatSpeed>0.2f)
            {
                s3_boatSpeed=0.2f;
            }
        }
        else if (button==GLUT_RIGHT_BUTTON)
        {
            s3_boatSpeed=s3_boatSpeed-0.02f;

            if (s3_boatSpeed<-0.2f)
            {
                s3_boatSpeed=-0.2f;
            }
        }
    }

    glutPostRedisplay();
}


void reshape(int w, int h)
{
    winW = max(1, w);
    winH = max(1, h);

    glViewport(0, 0, winW, winH);

    s2_initRain();
}



// ============================================================
//  MAIN
// ============================================================
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);

    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Computer Graphics project (Village view");

    glClearColor(1, 1, 1, 1);

    s2_initRain();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutTimerFunc(0, Timer, 0);

    glutMainLoop();
    return 0;
}

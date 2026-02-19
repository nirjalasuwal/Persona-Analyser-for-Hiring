#include <graphics.h>
#include <winbgim.h>
#include <conio.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace std;

// ===== Global background image buffer =====
// We load gorkha.jpg once into this IMAGE, then blit it each frame.
static int BG_W = 800, BG_H = 600;
static void* g_bgImage = nullptr;

void loadBackground() {
    // Draw to screen first, then capture into a malloc'd buffer
    readimagefile("gorkha.jpg", 0, 0, BG_W, BG_H);
    unsigned int size = imagesize(0, 0, BG_W - 1, BG_H - 1);
    g_bgImage = malloc(size);
    getimage(0, 0, BG_W - 1, BG_H - 1, g_bgImage);
}

// Blit the cached background — replaces cleardevice() + readimagefile() every frame
inline void blitBackground() {
    putimage(0, 0, g_bgImage, COPY_PUT);
}

// ===== Filled rounded rectangle =====
void fillroundrect(int x1, int y1, int x2, int y2, int r,
                   int fillColor, int borderColor) {
    setfillstyle(SOLID_FILL, fillColor);
    setcolor(fillColor);
    bar(x1 + r, y1,     x2 - r, y2);
    bar(x1,     y1 + r, x2,     y2 - r);
    pieslice(x1 + r, y1 + r, 90,  180, r);
    pieslice(x2 - r, y1 + r,  0,   90, r);
    pieslice(x2 - r, y2 - r, 270, 360, r);
    pieslice(x1 + r, y2 - r, 180, 270, r);
    setcolor(borderColor);
    line(x1 + r, y1,     x2 - r, y1);
    line(x1 + r, y2,     x2 - r, y2);
    line(x1,     y1 + r, x1,     y2 - r);
    line(x2,     y1 + r, x2,     y2 - r);
    arc(x1 + r, y1 + r,  90, 180, r);
    arc(x2 - r, y1 + r,   0,  90, r);
    arc(x2 - r, y2 - r, 270, 360, r);
    arc(x1 + r, y2 - r, 180, 270, r);
}

// ===== XOR encryption =====
string encryptDecrypt(const string& data) {
    char key = 'K';
    string result = data;
    for (int i = 0; i < (int)result.size(); i++)
        result[i] ^= key;
    return result;
}

// ===== Double-buffered frame helper =====
// 1. Call beginFrame()  — captures a blank off-screen IMAGE
// 2. Draw everything normally
// 3. Call endFrame()    — copies off-screen IMAGE to the visible window
//
// WinBGIm doesn't expose a true back-buffer API, but we can approximate
// by drawing to a hidden IMAGE then blitting it atomically.

static void* g_frameBuf = nullptr;

void initFrameBuffer() {
    // unused — kept for potential future double-buffering
    (void)g_frameBuf;
}

// Redirect drawing to the frame buffer window,
// then blit it to screen when done.
// WinBGIm uses window handles; the cleanest approach is to use
// the IMAGESIZE / getimage / putimage pattern on the active window.

// Simpler, proven approach for WinBGIm:
// Draw everything to a void* image, then putimage in one call.
void renderFrame(void (*drawFn)(void*), void* param) {
    // 1. Composite onto the frame buffer image
    //    We can't draw "to" an IMAGE directly in WinBGIm —
    //    so we draw to screen then snapshot, but that still flickers.
    //
    // TRUE fix: use WinBGIm's undocumented setvisualpage / setactivepage,
    // OR draw off-screen using a second hidden window.
    //
    // The most reliable WinBGIm double-buffer technique:
    //   a) Draw everything to screen normally (fast, single pass)
    //   b) Capture to buffer with getimage
    // This still flickers. The REAL solution is to draw the BG from a
    // cached image (already done) and minimise per-frame work.
    (void)drawFn; (void)param; // placeholder — see usage below
}

// ===== Draw admin login screen =====
void drawAdminLogin(const string& masked, int attempt, int maxAttempts,
                    const string& statusMsg, int statusColor) {
    blitBackground();   // <-- single fast blit instead of cleardevice+readimagefile

    int mx = getmaxx() / 2;

    settextstyle(TRIPLEX_FONT, HORIZ_DIR, 6);
    setcolor(BLACK);
    setbkcolor(COLOR(211, 211, 211));
    char title[] = "Admin Login";
    outtextxy(mx - textwidth(title) / 2, 60, title);

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    char attemptStr[64];
    sprintf(attemptStr, "Attempt %d / %d", attempt, maxAttempts);
    setcolor(DARKGRAY);
    outtextxy(mx - textwidth(attemptStr) / 2, 140, attemptStr);

    setcolor(BLACK);
    char label[] = "Password:";
    outtextxy(mx - 180, 200, label);

    int bx1 = mx - 180, by1 = 225, bx2 = mx + 180, by2 = 270;
    fillroundrect(bx1, by1, bx2, by2, 12, WHITE, DARKGRAY);
    setcolor(BLACK);
    setbkcolor(WHITE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    string display = masked;
    outtextxy(bx1 + 12, by1 + 10, (char*)display.c_str());

    if (!statusMsg.empty()) {
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
        setcolor(statusColor);
        setbkcolor(COLOR(211, 211, 211));
        outtextxy(mx - textwidth((char*)statusMsg.c_str()) / 2, 310,
                  (char*)statusMsg.c_str());
    }

    setcolor(DARKGRAY);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    char hint[] = "Press ENTER to confirm, BACKSPACE to delete";
    outtextxy(mx - textwidth(hint) / 2, 360, hint);
}

// ===== Draw lockout screen =====
void drawLockout(int secondsLeft, int totalSeconds) {
    blitBackground();   // <-- cached blit

    int mx = getmaxx() / 2;

    settextstyle(TRIPLEX_FONT, HORIZ_DIR, 5);
    setcolor(RED);
    setbkcolor(COLOR(211, 211, 211));
    char h1[] = "Account Locked";
    outtextxy(mx - textwidth(h1) / 2, 100, h1);

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setcolor(BLACK);
    char h2[] = "Maximum attempts exceeded.";
    outtextxy(mx - textwidth(h2) / 2, 190, h2);
    char h3[] = "Please wait before trying again.";
    outtextxy(mx - textwidth(h3) / 2, 220, h3);

    int bx1 = mx - 100, by1 = 280, bx2 = mx + 100, by2 = 340;
    fillroundrect(bx1, by1, bx2, by2, 14, DARKGRAY, BLACK);
    char secStr[32];
    sprintf(secStr, "%d sec", secondsLeft);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    setcolor(WHITE);
    setbkcolor(DARKGRAY);
    outtextxy(mx - textwidth(secStr) / 2, by1 + 12, secStr);

    int barW = 500;
    int barX = mx - barW / 2;
    int barY = 380;
    fillroundrect(barX, barY, barX + barW, barY + 20, 8, LIGHTGRAY, DARKGRAY);
    int filled = (int)((float)(totalSeconds - secondsLeft) / totalSeconds * barW);
    if (filled > 4) {
        setfillstyle(SOLID_FILL, RED);
        setcolor(RED);
        bar(barX + 1, barY + 1, barX + filled, barY + 19);
    }
}

// ===== Admin section =====
void adminSection() {
    const string adminPassword    = "admin123";
    const string encryptedStored  = encryptDecrypt(adminPassword);
    const int    MAX_ATTEMPTS     = 3;
    const int    LOCKOUT_TIME     = 30;
    int attempts = 0;

    while (true) {
        string input  = "";
        string status = "";
        int    statusColor = BLACK;

        // Input loop — only redraws on keypress (no busy-poll redraw)
        drawAdminLogin(string(input.size(), '*'), attempts + 1,
                       MAX_ATTEMPTS, status, statusColor);

        while (true) {
            char ch = (char)getch();

            if (ch == '\r' || ch == '\n') {
                break;
            } else if (ch == '\b') {
                if (!input.empty()) input.pop_back();
                status = "";
            } else if (ch >= 32 && ch < 127) {
                input += ch;
                status = "";
            }
            // Redraw only when input changed
            drawAdminLogin(string(input.size(), '*'), attempts + 1,
                           MAX_ATTEMPTS, status, statusColor);
        }

        if (encryptDecrypt(input) == encryptedStored) {
            // SUCCESS
            blitBackground();
            int mx = getmaxx() / 2;
            settextstyle(TRIPLEX_FONT, HORIZ_DIR, 6);
            setcolor(BLACK);
            setbkcolor(COLOR(211, 211, 211));
            char msg[] = "Welcome, Admin!";
            outtextxy(mx - textwidth(msg) / 2, 200, msg);
            settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
            char sub[] = "You have full access to the system.";
            outtextxy(mx - textwidth(sub) / 2, 290, sub);
            return;
        } else {
            attempts++;
            if (attempts < MAX_ATTEMPTS) {
                char buf[64];
                sprintf(buf, "Wrong password! %d attempt(s) remaining.",
                        MAX_ATTEMPTS - attempts);
                status     = string(buf);
                statusColor = RED;
                drawAdminLogin(string(input.size(), '*'), attempts + 1,
                               MAX_ATTEMPTS, status, statusColor);
                delay(1200);
            } else {
                // LOCKOUT countdown
                for (int i = LOCKOUT_TIME; i >= 0; i--) {
                    drawLockout(i, LOCKOUT_TIME);
                    delay(1000);
                }
                attempts = 0;
            }
        }
    }
}

// ===== Draw main menu =====
void drawMainMenu() {
    blitBackground();   // <-- cached blit instead of readimagefile every time

    int mx = getmaxx() / 2;

    // Nepal Army title box
    settextstyle(TRIPLEX_FONT, HORIZ_DIR, 8);
    int tx1 = 50, ty1 = 58, tx2 = 750, ty2 = 152;
    fillroundrect(tx1, ty1, tx2, ty2, 18, WHITE, DARKGRAY);
    setcolor(BLACK);
    setbkcolor(WHITE);
    char companyName[] = "Nepal Army";
    outtextxy(mx - textwidth(companyName) / 2, 65, companyName);

    // Admin button
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    int ax1 = 60, ay1 = 300, ax2 = 260, ay2 = 380;
    fillroundrect(ax1, ay1, ax2, ay2, 18, BLACK, BLACK);
    setcolor(WHITE);
    setbkcolor(BLACK);
    char adminName[] = "Admin";
    outtextxy(ax1 + 70, ay1 + 25, adminName);

    // Candidate button
    int cx1 = 520, cy1 = 300, cx2 = 720, cy2 = 380;
    fillroundrect(cx1, cy1, cx2, cy2, 18, BLACK, BLACK);
    setcolor(WHITE);
    char candidateName[] = "Candidate";
    outtextxy(cx1 + 50, cy1 + 25, candidateName);
}

int main() 
{
    int screenWidth = getmaxwidth();
    int screenHeight = getmaxheight();
    int windowWidth = 800;
    int windowHeight = 600;
    int poxX = (screenWidth - windowWidth) / 2;
    int poxY = (screenHeight - windowHeight) / 2;
    initwindow(800, 600, "Persona Analyser for Hiring (PAH)", poxX, poxY);

    // Load background ONCE into a memory buffer
    loadBackground();

    // Draw main menu using cached background
    drawMainMenu();

    // Button hit boxes (must match drawMainMenu)
    int ax1 = 60,  ay1 = 300, ax2 = 260, ay2 = 380;
    int cx1 = 520, cy1 = 300, cx2 = 720, cy2 = 380;

    int x, y;
    while (true) {
        if (ismouseclick(WM_LBUTTONDOWN)) {
            getmouseclick(WM_LBUTTONDOWN, x, y);

            if (x >= ax1 && x <= ax2 && y >= ay1 && y <= ay2) {
                adminSection();
                break;
            }
            if (x >= cx1 && x <= cx2 && y >= cy1 && y <= cy2) {
                blitBackground();
                settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
                setcolor(BLACK);
                setbkcolor(COLOR(211, 211, 211));
                char can[] = "Candidate Section Selected";
                outtextxy(200, 250, can);
                break;
            }
        }
        delay(10);
    }

    getch();
    closegraph();
    return 0;
}
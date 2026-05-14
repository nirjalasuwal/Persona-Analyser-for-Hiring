#ifndef DRAW_BAR_GRAPH_H
#define DRAW_BAR_GRAPH_H

#include <graphics.h>
#include <string>
#include <map>
#include <vector>
#include <cstring>  // for strdup / strcpy on older compilers

// ─────────────────────────────────────────────────────────────────
//  drawCandidateBarGraph
//
//  Opens a BGI graphics window and draws a bar graph for one
//  candidate's Big-Five trait scores.
//
//  Parameters:
//    candidateID   – e.g. "C001"
//    candidateName – e.g. "Nirjala"
//    traitTotals   – map<string,int> with keys:
//                      "Neuroticism", "Openness", "Extraversion",
//                      "Agreeableness", "Conscientiousness"
//                    each value in the range 0–20.
//
//  The window is positioned at (550, 50) to sit beside the console.
//  Press any key inside the graphics window to close it.
// ─────────────────────────────────────────────────────────────────

void drawCandidateBarGraph(const std::string& candidateID,
                           const std::string& candidateName,
                           const std::map<std::string, int>& traitTotals)
{
    // ── Layout constants ─────────────────────────────────────────
    const int WIN_W      = 900;
    const int WIN_H      = 600;

    // Graph area boundaries
    const int X_ORIGIN   = 80;   // left  edge of Y-axis
    const int Y_ORIGIN   = 400;  // bottom edge of X-axis (score = 0)
    const int Y_TOP      = 100;  // top    of graph area  (score = 20)
    const int X_END      = 800;  // right  edge of graph area

    // Each bar: 60 px wide, 80 px gap between bar centres
    const int BAR_WIDTH  = 60;
    const int BAR_STEP   = 96;   // centre-to-centre spacing

    // Pixel height that represents a score of 20
    const int MAX_H      = Y_ORIGIN - Y_TOP;   // 310 px

    // ── Trait order and colours ───────────────────────────────────
    const std::vector<std::string> ORDER = {
        "Openness",
        "Conscientiousness",
        "Extraversion",
        "Agreeableness",
        "Neuroticism"
    };

    // Distinct colours – BGI colour constants
    const int COLORS[5] = { CYAN, YELLOW, GREEN, MAGENTA, RED };

    // Short labels that fit under each bar
    const std::vector<std::string> SHORT_LABELS = {
        "Openness",
        "Consciousn.",   // truncated to fit
        "Extraversion",
        "Agreeablns.",
        "Neuroticism"
    };

    // ── Open window ───────────────────────────────────────────────
    // Title string: "Nirjala - C001  |  Trait Bar Graph"
    std::string winTitle = candidateName + " - " + candidateID + "  |  Trait Bar Graph";

    // initwindow is WinBGIm / WinBGI extended BGI
    initwindow(WIN_W, WIN_H, const_cast<char*>(winTitle.c_str()), 550, 50);

    // ── Background ────────────────────────────────────────────────
    setbkcolor(WHITE);
    cleardevice();

    // ── Title text inside the window ──────────────────────────────
    setcolor(DARKGRAY);
    settextstyle(BOLD_FONT, HORIZ_DIR, 2);
    std::string title = candidateName + " (" + candidateID + ") - Big Five Trait Scores";
    outtextxy(WIN_W / 2 - (int)title.size() * 5, 18, const_cast<char*>(title.c_str()));

    // ── Draw axes ─────────────────────────────────────────────────
    setcolor(BLACK);
    setlinestyle(SOLID_LINE, 0, 2);

    // Y-axis
    line(X_ORIGIN, Y_ORIGIN, X_ORIGIN, Y_TOP - 10);
    // X-axis
    line(X_ORIGIN, Y_ORIGIN, X_END, Y_ORIGIN);

    // Y-axis arrowhead
    line(X_ORIGIN, Y_TOP - 10, X_ORIGIN - 5, Y_TOP);
    line(X_ORIGIN, Y_TOP - 10, X_ORIGIN + 5, Y_TOP);

    // ── Y-axis tick marks and labels (0, 5, 10, 15, 20) ──────────
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    setlinestyle(SOLID_LINE, 0, 1);

    for (int score = 0; score <= 20; score += 5) {
        int yPix = Y_ORIGIN - (int)((double)score / 20.0 * MAX_H);

        // tick
        line(X_ORIGIN - 5, yPix, X_ORIGIN, yPix);

        // grid line (light)
        setcolor(LIGHTGRAY);
        line(X_ORIGIN + 1, yPix, X_END, yPix);
        setcolor(BLACK);

        // label
        std::string lbl = std::to_string(score);
        outtextxy(X_ORIGIN - 35, yPix - 6, const_cast<char*>(lbl.c_str()));
    }

    // Y-axis label "Score" - moved above the arrow
    setcolor(DARKGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(X_ORIGIN - 35, Y_TOP - 30, const_cast<char*>(std::string("Score").c_str()));

    // ── Draw bars ─────────────────────────────────────────────────
    int barCentreX = X_ORIGIN + BAR_STEP;   // first bar centre

    for (int i = 0; i < 5; i++) {
        const std::string& trait = ORDER[i];
        auto it = traitTotals.find(trait);
        int score = (it != traitTotals.end()) ? it->second : 0;
        if (score < 0)  score = 0;
        if (score > 20) score = 20;

        int barH   = (int)((double)score / 20.0 * MAX_H);
        int left   = barCentreX - BAR_WIDTH / 2;
        int right  = barCentreX + BAR_WIDTH / 2;
        int top    = Y_ORIGIN - barH;
        int bottom = Y_ORIGIN;

        // Filled bar
        setfillstyle(SOLID_FILL, COLORS[i]);
        bar(left, top, right, bottom);

        // Bar border
        setcolor(DARKGRAY);
        setlinestyle(SOLID_LINE, 0, 1);
        rectangle(left, top, right, bottom);

        // Trait labels removed - using legend instead

        barCentreX += BAR_STEP;
    }

    // ── Colour legend (bottom) ──────────────────────────────────
    int legendX = X_ORIGIN + 20;
    int legendY = Y_ORIGIN + 50;
    setcolor(DARKGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(legendX, legendY - 20, const_cast<char*>(std::string("Legend:").c_str()));

    for (int i = 0; i < 5; i++) {
        setfillstyle(SOLID_FILL, COLORS[i]);
        bar(legendX, legendY + i * 22, legendX + 18, legendY + i * 22 + 16);
        setcolor(DARKGRAY);
        rectangle(legendX, legendY + i * 22, legendX + 18, legendY + i * 22 + 16);
        setcolor(BLACK);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        outtextxy(legendX + 25, legendY + i * 22 + 2,
                  const_cast<char*>(ORDER[i].c_str()));
    }

    // ── Footer prompt ─────────────────────────────────────────────
    setcolor(DARKGRAY);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(WIN_W / 2 - 150, WIN_H - 35,
              const_cast<char*>(std::string("Press any key to close this window...").c_str()));

    // ── Wait for keypress then close ─────────────────────────────
    getch();
    closegraph();
}

#endif // DRAW_BAR_GRAPH_H
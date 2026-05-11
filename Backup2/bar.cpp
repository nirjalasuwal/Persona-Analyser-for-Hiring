#include<graphics.h>
#include<iostream>
#include<direct.h>
#include<conio.h>
using namespace std;

int main()
{

    initwindow(800, 400, "Bar Graph", 550, 50);
    //setbkcolor(WHITE);

    line(50, 350, 700, 350); // X-axis
    line(50, 350, 50, 50);   // Y-axis
    rectangle(110, 100, 170, 350); // Example bar
    rectangle(230, 250, 290, 350); // Another bar
    rectangle(350, 150, 410, 350); // Third bar
    rectangle(470, 200, 530, 350); // Fourth bar  
    rectangle(590, 50, 650, 350);  // Fifth bar
    outtextxy(110, 360, "Openness"); 
    outtextxy(200, 360, "Conscientiousness"); 
    outtextxy(340, 360, "Extraversion"); 
    outtextxy(450, 360, "Agreeableness"); 
    outtextxy(580, 360, "Neuroticism"); 
    outtextxy(20, 30, "Score");
    outtextxy(30, 350, "0");
    outtextxy(30, 280, "5");
    outtextxy(30, 210, "10");
    outtextxy(30, 140, "15");
    outtextxy(30, 70, "20");


    //cleardevice();
    getch();
    closegraph();
    return 0;
}
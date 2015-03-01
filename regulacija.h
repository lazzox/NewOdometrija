#ifndef REGULACIJA_H
#define	REGULACIJA_H

#ifndef FCY
#define FCY 29491200ULL
#endif

#define BIG     1
#define SMALL   2
#define ROBOT SMALL

#if (ROBOT == BIG)
#define PI	3.1415926535897932384626433832795
#define d_tocka	69.1978//69.19781//69.200//68.695// precnik odometrijskog tocka 69.34Levi
#define D_tocka	301.700//301.72//299.350//301.7//302//rastojanje izmedju tockova

//enkoder daje 5000 inkremenata po krugu
#define K1	(long)(40000.0f * D_tocka / d_tocka)  //broj ikremenata po krugu
#define K2	(long)(20000.0f / (d_tocka * PI))  //za konverziju mm u inkremente == 121.26
#define Gp_D	2.5//1.2//1.4//1.2//distanca
#define Gd_D	65//37//48//18
#define Gp_T	2.5//0.7//2.5//1.2 //0.8//rotacija
#define Gd_T	50//20//65//35//12




#elif ROBOT == SMALL
#define PI	3.1415926535897932384626433832795
#define d_tocka	51.975
#define D_tocka	168.340 //340  //169.7 bi trebalo   napisao Rade

//enkoder daje 5000 inkremenata po krugu
#define K1	(long)(2 * 4 * 2048.0f * D_tocka / d_tocka)  //broj ikremenata po krugu
#define K2	(long)(4 * 2048.0f / (d_tocka * PI))  //za konverziju mm u inkremente == 121.26
#define Gp_D	2.08
#define Gd_D	65
#define Gp_T	1.85
#define Gd_T	63
#endif

/*pROVERI ENKODER: Part no:MA5D1N4FBK1SA0; Type no.: sca24-5000-N-04-09-64-01-S-00*/

enum States
{
    STATUS_IDLE,
    STATUS_MOVING,
    STATUS_ROTATING,
    STATUS_STUCK,
    STATUS_ERROR
};

void test(void);

void resetDriver(void);
// zadavanje pozicije
void setPosition(int X, int Y, int orientation);

void sendStatusAndPosition(void);
//zadavanje brzine i ubrzanja
void setSpeedAccel(float v);

//funkcija za stizanje u tacku (Xd, Yd)
void gotoXY(int Xd, int Yd, unsigned char krajnja_brzina, char smer);

//funkcija za kretanje pravo s trapezoidnim profilom brzine
void kretanje_pravo(int duzina, unsigned char krajnja_brzina);

//funkcija za dovodjenje robota u zeljenu apsolutnu orjentaciju
void apsolutni_ugao(int ugao);

//funkcija za okretanje oko svoje ose s trapezoidnim profilom brzine
char okret(int ugao);//ne vraca indikaciju

//funkcija za kretanje po kruznoj putanji
void kurva(long Xc, long Yc, int Fi, char smer);

void stop(void);
void setSpeed(unsigned char tmp);

enum States getStatus(void);

void forceStatus(enum States);

#endif	/* REGULACIJA_H */


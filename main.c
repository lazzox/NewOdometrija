#define FCY 29491200ULL

#include "regulacija.h"
#include "uart.h"
#include "pwm.h"
#include <p33FJ128MC802.h>
#include <libpic30.h>


_FWDT (FWDTEN_OFF);
_FOSCSEL(FNOSC_PRI);	// primarni bez plla
_FOSC(FCKSM_CSECMD & OSCIOFNC_ON & POSCMD_HS & IOL1WAY_OFF);
_FPOR(PWMPIN_ON  & HPOL_ON & LPOL_ON & FPWRT_PWR128 & ALTI2C_ON);


void TimerInit(void)
{
    IEC0bits.T1IE = 0;      /* Disable the Timer1 interrupt */
    T1CONbits.TON = 0;      /* Disable timer1 */
    IFS0bits.T1IF = 0;      /* Clear Timer interrupt flag */

    T1CONbits.TGATE = 0;
    T1CONbits.TCKPS = 0;
    T1CONbits.TCS = 0;

    TMR1 = 0;
    PR1 = 29479;    // ide na 1ms

    IPC0bits.T1IP = 5; // prioritet prekida == 2
    IFS0bits.T1IF = 0;// Clear Timer1 Interrupt Flag
    IEC0bits.T1IE = 1;// Enable Timer1 interrupt
    T1CONbits.TON = 1;
}

void QEIinit()
{
    //konfigurisi registre:
    QEI1CONbits.POSRES=0;       //index impuls ne resetuje brojac
    QEI1CONbits.TQCS=1;         //brojac broji impulse sa QEA ulaza
    QEI1CONbits.UPDN_SRC=1;     //za to vreme QEB odredjuje smer brojanja
    QEI1CONbits.QEIM=6;         //Quadrature Encoder Interface enabled (x4 mode) with index pulse reset of position counter
    QEI1CONbits.TQCKPS=0;

    MAX1CNT=0000;
    POS1CNT=0;

    //konfigurisi registre:
    QEI2CONbits.POSRES=0;       //index impuls ne resetuje brojac
    QEI2CONbits.TQCS=1;         //brojac broji impulse sa QEA ulaza
    QEI2CONbits.UPDN_SRC=1;     //za to vreme QEB odredjuje smer brojanja
    QEI2CONbits.QEIM=6;         //Quadrature Encoder Interface enabled (x4 mode) with index pulse reset of position counter
    QEI2CONbits.TQCKPS=0;

    MAX2CNT=0000;
    POS2CNT=0;
}

void PortInit()
{

    TRISAbits.TRISA4=0;

    TRISBbits.TRISB8=0;
    TRISBbits.TRISB9=0;
    TRISBbits.TRISB10=0;
    TRISBbits.TRISB11=0;
    TRISBbits.TRISB12=0;
    TRISBbits.TRISB13=0;
    TRISBbits.TRISB14=0;
    TRISBbits.TRISB15=0;

    LATAbits.LATA4 = 0;

    LATBbits.LATB8 = 0;
    LATBbits.LATB9 = 0;
    LATBbits.LATB10 = 0;
    LATBbits.LATB11 = 1;
    LATBbits.LATB12 = 1;
    LATBbits.LATB13 = 0;
    LATBbits.LATB14 = 0;
    LATBbits.LATB15 = 0;
}


int main(void)
{
    int tmpX, tmpY, tmp, tmpO;
    char komanda, v, smer;

    /* Configure Oscillator to operate the device at 30Mhz
       Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
       Fosc= 7.37*(32)/(2*2)=58.96Mhz for Fosc, Fcy = 29.48Mhz */

    /* Configure PLL prescaler, PLL postscaler, PLL divisor */

    PLLFBDbits.PLLDIV = 30;   /* M = PLLFBD + 2 */
    CLKDIVbits.PLLPOST = 0;   /* N1 = 2 */
    CLKDIVbits.PLLPRE = 0;    /* N2 = 2 */
    //while (OSCCONbits.LOCK != 0b1);
    __builtin_write_OSCCONH(0b011);
    __builtin_write_OSCCONL (OSCCONL | (1<<0)); 	//OSWEN
    //wait for PLL lock -> wait to new settings become available
    while (OSCCONbits.COSC != 0b011);
    //wait for PLL lock
    while (OSCCONbits.LOCK != 0b1);

    AD1PCFGL = 0xFFFF;// all PORT Digital

    RPINR18bits.U1RXR = 0;		//UART1 RX na RP0- pin 4
    RPOR0bits.RP1R = 3;			//UART1 TX na RP1- pin 5
    RPINR14bits.QEA1R = 2;		//QEI1A na RP2
    RPINR14bits.QEB1R = 3;		//QEI1B na RP3

    RPINR16bits.QEA2R = 4;		//QEI2A na RP4
    RPINR16bits.QEB2R = 7;		//QEI2B na RP7

    //INTCON1bits.NSTDIS = 1; // zabranjeni ugnjezdeni prekidi
    
    PortInit();
    
    TimerInit();
    UART_Init(57600);
    QEIinit();
    CloseMCPWM();
    PWMinit();

    resetDriver();

    setSpeed(0x80);
    setSpeedAccel(K2);	//K2 je za 1m/s /bilo je 2

    while(1)
    {
        if(getStatus() == STATUS_MOVING)
            komanda = UART_GetLastByte();
        else
            komanda = getch();
    
        switch(komanda)
        {

            case 'X':
                test();
                break;
            // zadavanje pozicije
            case 'I':
                tmpX = (getch() << 8) | getch();
                tmpY = (getch() << 8) | getch();
                tmpO = (getch() << 8) | getch();

                setPosition(tmpX, tmpY, tmpO);

                break;

            // citanje pozicije i statusa
            case 'P':
                sendStatusAndPosition();

                break;

            //zadavanje max. brzine (default K2/2)
            case 'V':
                tmp = getch();
                setSpeed(tmp);

                break;

            //kretanje pravo [mm]
            case 'D':   
                tmp = getch() << 8;
                tmp |= getch();
                v = getch();

                PWMinit();
                kretanje_pravo(tmp, v);
		
                break;

            //relativni ugao [stepen]
            case 'T':   
                tmp = getch() << 8;
                tmp |= getch();

                PWMinit();
                okret(tmp);

                break;

            //apsolutni ugao [stepen]
            case 'A':   
                tmp = getch() << 8;
                tmp |= getch();

                PWMinit();
                apsolutni_ugao(tmp);

                break;

            //idi u tacku (Xc, Yc) [mm]
            case 'G':   
                tmpX = getch() << 8;
                tmpX |= getch();
                tmpY = getch() << 8;
                tmpY |= getch();
                v = getch();
                smer = getch();

                PWMinit();
                gotoXY(tmpX, tmpY, v, smer);

                break;

            //kurva
            case 'Q':  
                tmpX = getch() << 8;
                tmpX |= getch();
                tmpY = getch() << 8;
                tmpY |= getch();
                tmpO = getch() << 8;
                tmpO |= getch();
                smer = getch();

                PWMinit();
                kurva(tmpX, tmpY, tmpO, smer);

                break;

             //ukopaj se u mestu
            case 'S':
                stop();
       
                break;

            //stani i ugasi PWM
            case 's':
                stop();
                CloseMCPWM();

                break;

            default:
                forceStatus(STATUS_ERROR);
                break;
        }
    }

    return 0;
}

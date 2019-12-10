/*
 * File:   main.c
 * Author: daniele
 *
 * Created on 17 giugno 2017, 20.21
 */

// DSPIC33FJ128GP802 Configuration Bit Settings

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Mode (Internal Fast RC (FRC))

// FOSC
#pragma config POSCMD = NONE            // Primary Oscillator Source (Primary Oscillator Disabled)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function (OSC2 pin has clock out function)
#pragma config IOL1WAY = ON             // Peripheral Pin Select Configuration (Allow Only One Re-configuration)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config FWDTEN = OFF             // Watchdog Timer Enabled/disabled by user software
// (LPRC can be disabled by clearing SWDTEN bit in RCON register

#include <xc.h>
#include <libpic30.h>
#include <stdio.h>
#include <stdlib.h>
#include <p33FJ128GP802.h>
#include <libpic30.h>

#define FOSC    (80000000)
#define FP (FOSC/2)
#define BAUDRATE 19600
#define BRGVAL ((FP/BAUDRATE)/16)-1
#define DEFAULTPR1 0XFFFF
#define DEFAULTTCKPS 0B11

void oscillator_setup(void);
void uart_setup(void);
int read_line(char * s, int max_len);
void putch(char c);
char read_char(void);
void timer1_setup(void);
void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void);

int main(void) {
    oscillator_setup();
    uart_setup();
    printf("PIN: RA0\n\r");
    printf("Digitare un tasto per riconfigurare il timer.\n\r");
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;
    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 0;
    timer1_setup();
    while (1) {

    }
    return 0;
}

void oscillator_setup() {
    // Configure PLL prescaler, PLL postscaler, PLL divisor
    PLLFBD = 41; // M = 43
    CLKDIVbits.PLLPOST = 0; // N2 = 2
    CLKDIVbits.PLLPRE = 0; // N1 = 2
    OSCTUN = 0; // Tune FRC oscillator, if FRC is used
    // Disable Watch Dog Timer
    RCONbits.SWDTEN = 0;
    // Initiate Clock Switch to Internal FRC with PLL (NOSC = 0b001)
    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(0x01);
    // Wait for Clock switch to occur
    while (OSCCONbits.COSC != 0b001);
    // Wait for PLL to lock
    while (OSCCONbits.LOCK != 1) {
    };
}

void uart_setup() {
    AD1PCFGL = 0xFFFF; //all pins as digital
    TRISBbits.TRISB3 = 0; // TX as output
    TRISBbits.TRISB2 = 1; // RX as input

    RPINR18bits.U1RXR = 2; //U1RX on RP2 pin
    RPOR1bits.RP3R = 0b00011; //U1TX on RP3 pin

    U1MODEbits.STSEL = 0; // 1-stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8-data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud disabled
    U1MODEbits.BRGH = 0; // Standard-Speed mode
    U1BRG = BRGVAL; // Baud Rate setting for 9600
    U1MODEbits.UARTEN = 1; // Enable 
    U1STAbits.UTXEN = 1; // Enable UART TX
    __C30_UART = 1;
}

void putch(char c) {
    // wait the end of transmission
    while (U1STAbits.TRMT == 0) {
    };
    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag
    U1TXREG = c; // send the new byte
}

char read_char(void) {
    while (U1STAbits.URXDA == 0) { //UART1 Receiver Interrupt Flag Status bit
        if (U1STAbits.OERR == 1) {
            U1STAbits.OERR = 0;
        }
    }
    return U1RXREG;
}

int read_line(char * s, int max_len) {
    unsigned int i = 0;
    while (1) {
        char c = read_char();
        if (c == 13) {
            putchar(c);
            putchar(10);
            s[i] = 0;
            return i;
        } else if (c == 127 || c == 8) {
            if (i > 0) {
                putchar(c);
                putchar(' ');
                putchar(c);
                --i;
            }
        } else if (c >= 32) {
            if (i < max_len) {
                putchar(c);
                s[i] = c;
                ++i;
            }
        }
    }
    return i;
}

void timer1_setup(void) {
    T1CONbits.TON = 0; // Disable Timer
    T1CONbits.TCS = 0; // Select internal instruction cycle clock
    T1CONbits.TGATE = 0; // Disable Gated Timermode
    T1CONbits.TCKPS = DEFAULTTCKPS; // Select 1:1 Prescaler
    TMR1 = 0x0; // Clear timer register
    PR1 = DEFAULTPR1; // Load the period value
    //IPC0bits.T1IP = 0x01;// Set Timer1 Interrupt Priority Level
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
    IEC0bits.T1IE = 1; // Enable Timer1 interrup
    T1CONbits.TON = 1; // Start Timer
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void) { /* Interrupt Service Routine code goes here */
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
    LATAbits.LATA0 = !LATAbits.LATA0;
}

void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void) {
    char number[5];
    long unsigned int buff, cnt;
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 0;
    read_char();
    T1CONbits.TON = 0; // Stop Timer
    printf("Valori di default: PR1 = %u e T1CONbits.TCKPS = %u.\n\r", DEFAULTPR1, DEFAULTTCKPS);
    printf("Valori correnti: PR1 = %u e T1CONbits.TCKPS = %u.\n\r", PR1, T1CONbits.TCKPS);
    printf("L'equazione: 1/(40000000/TCKPS)*PR1 = (1/<frequenza desiderata>)/2\n\r");
    printf("TCKPS<1:0>: Timer Input Clock Prescale Select bits\n\r");
    printf("3 = 1:256 prescale value\n\r");
    printf("2 = 1:64 prescale value\n\r");
    printf("1 = 1:8 prescale value\n\r");
    printf("0 = 1:1 prescale value\n\r");
    printf("\n\rDigitare i valori dei registri PR1 e T1CONbits.TCKPS o lasciarli vuoti per non modificarli.\n\r");
    cnt = 0;
    while (cnt == 0) {
        printf("PR1 = ");
        cnt = read_line(number, 5);
        buff = atol(number);
        if (buff <= 0xffff) {
            if (cnt != 0) PR1 = buff;
            else cnt = 1;
        } else {
            printf("ERRORE: immettere un valore minore o uguale a %u\n\r", 0xFFFF);
            cnt = 0;
        }
    }
    cnt = 0;
    while (cnt == 0) {
        printf("T1CONbits.TCKPS = ");
        cnt = read_line(number, 1);
        buff = atol(number);
        if (buff <= 0b11) {
            if (cnt != 0) T1CONbits.TCKPS = buff;
            else cnt = 1;
        } else {
            printf("ERRORE: immettere un valore minore o uguale a %u\n\r", 0b11);
            cnt = 0;
        }
    }
    printf("\n\rPIN: RA0\n\r");
    printf("Digitare un tasto per riconfigurare il timer.\n\r");
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;
    T1CONbits.TON = 1; // Start Timer
}
/*
 * File:   newmain.c
 * Author: Jeffrey
 *
 * Created on October 25th, 2024
 */

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = OFF      // Master Clear Enable bit (MCLR pin function is port defined function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config BBSIZE = BB512   // Boot Block Size Selection bits (512 words boot block size)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration Register not write protected)
#pragma config WRTSAF = OFF     // Storage Area Flash Write Protection bit (SAF not write protected)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (Low Voltage programming disabled)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (UserNVM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "transmit.c"

#define _XTAL_FREQ 32000000 //Specify the XTAL crystal FREQ 32MHz

void main(void) {
//    IO ports initialization
//    All ports are input in the beginning, B6 will be PWM output later, A4 and A5 are output
    TRISA |= 0b00110111;
    TRISB |= 0b11110000;
    TRISC |= 0b11111111;
    TRISA4 = 0;
    TRISA5 = 0;
        
    LATA4 = 0;
    LATA5 = 1;
    
    ANSELA &= 0b11001000;
    ANSELB &= 0b00001111;
    ANSELC &= 0b00000000;
    
    WPUA |= 0b00111111;
    WPUB |= 0b11110000;
    WPUC |= 0b11111111;
    
//    A0, A1, A3 are unused, configure A0 and A1 as output and drive to low, MCLR and ICSP pins are strange to use
//    A3 is MCLR, cannot be used as output
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 0;
    
//    interrupt initialization
    GIE = 1;
    PEIE = 1;
    TMR0IE = 1;
    TMR1IE = 1;
    IOCIE = 1;
    
//    timer0 initialization
    T0EN = 0;
    T016BIT = 1;
    T0CON1 = 0b01000001;    // prescaler 1:2
    nxt_tx_state = IDLE;
    
//    timer1 initialization
    T1CON = 0b00000111;
    T1CLKbits.CS = 0b0110;  // 32kHz
    
//    ioc initialization
//A4 and A5 are output
    IOCAN = 0x04;
    IOCBN = 0xB0;   //B6 is used as pwm output
    IOCCN = 0xFF;
    
    pwm_initialization();
    
    while(1) 
    {
    }
    return;
}

void __interrupt() isr(void)
{
    if (TMR0IF) {
    //    Delay = ((65536-REG_val)*(Prescal*4))/Fosc
        TMR0IF = 0;
        switch (nxt_tx_state) {
            case IDLE:
                PWM3EN = 0;
                break;
            case HEADER_MARK:   //9ms
                if (button_pressed()) { // check if button is pressed after debounce
                    PWM3EN = 1;
                    set_TMR0_9ms();
                    nxt_tx_state = HEADER_SPACE;
                } else {
                    nxt_tx_state = IDLE;
                    T0EN = 0;
                }
                break;
            case HEADER_SPACE:  //4.5ms
                PWM3EN = 0;
                set_TMR0_4p5ms();
                proceed_to_next_data_bit();
                break;
            case LOGIC0_MARK:   //  564us
                PWM3EN = 1;
                set_TMR0_564us();
                nxt_tx_state = LOGIC0_SPACE;
                break;
            case LOGIC0_SPACE:  // 564us
                PWM3EN = 0;
                set_TMR0_564us();
                if (!sampler) {
                    nxt_tx_state = STOP_BIT;
                    break;
                }
                proceed_to_next_data_bit();
                break;
            case LOGIC1_MARK:   // 564us
                PWM3EN = 1;
                set_TMR0_564us();
                nxt_tx_state = LOGIC1_SPACE;
                break;
            case LOGIC1_SPACE:  // 1.692ms
                PWM3EN = 0;
                set_TMR0_1p692ms();
                if (!sampler) {
                    nxt_tx_state = STOP_BIT;
                    break;
                }
                proceed_to_next_data_bit();
                break;
            case STOP_BIT:  //564us
                PWM3EN = 1;
                set_TMR0_564us();
                nxt_tx_state = LEAD_OUT;
                break;
            case LEAD_OUT:  // padded to 108ms
                PWM3EN = 0;
                T0CON1bits.T0CKPS = 0b0100;    // prescaler 1:16

                set_TMR0_padding();
                nxt_tx_state = END;
                break;
            case END:
                T0CON1bits.T0CKPS = 0b0001;    // change prescaler back
//                check for button holding here
                if (button_pressed()){
                    // if any button is pressed(pulled down), send repeat signal
                    nxt_tx_state = REPEAT_MARK;
                    TMR0IF = 1;
                } else {
                    nxt_tx_state = IDLE;
                    T0EN = 0;
                }
                break;
            case REPEAT_MARK:   //9ms
                PWM3EN = 1;
                set_TMR0_9ms();
                nxt_tx_state = REPEAT_SPACE;
                break;
            case REPEAT_SPACE:  // 2.25ms
                PWM3EN = 0;
                set_TMR0_2p25ms();
                nxt_tx_state = REPEAT_STOP;
                break;
            case REPEAT_STOP:
                PWM3EN = 1;
                set_TMR0_564us();
                nxt_tx_state = REPEAT_LEAD_OUT;
                break;
            case REPEAT_LEAD_OUT:
                PWM3EN = 0;
                T0CON1bits.T0CKPS = 0b0100;    // prescaler 1:16

                set_TMR0_repeat_padding();
                nxt_tx_state = END;
                break;
            case DEBOUNCE:
                PWM3EN = 0;
                set_TMR0_4p5ms();
                nxt_tx_state = HEADER_MARK;
                break;
        }
    } else if (IOCIF && nxt_tx_state == IDLE) {
        if (IOCCF0) {
            IOCCF = 0;
            tx_data = switch_select(TV_VOL_N, TV_VOL_P);
        } else if (IOCAF2) {
            IOCAF = 0;
            tx_data = switch_select(TV_1, TV_2);
        } else if (IOCCF1) {
            IOCCF = 0;
            tx_data = switch_select(TV_DOWN, TV_UP);
        } else if (IOCCF2) {
            IOCCF = 0;
            tx_data = switch_select(TV_RETURN, TV_ENTER);
        } else if (IOCBF4) {
            IOCBF = 0;
            tx_data = switch_select(TV_POWER, TV_MUTE);
        } else if (IOCBF5) {
            IOCBF = 0;
            tx_data = switch_select(TV_RIGHT, TV_LEFT);
        } else if (IOCCF5) {
            IOCCF = 0;
            tx_data = switch_select(TV_3, TV_NETFLIX);
        } else if (IOCCF6) {
            IOCCF = 0;
            tx_data = switch_select(/*BUTTON01*/, /*BUTTON02*/);
        } else if (IOCCF7) {
            IOCCF = 0;
            tx_data = switch_select(/*BUTTON03*/, /*BUTTON04*/);
        } else if (IOCCF3) {
            IOCCF = 0;
            tx_data = switch_select(/*BUTTON05*/, /*BUTTON06*/);
        } else if (IOCBF7) {
            IOCBF = 0;
            tx_data = switch_select(/*BUTTON07*/, /*BUTTON08*/);
        } else if (IOCCF4) {
            IOCCF = 0;
            tx_data = switch_select(/*BUTTON09*/, /*BUTTON10*/);
        } else if (IOCAF3) {
//            pins that don't function properly
            IOCAF = 0;
            tx_data = switch_select(/*BUTTON11*/, /*BUTTON12*/);
        } else if (IOCAF0) {
            IOCAF = 0;
            tx_data = switch_select(/*BUTTON13*/, /*BUTTON14*/);
        } else if (IOCAF1) {
            IOCAF = 0;
            tx_data = switch_select(/*BUTTON15*/, /*BUTTON16*/);
        } else {
            return;
        }
        
        sampler = 0x80000000;
        nxt_tx_state = DEBOUNCE;
        T0EN = 1;
        TMR0IF = 1;
    } else if (TMR1IF) {
//        power A4 and A5 on and off every 49.12ms
        TMR1IF = 0;
        if (nxt_tx_state == IDLE && !button_pressed()){
            LATA ^= 0b00110000;        
        }
        TMR1H = 0xFA;   //49.12ms
        TMR1L = 0x00;
    }
}


#include "transmit.h"

int unsigned long switch_select(int unsigned long data_1, int unsigned long data_2){
//    when given 2 different NEC codes that represent 2 switches, return one based on
//    whether A4 or A5 is high
    if (PORTA & 0b00100000) {
        return data_2;
    } else {
        return data_1;
    }
}

void pwm_initialization() {
//    pwm initialization
/*
 * PWM Period
 F_pwm = 1/([(PR2)+1]*4*(TMR2 Prescale Value)/F_osc), given F_pwm = 38kHz  F_osc = 32MHz
 PR2 = F_osc/(F_pwm/4/prescaler)-1
 Prescaler = 1  PR2 = 0xD1
 * 
 * Duty Cycle
 Duty_cycle = (PWM3DC)/(4(PR2+1)), given Duty_cycle = 0.5
 PWM3DC = Duty_cycle*4*(PR2+1)
 PWM3DC = 0x1A5 = 0b1_1010_0101
 PWM3DCL[7:6] = 0b01
 */
    RB6PPS = 0x0B;  //RB6->PWM3:PWM3OUT;
    TRISBbits.TRISB6 = 0;

    T2CON = 0x00;
    T2CLKCON = 0x01;
    T2PR = 0xD1;
    T2TMR = 0x00;
    T2CONbits.TMR2ON = 1;
    
    PWM3CON = 0x80;    
    PWM3DCH = 0x69;      
    PWM3DCL = 0x40;
    
    PWM3EN = 0;

    TRISBbits.TRISB6 = 0;

}

int button_pressed() {
    //                if ( ((PORTA | 0xC0) & (PORTB | 0x4F) & PORTC) != 0xFF ){ //PORTA pin 0,1,3 interfere with the condition
    if ( ((PORTA | 0xFB) & (PORTB | 0x4F) & PORTC) != 0xFF ){
        return 1;
    } else {
        return 0;
    }
}

void proceed_to_next_data_bit(){
    if (tx_data & sampler){
        nxt_tx_state = LOGIC1_MARK;
    } else {
        nxt_tx_state = LOGIC0_MARK;
    }
    sampler >>= 1;
}
void set_TMR0_9ms(){
    //            TMR0 = 29536;
    TMR0H = 0x73;
    TMR0L = 0x60;
}
void set_TMR0_4p5ms(){
//            TMR0 = 47536;
    TMR0H = 0xB9;
    TMR0L = 0xB0;
}
void set_TMR0_1p692ms(){
//            TMR0 = 58768;
    TMR0H = 0xE5;
    TMR0L = 0x90;
}
void set_TMR0_564us(){
    //            TMR0 = 63280;
    TMR0H = 0xF7;
    TMR0L = 0x30;
}
void set_TMR0_2p25ms(){
    //  TMR0 = 56536;
    TMR0H = 0xDC;
    TMR0L = 0xD8;
}
void set_TMR0_padding(){
//    prescaler 1:16
//    1 - 564us*4 = 2.256ms
//    0 - 564us*2 = 1.128ms
//    tv delay (ms) 108-(9+4.5+(1.128*16+2.256*16)+0.564)= 39.79
//    set the lead out zero duration of the IR signal 
    TMR0H = 0xB2;
    TMR0L = 0x49;
}
void set_TMR0_repeat_padding(){
    //    repeat   (ms) 108-(9+2.25+0.564) =  96.186
    // prescaler 1:16
    //    TMR0 = 17443; //repeat
    TMR0H = 0x44;
    TMR0L = 0x23;
}

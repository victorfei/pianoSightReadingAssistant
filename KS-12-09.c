// DDS output thru PWM on timer0 OC0A (pin B.3)
// Mega644 version
// Produces a Karplus-Strong string sound
 
// works as intended only with uart_init()
// comment out BAUD RATE in uart.c
 
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>       // for rand
#include <stdio.h>
 
#include "keyboardInput_1207.c"
#include "uart.c"
 
// set up serial for debugging
//FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
 
//I like these definitions
#define begin {
#define end   } 
 
#define num_keys 14
#define nss 1 // num simul strings
#define ns  2  // num strings
#define np  2  // num pins
 
//Convert float to fix. a is a float
#define float2fix(a) ((int)((a)*256.0)) 
 
// Fast fixed point multiply assembler macro
#define multfix(a,b)              \
({                                \
int prod, val1=a, val2=b ;        \
__asm__ __volatile__ (            \ 
"muls %B1, %B2  \n\t"              \
"mov %B0, r0 \n\t"                 \ 
"mul %A1, %A2\n\t"                 \ 
"mov %A0, r1   \n\t"              \ 
"mulsu %B1, %A2 \n\t"          \ 
"add %A0, r0  \n\t"               \ 
"adc %B0, r1 \n\t"                \ 
"mulsu %B2, %A1 \n\t"          \ 
"add %A0, r0 \n\t"           \ 
"adc %B0, r1  \n\t"          \ 
"clr r1  \n\t"               \ 
: "=&d" (prod)               \
: "a" (val1), "a" (val2)      \
);                            \
prod;                        \
})
 
// tune the string by it's length - assumes FS = 16000, 8000 
 
// legend
  // C4
  // D4
  // E4
  // F4
  // G4
  // A4
  // B4
  // C5
  // D5
  // E5
  // F5
  // G5
  // A5
  // B5
 
// jOSE: works better with *2
//       keys sustain
 
int string_length;
int string_length_vect[num_keys] = {
  61,//30,  // 0    C4
  54,//27,  // 1    D4
  48,//24,  // 2    E4
  45,//22,  // 3    F4
  40,//20,  // 4    G4
  36,//18,  // 5    A4
  32,//16,  // 6    B4
  30,//122,//15,    // 7    C5
  27,//13,  // 8    D5
  24,//30,//12, // 9    E5
  23,//22,//11, // 10   F5
  20,//10,  // 11   G5
  18,//9,   // 12   A5
  16,//8,   // 13   B5
};
 
// in 8.8 fixed point format
    // 0.5 -> 128
signed int tune;
signed int tune_vect[num_keys] = {
  39,//147, // 0    C4
  124,//62,     // 1    D4
  138,//69,     // 2    E4
  208,//232,    // 3    F4
  208,//104,    // 4    G4
  93,//46,  // 5    A4
  101,//50,     // 6    B4
  147,//73,     // 7    C5
  61,//158, // 8    D5
  69,//34,  // 9    E5
  132,//232,//116,  // 10   F5
  104,//52,     // 11   G5
  46,//23,  // 12   A5
  50,//25,  // 13   B5
};
 
//
char note;
char last_note;
 
        volatile int  currentKeyPressed, lastKeyPressed; // could be char
        volatile char aKeyIsPressed; // only one key can be pressed
        volatile int  keys_Jose; // the bit i of keys_Jose is high if key i   is pressed
        volatile int  pluck, pushed ;
 
// time counts at 8000 Hz
// all string amplitudes are stored in 8.8 fixed point
volatile unsigned int time ;
volatile signed int string[256], last_tune_out, last_tune_in, lowpass_out ;
signed int tune, damping ;
volatile int ptrin, ptrout ;
volatile int isr_begin_time, isr_end_time;
 
// noise table 
volatile unsigned int i;
volatile signed int noise_table[256] ;
 
ISR (TIMER1_COMPA_vect) // Fs = 16000
begin 
    isr_begin_time = TCNT1;
    OCR0A = 128 + (string[ptrin]>>8) ; //sample
 
    // dyn determine str_length & tune
    if(aKeyIsPressed){
        string_length = string_length_vect[ currentKeyPressed ];
        tune          = tune_vect         [ currentKeyPressed ];
        if(lastKeyPressed != currentKeyPressed){
            ptrin  = 1;
            ptrout = 2;
        }
        damping = 0x00ff;
    }
    else{
        damping = 0x00f0;
    }
     
    // low pass filter
    lowpass_out = multfix(damping, (string[ptrin] + string[ptrout])>>1) ;
    // tuning all-pass filter
    string[ptrin] = multfix(tune,(lowpass_out - last_tune_out)) + last_tune_in ;
    // all-pass state vars
    last_tune_out = string[ptrin];
    last_tune_in = lowpass_out;
 
    //last_note = note;
    lastKeyPressed = currentKeyPressed;
     
    // update and wrap pointers
    if (ptrin==string_length) ptrin=1;
    else ptrin=ptrin+1;

    if (ptrout==string_length) ptrout=1; 
    else ptrout=ptrout+1;
 
    if (pluck){
        for (i=0; i<string_length; i++) begin
            string[i] = noise_table[i] ;
        end
        pluck = 0;
    }
 
     
    time++ ;
    isr_end_time = TCNT1;
end 
  
int main(void)
begin 
    
   // make B.3 an output
   DDRB = (1<<PORTB3) ;
    
   //init the UART -- uart_init() is in uart.c
    uart_init();

   // init the noise table
   for (i=0; i<256; i++) {
        // add a few terms to lowpass the noise a lilttle
        noise_table[i] = (rand() + rand() + rand() + rand()) >> 1 ;

   }
     
    ptrout = 2; // circular pointer 
    ptrin = 1; // circular pointer 
    last_tune_out = 0 ;
    last_tune_in = 0 ;
    string_length = 122;//122 ;
    tune = float2fix(0.578);//float2fix(0.12) ; //0.8
    damping = float2fix(0.98) ; // must be 0.5<damping<=1.0
    pluck = 0x00;
 
   // timer 0 runs at full rate
   TCCR0B = 1 ;
   // turn on PWM
   // turn on fast PWM and OC0A output
   // at full clock rate, toggle OC0A (pin B3) 
   // 16 microsec per PWM cycle sample time
   TCCR0A = (1<<COM0A0) | (1<<COM0A1) | (1<<WGM00) | (1<<WGM01) ; 
   OCR0A = 128 ; // set PWM to half full scale
    
   // timer 1 ticks at 8000 Hz or 125 microsecs period=2000 ticks
    OCR1A = 999 ; // 1000 ticks
    TIMSK1 = (1<<OCIE1A) ;
    TCCR1B = 0x09;  //full speed; clear-on-match
    TCCR1A = 0x00;  //turn off pwm and oc lines
     
    keysReadingSetup();
 
   // turn on all ISRs
   sei() ;
 
   // enable pull up resistor
   PORTC = 0x01 ;
   PORTD = 0x01 ;
 
   pushed = 0x00 ;
 
        keys_Jose = 0x0000;
    
   while(1) 
   begin  
     
        // Check pushbutton to pluck string
        // and oneshot it
        //   every 0.1 sec
         
        // connect one end of switch to PINC.0 and
        //  the other end to GND through a resistor
         
        // switch is active low
        // ~PINC & 0x01 is pressed
 
        char state = 0; // 0 - Strongly Not Pressed
                        // 1 - Weakly   Not Pressed
                        // 2 - Weakly       Pressed
                        // 3 - Strongly     Pressed
            // REPLACES pushed
        if(time == 800){
            packAndTransmit(keys);
        }
         
        if (time == 1600) {
            packAndTransmit(keys);
            // testing 2
 
                keys_Jose = 0x0000;
 
                currentKeyPressed = getKeyPressed();
 
                if      (currentKeyPressed == -1){
                        aKeyIsPressed = 0;
                        keys_Jose = 0x0000;
                }
                else if (currentKeyPressed < 16){
                        aKeyIsPressed = 1;
                        keys_Jose = ( 1 << currentKeyPressed);
 
                }
                else{
                    //fprintf(stdout, "error with currentKeyPre_Josessed");
                }
 
                // deboucer
                if ( aKeyIsPressed ){
                    if(state == 3){
                        //state = 3;
                        pluck = keys_Jose;
                    }
                    else{
                        state++;
                    }
                }
                else{
                    if(state == 0){
                        //pluck = 0;
                    }
                    else{
                        state--;
                    }
                }
 
                // experimental
                 //if (!pushed){
                 if( aKeyIsPressed && !pushed){
                    pluck  = keys_Jose;
                    pushed = keys_Jose;
                 }
                 if(!aKeyIsPressed && pushed){
                    pushed = 0;
                 }
                // end experimental

            time = 0;
        }
 
   end
 
end  //end main
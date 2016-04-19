 
#define F_CPU 16000000UL
/* UART baud rate */
#define UART_BAUD  57600
 
 
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
 
#include "uart.h"
#include "uart.c"
 
#define BIT6 0b01000000
 
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
 
volatile unsigned int cycles;
volatile unsigned int ticks;
 
int keys[14] = {0}; // TODO: JOSE changed 
char keyPressed[19];
 
/* setup the digital input ports for reading keys from the three shift registers
 * also setup the clock signal feeding the shift registers */
void keysReadingSetup()
{
 
  /*output compare 0A register Interrupt enable
   * Timer Interrupt Mask Register */
  TIMSK2= (1<<OCIE2A);  //turn on timer 1 cmp match ISR 
 
    /* set A0, A1, A2 to digital input */
    DDRA = 0;
 
  /* enable pull-ups */
  //PORTA |= (1<<PINA0) | (1<<PINA1) | (1<<PINA2);
 
    /* clock setup */
    /* Min Requirement: 
     * we want to read 10 inputs per sec. that means being able to sample 
     * all keys per 100ms (1s/10 = 0.1 sec). Since each shift register has
     * 8 inputs, we need 8 cycles to serially shift out 
     * all the keys. Thus total of 8 cycles is needed. Reads are doen in parallel
   * while the values are shfited out We are toggling the output
   * so we need 8*2=16 ticks. 100ms/16 = 6.25 ms. 
     * so the PWM must run with a period of 6.25ms = 160 Hz. or Greater 
     * than 160Hz */
 
  /* we use prescalar 1024, OCRA = 96 (total 97 tickss). 
   * 16e6 / (1024 * OCR2A) = 160 Hz */
  OCR2A = 96; 
  //OCR2A = 10; 
 
  /* prescalar = 1024, according to datasheet pg 160 */
  TCCR2B |=  (1 << CS20) | (1 << CS22); 
 
  /* WGM21 CTC, COM2A1=1 clear OC2A on Compare Match CTC Mode pg 156.*/
 
  /* COM2A0 =1 toggle OC2A (output) on compare match. pg 156
   * WGM21 = 1, CTC mode */
  TCCR2A = (1<<COM2A0) | (1<<WGM21);
 
  /* output the PWM wave form. PORT D.7 is OC2A */
  DDRD = (1<<PIND7)|(1<<PIND6);
  /* the latch pin output (active low) */
  PORTD |= (1 << PIND6);
 
  /* init uart */
  uart_init();
  sei();
 
  stdout = stdin = stderr = &uart_str;
 //fprintf(stdout,"\n\r ================ TRT 09feb09======================\n\r\n\r");
}
 
 
/* print keys for UART */
void printKeys()
{
  /* print keys out to screen */
  int i;
  fprintf(stdout, "[");
  for (i = 0; i < 8; i++)
  {
    fprintf(stdout, "%d ", keys[i]);
  }
  fprintf(stdout, "] [");
  for (i = 0; i < 8; i++)
  {
    fprintf(stdout, "%d ", keys[i+8]);
  }
  fprintf(stdout, "] ");
 
  fprintf(stdout, "\n");
 
}
 
/* return string for printing to screen */
char * getInputArray()
{
  int i;
  keyPressed[0] = '[';
  for (i = 0; i < 8; i++)
  {
    keyPressed[i+1] = keys[i];
  }
  keyPressed[9] = ']';
  keyPressed[10] = '[';
  for (i = 0; i < 8; i++)
  {
    keyPressed[i+10] = keys[i+8];
  }
  keyPressed[18] = ']';
 
  return keyPressed;
}
 
/* search through an array, if a key is pressed return the index of it. */
int getKeyPressed()
{
  int i;
  for (i = 0; i < 14; i ++)
  {
    if (keys[i] == 1)
      return i;
  }
  
  return -1;
}
 
void packAndTransmit(int * arr)
{
  int i;
  char packet[2] = {0};
  for (i = 0; i < 7; i++)
  {
    packet[0] += (1 << (6-i))*arr[i];
  }
  packet[0] &= 0x7f; 
   
  for (i = 7; i < 14; i++)
  {
    packet[1] += (1 << (13 -i))*arr[i];
  }
  packet[1] |= 0x80;
   
  //!!!!!!!! CHANGE %c to %d to print directly to UART
  fprintf(stdout, "%c%c", packet[0], packet[1]);
  //fprintf(stdout, "\n");
}
 
 
//**********************************************************
/* ISR generates signal for parallel enable input (active low) on the
 * shift register. Which enables the shift register to sample new values
 * every 9th cycle, meanwhile, the uC reads keys. ticks != cycle */
ISR (TIMER2_COMPA_vect) 
{ 
  /* HAVE TO re-enable interrupt so it does not mess with video generation */
  //sei();
  ticks++;
  /* hope reading is not too slow */
  /* read keys */
  cycles = ticks/2 + 1;
  if (cycles == 8)
    cycles = 0;
   
  /* D0 maps to keys[7], keys[0-7]*/
  keys[7-cycles] = !(PINA & 0x01);
  /* keys[8-15] */
  keys[15-cycles] = !((PINA >> 1) & 0x01);
  /* note[16-23] */
  //keys[23-cycles] = !((PINA >> 2) & 0x01);  
 
  if ((ticks == 14) || (ticks == 15))
  {
    /* output 1 to parralel enable in input (active low). To enable the input */
    PORTD &= ~BIT6; 
  }
  else if (ticks == 16)
  {
    ticks = 0;
 
    /* disable the parallel input. As values shifted out from the shift reg,
     read keys*/
    PORTD |= (1 << PIND6);
 
  }
}  
 
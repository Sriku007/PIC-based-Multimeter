/*
void main() {

  TRISA = 0;           // set direction to be output
  TRISB = 0;           // set direction to be output
  TRISC = 0;           // set direction to be output
  TRISD = 0;           // set direction to be output
  TRISE = 0;           // set direction to be output

  do {
    LATA = 0x00;       // Turn OFF LEDs on PORTA
    LATB = 0x00;       // Turn OFF LEDs on PORTB
    LATC = 0x00;       // Turn OFF LEDs on PORTC
    LATD = 0x00;       // Turn OFF LEDs on PORTD
    LATE = 0x00;       // Turn OFF LEDs on PORTE
    Delay_ms(1000);    // 1 second delay

    LATA = 0xFF;       // Turn ON LEDs on PORTA
    LATB = 0xFF;       // Turn ON LEDs on PORTB
    LATC = 0xFF;       // Turn ON LEDs on PORTC
    LATD = 0xFF;       // Turn ON LEDs on PORTD
    LATE = 0xFF;       // Turn ON LEDs on PORTE
    Delay_ms(1000);    // 1 second delay
  } while(1);          // Endless loop
}      */
// LCD module connections
sbit LCD_RS at RC6_bit;
sbit LCD_EN at RC7_bit;
sbit LCD_D4 at RB4_bit;
sbit LCD_D5 at RB5_bit;
sbit LCD_D6 at RB6_bit;
sbit LCD_D7 at RB7_bit;
sbit LCD_RS_Direction at TRISC6_bit;
sbit LCD_EN_Direction at TRISC7_bit;
sbit LCD_D4_Direction at TRISB4_bit;
sbit LCD_D5_Direction at TRISB5_bit;
sbit LCD_D6_Direction at TRISB6_bit;
sbit LCD_D7_Direction at TRISB7_bit;
// End LCD module connections

sbit Va at RA0_bit;
sbit Start at RB0_bit;
sbit Switch at RB1_bit;
sbit TestPin1 at RA1_bit;
sbit TestPin2 at RA5_bit;
sbit MOS_sig at RA6_bit;
sbit MOS_sign at RA4_bit;

char intro_msg[] = "PIC Tweezer";
char msg_c[] = "C Mode";
char msg_v[] = "V Mode";
char msg_r[] = "R Mode";
char msg_d[] = "D Mode";
char msg_on[] = "Short";
char msg_off[] = "Open";
char msg_gud[] = "Good";

unsigned int T_Value, Num;
unsigned short i, j, TimerValue, OverRange = 0;
char Capacitance[] = "00.000 uF";

unsigned int ADC_Value, DisplayVolt,v;
unsigned int dpv1;
char *volt = "00.0";

unsigned int select, update_select, test1, test2;

void interrupt(){
  if(PIR1.TMR2IF){
  TMR2 = TimerValue;
  Num ++;
  if(Num > 9999) OverRange = 1; // Range is 99.99 uF
  PIR1.TMR2IF =0; // Clear TMR0 interrupt flag
  }
}

void Display_Cap(unsigned int n){
 Capacitance[0] = n/10000 + 48;
 Capacitance[1] = (n/1000)%10 + 48;
 Capacitance[3] = (n/100)%10 + 48;
 Capacitance[4] = (n/10)%10 + 48;
 Capacitance[5] = (T_Value*10)/520 + 48;
 Lcd_Cmd(_Lcd_Clear);
 Lcd_Out(1, 1, "C = ");
 Lcd_Out(1, 5, Capacitance);

}

void reset(){
 TRISA = 0b00000100;
 CMCON = 7;
 RA1_bit = 0;
 Delay_ms(2000);
 TRISA = 0b00000110;
 CMCON = 2;
}

void debounce_delay() {
  Delay_ms(200);
}

void main(){
  /*CMCON = 0x07;   // Disable comparators
  ADCON1 = 0x0F;  // Disable Analog functions
  TRISC = 0x00;
  TRISB = 0x00;   */
//   char cap_size;
  CMCON = 0x07;
  ADCON1 = 0x0F;
  TRISB = 0b00000011;
  PORTB = 0;
  PORTA = 0;
  TRISA = 0b00000000;

  Lcd_Init();                      // Initialize LCD
  Lcd_Cmd(_LCD_CLEAR);             // CLEAR display
  Lcd_Cmd(_LCD_CURSOR_OFF);        // Cursor off
  Lcd_Out(1,2,intro_msg);
  select = 0;
  update_select = 1;

  do {
   if(!Switch){
     debounce_delay();
     update_select = 1;
       switch (select) {
         case 0 : select=1;
         break;
         case 1 : select=2;
         break;
         case 2 : select=3;
         break;
         case 3 : select=4;
         break;
         case 4 : select=1;
         break;
    } //case end
             }

  if (select == 1)
  {
      if(update_select==1){
            Lcd_Init();
            Lcd_Cmd(_Lcd_Clear);
            Lcd_Cmd(_LCD_CURSOR_OFF);
            Lcd_Out(1, 1, msg_c);
            update_select = 0;
      }

  TRISA = 0b00001110;
  //T2CON.T0CS = 0;
  RCON.IPEN = 0;
  INTCON.GIE = 1; //Enable global interrupt
  INTCON.PEIE = 1; //Enable peripheral interrupt
  MOS_sig = 0;
  MOS_sign = 1;
  //set the internal oscillator frequency
  OSCCON.IRCF2 = 1;
  OSCCON.IRCF1 = 1;
  OSCCON.IRCF0 = 0;
  OSCCON.SCS1 = 1;
  //set the timer initially
  PIE1.TMR2IE = 1;
  T2CON = 0;
  PIR1.TMR2IF =0;

  CMCON = 2;


  Va = 0;
  TimerValue = 177;  // 173 + 4 additional clock cycles delay on branching to ISR

  while(1){
       if(!Start) {
       Num = 0;
       OverRange =0;
       Lcd_Cmd(_Lcd_Clear);
       Lcd_Out(1, 1, "Testing.");
       Lcd_Out(2, 1, "...");
       TMR2 = TimerValue;       // Initialize Timer2
       Va = 1; //apply voltage
       T2CON.TMR2ON = 1; // start timer
       while(CMCON.C2OUT) {
        if(OverRange) break;
       }
       T2CON.TMR2ON = 0; // stop timer
       T_Value = TMR2 - TimerValue;   // T_Value is used for improved resolution
       Va = 0;
  //---------------------------------
  if(!OverRange){
    Display_Cap(Num*10);
  }
  else{
    OverRange = 0;
    Lcd_Cmd(_Lcd_Clear);
    Lcd_Out(1, 1, "Out of Range!");
   }
   reset();
  }
  else if (!Switch) {
      debounce_delay();
      select=2;
      update_select=1;
      break;

  }
 }

}


if (select==2)
 {
     if(update_select==1){
         Lcd_Init();
         Lcd_Cmd(_LCD_CLEAR);
         Lcd_Cmd(_LCD_CURSOR_OFF);
         Lcd_Out(1,1,msg_v);
         Lcd_Chr(2,10,'V');
         update_select=0;
     }

 //ADCON1 = 0x00;   // Reference voltage is Vdd
 CMCON = 0x07 ; // Disable comparators
 TRISC = 0b00000000; // PORTC All Outputs
 TRISA = 0b00001000; // PORTA All Outputs, Except RA3 and RA2
  MOS_sig = 0;
  MOS_sign = 1;
 do {
    if(!Start){
    
  ADCON1 = 0x00;
  ADC_Value = ADC_Read(3);

  DisplayVolt = (ADC_Value) * 2;

  volt[0] = DisplayVolt/1000 + 48;

  volt[1] = (DisplayVolt/100)%10 + 48;

  volt[3] = (DisplayVolt/10)%10 + 48;

  Lcd_Out(2,5,volt);

  delay_ms(500);   // Hold for 500 ms
  ADCON1 = 0x0F;
    }
 else if(!Switch){
      debounce_delay();
      select=3;
      update_select=1;
      break;
  }

 } while(1);
 }

if (select==3){

         if(update_select==1){
         Lcd_Init();
         Lcd_Cmd(_LCD_CLEAR);
         Lcd_Cmd(_LCD_CURSOR_OFF);
         Lcd_Out(1,1,msg_r);
        // Lcd_Chr(2,10,'O');
         update_select=0;
     }
   MOS_sig = 0;
  MOS_sign = 1;
 //ADCON1 = 0x00;   // Reference voltage is Vdd
 CMCON = 0x07 ; // Disable comparators
 TRISC = 0b00000000; // PORTC All Outputs
 TRISA = 0b00001010; // PORTA All Outputs, Except RA3 and RA2
 Va=1;
 do {
     if(!Start){
  ADCON1 = 0x00;
  ADC_Value = ADC_Read(1);

  v = (ADC_Value) / 2;

  dpv1 = 500 - v*2;
  DisplayVolt = (v*240)/dpv1;

  volt[0] = DisplayVolt/1000 + 48;

  volt[1] = (DisplayVolt/100)%10 + 48;

  volt[3] = (DisplayVolt/10)%10 + 48;

  Lcd_Out(2,5,volt);

  delay_ms(500);   // Hold for 500 ms
  ADCON1 = 0x0F;
     }
 else if(!Switch){
      debounce_delay();
      select=4;
      update_select=1;
      break;
  }

 } while(1);

}

if (select==4){
    //do nothing as of now
    delay_ms(500);
    if(update_select==1){
         Lcd_Init();
         Lcd_Cmd(_LCD_CLEAR);
         Lcd_Cmd(_LCD_CURSOR_OFF);
         Lcd_Out(1,1,msg_d);
         update_select=0;
     }

    while(1){
        if(!Switch){
            select=4;
      update_select=1;
      break;
        }
        else if(!Start)
        {
        MOS_sig = 1;
        MOS_sign = 0;
        TRISA = 0b00100000; // RA4 O/P, RA5 I/P
        TestPin1 = 1;
        test1 = TestPin2 ; // Read I/P at RA5
        TestPin1 = 0;
        TRISA = 0b00000010; // RA4 I/P, RA5 O/P
        TestPin2 = 1;
        test2 = TestPin1;
        TestPin2 = 0;
        MOS_sig = 0;
        if((test1==1) && (test2 ==1)){
        Lcd_Out(2,10,msg_on);
        }
        if((test1==1) && (test2 ==0)){
        Lcd_Out(2,10,msg_gud);
        }
        if((test1==0) && (test2 ==1)){
        Lcd_Out(2,10,msg_gud);
                       }
        if((test1==0) && (test2 ==0)){
        Lcd_Out(2,10,msg_off);
        }}

}          }

}while(1);

}

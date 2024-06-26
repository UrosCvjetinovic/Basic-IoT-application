/*
 * Project name:
     clicker 2 for STM32 (Simple 'Hello World' project)
 * Copyright:
     (c) Mikroelektronika, 2014.
 * Revision History:
     20140924:
       - initial release (FJ);
 * Description:
     This is a simple 'Hello World' project. It turns on/off LEDs connected to
     PE12 and PE15 pins depending on pressed buttons. Left button T1 changes mode
     of blinking and right button changes frequency of blinking.
 * Test configuration:
     MCU:             STM32F407VG
                      http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00037051.pdf
     Dev.Board:       clicker 2 for STM32 - ac:clicker_2_STM32
                      http://www.mikroe.com/stm32/clicker-2/
     Oscillator:      HSI-PLL, 120.000MHz
     Ext. Modules:    None.
     SW:              mikroC PRO for ARM
                      http://www.mikroe.com/mikroc/arm/
 */

// pin definitions
sbit LD1 at ODR12_GPIOE_ODR_bit;
sbit LD2 at ODR15_GPIOE_ODR_bit;

// globals
char oldstate1 = 0, oldstate2 = 0;
char Example_State = 0;
char count;

//Timer2 Prescaler :959; Preload = 62499; Actual Interrupt Time = 500 ms
void InitTimer2(){
  RCC_APB1ENR.TIM2EN = 1;
  TIM2_CR1.CEN = 0;
  TIM2_PSC = 959;
  TIM2_ARR = 62499;
  NVIC_IntEnable(IVT_INT_TIM2);
  TIM2_DIER.UIE = 1;
  TIM2_CR1.CEN = 1;
}

// ISR
void Timer2_interrupt() iv IVT_INT_TIM2 {
char temp;
  TIM2_SR.UIF = 0;
  // check T1 button state
  switch (Example_State & 0x0F){
    case 0 : LD1 = 0;                     // Both LEDs are OFF
             LD2 = 0;
             break;
    case 1 : LD1 ^= 1;                    // Only LD1 blinks
             LD2 = 0;
             break;
    case 2 : LD1 = 0;                     // Only LD2 blinks
             LD2 ^= 1;
             break;
    case 3 : LD1 ^= 1;                    // Both LEDs blinks alternately
             LD2  = !LD1;
             break;
    case 4 : LD1 ^= 1;                    // Both LEDs blink simultaneously
             LD2  = LD1;
             break;
    default : Example_State &= 0xF0;      // reset T1 state to zero
              break;
  }
  // check T2 button state
  switch (Example_State & 0xF0){
    case 0x00 : TIM2_PSC = 959;           // Set Timer2 Interrupt time to 500ms
                TIM2_ARR = 62499;
                break;
    case 0x10 : TIM2_PSC = 749;           // Set Timer2 Interrupt time to 400ms
                TIM2_ARR = 63999;         
                break;
    case 0x20 : TIM2_PSC = 575;           // Set Timer2 Interrupt time to 300ms
                TIM2_ARR = 62499;
                break;
    case 0x30 : TIM2_PSC = 374;           // Set Timer2 Interrupt time to 200ms
                TIM2_ARR = 63999;
                break;
    case 0x40 : TIM2_PSC = 191;           // Set Timer2 Interrupt time to 100ms
                TIM2_ARR = 62499;
                break;
    default :   TIM2_PSC = 959;           // Set Timer2 Interrupt time to 500ms
                TIM2_ARR = 62499;
                Example_State &= 0x0F;    // reset T1 state to zero
                break;
  }
}

// main function
void main() {
  // Set GPIO_PORTE pin 0 as digital input
  GPIO_Digital_Input(&GPIOE_BASE, _GPIO_PINMASK_0);

  // Set GPIO_PORTA pin 10 as digital input
  GPIO_Digital_Input(&GPIOA_BASE, _GPIO_PINMASK_10);

  // Set GPIO_PORTE pins 12 and 15 as digital output
  GPIO_Digital_Output(&GPIOE_BASE, _GPIO_PINMASK_12 | _GPIO_PINMASK_15);

  LD1 = 0;                  // turn off LEDs
  LD2 = 0;
  
  Example_State = 0;        // set default Example state

  InitTimer2();             // initialize Timer2

  while(1){                 // Endless loop
    // check T1 button
    if (Button(&GPIOE_IDR, 0, 2, 0)) {            // Detect logical zero
      oldstate1 = 1;                              // Update flag
    }
    if (oldstate1 && Button(&GPIOE_IDR, 0, 2, 1)) {   // Detect zero-to-one transition
      oldstate1 = 0;                              // Update flag
      Example_State += 0x01;                      // set new Example state
      if ((Example_State & 0x0F) > 4)
        Example_State &= 0xF0;
    }
    // check T2 button
    if (Button(&GPIOA_IDR, 10, 2, 0)) {           // Detect logical zero
      oldstate2 = 1;                              // Update flag
    }
    if (oldstate2 && Button(&GPIOA_IDR, 10, 2, 1)) {   // Detect zero-to-one transition
      oldstate2 = 0;                              // Update flag
      Example_State += 0x10;                      // set new Example state
      if ((Example_State & 0xF0) > 0x40)
        Example_State &= 0x0F;
    }
  }
}
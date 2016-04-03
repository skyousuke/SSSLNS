
/** 
 * Author: Surasek Nusati (S-Kyousuke)
 * 
 */

#include "all.h"
#include "config.h"

#define MAX_PAGE 4

#define TRIS_OUTPUT 0
#define TRIS_INPUT 1

#define SW_TRIS _TRISA2

#define RELAY_SOURCE _LATA6
#define RELAY_SOURCE_TRIS _TRISA6
#define SOURCE_GRID 0
#define SOURCE_BATTERY 1

#define RELAY_LED _LATA7
#define RELAY_LED_TRIS _TRISA7
#define TURN_OFF_LED 0
#define TURN_ON_LED 1

#define LDR_ACTIVE_VOLTAGE 1.60  //Get from experiment only

unsigned char page = 0;

// interrupt 4 service 
void __attribute__ ((interrupt, no_auto_psv)) _INT3Interrupt(void) { 
    ++page;
    if(page == MAX_PAGE)
        page = 0;
    _INT3IF = 0;
}

void init() {
    // Clock Initialize
    Clock_Init();    
    Timer_Init();
    _NSTDIS = 1;  //Interrupt Nesting Disable    
            
    // Digital Communication Initialize
    UART1_Init(); 
    ADC_Init();
    I2C1_Init();    
            
    // Peripherals Initialize
    LCD_Init();
     
    // Interrupt 3 & 4 Initialize    
    SW_TRIS = TRIS_INPUT; // RA2/INT3 as input;
    
    _INT3IP = 1;
    _INT3IF = 0;
    _INT3IE = 1;
    _INT3EP = 1; //Interrupt on negative edge
    
    // Make relay drive port as output
    RELAY_SOURCE_TRIS = TRIS_OUTPUT; 
    RELAY_LED_TRIS = TRIS_OUTPUT;
    
    // Select battery as voltage source and turn on LED light
    RELAY_SOURCE = SOURCE_BATTERY; 
    RELAY_LED = TURN_ON_LED;
}

void LCD_Update() {
    _INT3IP = 0;    // Disable INT3 for temporary    
    AdcData data = ADC_GetData();
    LCD_Home();
    switch(page) {
        case 0:
            LCD_Puts("Load Data           ");
            LCD_Row(2);
            sprintf(lcdChar, "V:%7.3f  I:%5.3f  ", data.vLoadRms, data.iLoadRms);
            LCD_Puts(lcdChar);
            LCD_Row(3);
            sprintf(lcdChar, "P:%7.3f PF:%5.3f  ", data.pLoad, data.pfLoad);
            LCD_Puts(lcdChar);
            break;
            
        case 1:            
            LCD_Puts("PV Data             ");
            LCD_Row(2);
            sprintf(lcdChar, "V:%6.3f I:%6.3f   ", data.vPV, data.iPV);
            LCD_Puts(lcdChar); 
            LCD_Row(3);
            LCD_Puts("                    ");
            break;
            
        case 2:                        
            LCD_Puts("Battery Data        ");
            LCD_Row(2);
            sprintf(lcdChar, "V:%6.3f I:%6.3f   ", data.vBatt, data.iBatt);
            LCD_Puts(lcdChar);         
            LCD_Row(3);
            LCD_Puts("                    ");
            break;
            
        case 3:
            LCD_Puts("Date & Time         ");
            LCD_Row(2);
            LCD_Puts(RTCC_GetLcdString());         
            LCD_Row(3);
            LCD_Puts("                    "); 
            break;
            
        default:
            break;
    }
    _INT3IP = 1; // Re-enable INT3
}

int main(void) {
    init();
  
    while(1) {        
        LCD_Update();
        SDC_Update();
        RELAY_LED = (ADC_GetLdrVolt() <= LDR_ACTIVE_VOLTAGE)? TURN_OFF_LED : TURN_ON_LED;           
    }             
    return 0;
}
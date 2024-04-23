//-----------------------------------------------------------------------------
#include <C8051F040.h>                 			// SFR declarations
#include <stdio.h>
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define SYSCLK		22118400L					// system clock =22.118400MHz

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F04x
//-----------------------------------------------------------------------------
sfr16 ADC0	   = 0xBE;				   			// ADC0 data
//sfr16 DAC0     = 0xD2;				   			// DAC0 data
//sfr16 DAC1     = 0xD2;				   			// DAC1 data
//sfr16 RCAP4    = 0xCA;				   			// Timer4 capture/reload
//sfr16 TMR4     = 0xCC;                // Timer4
//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void OSCILLATOR_Init (void);
void PORT_Init (void);

void UART01_Init (void);
void ADC0_Init(void);

void SMB0_Init(void);

//void DAC0_Init(void);
//void DAC1_Init(void);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

int Ram_tp = 0;
int AD_Ram[35] = {0x0010, 0x0011, 0x0014, 0x0016, 0x0018, 0x0232, 0x0232, 0x0232, 0x0018, 0x0232, 0x0232, 0x0232,
                  0x001C, 0x001E, 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9,
                  0x00FA, 0x00FB, 0x0192, 0x0195, 0x0196, 0x019B, 0x01E0, 0x01E1, 0x0232, 0x0232, 0x0232};
char Ram_Value[35] = {0x7C, 0x02, 0x01, 0x09, 0x06, 0x00, 0x01, 0x00, 0x06, 0x00, 0x01, 0x00,
                      0x02, 0x16, 0x65, 0x65, 0x65, 0x65, 0x65, 0x65, 0xA4, 0x65, 0x65, 0x65,
                      0x65, 0x65, 0x04, 0x04, 0xFF, 0x04, 0x02, 0x22, 0x00, 0x01, 0x00};

char UART0_rbuf[64];			//UART0 variable define
int UART0_rp=0,UART0_tp=0;
int UART0_TX_flag=0;
long i = 0, j = 0, k = 0, t = 0;
char Byte;

char SMB_rbuf[64];  //SMB variable define used for I2C operation
char SMB_rp=0;
char SMB_Adr=0x58;      //SMB_Adr=0x59;

int ADC0_buf = 0;						   			//ADC0: temperature buffer
int ADC0_min,ADC0_max;
float ADC0_mid;

long ADC0_x = 0;
int UART0_tbuf = 0;


//-----------------------------------------------------------------------------
// main() Routine
//-----------------------------------------------------------------------------

void main (void)
{
		
		char SFRPAGE_SAVE = SFRPAGE;
		float x;

		SFRPAGE = CONFIG_PAGE;			   			// page for watchdog set
		WDTCN = 0xDE;                      // Disable watchdog timer
		WDTCN = 0xAD;
		SFRPAGE = SFRPAGE_SAVE; 					  // restore default page
		
		OSCILLATOR_Init ();                 // Initialize oscillator
		PORT_Init ();                       // Initialize crossbar and GPIO
		UART01_Init ();                      // Initialize UART0	
		ADC0_Init();								// Initialize ADC0
        SMB0_Init();                                // Initialize SMBus (I2C)
	
		EA = 1;
//-----------------------------------------------------------------------------
	printf("%s\n","Configure AD9520_0 registers.");
//	if (getchar() == 'W')
	{
// Configure First AD9520
        SMB_Adr=0x59;
        for(k = 0; k <= 34; k++)
        {
                Ram_tp = 1;
                SFRPAGE = SMB0_PAGE;
                STA=1;                                      // Start I2C TX
                SFRPAGE = SFRPAGE_SAVE;
                for (i=0; i<22119; i++)
                {
                        if(Ram_tp == 0) break;
                }
        }
//-----------------------------------------------------------------------------
// Configure Second AD9520
        SMB_Adr=0x58;
        for(k = 0; k <= 34; k++)
        {
                Ram_tp = 1;
                SFRPAGE = SMB0_PAGE;
                STA=1;                                      // Start I2C TX
                SFRPAGE = SFRPAGE_SAVE;
                for (i=0; i<22119; i++)
                {
                        if(Ram_tp == 0) break;
                }
        }
	}
	printf("%s\n","AD9520_0 registers are set.");
//-----------------------------------------------------------------------------
			
		while (1)
		{
			ADC0_min = 0x1000;
			ADC0_max = 0x0000;

			ADC0_x = 0;
			for(j = 0; j < 16384; j++)
			{
					SFRPAGE = ADC0_PAGE;								// set ADC0 page
					AD0BUSY=1;	
					while (AD0BUSY) {}
					ADC0_x += (long)ADC0_buf;
					if (ADC0_buf < ADC0_min)
					{
						ADC0_min = ADC0_buf;
					}
					if (ADC0_buf > ADC0_max)
					{
						ADC0_max = ADC0_buf;
					}
			}
//			x=ADC0_buf;					 //Current value
//			printf("%f %s",x,"	");
//			x=ADC0_min;	 				 //Minium value
//			printf("%f %s",x,"	");
			x=ADC0_x/16384.0;			 //Mean value
			printf("%f\n",x);
		}
}



//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------
// OSCILLATOR_Init. This function initializes the system clock to 22.1184MHz
//-----------------------------------------------------------------------------
void OSCILLATOR_Init(void)
{
		char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   	SFRPAGE = CONFIG_PAGE;              // Set SFR page
   	CLKSEL = 0x00;                      // Select the internal osc as system clock  
		OSCICL = OSCICL+0x0E; 				// set SYSCLK=22.1184MHz???????????
		OSCICN = 0xC3;						// selecte internal clock and divided by 1
// Initialize external crystal oscillator.
   	OSCXCN = 0x67;                      // Enable external crystal osc (22.1184MHz)
//	OSCXCN = 0x77;                      // Enable external crystal osc/2.
		for (i=0; i<22119; i++);        	// Wait at least 1 ms. At 22.1184 MHz, 1 ms = 22119 SYSCLKs
   	while (!(OSCXCN & 0x80));           // Wait for crystal osc to settle
   	CLKSEL = 0x01;                      // Select external crystal as system clock
    									// use CLKSEL select internal or external OSC                                   
   	SFRPAGE = SFRPAGE_SAVE;             // Restore SFRPAGE
}
//-----------------------------------------------------------------------------
// PORT_Init
// configuration: P0.0=TX0,P0.1=RX0,P0.2=SDA,P0.3=SCL,P0.4=TX1,P0.5=RX1,P0.6=CEX0,P0.7=ECI
//                P1.0=T2,P1.1=T2EX,P1.2=T3,P1.3=T3EX,P1.4=T4,P1.5=T4EX
//-----------------------------------------------------------------------------
void PORT_Init (void)
{
   	char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   	SFRPAGE = CONFIG_PAGE;              // Set SFR page
	//P0MDOUT |= 0x01;                    // P0.6 is configured as push-pull
    XBR0  = 0x05;				    	// enable UART0 and I2C routed to port
    XBR1  = 0x00;						// enable T2 and T2EX routed to port 
    XBR2  = 0x40;  					// crossbar enable, UART1,T4 and T4EX routed to port
    XBR3  = 0x00;	
	SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}
//-----------------------------------------------------------------------------
// ADC0 initialize to 	operate temperature sensor
void ADC0_Init(void)
{
		char SFRPAGE_SAVE = SFRPAGE;		// Save Current SFR page
		SFRPAGE = ADC0_PAGE;
		ADC0CN = 0xC0;	 					// enable ADC0  
		ADC0CF = 0xC0;						// AD0SC4:AD0SC0=24,set CLKsar=SYSYCLK/(AD0SC+1)=884,736Hz,gain=1
//		ADC0CF = 0xC4;						// AD0SC4:AD0SC0=24,set CLKsar=SYSYCLK/(AD0SC+1)=884,736Hz,gain=16
		REF0CN = 0x03;						// Disable temperature sensor,select referrence voltage and bias
		AMX0CF = 0x00;						// single-ended input 
		AMX0SL = 0x01;						// Select ADC0 channel 0000 0001 0010 0011 AIN0-3.
//		AMX0CF = 0x01;						// AIN0.0-AIN0.1
//		AMX0SL = 0x00;						// 
		EIE2 |= 0x01<<1;				 	// ADC0 interrupt enabled
   	SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page		
}
//-----------------------------------------------------------------------------
// UART0 and UART1_Init
//-----------------------------------------------------------------------------
// Configure UART0 and UART1 using Timer1, for baudrate and 8-N-1.
// <baudrate>: UART0 = 9600, UART1 = 76,800. also enable Timer0 interrupt.
//-----------------------------------------------------------------------------
void UART01_Init (void)		
{
   	char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   	SFRPAGE = TIMER01_PAGE;
		PCON	=0x00;						// Power on and not idle mode
		TCON	=0x45;						// TIMER0 is used for timing and TIMER1 is used for UART0 and UART1
		TMOD	=0x22;						// TIMER0 and TIMER1 with auto reload mode
		CKCON	=0x02;						// TIMER0 and TIMER1 use SYSCLK/48 clock
		TH0	=0x00;							// TIMER0 output:T0CLK=SYSCLK/48/(256-TH0)=1800Hz.
										// TIMER1 output:T1CLK=SYSCLK/48/(256-TH1)=153600Hz.
		TH1	= 0xFD;							// UART0 Baudrate =T1CLK/16 = 9600 at double clock
//	TH1	= 0xFE;							// UART0 Baudrate =T1CLK/16 = 14400 at double clock
		ET0 = 0;							// TIMER0 interrupt enabled 

   	SFRPAGE = UART0_PAGE;
   	SCON0   = 0x50;						// UART0 8 bits mode,TIMER1 as baudrate generator
   	SSTA0 	= 0x10;                     // Double UART0 baudrate
 
		ES0 = 0;							// UART0 interrupt enabled 
		TI0 = 1;
   	SFRPAGE = UART1_PAGE;
   	SCON1   = 0x10;						// UART1 8 bits mode, UART1 Baudrate= T1CLK/2 = 76,800
//	EIE2 |= 0x01<<6;				 	// UART1 interrupt enabled
   	SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}
//-----------------------------------------------------------------------------
// SMB0 initialization, i.e. I2C communication
void SMB0_Init(void)
{
        char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
        SFRPAGE = SMB0_PAGE;
        SMB0CN = 0x44;                      // enable SMB0, set AA =1
        SMB0ADR = SMB_Adr << 1; // slave address of this node and General Call Address Enable.
        SMB0CR = 0x00;                      // clock rate preset high=11.57us,low=12.28us, baudrate 42kbps
        EIE1 |= 0x01<<1;                    // SMB0 interrupt enabled
    SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}
//-----------------------------------------------------------------------------
//get a character from UART0_rbuf.
char getchar()
{
    char SFRPAGE_SAVE = SFRPAGE;
	SFRPAGE = UART0_PAGE;
    while (RI0 != 1)	{}
	RI0=0;
	Byte = SBUF0;
    SFRPAGE = SFRPAGE_SAVE;
    return (Byte);
}
//==============================================
char putchar (char c)
{
    char SFRPAGE_SAVE = SFRPAGE;
	SFRPAGE = UART0_PAGE;
    while (TI0 != 1)	{}
	TI0=0;
	SBUF0 = c;
    SFRPAGE = SFRPAGE_SAVE;
    return c;
}

// UART0 is used to input and output a character.
void UART0_ISR (void) interrupt 4
{
		if(TI0)
		{
				TI0 = 0;

		} 
		if(RI0)
		{
				RI0 = 0;
		}
}
//-----------------------------------------------------------------------------
// ADC0 is used to sense temperature
void ADC0_ISR (void) interrupt 15
{
		if(AD0INT)
		{
			ADC0_buf = ADC0;
			AD0INT= 0;
		}
}
//-----------------------------------------------------------------------------
//------------------------------------------------------------------------------
// I2C interrupt
void SMB0_ISR (void) interrupt 7
{
    switch (SMB0STA)
    {
        case 0x00:          //Bus Error (illegal START or STOP)
        STO = 1;            //Set STO to reset SMBus
        break;

        case 0x08:          //START condition transmitted. Action: Load SMB0DAT with Slave Address + R/W. Clear STA.
        SMB0DAT = SMB_Adr << 1; //Slave address (bit7:bit1) +R/W (bit0=0=W,bit0=1=R).
        STA=0;              //Slave address 3F may be replaced. It depends.
        break;

        case 0x10:          //Repeated START condition transmitted. Action: Load SMB0DAT with Slave Address + R/W.
        SMB0DAT = SMB_Adr << 1;     
        STA=0;              //Clear STA.
        break;

//The typical Master Transmitter action. 
//Sequence��Start��SLA��WR��ACK��DATA��ACK��DATA��ACK��STOP
//TX/RX:    TX     TX   TX  RX   TX    RX   TX    RX   TX
        case 0x18:          //Slave Address + W transmitted. ACK received.      
        SMB0DAT = AD_Ram[k] >> 8;      //Action: Load SMB0DAT with data to be transmitted.
        break;

        case 0x20:          //Slave Address + W transmitted. NACK received. Action: Set STO + STA.
        STO = 1;
//      STA=1;              //Set STA will restart transfer.
        break;

        case 0x28:          //Data byte transmitted. ACK received.
        if(t == 0){
                SMB0DAT = AD_Ram[k] & 0x00FF;
                t = 1;
        }
        else if(t == 1){
                SMB0DAT = Ram_Value[k];
                t = 2;
        }
        else{           
                t = 0;
                Ram_tp = 0;
                STO = 1;                        //Set STA will restart transfer.
        }
        break;

        case 0x30:          //Data byte transmitted. NACK received. Action: 1) Retry transfer
        STO=1;              //OR 2) Set STO to reset SMBus
        break;

        case 0x38:          //Arbitration Lost. Action: Save current data
        STO=1;              //Set STO to reset SMBus
        break;

//The typical Master Receiver action.
//Sequence��Start��SLA��RD��ACK��DATA��ACK��DATA��NACK��STOP
//TX/RX:    TX     TX   TX  RX   RX    TX   RX    TX    TX
        case 0x40:          //Slave Address + R transmitted. ACK received.
        break;              //If only receiving one byte, clear AA (send NACK after received byte). Wait for received data.

        case 0x48:          //Slave Address + R transmitted. NACK received.
        STO=1;              //Set STO to reset SMBus
//      STA=1;              //Acknowledge poll to retry. Set STO + STA
        break;

        case 0x50:          //Data byte received. ACK transmitted.
            SMB_rbuf[SMB_rp] = SMB0DAT;
            SMB_rp=(SMB_rp+1) & 0x3f;
        break;

        case 0x58:          //Data byte received. NACK transmitted.
        STO=1;              //Set STO to reset SMBus
        AA=1;
        break;

//The typical slave receiver action.
//Sequence��Start��SLA��WR��ACK��DATA��ACK��DATA��ACK��STOP
//TX/RX:    RX     RX   RX  TX   RX    TX   RX    TX   RX
        case 0x60:          //Own slave address + W received. ACK transmitted.
        break;              //Action: wait for data

        case 0x68:          //Arbitration lost in sending SLA + R/W as master. Own address + W received. ACK transmitted
        break;              //Action: Save current data for retry when bus is free. Wait for data       

        case 0x70:          //General call address received. ACK transmitted.
        break;              //Action: wait for data

        case 0x78:          //Arbitration lost in sending SLA + R/W as master. General call address received. ACK transmitted
        break;              //Action: Save current data for retry when bus is free.

        case 0x80:          //Data byte received. ACK transmitted.
        break;

        case 0x88:          //Data byte received. NACK transmitted.     
        break;

        case 0x90:          //Data byte received after general call address. ACK transmitted.
        break;

        case 0x98:          //Data byte received after general call address. NACK transmitted.      
        break;

        case 0xA0:          //STOP or repeated START received.  
        break;              //No action necessary       

//Typical slave transmitter action.
//Sequence��Start��SLA��RD��ACK��DATA��ACK��DATA��NACK��STOP
//TX/RX:    RX     RX   RX  TX   TX    RX   TX    RX    RX
        case 0xA8:          //Own address + R received. ACK transmitted.
        break;

        case 0xB0:          //Arbitration lost in transmitting SLA + R/W as master. Own address + R received. ACK transmitted.     
        break;

        case 0xB8:          //Data byte transmitted. ACK received.
        break;

        case 0xC0:          //Data byte transmitted. NACK received.  
        break;              //Wait for STOP 

        case 0xC8:          //Last data byte transmitted (AA=0). ACK received.    
        break;

        case 0xD0:          //SCL Clock High Timer per SMB0CR timed out
        STO=1;              //Set STO to reset SMBus
        break;

        case 0xF8:          //State does not set SI
                            //No operation
        break;      

        default:
        break;
    }
    SI=0;                  //Clear Interrupt
}
//-----------------------------------------------------------------------------



/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

}
// End Of File

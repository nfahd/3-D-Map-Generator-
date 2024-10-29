// NAME: Nuha Fahad
// ASSIGNED BUS SPEED: 30 MHz
// ASSIGNED MEASUREMENT STATUS PIN: PF4
// ASSIGNED ADDUTIONAL STATUS PIN: PF0


#include <stdint.h>
#include "Systick.h"
#include "PLL.h"
#include "tm4c1294ncpdt.h"
#include "vl53l1x_api.h"
#include "uart.h"
#include "onboardLEDs.h"
#include "pushButton.h"
#include "I2CInit.h"
#include "variables.h"


uint16_t	dev=0x52;

int status = 0;

volatile int IntCount;

//interupt state variable
int state = 0;

//capture values from VL53L1X(ToF) for inspection
uint16_t debugArray[512];

//////////PORT INITIALIZATIONS/////////////////////////////////////////////////////////////////////////////////////////////////
//For AD2 
void PortE_Init(void){
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; // activate the clock for Port E
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R4) == 0){}; // allow time for clock to stabilize
  GPIO_PORTE_DIR_R = 0b00000010;
  GPIO_PORTE_DEN_R = 0b00000010; // Enable PE1
  return;
}

//For stepper motor
void PortH_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R7) == 0){};	
	GPIO_PORTH_DIR_R |= 0xF;												
	GPIO_PORTH_AFSEL_R &= ~0xF;		 								
	GPIO_PORTH_DEN_R |= 0xF;																												
	GPIO_PORTH_AMSEL_R &= ~0xF;		 								
	return;
}


////FUNCTION TO TEST SPEED WITH SCOPE
void testSpeed(void){
	GPIO_PORTE_DATA_R ^= 0b00000010;
	SysTick_Wait10ms(10);
	
}



//////CODE FOR STEPPER MOTOR ROTATIONS ////////////////////////////////////////////////////////////////////////////////////////////////////
void rotateCW(int delay) {
		GPIO_PORTH_DATA_R = 0b00001100;
		SysTick_Wait10ms(delay); 
		GPIO_PORTH_DATA_R = 0b00000110;
		SysTick_Wait10ms(delay); 
		GPIO_PORTH_DATA_R = 0b00000011;
		SysTick_Wait10ms(delay); 
		GPIO_PORTH_DATA_R = 0b00001001;
		SysTick_Wait10ms(delay); 
}

void rotateCCW(int delay) {
		GPIO_PORTH_DATA_R = 0b00001001;
		SysTick_Wait10ms(delay); 
		GPIO_PORTH_DATA_R = 0b00000011;
		SysTick_Wait10ms(delay); 
		GPIO_PORTH_DATA_R = 0b00000110;
		SysTick_Wait10ms(delay); 
		GPIO_PORTH_DATA_R = 0b00001100;
		SysTick_Wait10ms(delay); 
}

void rotate(int steps, int delay, int dir) { // rotation function based on direction
	if(dir == 1) {
		for (int i=0; i<steps; i++){
			rotateCW(delay);
		}
	} else if(dir == -1) {
		for (int i=0; i<steps; i++){
			rotateCCW(delay);
		}
	}
}



////MAIN LOGIC////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {

  uint8_t byteData, sensorState=0, myByteArray[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , i=0;
  uint16_t wordData;
  uint8_t ToFSensor = 1; // 0=Left, 1=Center(default), 2=Right
  uint16_t SignalRate;
  uint16_t AmbientRate;
  uint16_t SpadNum; 
	uint8_t RangeStatus;
	uint8_t dataReady;
	uint16_t Distance;
  
	//initializations
	PLL_Init();	
	SysTick_Init();
	onboardLEDs_Init();
	I2C_Init();
	UART_Init();
	PortH_Init();//used for stepper motor
	PortE_Init(); //AD2
	
	// hello world!
	UART_printf("Program Begins\r\n");
	sprintf(printf_buffer,"2DX4 Project Code, fahadn, 400457365\r\n");
	UART_printf(printf_buffer);
	
	/* Those basic I2C read functions can be used to check your own I2C functions */
  status = VL53L1_RdByte(dev, 0x010F, &byteData);					// This is the model ID.  Expected returned value is 0xEA
  myByteArray[i++] = byteData;

  status = VL53L1_RdByte(dev, 0x0110, &byteData);					// This is the module type.  Expected returned value is 0xCC
  myByteArray[i++] = byteData;
	
	status = VL53L1_RdWord(dev, 0x010F, &wordData);
	status = VL53L1X_GetSensorId(dev, &wordData);

	sprintf(printf_buffer,"Model_ID=0x%x , Module_Type=0x%x\r\n",myByteArray[0],myByteArray[1]);
	UART_printf(printf_buffer);

	// Booting ToF chip
	while(sensorState==0){
		status = VL53L1X_BootState(dev, &sensorState);
		SysTick_Wait10ms(10); //change
  }
	
	FlashAllLEDs(); //Flash all LEDS to indicate boot
	UART_printf("ToF Chip Booted!\r\n");
 	UART_printf("One moment...\r\n");
	
	status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/

	
  /* This function must to be called to initialize the sensor with the default setting  */
  status = VL53L1X_SensorInit(dev);
	Status_Check("SensorInit", status);


	UART_printf("Press GPIO J1 on the microcontroller to start/stop data collection\r\n");
	
	PushButton_Init(); //Enable PJ1 button interupt
	
	while (1) { //Infinite Loop
		if (state == 1) {
    status = VL53L1X_StartRanging(dev); // Start ranging
    Status_Check("StartRanging", status);
    
    int steps = 16; // Perform a scan every 16 steps for a total of 32 scans per rotation
    int totalSteps = 512; // Total steps for a full rotation
    int stepCount = 0; // Counter to keep track of the number of steps
			

    
    // Perform rotation and scanning
    for(int i = 0; i < totalSteps; i += steps) {
        for (int step = 0; step < steps; ++step) {
            rotate(1, 1, 1); // Rotate 1 step at a time
            ++stepCount; // Increment step counter
        }
        
        // Ensure data is ready
        while (dataReady == 0) {
            status = VL53L1X_CheckForDataReady(dev, &dataReady);
            FlashLED3(1); 
            VL53L1_WaitMs(dev, 5);
        }

        dataReady = 0;
        status = VL53L1X_GetRangeStatus(dev, &RangeStatus);
        status = VL53L1X_GetDistance(dev, &Distance);
        FlashLED3(1); // Flash every time data is collected
        
        debugArray[i / steps] = Distance; // Store distance
        
        status = VL53L1X_ClearInterrupt(dev); // Clear interrupt to enable next interrupt

        sprintf(printf_buffer, "%u\r\n", Distance);
        UART_printf(printf_buffer); // Transmit distance measurement

        FlashLED4(1); // Flash every rotation or scan
        
        // Check if the stop condition is met
        if (state == 0) {
            break;
        }
    }
    
    // If stopped before full rotation go backwards 
    if (state == 0 || stepCount == totalSteps) {
        rotate(stepCount, 1, -1); // Unwind the rotation
    }
    
    VL53L1X_StopRanging(dev);
    UART_printf("Data collection is stopped, press the button PJ1 to restart data collection\r\n");
    state = 0;
}
	
		if (state == 0) { // If data capture is off
			UART_printf("Press GPIO J1 on the microncontroller to start data collection\r\n"); 
			while (state == 0) {
				SysTick_Wait10ms(10); //change
				if (state == 1) break; // If interrupt triggered, exits loop to enter data capture loop
			}
		}
		//testSpeed();
		
	}
}





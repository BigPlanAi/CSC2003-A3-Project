# **MSP432 HC-05**

We all have used this! The venerable [HC-05](https://www.amazon.com/KEDSUM-Arduino-Bluetooth-Wireless-Transceiver/dp/B0093XAV4U/ref=sr_1_3?ie=UTF8&qid=1505098254&sr=8-3&keywords=arduino+bluetooth) Bluetooth module. These modules are so simple to use and hook up, that you can have a Bluetooth communication link working in minutes! :smile:
***

### **Project Setup**
***
In order to use the project you will need to have the [MSP432 SimpleLink SDK](http://www.ti.com/tool/SIMPLELINK-MSP432-SDK) library installed. To install it follow the link and select the SIMPLELINK-MSP432-SDK option, after the installation is complete import this project into Code Composer's Workspace. 

The HC-05 project is divided into two folders:

- **Hardware**: This folder contains all hardware peripheral code

- **Devices**: This folder contains [MSPIO](https://github.com/amartinezacosta/MSPIO)

**IMPORTANT**: This project will require a serial terminal on a computer and a bluetooth terminal on an android device open. Use your preferred serial terminals.

### **How to use it**
***
Include the following header files
```c
#include <Hardware/CS_Driver.h>
#include <Devices/MSPIO.h>
```
Initialize all the necessary hardware peripherals as follows:
```c
void main()
{
	MAP_WDT_A_holdTimer();

    /*MSP432 Running at 24 MHz*/
	CS_Init();

	/*Initialize required hardware peripherals for the ESP8266*/
	/*We are using UART A0 for UART PC*/
	UART_Init(EUSCI_A0_BASE, UART0Config);
	/*UART A2 is used for the HC-05*/
	UART_Init(EUSCI_A2_BASE, UART2Config);

    MAP_Interrupt_enableMaster();
}
```
To communicate to the HC-05 you will simply call the MSPIO MSPrintf function and pass as arguments the UART base address and the formatted string 
```c
/*simple string*/
MSPrintf(EUSCI_A2_BASE, "Hello HC-05");

/*Or formatted string*/
char c = 'e'
int no = 1234;
char str[] = "I am a string";
MSPrintf(EUSCI_A2_BASE, "%c, %i, %s", c, no, str);
```
You can also wait for input from the HC-05 Bluetooth module by calling the MSPgets function
```c
const int BUFFER_SIZE = 128;
char Buffer[BUFFER_SIZE];

/*Wait for input coming from the HC-05, will not return until a '\r' or '\n' is received*/
MSPgets(EUSCI_A2_BASE, Buffer, BUFFER_SIZE);
```
if you want to print the received string coming from the HC-05 you will simply use the MSPrintf function, but this time you will pass the UART base address of the PC UART (EUSCI_A0_BASE)
```c
MSPrintf(EUSCI_A0_BASE, "%s", Buffer);
```
The same is true if you want to wait for input coming from the PC UART
```c
MSPgets(EUSCI_A0_BASE, Buffer, BUFFER_SIZE);
```

### **Other Details and Future Work**
***
**THIS DOES NOT WORK FOR IOS!** Unfortunately IOS has a limited list of bluetooth profiles that can recognize, the HC-05 profile is not among those :disappointed:. You will need an Android device and a bluetooth terminal to make this work. 

For some reason some of the android terminals send an extra carriage return. If you encounter this problem, select the "append carriage return only" option. Most Bluetooth serial terminals have this option

***

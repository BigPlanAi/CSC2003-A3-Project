/* DriverLib Includes */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PERIOD 1250

//Prints string
void uPrintf(unsigned char * TxArray);
//Prints int
void print(int num);
///Prints double
void printDouble(double num);

//Functions used
void runBarcode(void);
void addCharactersToArray(void);
void logicForScannedOutput(char inputChar, int scannedOutputLength);
void logicToCheckAsterisk(void);
void identifyCharacter(void);
void resetScan(void);
void startConversion(void);

//UART config for displaying in putty
const eUSCI_UART_Config uartConfig = {
EUSCI_A_UART_CLOCKSOURCE_SMCLK,                 // SMCLK Clock Source
        1,                                              // BRDIV = 1
        10,                                             // UCxBRF = 10
        0,                                              // UCxBRS = 0
        EUSCI_A_UART_ODD_PARITY,                        // ODD Parity
        EUSCI_A_UART_LSB_FIRST,                         // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,                      // One stop bit
        EUSCI_A_UART_MODE,                              // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION,  // Oversampling
        };

//Timer config for scanning of barcode. Runs every 10ms
const Timer_A_UpModeConfig upConfig = {
TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source = 3Mhz
        TIMER_A_CLOCKSOURCE_DIVIDER_24,         // TACLK = 3MHz / 24
        PERIOD,                           // 3000000 / 24 = 125000, 1/125000 x 1250 = 0.010 (10ms)
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
        };

// Statics
static volatile uint16_t curADCResult;
static volatile float normalizedADCRes;

//The scanned result
char scannedOutput[50];

struct BarcodeChar
{
    char barcodeBinary[100];
    char barcodeChar;
};

//Array of barcode characters
struct BarcodeChar characters[44];

int matched; //Variable to check if first 16 characters has a match with the asterisk character
int checkToStartScan; //Variable to start scan when sensor sense the specific white value range
int scanCount; //Count total scans per 10ms
int totalVal; //Total added value for the total scans
float avgVal; //Average value per 10ms
char outputCh; //The scanned output. 1 for black, 0 for white
int scannedTicks; //The number of times scanned per barcode line (Used to differentiate long and short bars)
int numOfTicks; //Used for different sizes of barcodes
int delayAfterFinishScan;

void runBarcode(void)
{
    // Halting WDT
    WDT_A_holdTimer();

    // Initialize relevant variables for a default value on start
    curADCResult = 0;
    matched = 0;
    checkToStartScan = 0;
    scanCount = 0;
    totalVal = 0;
    avgVal = 0;
    outputCh = '0';
    scannedTicks = 0;
    numOfTicks = 5; //15 For biggest 10 Med, 5 Small (2500 duty cycle)
    delayAfterFinishScan = 0;

    //Adding all barcode characters to characters array
    addCharactersToArray();

    // Setting Flash wait state
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);

    // Enabling the FPU for floating point operation
    FPU_enableModule();
    FPU_enableLazyStacking();

    //Set P1.1 BtnS1
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    Interrupt_enableInterrupt(INT_PORT1);

    // Selecting P1.2 and P1.3 in UART mode
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
    GPIO_PIN2 | GPIO_PIN3,
                                               GPIO_PRIMARY_MODULE_FUNCTION);

    // Configuring UART Module
    UART_initModule(EUSCI_A0_BASE, &uartConfig);

    // Enable UART module
    UART_enableModule(EUSCI_A0_BASE);
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);

    // Initializing ADC (MCLK/1/4)
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4, 0);

    // Configuring P1.0 as output
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

    // Configuring GPIOs (5.5 A0)
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,
    GPIO_TERTIARY_MODULE_FUNCTION);

    // Configuring ADC Memory
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A0,
                                    false);

    // Configuring Sample Timer
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    // Enabling/Toggling Conversion

    //ADC14_enableConversion();
    //ADC14_toggleConversionTrigger();

    // Enabling interrupts
    ADC14_enableInterrupt(ADC_INT0);
    Interrupt_enableInterrupt(INT_ADC14);

    // Enabling interrupts (Rx)
    //Interrupt_enableMaster();

    Timer_A_configureUpMode(TIMER_A2_BASE, &upConfig);
    Interrupt_enableInterrupt(INT_TA2_0);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);

    /*
     ADC14_enableConversion();
     ADC14_toggleConversionTrigger();
     */

    uPrintf("Starting barcode program\n\r");

    /*while (1)
     {
     PCM_gotoLPM3InterruptSafe();
     }*/
}

void startConversion(void)
{
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();
}

// ADC Interrupt Handler. This handler is called whenever there is a conversion that is finished for ADC_MEM0.
void ADC14_IRQHandler(void)
{
    uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
    MAP_ADC14_clearInterruptFlag(status);

    if (ADC_INT0 & status)
    {
        curADCResult = MAP_ADC14_getResult(ADC_MEM0);
        //normalizedADCRes = (curADCResult * 3.3) / 16384;
        //print(scanCount);
        //printDouble(curADCResult);

        scanCount++; //Increment for every scan occurred
        totalVal += curADCResult; //Use current scanned value to add into a totalVal variable

        ADC14_toggleConversionTrigger(); //Keep scanning

    }
}

void TA2_0_IRQHandler(void)
{

    //print(scanCount);
    int scannedOutputLength = strlen(scannedOutput); //Length of current scanned output string
    scannedTicks++; //Increment scannedTicks for everytime timer handler is called
    avgVal = (float) totalVal / scanCount; //Get average value every 10ms

    //Check if start of white surface of the barcode paper
    if (checkToStartScan == 1)
    {
        uPrintf("The avg per 0.010 Sec is: ");
        printDouble(avgVal);
        uPrintf("\n\r");

        //If scanned bar is white 650 for big and med, 620 for small
        if (avgVal < 650)
        {
            logicForScannedOutput('0', scannedOutputLength);
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        }
        //If scanned bar is black
        else
        {
            logicForScannedOutput('1', scannedOutputLength);
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        }

        //Once scannedOutput hits 16 characters, check if it matches with asterisk
        if (scannedOutputLength >= 16 && matched == 0)
        {
            logicToCheckAsterisk();
        }

        //Check if first 16 characters is an asterisk
        if (matched == 1)
        {
            //Once scannedOutput hits 32 characters, identify the character
            if (strlen(scannedOutput) >= 32)
            {
                identifyCharacter();
            }
        }
    }

    //Only prints if it is scanning a barcode
    if (checkToStartScan == 1 && scanCount > 1)
    {
        printChar(outputCh);
        uPrintf("The scanned output is: ");
        uPrintf(scannedOutput);
        uPrintf("\n\r");
    }

    //Start scanning if sensor hits the start of the barcode paper
    if (avgVal > 540 && avgVal < 600) //curADCResult > 540 && curADCResult < 600
    {
        checkToStartScan = 1;
    }

    //Empty string if scannedOutput hits 49 characters
    /*
    if (strlen(scannedOutput) == 49)
    {
        strcpy(scannedOutput, "");
    }*/

    if (strlen(scannedOutput) >= 48)
    {
        resetScan();
    }

    //Reset scannedTicks if it hits numOfTicks (Used to check if bar is long or short)
    if (scannedTicks == numOfTicks)
    {
        scannedTicks = 0;
    }

    //Reset scanCount and totalVal after each scan
    scanCount = 0;
    totalVal = 0;
    Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

/*
 void PORT1_IRQHandler(void)
 {
 uint32_t status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
 GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

 if (status & GPIO_PIN1) //If P1.1 is pressed
 {
 resetScan();
 }

 }
 */
void uPrintf(unsigned char * TxArray)
{
    unsigned short i = 0;
    while (*(TxArray + i))
    {
        UART_transmitData(EUSCI_A0_BASE, *(TxArray + i)); // Write the character at the location specified by pointer
        i++;                 // Increment pointer to point to the next character
    }
}

void print(int num)
{
    char p[100];
    sprintf(p, "%d", num);
    uPrintf(p);
    uPrintf("\n\r");
}

void printDouble(double num)
{
    char p[100];
    sprintf(p, "%f", num);
    uPrintf(p);
    uPrintf("\n\r");
}

void printChar(char c)
{
    char p[100];
    sprintf(p, "%c", c);
    uPrintf(p);
    uPrintf("\n\r");
}

void addCharactersToArray(void)
{
    //Adding all barcode characters to characters array
    strcpy(characters[0].barcodeBinary, "1010001110111010"); //0
    characters[0].barcodeChar = '0';

    strcpy(characters[1].barcodeBinary, "1110100010101110"); //1
    characters[1].barcodeChar = '1';

    strcpy(characters[2].barcodeBinary, "1011100010101110"); //2
    characters[2].barcodeChar = '2';

    strcpy(characters[3].barcodeBinary, "1110111000101010"); //3
    characters[3].barcodeChar = '3';

    strcpy(characters[4].barcodeBinary, "1010001110101110"); //4
    characters[4].barcodeChar = '4';

    strcpy(characters[5].barcodeBinary, "1110100011101010"); //5
    characters[5].barcodeChar = '5';

    strcpy(characters[6].barcodeBinary, "1011100011101010"); //6
    characters[6].barcodeChar = '6';

    strcpy(characters[7].barcodeBinary, "1010001011101110"); //7
    characters[7].barcodeChar = '7';

    strcpy(characters[8].barcodeBinary, "1110100010111010"); //8
    characters[8].barcodeChar = '8';

    strcpy(characters[9].barcodeBinary, "1011100010111010"); //9
    characters[9].barcodeChar = '9';

    strcpy(characters[10].barcodeBinary, "1110101000101110"); //A
    characters[10].barcodeChar = 'A';

    strcpy(characters[11].barcodeBinary, "1011101000101110"); //B
    characters[11].barcodeChar = 'B';

    strcpy(characters[12].barcodeBinary, "1110111010001010"); //C
    characters[12].barcodeChar = 'C';

    strcpy(characters[13].barcodeBinary, "1010111000101110"); //D
    characters[13].barcodeChar = 'D';

    strcpy(characters[14].barcodeBinary, "1110101110001010"); //E
    characters[14].barcodeChar = 'E';

    strcpy(characters[15].barcodeBinary, "1011101110001010"); //F
    characters[15].barcodeChar = 'F';

    strcpy(characters[16].barcodeBinary, "1010100011101110"); //G
    characters[16].barcodeChar = 'G';

    strcpy(characters[17].barcodeBinary, "1110101000111010"); //H
    characters[17].barcodeChar = 'H';

    strcpy(characters[18].barcodeBinary, "1011101000111010"); //I
    characters[18].barcodeChar = 'I';

    strcpy(characters[19].barcodeBinary, "1010111000111010"); //J
    characters[19].barcodeChar = 'J';

    strcpy(characters[20].barcodeBinary, "1110101010001110"); //K
    characters[20].barcodeChar = 'K';

    strcpy(characters[21].barcodeBinary, "1011101010001110"); //L
    characters[21].barcodeChar = 'L';

    strcpy(characters[22].barcodeBinary, "1110111010100010"); //M
    characters[22].barcodeChar = 'M';

    strcpy(characters[23].barcodeBinary, "1010111010001110"); //N
    characters[23].barcodeChar = 'N';

    strcpy(characters[24].barcodeBinary, "1110101110100010"); //O
    characters[24].barcodeChar = 'O';

    strcpy(characters[25].barcodeBinary, "1011101110100010"); //P
    characters[25].barcodeChar = 'P';

    strcpy(characters[26].barcodeBinary, "1010101110001110"); //Q
    characters[26].barcodeChar = 'Q';

    strcpy(characters[27].barcodeBinary, "1110101011100010"); //R
    characters[27].barcodeChar = 'R';

    strcpy(characters[28].barcodeBinary, "1011101011100010"); //S
    characters[28].barcodeChar = 'S';

    strcpy(characters[29].barcodeBinary, "1010111011100010"); //T
    characters[29].barcodeChar = 'T';

    strcpy(characters[30].barcodeBinary, "1110001010101110"); //U
    characters[30].barcodeChar = 'U';

    strcpy(characters[31].barcodeBinary, "1000111010101110"); //V
    characters[31].barcodeChar = 'V';

    strcpy(characters[32].barcodeBinary, "1110001110101010"); //W
    characters[32].barcodeChar = 'W';

    strcpy(characters[33].barcodeBinary, "1000101110101110"); //X
    characters[33].barcodeChar = 'X';

    strcpy(characters[34].barcodeBinary, "1110001011101010"); //Y
    characters[34].barcodeChar = 'Y';

    strcpy(characters[35].barcodeBinary, "1000111011101010"); //Z
    characters[35].barcodeChar = 'Z';

    strcpy(characters[36].barcodeBinary, "1000101011101110"); //-
    characters[36].barcodeChar = '-';

    strcpy(characters[37].barcodeBinary, "1110001010111010"); //.
    characters[37].barcodeChar = '.';

    strcpy(characters[38].barcodeBinary, "1000111010111010"); //<SPACE>
    characters[38].barcodeChar = ' ';

    strcpy(characters[39].barcodeBinary, "1000100010001010"); //$
    characters[39].barcodeChar = '$';

    strcpy(characters[40].barcodeBinary, "1000100010100010"); ///
    characters[40].barcodeChar = '/';

    strcpy(characters[41].barcodeBinary, "1000101000100010"); //+
    characters[41].barcodeChar = '+';

    strcpy(characters[42].barcodeBinary, "1010001000100010"); //%
    characters[42].barcodeChar = '%';

    strcpy(characters[43].barcodeBinary, "1000101110111010"); //*
    characters[43].barcodeChar = '*';
}

void logicForScannedOutput(char inputChar, int scannedOutputLength)
{
    //Check if previous scanned character is different from current scanned character
    if (outputCh != inputChar)
    {
        outputCh = inputChar; //Change to current scanned character
        strncat(scannedOutput, &outputCh, 1); //Concatenate into scanned output string
        scannedTicks = 0; //Reset ticks cause it just changed from black to white or vice versa

    }
    //If previous scanned character is same colour
    else
    {
        //Check if scanned output is not empty and first character is not white (Prevent scanning white for first value)
        if (scannedOutputLength != 0 && scannedOutput[0] != '0')
        {
            //Retrieve the last, second last, and third last elements (Prevents more than 3 of the same character in the scanned output string)
            int lastEle = scannedOutputLength - 1;
            int secondLastEle = scannedOutputLength - 2;
            int thirdLastEle = scannedOutputLength - 3;

            //If scanned output has more than 2 characters
            if (scannedOutputLength >= 3)
            {

                //If inputChar has previously appeared 3 times consecutively, do nothing
                if (scannedOutput[lastEle] == inputChar
                        && scannedOutput[secondLastEle] == inputChar
                        && scannedOutput[thirdLastEle] == inputChar)
                {

                }
                else
                {
                    //Add 2 more 1/0 to scanned output when the scannedTicks exceeds numOfTicks (Means sensor is scanning same colour for awhile = Long bar)
                    if (scannedTicks >= numOfTicks)
                    {
                        strncat(scannedOutput, &inputChar, 1);
                        strncat(scannedOutput, &inputChar, 1);
                    }
                }
            }
            //If scanned output has less than 3 characters
            else
            {
                //Check if scannedTicks exceeds numOfTicks
                if (scannedTicks >= numOfTicks)
                {
                    //If previous 2 characters are same as inputChar, and scanned output has more than 1 character add only 1 more 1/0
                    if (scannedOutputLength >= 2
                            && scannedOutput[lastEle] == inputChar
                            && scannedOutput[secondLastEle] == inputChar)
                    {
                        strncat(scannedOutput, &inputChar, 1);
                    }
                    //If not add 2 more 1/0
                    else
                    {
                        strncat(scannedOutput, &inputChar, 1);
                        strncat(scannedOutput, &inputChar, 1);
                    }
                }
            }
        }

    }
}

void logicToCheckAsterisk(void)
{
    int result = strcmp(scannedOutput, characters[43].barcodeBinary); //Check if 100% match with asterisk
    int i, numberOfSameChar = 0;
    float matchPercentage;
    //For every character in scanned output, check how many characters are same as asterisk
    for (i = 0; i < 16; i++)
    {
        if (scannedOutput[i] == characters[43].barcodeBinary[i])
        {
            numberOfSameChar++;
        }

    }
    matchPercentage = ((float) numberOfSameChar / 16) * 100; //Check the percentage of similarity
    uPrintf("The number of same char is ");
    print(numberOfSameChar);
    uPrintf("The match percentage is ");
    printDouble(matchPercentage);
    uPrintf("\n\r");
    uPrintf("Is it a barcode? ");
    print(result);
    uPrintf("\n\r");

    //If it is an asterisk, we should continue scanning the next 16 characters
    if (matchPercentage >= 75)
    {
        uPrintf("Very likely it is a barcode");
        matched = 1;

    }
    //Reset scan since it is not an asterisk
    else
    {
        resetScan();
    }
}

void identifyCharacter(void)
{
    int j, i, numberOfSameChar = 0, highestNumOfSameChar = 0;
    int arrayLen = (int) (sizeof(characters) / sizeof(characters[0]));
    //print(arrayLen);
    char mostLikelyChar;

    //Compare every binary with all characters in characters array
    for (j = 0; j < arrayLen; j++)
    {
        for (i = 16; i < 32; i++)
        {
            if (scannedOutput[i] == characters[j].barcodeBinary[i - 16])
            {
                numberOfSameChar++;
            }
        }

        //Replace highest match
        if (numberOfSameChar > highestNumOfSameChar)
        {
            highestNumOfSameChar = numberOfSameChar;
            mostLikelyChar = characters[j].barcodeChar;
        }
        numberOfSameChar = 0;
    }

    //Reset
    uPrintf("The character is ");
    printChar(mostLikelyChar);
    uPrintf("\n\r");
    //resetScan();
}

void resetScan(void)
{
    matched = 0;
    checkToStartScan = 0;
    scanCount = 0;
    outputCh = '0';
    scannedTicks = 0;
    strcpy(scannedOutput, "");
    uPrintf("Pause and reset scan.\n\r");
}

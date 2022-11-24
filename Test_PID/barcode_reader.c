//// DriverLib Includes
#include <driverlib.h>
//
//// Standard Includes
#include <stdint.h>
#include <math.h>
//
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "modules.h"


#define BUFFER_SIZE 30
#define BARCODE_TIME_DELAY 50
#define COLOR_THRESHOLD 650


void enqueue(void);
void dequeue(void);
uint16_t isValidBarcode(void);
uint16_t store_distance(char);                       // To Storing distance into global array "distance[]"
uint16_t get_distance_Threshold(uint16_t *dist_arr); // Calculate threshold to differentiate wide and slim lines
void get_pattern_array(char);                        // This is called in ADC interrupt handler

void enqueue_barBuffer(uint16_t *bar);
void add_char_To_charBuff(char value);
void add_char_To_reverse_charBuff(char value);
uint16_t check_barBuffer(void);
void dequeue_barBuffer(uint16_t skips);
void scan_barBuffer();
void reverse_bar(char *bar);

void setup_barcode(void);
void barcode_ADC14_IRQ(void);
void barcode_TIMERA1_IRQ(void);
//void restart_queue(void);




uint16_t distance[ARR_SIZE] = { 0 }; // To store bar width , QUEUE ARRAY
char pattern[20] = "000000000";      // To store bar pattern , 1 is wide, 0 is short
uint16_t rear = 0;                   // Index of distance array


char current_color = 'S';           // Variable to store current Color Detected
char color_arr[ARR_SIZE+1] = "1111111111" ; // Array To store the color of bars,   BLACK '1' , WHITE '0'

struct barcodeBuffer {                   // Buffer structure to store valid barcodes globally
    uint16_t buffer[BUFFER_SIZE][ARR_SIZE-1] ;    // 50 arrays of 9 bar width;
    short int rear_index;
};

struct barcodeBuffer bar_buffer  = {.rear_index = -1};   // Buffer structure to store valid barcodes globally


//struct characterBuffer {
//    char buffer[ARR_SIZE];
//    short int rear_index;
//};

struct characterBuffer charBuff  ={.rear_index = -1};  // Buffer structure to store identified characters globally
struct characterBuffer reverse_charBuff  = {.rear_index = -1}; // Buffer structure to store reversed identified characters globally
//struct BarcodeChar
//{
//    char barcodeBinary[100];
//    char barcodeChar;
//};

//struct BarcodeChar characters[4] ;


uint16_t current_distance = 0;


uint32_t scanCount;
uint32_t totalVal ;
uint16_t prev_velocity;
//
//void barcode_setup(){
//    strcpy(characters[0].barcodeBinary, "100001001"); //A 1110101000101110
//    characters[0].barcodeChar = 'A';
//
//    strcpy(characters[1].barcodeBinary, "001011000"); //F 1011101110001010
//    characters[1].barcodeChar = 'F';
//
//    strcpy(characters[2].barcodeBinary, "011010000"); //Z 1000111011101010
//    characters[2].barcodeChar = 'Z';
//
//    strcpy(characters[3].barcodeBinary, "010010100"); //* 1000101110111010
//    characters[3].barcodeChar = '*';
//
//}


void setup_barcode(void){
    printf("\n adc setup INT  working");
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4, 0);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,GPIO_TERTIARY_MODULE_FUNCTION);

    // Configuring ADC Memory
    ADC14_configureSingleSampleMode(ADC_MEM0, true);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,ADC_INPUT_A0,false);

    // Configuring Sample Timer
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    // Enabling/Toggling Conversion

    //ADC14_enableConversion();
    //ADC14_toggleConversionTrigger();

    // Enabling interrupts
    ADC14_enableInterrupt(ADC_INT0);
    Interrupt_enableInterrupt(INT_ADC14);

    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();


}


void barcode_ADC14_IRQ(void){

    uint32_t status = ADC14_getEnabledInterruptStatus();
    if (ADC_INT0 & status)
    {
        //printf("\n adc INT  working");
        //printf("ADC");
        uint32_t curADCResult = ADC14_getResult(ADC_MEM0);
        //printf("%d " , curADCResult);
        scanCount++;
        totalVal += curADCResult;
        //printf("%d \n" , scanCount);
        //uPrintf("The scanned output is: ");
        //uPrintf(scannedOutput);
        //uPrintf("\n\r");
        ADC14_clearInterruptFlag(status);
        ADC14_toggleConversionTrigger();
    }

}

void barcode_TIMERA1_IRQ(void){
    static uint16_t  t_counter = 0 ;
    uint32_t avgVal = 0;
    t_counter++;
    if(t_counter > BARCODE_TIME_DELAY){
        t_counter = 0;
        avgVal = totalVal / scanCount;
        //printf("\n AVG VAL : %d",avgVal );
        if (avgVal < COLOR_THRESHOLD)
        {
            get_pattern_array(WHITE);
        }
        else
        {
            get_pattern_array(BLACK);
        }
        totalVal  = 0;
        scanCount = 0;
    }
}

/* Function to match valid barcodes to characters from A- Z
 * - If none is matched , '0' is returned
 *
 *  Input : char *arr, address of array
 *  Output : char value , matched character
 *
 *  This function is called in scan_barBuffer()
 */

char match_barcode(char *bars){
    if (bars[1] == '1'){
        if (!strcmp(bars, "110000001") ){
            return 'U';
        }
        if (!strcmp(bars, "011000001") ){
            return 'V';
        }
        if (!strcmp(bars, "111000000") ){
            return 'X';
        }
        if (!strcmp(bars, "010010001") ){
            return 'Y';
        }
        if (!strcmp(bars, "011010000") ){
            return 'Z';
        }
        if (!strcmp(bars, "010010100") ){
            return '*';
        }

    }
    else if(bars[5] == '1'){
        if (!strcmp(bars, "100001001") ){
            return 'A';
        }
        if (!strcmp(bars, "001001001") ){
            return 'B';
        }
        if (!strcmp(bars, "101001000") ){
            return 'C';
        }
        if (!strcmp(bars, "000011001") ){
            return 'D';
        }
        if (!strcmp(bars, "100011000") ){
            return 'E';
        }
        if (!strcmp(bars, "001011000") ){
            return 'F';
        }
        if (!strcmp(bars, "000001101") ){
            return 'G';
        }
        if (!strcmp(bars, "100001101") ){
            return 'H';
        }
        if (!strcmp(bars, "001001100") ){
            return 'I';
        }
        if (!strcmp(bars, "000011100") ){
            return 'J';
        }
    }
    else if(bars[7] == '1'){
         if (!strcmp(bars, "100000011") ){
             return 'K';
         }
         if (!strcmp(bars, "001000011") ){
             return 'L';
         }
         if (!strcmp(bars, "101000010") ){ //101000010
             return 'M';
         }
         if (!strcmp(bars, "000010011") ){
             return 'N';
         }
         if (!strcmp(bars, "100010010") ){
             return 'O';
         }
         if (!strcmp(bars, "001010010") ){
             return 'P';
         }
         if (!strcmp(bars, "000000111") ){
             return 'Q';
         }
         if (!strcmp(bars, "100000110") ){
             return 'R';
         }
         if (!strcmp(bars, "001000110") ){
             return 'S';
         }
         if (!strcmp(bars, "000010110" )){
             return 'T';
         }

    }
    else {
       return '0';
    }
    return '0';
}



void add_char_To_charBuff(char value){
    uint16_t i;
    if (charBuff.rear_index >= ARR_SIZE -1 ){
        for (i = 0; i < ARR_SIZE-1  ; i++){
            charBuff.buffer[i] =  charBuff.buffer[i+1];
        }
        charBuff.rear_index--;
    }
    charBuff.rear_index++;
    charBuff.buffer[charBuff.rear_index] = value;

}

void add_char_To_reverse_charBuff(char value){
    uint16_t i;
    if (reverse_charBuff.rear_index >= ARR_SIZE -1 ){
        for (i = 0; i < ARR_SIZE-1  ; i++){
            reverse_charBuff.buffer[i] =  reverse_charBuff.buffer[i+1];
        }
        reverse_charBuff.rear_index--;
    }
    reverse_charBuff.rear_index++;
    reverse_charBuff.buffer[reverse_charBuff.rear_index] = value;

}


void enqueue_barBuffer(uint16_t *bar){
    if (bar_buffer.rear_index >= BUFFER_SIZE -1 ){
        dequeue_barBuffer(1);
    }
    bar_buffer.rear_index++;
    uint16_t i;
    for (i = 0; i < ARR_SIZE-1  ; i++){
        bar_buffer.buffer[bar_buffer.rear_index][i] = bar[i];
    }
    //strcpy(bar_buffer.buffer[bar_buffer.rear_index], bar);
}


uint16_t check_barBuffer(){
    return (bar_buffer.rear_index >=0);
}


void dequeue_barBuffer(uint16_t skips){
    uint16_t i;
    uint16_t x;
    if(bar_buffer.rear_index == -1) {
        return;
    }
    for (i = 0; i < BUFFER_SIZE-skips  ; i++){
        for (x = 0; x < ARR_SIZE-1   ; x++){    // 9 values
            bar_buffer.buffer[i][x] = bar_buffer.buffer[i+skips][x];
        }
        //strcpy(bar_buffer.buffer[i], bar_buffer.buffer[i+1]);
    }
    bar_buffer.rear_index -= skips;
}


/* scan_barBuffer()
 *   - checks barcode buffer for available barcodes and match the oldest barcode
 *     in barcode buffer.
 *   - If character match is found , stores the character into character buffer , charbuff
 *
 *
 *  Input : void
 *  Output : void
 *
 *  This function is called in main()
 */
void scan_barBuffer(){
    if(check_barBuffer()){
        uint16_t i;
//        for ( i = 0; i <bar_buffer.rear_index + 1  ; i++){
//            printf("\nComparing results: ");
//            printf("%d \n " , strcmp(characters[0].barcodeBinary, bar_buffer.buffer[i]));
//
//            if( 0 == strcmp(characters[0].barcodeBinary,  bar_buffer.buffer[i])){
//                add_char_To_charBuff(characters[0].barcodeChar);
//            }
//        }
        uint16_t current_bar[ARR_SIZE -1];
        for (i = 0; i < ARR_SIZE-1  ; i++){
            current_bar[i] = bar_buffer.buffer[0][i];
            printf(" %d",bar_buffer.buffer[0][i]);
        }
        //strcpy(current_bar, bar_buffer.buffer[0]);
        dequeue_barBuffer(1);

        char current_pattern[ARR_SIZE];

        uint16_t threshold = get_distance_Threshold(current_bar);
        printf(" threshold : %d , ", threshold);
//        for (i = 0; i < ARR_SIZE -1 ; i++)
//        {
//            printf(" %d",current_bar[i]);
//
//        }

        printf(" pattern : ");
        for (i = 0; i < ARR_SIZE -1 ; i++)
        {
            current_pattern[i] = current_bar[i] > threshold ? WIDE : SLIM;
//            for (j = 0; j < ARR_SIZE -1 ; i++)
//            {
//                printf(" %d",current_bar[i]);
//
//            }
            printf(" %c ",current_bar[i] > threshold ? WIDE : SLIM);
        }

        current_pattern[i] = '\0';

        printf("\n new pattern : %s" , current_pattern);

        char match = match_barcode(current_pattern);
        if (match !='0' ){      //!strcmp("100001001",  current_pattern)
            //add_char_To_charBuff(match);
            add_char_To_charBuff(match);

            //dequeue_barBuffer(1); // To clear future bars

        }
        reverse_bar(current_pattern);
        match =  match_barcode(current_pattern);
        if ( match !='0' ){
            add_char_To_reverse_charBuff(match);
            //dequeue_barBuffer(1);
        }





    }
}

void enqueue()
{
    rear++;
    if (rear >= ARR_SIZE)
    {
        dequeue();
    }
}
void dequeue()
{
    if (rear == 0)
    {
        return;
    }
    else
    {
        uint16_t i;
        for (i = 0; i < ARR_SIZE-1  ; i++)
        {
            distance[i] = distance[i + 1];
            color_arr[i] = color_arr[i + 1];
        }

        rear--;
    }

}

void reverse_bar(char *bar){
    printf("\n  reverse: ");
    printf(bar);
    uint16_t i;
    char temp;
    for(i  =0 ; i<4; i++){
        temp = bar[i];
        bar[i] = bar[8-i];
        bar[8-i] = temp;
    }
    printf(" ");
    printf(bar);
}

void restart_queue()
{
    rear = 0;
}


/* isValidBarcode() checks if current set of barcode is valid
 *
 * - reads global array "color_arr[]"
 *
 */
uint16_t isValidBarcode()
{
    char array[10] = "101010101";
    char checker = '1';
    int16_t i;
    for ( i = 0; i < ARR_SIZE- 1; i++){
        if (array[i] != color_arr[i]){
            checker = '0';
            break;
        }
    }

    if ((rear >= ARR_SIZE -2) && ('1'== checker))
    {
        printf("\n Color array pass : " );
        printf(color_arr);
        return (uint16_t) 1;
    }
    else
    {
        printf("\n Color array fail : " );
        printf(color_arr);
        return (uint16_t) 0;
    }
}

/* get_distance_Threshold() calculates the threshold value from
 * array of bar width, threshold is used to differentiate wide and slim bars
 *
 *  INPUT : (uint16_t * dist_ar) , pointer to an int16_t array that stores width of bars
 *  OUTPUT: (uint16_t ) , threshold value calculated from array
 */
uint16_t get_distance_Threshold(uint16_t *dist_arr)
{
    uint16_t max = dist_arr[0];
    uint16_t min = dist_arr[0];

    uint16_t i;
    for (i = 1; i < ARR_SIZE-1; i++)
    {
        if (max < dist_arr[i])
        {
            max = dist_arr[i];
        }
        if (min > dist_arr[i])
        {
            min = dist_arr[i];
        }
    }

    return (max- min) / 2 + min;
}


/* get_pattern_array() ,
 *  -  calls store_distance() to store the previous velocity of detected bar,
 *  - Checks if current captured barcode is valid
 *      - If valid, calls enqueue_barBuffer() to store barcode pattern into barcode buffer, bar_buffer
 *
 * INPUT : char isBlack , color of bar
 * OUTPUT: output
 */
void get_pattern_array(char isBlack)
{
    uint16_t changed = store_distance(isBlack);
    if (changed)
    {
        if (isValidBarcode())
        {
            enqueue_barBuffer(distance);
        }
//            printf("\n threshold value %d", threshold);
//            uint16_t i;
//            for (i = 0; i < ARR_SIZE -1 ; i++){
//                printf(" %d ",distance[i] );
//            }

//            uint16_t threshold = get_distance_Threshold();
//            for (i = 0; i < ARR_SIZE -1; i++)
//            {
//                pattern[i] = distance[i] > threshold ? wide : slim;  // threshold
//                printf(" %c", distance[i] > threshold ? wide : slim); // 100
//            }
//            pattern[i] = '\0';
//            printf("\n Pattern :");
//            printf(pattern);
//            printf("\n");

    }
}

uint16_t store_distance(char isBlack)
{

    uint16_t did_change = 0;

    if (current_color == 'S')
    {
        current_color = isBlack;
    }

    if ((current_distance + prev_velocity) < 65000)
    {

        //current_distance += log10prev_velocity);
        current_distance += prev_velocity;
        //printf("\n Added Distance to current bar %d", distance[rear]);
    }
    //printf("\n Added current_distance  %d",current_distance);
    if (current_color != isBlack );
    {
        color_arr[rear] = current_color;
        distance[rear] = current_distance;
        //printf("\n Added Distance to current bar %d",current_distance);
        //printf("\n Current Distance %d", current_distance);
        enqueue();
        current_distance = 0;
        current_color = isBlack;
        did_change++;
    }
    prev_velocity = MIN( wheel_Right_Velocity, wheel_Left_Velocity);

    //printf("\n Added prev_velocity  %d",prev_velocity);

    return did_change;

}




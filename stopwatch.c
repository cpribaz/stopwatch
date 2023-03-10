//pointers to point to hardware
volatile int* buttons = (int*)0xFF200050;	//4 push buttons
volatile int* timer = (int*)0xFFFEC600;	//
volatile int* SWITCH = (int*)0xFF200040;
volatile int* hex_ptr = (int*)0xFF200020;
volatile int* hex_ptr2 = (int*)0xFF200030;

volatile int* t_control = (int*)0xFFFEC608; //the control register for the timer
volatile int* t_interrupt = (int*)0xFFFEC60C; //the interrupt status register for the timer


int h1, h2, m1, m2, s1, s2 = 0;
int lapH1, lapH2, lapM1, lapM2, lapS1, lapS2 = 0;
int startToggle = 0;

void displayLapped();

int getTimerDisplayHex(int value)
{
    // Value table for converting display value to hex 
    int lookUpTable[10];

    // Populate values from 0 to 9
    lookUpTable[0] = 0x3F;
    lookUpTable[1] = 0x6;
    lookUpTable[2] = 0x5B;
    lookUpTable[3] = 0x4F;
    lookUpTable[4] = 0x66;
    lookUpTable[5] = 0x6D;
    lookUpTable[6] = 0x7D;
    lookUpTable[7] = 0x7;
    lookUpTable[8] = 0x7F;
    lookUpTable[9] = 0x67;

    return lookUpTable[value];
}
void displayTime() {
    *((char*)hex_ptr) = getTimerDisplayHex(h1);
	*((char*)hex_ptr + 1) = getTimerDisplayHex(h2);
	*((char*)hex_ptr + 2) = getTimerDisplayHex(s1);
    *((char*)hex_ptr + 3) = getTimerDisplayHex(s2);
    *((char*)hex_ptr2) = getTimerDisplayHex(m1);
    *((char*)hex_ptr2 + 1) = getTimerDisplayHex(m2);
}
void displayLapped() {
    *((char*)hex_ptr) = getTimerDisplayHex(lapH1);
    *((char*)hex_ptr + 1) = getTimerDisplayHex(lapH2);
    *((char*)hex_ptr + 2) = getTimerDisplayHex(lapS1);
    *((char*)hex_ptr + 3) = getTimerDisplayHex(lapS2);
    *((char*)hex_ptr2) = getTimerDisplayHex(lapM1);
    *((char*)hex_ptr2 + 1) = getTimerDisplayHex(lapM2);
}
void updateTimer(int time){
    //look at seconds
    h1 += time;
    if (h1 > 9){
        h1 = 0;
        h2++;
    }
    if (h2 > 9){
        h2 = 0;
        s1++;
    }

    if (s1 > 9){
        s1 = 0;
        s2++;
    }
    if (s2 > 5){
        s2 = 0;
        m1++;
    }
    if (m1 > 9){
        m2++;
        m1 = 0;
    }
    if (m2 > 5){
        m2 = 0;
        h1++;
    }


}
void clearTime(){
    //reset the values
    h1 = 0;
    h2 = 0;
    m1 = 0;
    m2 = 0;
    s1 = 0;
    s2 = 0;
    lapS1 = 0;
    lapS2 = 0;
    lapM2 = 0;
    lapM1 = 0;
    lapH1=0;
    lapH2 = 0;
}

void storeLapTime(){
//capture the current time
    lapH1 = h1;
    lapH2 = h2;
    lapM1 = m1;
    lapM2 = m2;
    lapS1 = s1;
    lapS2 = s2;
}

void setTimer(){
    //set the hardware timer load value to 0x001E8480
    *(timer) = 0x001e8480;
}
void startTimer(){
    //start the hardware timer
    *(t_control) = 0x00000001;  //sets enable bit to 1, now we can start the timer
}
void stopTimer(){
    //pause the time, store the current time
    *(t_control) = 0x00000000; //set enable bit of control register to 0
}

int checkTimer(){
    //check if the timer has counted all the way down
    //if it has, return 1, else return 0
    //set the timer back to the interval

    if (*(t_interrupt) == 0b001){   //check the 0 bit in interrupt status register
        //timer has counted down
        *(t_interrupt) = 0b001;
        setTimer();
        return 1;
    }else{
        return 0;
    }

}


int main() {

    *(timer) = 0x001e8480;
    //*(t_control) = 0x00000001;  //sets enable bit to 1, now we can start the timer

    while(1){
        if (*buttons == 0b0001){//if button pressed, set condition to start timer
            startToggle = 1;
        }
        if (startToggle == 1){ //start the timer
            startTimer();
        }

        if (*buttons == 0b0010){    //if the stop button is pressed
            //stop the timer
            startToggle = 0;    //turn off the condition
            stopTimer();    //write enable bit 0
        }

        if (*buttons == 0b1000){
            //clear all values to 0
            clearTime();
        }

        if (*buttons == 0b0100){
            //store the lap
            storeLapTime();

        }

        if (checkTimer() == 1 && startToggle == 1){ //if the timer has counted down from its value, add a ms to stopwatch
            updateTimer(1);
            if(*SWITCH == 0){
                displayTime();
            } else {
                displayLapped();
            }
            
        }
        if(*buttons == 0b0010){
            stopTimer();
        }
    }
    return 0;
}
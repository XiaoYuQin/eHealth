
/*
>> Pulse Sensor Amped 1.1 <<
This code is for Pulse Sensor Amped by Joel Murphy and Yury Gitman
    www.pulsesensor.com 
    >>> Pulse Sensor purple wire goes to Analog Pin 0 <<<
Pulse Sensor sample aquisition and processing happens in the background via Timer 2 interrupt. 2mS sample rate.
PWM on pins 3 and 11 will not work when using this code, because we are using Timer 2!
The following variables are automatically updated:
Signal :    int that holds the analog signal data straight from the sensor. updated every 2mS.
IBI  :      int that holds the time interval between beats. 2mS resolution.
BPM  :      int that holds the heart rate value, derived every beat, from averaging previous 10 IBI values.
QS  :       boolean that is made true whenever Pulse is found and BPM is updated. User must reset.
Pulse :     boolean that is true when a heartbeat is sensed then false in time with pin13 LED going out.

This code is designed with output serial data to Processing sketch "PulseSensorAmped_Processing-xx"
The Processing sketch is a simple data visualizer. 
All the work to find the heartbeat and determine the heartrate happens in the code below.
Pin 13 LED will blink with heartbeat.
If you want to use pin 13 for something else, adjust the interrupt handler
It will also fade an LED on pin fadePin with every beat. Put an LED and series resistor from fadePin to GND.
Check here for detailed cod e walkthrough:
http://pulsesensor.myshopify.com/pages/pulse-sensor-amped-arduino-v1dot1

Code Version 02 by Joel Murphy & Yury Gitman  Fall 2012
This update changes the HRV variable name to IBI, which stands for Inter-Beat Interval, for clarity.
Switched the interrupt to Timer2.  500Hz sample rate, 2mS resolution IBI value.
Fade LED pin moved to pin 5 (use of Timer2 disables PWM on pins 3 & 11).
Tidied up inefficiencies since the last version. 
*/


//  VARIABLES
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadePin = 5;                  // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin


// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

const int function1Key = 7;     //高电平有效
const int function2Key = 8;     //低电平有效

int function1Count;
int function2Count;
int function2BMP;

int beatInvok;
int beatInvokFlag;

typedef enum{
  INIT_SIM808ING,
  INIT_SIM8080K,
  NONE_FUNCTION,
  FUNCTION1,
  FUNCTION2
}Status;

Status status;

void setup(){
  uartInit();
  buttonInit();
  status = INIT_SIM808ING;
  //gpsInit();
#if 1  
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(fadePin,OUTPUT);          // pin that will fade to your heartbeat!
  //Serial.begin(115200);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
   // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
   // AND APPLY THAT VOLTAGE TO THE A-REF PIN
   //analogReference(EXTERNAL);   
#endif   
}



void loop(){

  /*if (QS == true){                    
        fadeRate = 255;     
        //debugx("AT+BT=");            
        //debug(BPM);  
        QS = false;     
        beatInvokFlag ++;
        beatInvok += BPM;
        if(beatInvokFlag > 9)
        {
          beatInvok = (int)(beatInvok/10);
          debugx("AT+BT=");
          debugx(beatInvok);            
          debugx("\n");            
          beatInvokFlag = 0;
          beatInvok = 0;
        }
     }*/
  
switch(status){

    /*初始化sim808，等待sim808发送初始化成功数据*/
    case INIT_SIM808ING:
    {
      setLedFreq(100);
      debug("INIT_SIM808ING");
      delay(5000);
     status = INIT_SIM8080K;
     gpsInit();
     // gpsDisinit();
      //status = FUNCTION1;
    }
    break;
    /*开启sim808 gps 模块，计算sim808 位置及海拔*/
    case INIT_SIM8080K:
    {
      handleUartData();
      if(isGpsPositioned() == true){
        status = NONE_FUNCTION;
      }
    }
    break;

    case NONE_FUNCTION:
    {
       handleUartData();
      if (QS == true){                    
        fadeRate = 255;     
        //debugx("AT+BT=");            
        debug(BPM);  
        QS = false;     
        beatInvokFlag ++;
        beatInvok += BPM;
        if(beatInvokFlag > 24)
        {
          beatInvok = (int)(beatInvok/25);
          debugx("AT+BT=");
          debugx(beatInvok);            
          debugx("\n");            
          beatInvokFlag = 0;
          beatInvok = 0;
        }
      }
    }
    break;
}
        
#if 0
  switch(status){
    #if 0
    /*初始化sim808，等待sim808发送初始化成功数据*/
    case INIT_SIM808ING:
    {
      setLedFreq(100);
      debug("INIT_SIM808ING");
      delay(5000);
     status = INIT_SIM8080K;
     gpsInit();
     // gpsDisinit();
      //status = FUNCTION1;
    }
    break;
    /*开启sim808 gps 模块，计算sim808 位置及海拔*/
    case INIT_SIM8080K:
    {
      handleUartData();
      if(isGpsPositioned() == true){
        status = NONE_FUNCTION;
      }
    }
    break;
    #endif
    /*一切就绪后等待用户选择模式*/
   /* case NONE_FUNCTION:
    {
      setLedFreq(500);
      if((digitalRead(function1Key) == HIGH)){
        delay(3000);
        if(digitalRead(function1Key) == HIGH)
        {
          debug("function1Key press");
          setLedFreq(20);
          status = FUNCTION1;
          function1Count = 0;
        }
      }
      if(digitalRead(function2Key) == LOW){
        delay(3000);
        if(digitalRead(function2Key) == LOW)
        {
          debug("function2Key press");
          status = FUNCTION2;
          function2BMP = 0;
          setLedFreq(30);
          function2Count = 0;
        }
      }
    }
    break;*/
    /*每10秒上传一次当前数据，连续采集10次，上传10次*/
    case FUNCTION1:
    {
      //debug("FUNCTION1");
      {
        if (QS == true){   
          fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
              String sss = "AT+BT="+BPM;
              debug(BPM);
          debug(sss);
          //upDateHrvDataToThingworx(BPM);

          QS = false;                      // reset the Quantified Self flag for next time    
         // function1Count++;
         // if(function1Count >= 10)  status = NONE_FUNCTION;
        }
        ledFadeToBeat();
      }      
    }
    break;
    
    /*连续采集20次心率数据，求平均值后将数据上传到云，并且发送短信到固定的手机*/
    case FUNCTION2:
    {
      //debug("FUNCTION2");
        if (QS == true){                   // Quantified Self flag is true when arduino finds a heartbeat
          fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
     debug(BPM);
          String sss = "AT+BT="+BPM;
          debug(sss);
          QS = false;                      // reset the Quantified Self flag for next time    
          if(function2BMP == 0) function2BMP = BPM;
          else
            function2BMP = (function2BMP+BPM)/2;
 
          function2Count++;
          if(function2Count >= 20){
              //upDateHrvDataToThingworx(function2BMP);
              //sendMessageToPhone(function2BMP);
              status = NONE_FUNCTION;
          }
        }
        ledFadeToBeat();
    }
    break;
  }
  #endif
  // handleUartData();
#if 0  
  sendDataToProcessing('S', Signal);     // send Processing the raw Pulse Sensor data
  if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
        fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
        sendDataToProcessing('B',BPM);   // send heart rate with a 'B' prefix
        sendDataToProcessing('Q',IBI);   // send time between beats with a 'Q' prefix
        QS = false;                      // reset the Quantified Self flag for next time    
     }
  
  ledFadeToBeat();
  
  delay(20);                             //  take a break
#endif 
}


void ledFadeToBeat(){
    fadeRate -= 15;                         //  set LED fade value
    fadeRate = constrain(fadeRate,0,255);   //  keep LED fade value from going into negative numbers!
    analogWrite(fadePin,fadeRate);          //  fade LED
  }


void sendDataToProcessing(char symbol, int data ){
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
  }


void buttonInit(){
  pinMode(function1Key, INPUT);   
  pinMode(function2Key, INPUT_PULLUP);   
}





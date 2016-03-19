#include <SoftwareSerial.h>

#define GPD_BUF_LEN 120


bool isBeginRcv;
char rcvBuf[120];
int rcvIndex;
int lastPoint;

class Gps{
	public:
		String longitude;
		String latitude;
		bool isPositioned;
		double altitude;
};

Gps gpsData;


SoftwareSerial mySerial(10, 11); //本软串口用作打印调试输出
void uartInit()
{
	Serial.begin(9600);			//硬串口用作arduino与sim808通讯口
	pinMode(1, OUTPUT);
	mySerial.begin(9600);

	mySerial.println("uart init");
}
void upDateHrvDataToThingworx(int hrvData)
{
    mySerial.println("AT+BT="+hrvData);
	/*
    Serial.println("AT+CSTT=\"UNINET\"");
	uartDebug(1000);
	Serial.println("AT+CIICR");
	uartDebug(1000);
	Serial.println("AT+CIFSR");
	uartDebug(1000);
	Serial.println("AT+CIPSTART=\"tcp\",\"ge2-4022.cloud.thingworx.com\",\"80\"");
	uartDebug(8000);
	Serial.println("AT+CIPSEND");
	uartDebug(2000);
	String strHrv = ""+hrvData;
	//debug(strHrv);
	String ss;
	ss = "POST /Thingworx/Things/eHealthThing/Services/eHealthService?appKey=8498cdb8-c245-485f-96ff-de108ec74fc2&method=post&x-thingworx-session=true&heartBeat=";
	ss += hrvData;
	ss +="&";
	ss +="longitude";
	ss +="=";
	ss +=gpsData.longitude;
	ss +="&";
	ss +="latitude";
	ss +="=";
	ss +=gpsData.latitude;		
	ss +=" HTTP/1.1\r\n";
	ss +="Host: ge2-4022.cloud.thingworx.com\r\n";
	ss +="Content-Type: text/html\r\n";
	ss +="\r\n";

	Serial.print(ss);
	Serial.write(0x1a);
	uartDebug(3000);
	Serial.println("AT+CIPSHUT");
	uartDebug(2000);
	mySerial.println("loop");
    */
}
void uartDebug(int x)
{
  for(int i=0;i<x;i++)
  {
    if (Serial.available())
    mySerial.write(Serial.read());    
    delay(1);
  }
}
void debug(char *str)
{
	mySerial.println(str);
}
void debug(int d){
	mySerial.println(d);	
}
void debug(String d){
    mySerial.println(d);    
}
void debugx(String d)
{
    mySerial.print(d);    
}
void debugx(int d)
{
    mySerial.print(d);    
}
void handleUartData()
{
	if (Serial.available())
	{
		char in;
		in = Serial.read();
		//mySerial.print(in);
		handleGps(in);
	}
}
void gpsInit()
{
	mySerial.println("gps init");
	Serial.println("AT+CGNSPWR=1");
	delay(1000);
	Serial.println("AT+CGNSTST=1");
	delay(1000);
}
void gpsDisinit(){
	Serial.println("AT+CGNSTST=0");
	delay(300);
	Serial.println("AT+CGNSPWR=0");
	delay(300);
}
void clear()
{
    for(int i=0;i<120;i++)
    {
        rcvBuf[i]=0;
    }
    rcvIndex =0;
}
void parseGpsData()
{
    char pointArray[15];
    int x = 0;
    if((rcvBuf[1]!='G')||(rcvBuf[2]!='P')||(rcvBuf[3]!='G')||(rcvBuf[4]!='G')||(rcvBuf[5]!='A'))
        return;
    for(int i=0;i<rcvIndex;i++)
    {
        if(rcvBuf[i] == ',')
        {
            pointArray[x]=i;
            x++;
//            qDebug()<<QString::number(i,10);
        }
    }
//    qDebug()<<"x = "+QString::number(x,10);
    for(int i=0;i<x;i++)
    {
    	String data = "";
        if(i == 0)
        {
            //qDebug()<<QString::number(i,10)+"  len = "+QString::number(pointArray[i],10);
        }
        else
        {
            int len = pointArray[i]-pointArray[i-1];
			//qDebug()<<QString::number(i,10)+"  len = "+QString::number(len,10);
            //char values[20];
            //memset(values,0,20);
            int begin = pointArray[i-1]+1;
            for(int a = 0;a<len-1;a++)
            {
                //values[a] = rcvBuf[begin];
                data += rcvBuf[begin];
                //mySerial.print(values[a]);                
                begin++;
            }
            mySerial.print(i);
            mySerial.println(" = "+data);
            //if(i == )

            //qDebug()<<"value = "+QString(QLatin1String(values));
            //mySerial.println(values);
        }
        if((i == 6)&&(data == "1")){
        	gpsData.isPositioned = true;        	
        }
        else if((i == 2)&&(data!= "")){
        	// gpsData.longitude = atof(data.c_str());
        	gpsData.longitude = data;

            mySerial.println("AT+LO="+gpsData.longitude);
        	//mySerial.print("longitude = ");
        	//mySerial.println(gpsData.longitude);
        }
        else if((i == 4)&&(data!= "")){
        	// gpsData.latitude = atof(data.c_str());
        	gpsData.latitude = data;
            mySerial.println("AT+LA="+gpsData.latitude);
        	//mySerial.print("latitude = ");
        	//mySerial.println(gpsData.latitude);
        }
        // else if((i == 9)&&(data!= "")){
        // 	gpsData.altitude = atof(data.c_str());
        // 	mySerial.print("altitude = ");
        // 	mySerial.println(gpsData.altitude);
        // }
    }
    clear();
    if(gpsData.isPositioned == true){
    	gpsDisinit();
    }
    else{
    	gpsData.isPositioned = false;
    	gpsData.longitude = "";
    	gpsData.latitude = "";	
    	gpsData.altitude = 0;	
    }
}
void handleGps(char in)
{
    if(in == '$')
    {
        isBeginRcv = true;
        clear();
        //qDebug()<<"rcv begin";
    }
    if((in == '*')&&(isBeginRcv == true))
    {
        isBeginRcv = false;
        //QString string = QString(QLatin1String(rcvBuf));
        //qDebug()<<string;
        //qDebug()<<"rcv end";
        parseGpsData();
    }
    if(isBeginRcv == true)
    {
        rcvBuf[rcvIndex] = in;
        rcvIndex++;
    }
}
bool isGpsPositioned(){
	return gpsData.isPositioned;
}
// void testGpsData()
// {
// 	float a1 = 3746.73;
// 	float b1 = 11234.14;

// 	int a3 = a1*100;
// 	int b3 = b1*100;

// 	int longitude = a3/10000;
// 	int latitude = b3/10000;

// 	mySerial.println(longitude);
// 	mySerial.println(latitude);

// 	float a2 = int(a1*100)%10000;
// 	float b2 = int(b1*100)%10000;

// 	mySerial.println(a2);
// 	mySerial.println(b2);
// }

void sendMessageToPhone(int rate){
	Serial.println("AT+CGMR");
	uartDebug(2000);
	Serial.println("AT+CSCA?");
	uartDebug(2000);
	Serial.println("AT+CMGF=1");
	uartDebug(2000);
	Serial.println("AT+CMGS=\"18335120603\"");
	uartDebug(300);
	String ss;
	ss = "Your average heart rate is "/*+rate*/; 
	ss += rate;
	ss += " BPM.";
	// ss += " bpm.";
	Serial.print(ss);
	Serial.write(0x1a);
}

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include "pitches.h"
RTC_DS1307 rtc;

byte ue[8]={
 B10001,
 B00000,
 B10001,
 B10001,
 B10001,
 B10011,
 B01101,
 B00000};
byte upar[8]={
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};
byte dnar[8]={
 B00100,
 B00100,
 B00100,
 B00100,
 B00100,
 B10101,
 B01110,
 B00100 
};
byte degr[8]={
 B01110,
 B01010,
 B01110,
 B00000,
 B00000,
 B00000,
 B00000,
 B00000};
 
byte yes[8]={
  B00000,
  B00000,
  B00000,
  B00001,
  B00010,
  B10100,
  B01000};

//Thx Chris ;)
byte bell[8]={
   B00000,
   B00100,
   B01110,
   B01110,
   B01110,
   B11111,
   B00100,
   B00000}; 
  

//Buttons
 int butMid=5;
 int butMidOld=0;
 int butR=6;
 int butROld=0;
 int butL=7;
 int butLOld=0;
 int butMidstate=0;
 int butRstate=0;
 int butLstate=0;
//LDCD
  int RS=2;  //Registerselect
  int RW=3;
  int E=4;   //LCD-Clock
  int DB4=8; //Dataline 4
  int DB5=9;  // 5
  int DB6=10;  // 6
  int DB7=11;  // 7
  int backLight=12;
  int buzzer=13;
  LiquidCrystal lcd(RS,RW,E,DB4,DB5,DB6,DB7);
  //various vars.
  int pos=0;
  long prevLCDMillis;
  long dispDelay = 4000;  //standard display timeout
  boolean useTimeout = false;
  long lcdDelay = 1000; //Update interval for LCD
  int inMenu=0;
  int menuIndex=0;
  const int ANZENT=7;      //alarm    alarmtime  sound   Timepos.  Temperature 
  String entries[ANZENT] = {"Wecker","Weckzeit","Ton","Zeitpos.","Temperatur","Timeout","Back"};
  int timePos=0;
  int weckerUsed=0;
  int tempUsed=0;
  int dispOn=1;
  long timeMillis;
  int alarmTime[2]={6,15}; //default alarm time

void setup()
{
  pinMode(butMid,INPUT);
  pinMode(butR,INPUT);
  pinMode(butL,INPUT);
  pinMode(backLight,OUTPUT);
  digitalWrite(backLight,HIGH);
  lcd.createChar(0,ue);    //ü
  lcd.createChar(1,degr);  //°
  lcd.createChar(2,upar);  //uparrow
  lcd.createChar(3,dnar);  //downarrow
  lcd.createChar(4,yes);
  lcd.createChar(5,bell);
  Wire.begin();
  rtc.begin();
  if(!rtc.isrunning()){
   rtc.adjust(DateTime(__DATE__, __TIME__)); 
  }
  lcd.begin(16,2);
  lcd.print("CLK v1.7b");
  lcd.setCursor(0,1);
  lcd.print("By Orangensaft");
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long curMillis=millis();
  butMidstate=digitalRead(butMid);
  butRstate=digitalRead(butR);
  butLstate=digitalRead(butL);  
  if(butRstate!=butROld && butRstate==1){  //right button
   if(dispOn==0){
    digitalWrite(backLight,HIGH);
    dispOn=1;
   }else{
   if(inMenu==1){
     menuIndex=menuIndex+1;
     if(menuIndex==ANZENT){
      menuIndex=0; 
     }
    showMenu(menuIndex);
   }
   }
   timeMillis=curMillis;
  }
  if(butLstate!=butLOld && butLstate==1){  //left button
   if(dispOn==0){
    digitalWrite(backLight,HIGH);
    dispOn=1;
   }else{
   if(inMenu==1){
    menuIndex=menuIndex-1;
    if(menuIndex<0){
     menuIndex=ANZENT-1; 
    }
    showMenu(menuIndex);
   }
   }
   timeMillis=curMillis;
  }
  if(butMidstate!=butMidOld && butMidstate==1){  //middle button
   if(dispOn==0){
    digitalWrite(backLight,HIGH);
    dispOn=1;  
   }else{
   if(inMenu==0){      //Open Menu
     inMenu=1;
     showMenu(0);
     menuIndex=0;
   }else{//im menü
    if(menuIndex==ANZENT-1){  //Menüpunkt "back"
     inMenu=0; 
     lcd.clear();
     timeMillis=curMillis;
    } 
    if(menuIndex==0){       //Option "Alarm"
     weckerUsed=rev(weckerUsed);
     lcd.clear();
     showMenu(menuIndex);
    }
    if(menuIndex==1){
      increaseAlarm(15);
      lcd.clear();
      showMenu(menuIndex); 
      
    }
    if(menuIndex==3){     //Option "Timeposition"
      timePos=rev(timePos);
      lcd.clear();
      showMenu(menuIndex);
    }
    if(menuIndex==4){
     tempUsed=rev(tempUsed);
     lcd.clear();
     showMenu(menuIndex); 
    }
    if(menuIndex==5){
     if(useTimeout){
        if(dispDelay==10000){
        useTimeout=false;
        }else{
          dispDelay+=3000;
        }
     }else{
     useTimeout=true;
     dispDelay=4000;
     }
    lcd.clear();
    showMenu(menuIndex); 
   }
     }
   }
   timeMillis=curMillis;
  }
  //Update Time
  if(curMillis-prevLCDMillis > lcdDelay && inMenu==0){
    prevLCDMillis=curMillis;
    DateTime now = rtc.now();
    printDate(rev(timePos),now);
    printTime(timePos,0,now);
    if((now.second()==0 || now.second()==1) && weckerUsed==1 && now.hour()==alarmTime[0] && now.minute()==alarmTime[1]){
     alarm();
    }
  }
  if((curMillis-timeMillis > dispDelay) && inMenu==0 && useTimeout && dispOn==1){
    digitalWrite(backLight,LOW);
    dispOn=0;
  }
  
  butLOld=butLstate;  //"soft" debounce
  butROld=butRstate;
  butMidOld=butMidstate;
}

/*
Show Menu and current option
Option to show <-no
*/
void showMenu(int no){
 lcd.clear();
 lcd.setCursor(3,0);
 lcd.print("---Men");
 lcd.write(byte(0));
 lcd.print("---");
 lcd.setCursor(0,1);
 lcd.print(entries[no]); 
 if(no==0){//Alarm on/off
   if(weckerUsed==0){
    lcd.print(" x"); 
   }else{
    lcd.print(" ");
    lcd.write(byte(4)); 
   }
 }
 if(no==1){
  lcd.print(" ");
  printAlarm(); 
 }
 if(no==3){
   if(timePos==0){
     lcd.print(" ");
     lcd.write(byte(2));//uparrow
   }else{
    lcd.print(" ");
    lcd.write(byte(3));//downarrow 
   }
 }
 if(no==4){
  if(tempUsed==0){
   lcd.print(" x");
  }else{
   lcd.print(" ");
   lcd.write(byte(4));
  }
 }
 if(no==5){
   if(useTimeout){
    lcd.print(" ");
    lcd.print((int)dispDelay/1000); 
    lcd.print("s");
   }else{
    lcd.print(" x"); 
   }
 }
}

/*
Output date to LCD
*/
void printDate(int row,DateTime now){
    lcd.setCursor(0,row);
    lcd.print(getDay(now.dayOfWeek()));
    lcd.print(" ");
    if(now.day()<10){
       lcd.print("0"); 
    }
    lcd.print(now.day());
    lcd.print(".");
    if(now.month()<10){
       lcd.print("0"); 
    }
    lcd.print(now.month());
    lcd.print(".");
    lcd.print(now.year());
    if(weckerUsed==1){
    lcd.print("  ");
    lcd.write(byte(5));
    }
}

/*
Output time to LCD
*/
void printTime(int row,int offset,DateTime now){
    lcd.setCursor(offset,row);
    if(now.hour()<10){
       lcd.print("0"); 
    }
    lcd.print(now.hour());
    if(pos==0){
       lcd.print(":");
       pos=1; 
      }else{
       lcd.print(" ");
       pos=0; 
    }
    if(now.minute()<10){
       lcd.print("0"); 
    }
    lcd.print(now.minute());
    lcd.print(" Uhr");
    if(tempUsed==1){  //Temp!
     lcd.print(" ");
     lcd.print(getTemp());
     lcd.write(byte(1));
     lcd.print("C");
    }
}

void increaseAlarm(int offset){
  int hours=alarmTime[0];
  int mins=alarmTime[1];
  if(mins+offset>59){
     mins=(mins+offset)-60;
     hours=hours+1; 
  }else{
     mins=mins+offset; 
  }
  if(hours>23){
   hours=0; 
  }
  alarmTime[0]=hours;
  alarmTime[1]=mins;
}

void printAlarm(){
   if(alarmTime[0]<10){
    lcd.print("0");
   } 
   lcd.print(alarmTime[0]);
   lcd.print(":");
   if(alarmTime[1]<10){
    lcd.print("0"); 
   }
   lcd.print(alarmTime[1]);
}

/*
Pseudo invert
rev(n) = 1, if n=0
         0, else.
*/
int rev(int n){
 if(n==0){
  return 1; 
 }
 return 0;
}

/*
Return Day of week
*/
String getDay(uint8_t day){
  String days[7]={"So","Mo","Di","Mi","Do","Fr","Sa"};
  return days[day];
}

void alarm(){
  while(digitalRead(butMid)!=1 && digitalRead(butL)!=1 && digitalRead(butR)!=1){
   tone(buzzer,NOTE_E6);
   digitalWrite(backLight,LOW);
   delay(250);
   noTone(buzzer);
   tone(buzzer,NOTE_C6);
   digitalWrite(backLight,HIGH);
   delay(250); 
   noTone(buzzer);
  }
}


/*
Get temperature
(Implemented as soon as DHT22 arrives)
*/
float getTemp(){
    return 0.0;
}

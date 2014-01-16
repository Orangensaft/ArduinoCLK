#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

RTC_DS1307 rtc;

/*
Planned:
->Menü  (+4Buttons (Menü,+,-,Ok)   OK!
-->Wecker (+Buzzer)                OK! (BUZZER EINBAUEN!!)
-->Weckton                          OK! (MENÜeintrag)
-->Weckzeit + Tag?                  OK! (Menüeintrag)
-->Display nur alle X Minuten anmachen  (Weiß nich ob gut)
-->Display timeout                (Noch zu machen! LCD PIN 15 -> ARDUINO 12. 12 High -> LCD AN. 12 Low -> LCD aus.
-->Temperatur / Luftfeuchtigkeit                            OK!(Menüeintrag)

BUTTON PINS:###############
BUTMitte : 5
BUTy : 6
BUTz : 7
RTC PINS:###################
Arduino - RTC
A5  - SCL
A4  - SDA
LCD PINS:################
1- GND
2- +5
3-BACKGRND (GND)
4- PIN12    ->(RS)
5- PIN 6    ->(RW)
6 - PIN 11  ->(E)
7-10 NC    ->D0-D3
11- PIN 10 ->D4
12- PIN 9  ->D5
13- PIn 8  ->D6
14- PIN 7  ->D7
15- +5
16- GND
*/

byte ue[8]={
 B10001,
 B00000,
 B10001,
 B10001,
 B10001,
 B10011,
 B01101,
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
//zustände    //0-> Uhrzeit      1->Menüoberfläche   2-> ...
 int state=0;
//Wecker
  int RS=2;  //Registerauswahl
  int RW=3;
  int E=4;   //LCD-Clock
  int DB4=8; //Datenleitungen 4
  int DB5=9;  // 5
  int DB6=10;  // 6
  int DB7=11;  // 7
  LiquidCrystal lcd(RS,RW,E,DB4,DB5,DB6,DB7);
  int pos=0;
  long prevLCDMillis;
  long lcdDelay = 1000; //alle sekunde updaten
  int inMenu=0;
  int menuIndex=0;
  const int ANZENT=6;
  String entries[ANZENT] = {"Wecker","Weckzeit","Ton","Zeitpos.","Temperatur","Back"};
  int timePos=0;
  int weckerUsed=0;
void setup()
{
  pinMode(butMid,INPUT);
  pinMode(butR,INPUT);
  pinMode(butL,INPUT);
  lcd.createChar(0,ue);
  Wire.begin();
  rtc.begin();
  if(!rtc.isrunning()){
   rtc.adjust(DateTime(__DATE__, __TIME__)); 
  }
  lcd.begin(16,2);
  lcd.print("CLK v1.6c");
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
  if(butRstate!=butROld && butRstate==1){  //rechter button
   if(inMenu==1){
     menuIndex=menuIndex+1;
     if(menuIndex==ANZENT){
      menuIndex=0; 
     }
    showMenu(menuIndex);
   }
  }
  if(butLstate!=butLOld && butLstate==1){  //linker button
   if(inMenu==1){
    menuIndex=menuIndex-1;
    if(menuIndex<0){
     menuIndex=ANZENT-1; 
    }
    showMenu(menuIndex);
   }
  }
  if(butMidstate!=butMidOld && butMidstate==1){  //mittler button
   lcd.setCursor(15,0);
   lcd.print("M");
   if(inMenu==0){      //in menümodus wechseln
     inMenu=1;
     showMenu(0);
     menuIndex=0;
   }else{                      //im menü
    if(menuIndex==ANZENT-1){  //Menüpunkt "back"
     inMenu=0; 
     lcd.clear();
    } 
    if(menuIndex==0){       //Menüpunkt "Wecker"
     weckerUsed=rev(weckerUsed);
     lcd.clear();
     showMenu(menuIndex);
    }
    if(menuIndex==3){     //Menüpunkt "Zeitpos."
      timePos=rev(timePos);
      lcd.clear();
      showMenu(menuIndex);
    }
   }
  }
  //Update Time
  if(curMillis-prevLCDMillis > lcdDelay && inMenu==0){
    prevLCDMillis=curMillis;
    DateTime now = rtc.now();
    printDate(rev(timePos),now);
    printTime(timePos,0,now);
  }
  butLOld=butLstate;  //soft debounce
  butROld=butRstate;
  butMidOld=butMidstate;
}

/*
Menü und aktuellen
Menüpunkt anzeigen <-no
*/
void showMenu(int no){
 lcd.clear();
 lcd.setCursor(3,0);
 lcd.print("---Men");
 lcd.write(byte(0));
 lcd.print("---");
 lcd.setCursor(0,1);
 lcd.print(entries[no]); 
 if(no==0){//Wecker an/aus
   if(weckerUsed==0){
    lcd.print(" AUS"); 
   }else{
    lcd.print(" AN"); 
   }
 }
 if(no==3){//Zeitpos.
   if(timePos==0){
     lcd.print(" O"); 
   }else{
    lcd.print(" U"); 
   }
 }
}

/*
Datum auf LCD schreiben
row -> Zeile
now -> DateTime struct
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
    lcd.print("  W");
    }
}

/*
Zeit auf LCD schreiben
row -> Zeile
offset -> Verschiebeoffset
now -> DateTime struct
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
}

/*
Pseudo invertieren
rev(n) = 1, falls n=0
         0, sonst.
*/
int rev(int n){
 if(n==0){
  return 1; 
 }
 return 0;
}

/*
Wochentag vom DateTime-Format day
als String ausgeben.
*/
String getDay(uint8_t day){
  String days[7]={"So","Mo","Di","Mi","Do","Fr","Sa"};
  return days[day];
}

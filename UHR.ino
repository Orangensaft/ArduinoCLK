#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

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
  int backLight=12;
  LiquidCrystal lcd(RS,RW,E,DB4,DB5,DB6,DB7);
  int pos=0;
  long prevLCDMillis;
  long dispDelay = 4000;  //standart delay, bis display ausgeht
  boolean useTimeout = false;
  long lcdDelay = 1000; //alle sekunde updaten
  int inMenu=0;
  int menuIndex=0;
  const int ANZENT=7;
  String entries[ANZENT] = {"Wecker","Weckzeit","Ton","Zeitpos.","Temperatur","Timeout","Back"};
  int timePos=0;
  int weckerUsed=0;
  int tempUsed=0;
  int dispOn=1;
  long timeMillis;
void setup()
{
  pinMode(butMid,INPUT);
  pinMode(butR,INPUT);
  pinMode(butL,INPUT);
  pinMode(backLight,OUTPUT);
  digitalWrite(backLight,HIGH);
  lcd.createChar(0,ue);    //ü
  lcd.createChar(1,degr);  //°
  lcd.createChar(2,upar);  //pfeil nach oben
  lcd.createChar(3,dnar);  //pfeil nach unten
  lcd.createChar(4,yes);
  lcd.createChar(5,bell);
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
  Serial.begin(9600);
}

void loop() {
  unsigned long curMillis=millis();
  butMidstate=digitalRead(butMid);
  butRstate=digitalRead(butR);
  butLstate=digitalRead(butL);  
  if(butRstate!=butROld && butRstate==1){  //rechter button
   if(dispOn==0){
    digitalWrite(backLight,HIGH);
    dispOn=1; //display timer starten!
   }else{
   if(inMenu==1){
     menuIndex=menuIndex+1;
     if(menuIndex==ANZENT){
      menuIndex=0; 
     }
    showMenu(menuIndex);
   }
   }
   timeMillis=millis();
  }
  if(butLstate!=butLOld && butLstate==1){  //linker button
   if(dispOn==0){
    digitalWrite(backLight,HIGH);
    dispOn=1; //Auch hier displaytimer starten!
   }else{
   if(inMenu==1){
    menuIndex=menuIndex-1;
    if(menuIndex<0){
     menuIndex=ANZENT-1; 
    }
    showMenu(menuIndex);
   }
   }
   timeMillis=millis();
  }
  if(butMidstate!=butMidOld && butMidstate==1){  //mittler button
   if(dispOn==0){
    digitalWrite(backLight,HIGH);
    dispOn=1;  //Display timer starten! 
   }else{
   if(inMenu==0){      //in menümodus wechseln
     inMenu=1;
     showMenu(0);
     menuIndex=0;
   }else{                      //im menü
    if(menuIndex==ANZENT-1){  //Menüpunkt "back"
     inMenu=0; 
     lcd.clear();
     timeMillis=millis();
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
   timeMillis=millis();
  }
  //Update Time
  if(curMillis-prevLCDMillis > lcdDelay && inMenu==0){
    prevLCDMillis=curMillis;
    DateTime now = rtc.now();
    printDate(rev(timePos),now);
    printTime(timePos,0,now);
  }
  Serial.print("Curmillis: ");
  Serial.print(curMillis);
  Serial.print(" timeMillis: ");
  Serial.print(timeMillis);
  Serial.print(" c-t ");
  Serial.print(curMillis-timeMillis);
  Serial.print("\n");
  if((curMillis-timeMillis > dispDelay) && inMenu==0 && useTimeout && dispOn==1){
    digitalWrite(backLight,LOW);
    dispOn=0;
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
    lcd.print(" x"); 
   }else{
    lcd.print(" ");
    lcd.write(byte(4)); 
   }
 }
 if(no==3){//Zeitpos.
   if(timePos==0){
     lcd.print(" ");
     lcd.write(byte(2));//pfeil nach oben
   }else{
    lcd.print(" ");
    lcd.write(byte(3));//pfeil nach unten 
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
    lcd.print("  ");
    lcd.write(byte(5));
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
    if(tempUsed==1){  //Temp!
     lcd.print(" ");
     lcd.print(getTemp());
     lcd.write(byte(1));
     lcd.print("C");
    }
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

/*
Get temperature
(Implemented as soon as DHT22 arrives)
*/
float getTemp(){
    return 0.0;
}

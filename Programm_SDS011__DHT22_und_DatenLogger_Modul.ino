//Einbinden der Bibliotheken
#include <DHT.h>      //Sensor DHT-22 
#include <SPI.h>      //Datenlogger 
#include <SD.h>       //Datenlooger 
#include <Wire.h>     //Datenlogger
#include "RTClib.h"   //Echtzeituhr

//Definitionen 
#define DHTPIN 2
#define DHTTYPE DHT22
#define LEN 9 
#define windowSize 10 

//Zuschreibung der Klassen: Objekte tragen nun die Egenschaften der Klassen aus der Bibliothek 
DHT dht(DHTPIN, DHTTYPE); 
File myFile; 
RTC_DS1307 rtc; 

//Definition der Variablen
const int chipSelect = 10; 
float aveArr[windowSize];
int loopcnt = 0; 
unsigned char incomingByte = 0; 
unsigned char buf[LEN];
int PM2_5Val = 0; //flaot ? 
int PM10Val = 0;
long Zeit = 0; 
long Pause = 240000;// 4 min 
long Messen = 60000;// 1 min 
 
void setup() {
    //Start der Echtzeituhr, des DHT Sensors und der SD Karte
    rtc.begin();
    dht.begin();
    Serial.begin(9600);
    pinMode(10, OUTPUT);
    SD.begin(chipSelect); 

    //Erstellen der Datei auf der SD-Karte im Format JJMMTTXX.CSV
    char filename [] = "Ordner00.CSV"; 
      DateTime now = rtc.now();
      filename[0] = (now.year()/10)%10 + '0'; 
      filename[1] = now.year()%10 + '0'; 
      filename[2] = now.month()/10 + '0'; 
      filename[3] = now.month()%10 + '0'; 
      filename[4] = now.day()/10 + '0'; 
      filename[5] = now.day()%10 + '0'; 
    for (uint8_t i = 0; i < 100; i++) {
      filename[6] = i/10 + '0';
      filename[7] = i%10 + '0'; 
    if (! SD.exists(filename)) {
      myFile = SD.open(filename, FILE_WRITE);
      break;
    }
    }

    if (!myFile) {
      Serial.println("Es konnte keine Datei erstellt werden.");
    }
    Serial.print("Starte Speichern: "); 
    Serial.println(filename);
    myFile.print ("Datum ; Uhrzeit ; Luftfeuchtigkeit [%] ; Temperatur [°C] ; PM_2,5 [µg/m^3] ; PM10 [µg/m^3]"); 
    myFile.println();

}

void loop() {

    //Auslesen der Temperatur und Luftfeuchtigkeit als einstellige Nachkommazahl 
    float h = dht.readHumidity(); 
    int vh = (int) h; 
    int nh = (h+0,5-vh)*10.0;
    float t = dht.readTemperature();
    int vt = (int) t; 
    int nt = (t+0,5-vt)*10.0;

    //Auslesen der PM Werte
    int i; 
    unsigned char checksum; 

   if (Serial.available() > 0) {                 
    incomingByte = Serial.read();           
    if (incomingByte == 0xAA) {             
      Serial.readBytes(buf, LEN);             
      if ((buf[0] == 0xC0) && (buf[8] == 0xAB)) {               
        for (i=1; i<=6; i++) {                 
          checksum = checksum + buf[i];               
          }               
          if (checksum == buf[7]) {                 
            PM2_5Val=((buf[2]<<8) + buf[1])/10;        
            PM10Val=((buf[4]<<8) + buf[3])/10;                 
         
            //Darstellen der Werte auf dem Seriellen Monitor 
            DateTime now = rtc.now(); 
            Serial.print(loopcnt);                
            Serial.println();                 
            Serial.print(now.year(), DEC);                 
            Serial.print('/');                 
            Serial.print(now.month(), DEC);                 
            Serial.print('/');                 
            Serial.print(now.day(), DEC);                 
            Serial.print(" ");                 
            Serial.print(now.hour(), DEC);                 
            Serial.print(':');                 
            Serial.print(now.minute(), DEC);                 
            Serial.print(':');                 
            Serial.print(now.second(), DEC);                 
            Serial.println();
            Serial.print("PM2.5: "); 
            Serial.print(PM2_5Val);               
            Serial.println("  µg/m3");                
            Serial.print("PM10: "); 
            Serial.print(PM10Val);                 
            Serial.println("  µg/m3");                 
            Serial.println();
            Serial.print("Luftfeuchtigkeit: ");
            Serial.print(h);                  // Ausgeben der Luftfeuchtigkeit
            Serial.print("%\t");              // Tab
            Serial.print("Temperatur: ");
            Serial.print(t);                  // Ausgeben der Temperatur
            Serial.write('°');              
            Serial.println("C");//Einheit 
           
            
            //Speichern der Daten auf der SD-Karte, Excel Kompatibel  
            if (loopcnt%windowSize == 0) {  
              myFile.print(now.day(), DEC);                   
              myFile.print('.');                   
              myFile.print(now.month(), DEC);                   
              myFile.print('.');                   
              myFile.print(now.year(), DEC);                   
              myFile.print(';'); //Semikolon, damit Excel eine neue Spalte öffnet.                   
              myFile.print(now.hour(), DEC);                   
              myFile.print(':');                   
              myFile.print(now.minute(), DEC);                   
              myFile.print(':');                   
              myFile.print(now.second(), DEC);                   
              myFile.print(';');                   
              myFile.print(vh);
              myFile.print(';');
              myFile.print(nh);
              myFile.print(';');
              myFile.print(vt);
              myFile.print(';') ;
              myFile.print('nt');
              myFile.print(';');                    
              myFile.print(PM2_5Val);                   
              myFile.print(';');                     
              myFile.print(PM10Val);                   
              myFile.println();                       
              myFile.flush(); 
            }
            

               loopcnt++;               
               }              
        else {                
          Serial.println("checksum Error");  //Fehlermeldung, sodass keine Werte gespeichert werden.       
          }             
          }             
          else {               
            Serial.println("frame error");             
            }           
              }   
                } 

    //Einstellung der Messzeit und Pause  
    while (millis() > Messen + Zeit ) {
      
    if (millis() > Pause + Zeit ) {
    
    Zeit = millis();
   
    }
    } 

}

/*

TITRE          : Gestion du servo moteur avec RPC
AUTEUR         : Franck Nkeubou Awougang
DATE           : 28/09/2022
DESCRIPTION    : Modifier l'angle du servo moteur avec le RPC en fonction de la 
				  temperature
VERSION        : 0.0.1

*/

#include <Arduino.h>
#include <Adafruit_AHTX0.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <SD.h>
#include <stdio.h>
#include "Servo.h"
#include "Timer.h"
#include "WIFIConnector_MKR1000.h"
#include "MQTTConnector.h"

Adafruit_AHTX0 aht;
RTC_DS3231 rtc;
const int chipSelect = 4;
int Valeur;
unsigned long LimitSend = 10000;
unsigned long LimitWrite = 2000;
const int LED_PIN_BLEU     = A3;
const int LED_PIN_ROUGE    = A4;

Timer Temps;
Timer Send;
const char *FileName = "log.txt";
Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position

void ActionnementServoLEDs(){
	
	if (DataRecu == "true"){
		Valeur = 255;
		if (LEDColor == "Red")
		{
			for (pos = 0; pos <= 90; pos += 1)
			{ // goes from 0 degrees to 180 degrees
				// in steps of 1 degree
				myservo.write(pos); // tell servo to go to position in variable 'pos'
				delay(15);
			}
			analogWrite(LED_PIN_ROUGE, Valeur);
			analogWrite(LED_PIN_BLEU, 0);
		}
		else if (LEDColor == "Blue")
		{
			for (pos = 0; pos <= 180; pos += 1)
			{ // goes from 0 degrees to 180 degrees
				// in steps of 1 degree
				myservo.write(pos); // tell servo to go to position in variable 'pos'
				delay(15);
			}
			analogWrite(LED_PIN_BLEU, Valeur);
			analogWrite(LED_PIN_ROUGE, 0);
		}
	}
	else if(DataRecu == "false"){
		analogWrite(LED_PIN_BLEU, 0);
		analogWrite(LED_PIN_ROUGE, 0);
		for (pos = 90; pos >= 0; pos -= 1)
		{						// goes from 180 degrees to 0 degrees
			myservo.write(pos); // tell servo to go to position in variable 'pos'
			delay(15);			// waits 15 ms for the servo to reach the position
		}
	}
	LEDColor="";
	DataRecu = "";
	Payload = "";
}

void SendMQTTFile(const char *namefile)
{
	digitalWrite(LED_BUILTIN, HIGH);
	wifiConnect(); // Branchement au réseau WIFI
	MQTTConnect(); // Branchement au broker MQTT
	// *************** Récupération de chaque ligne dans le fichier. **********
	File file = SD.open(namefile, FILE_READ);

	while (file.available())
	{
		String Payload = file.readStringUntil('\n');
		sendPayload(Payload);
		Serial.println(Payload);
		
		ClientMQTT.loop(); 
		
		
		delay(100);
	}
	// On patiente une seconde de plus pour recevoir le message mqtt du serveur.
	Timer Wait;
	Wait.startTimer(500);
	while (!Wait.isTimerReady())
	{
		ClientMQTT.loop(); 
		
	}
	// On fait les actions nécessaires.
	ActionnementServoLEDs();
	file.close();

	// Supprimer le fichier une fois terminé. En supposant l'envoi a été faite correctement.
	SD.remove(FileName);
	status = WL_IDLE_STATUS;	
	WiFi.disconnect();
	WiFi.end();
	digitalWrite(LED_BUILTIN, LOW);
}

void WriteFile()
{
	DateTime now = rtc.now();
	char Chaine[80] = {0};
	sensors_event_t humidity, temp;
	aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data

	sprintf(Chaine, "{\"ts\":%.0f,\"values\":{\"Temperature\":%.3f,\"Humidite\":%.3f}}", (double)(now.unixtime() + 10800)* 1000, temp.temperature, humidity.relative_humidity);

	File file = SD.open(FileName, FILE_WRITE);
	int Status = file.println(Chaine);
	if (Status)
	{
		Serial.print("\nSize written = ");
		Serial.println(Status);
	}
	Serial.print("\nString written = ");
	Serial.println(Chaine);
	file.close();
	// On relance le compte à rebour pour l'envoie des données.
	Temps.startTimer(LimitWrite);
}

void setup()
{
  	myservo.attach(6);  // attaches the servo on pin 9 to the servo object

	Serial.begin(9600);
	// while (!Serial); // wait for serial port to connect. Needed for native USB
	Serial.println("Adafruit AHT10/AHT20 demo!");
	if (!aht.begin())
	{
		Serial.println("Could not find AHT? Check wiring");
		while (1)
			delay(1000);
	}
	Serial.println("AHT10 or AHT20 found");
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(LED_PIN_BLEU, OUTPUT);
	pinMode(LED_PIN_ROUGE, OUTPUT);
	/* ... */
	if (!rtc.begin())
	{
		Serial.println("Couldn't find RTC");
		Serial.flush();
		while (1)
			delay(1000);
	}
	rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	if (rtc.lostPower())
	{
		Serial.println("RTC lost power, let's set the time!");
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}

	Serial.print("Initializing SD card...");
	if (!SD.begin(chipSelect))
	{
		Serial.println("Card failed, or not present");
		// don't do anything more:
		while (1)
			delay(1000);
	}
	SD.remove(FileName);
	Serial.println("card initialized.");
	
	// Suppression du fichier s'il existe deja.
	Temps.startTimer(LimitWrite);
	Send.startTimer(LimitSend);

}


void loop()
{

	//{"TimeDate":2022-9-27 21:40:37,"Temperature":24.96,"Humidite":65.44}

	// *************** Ecriture dans la carte SD ***************
	
	if (Temps.isTimerReady())
	{
		WriteFile();
		Temps.startTimer(LimitWrite);
	}

	if (Send.isTimerReady())
	{
		SendMQTTFile(FileName);
		Send.startTimer(LimitSend);
	}
	/*
		for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
		// in steps of 1 degree
		myservo.write(pos);              // tell servo to go to position in variable 'pos'
		delay(15);                       // waits 15 ms for the servo to reach the position
	}
	for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
		myservo.write(pos);              // tell servo to go to position in variable 'pos'
		delay(15);                       // waits 15 ms for the servo to reach the position
	}
	*/
}















/*
void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
}

void loop() {
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
}

*/
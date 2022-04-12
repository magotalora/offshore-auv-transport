#include "cc1000.h"
#include <TinyGPS.h>
#include <DHT.h>
float lat, lon;
int year;
byte month, day, hour, minute, second, hundredths;
TinyGPS gps;
Cc1000 trx;

const int leakdigital = 4; // leak pin

#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


//////////////// SETUPS OF MODULES BELOW


void leaksetup(){
  pinMode(leakdigital, INPUT);
}

void dhtsetup(){
    dht.begin();
}

// the setup function runs once when you press reset or power the board
void txsetup() {
  // initialize digital pin 13 (LED diode) as an output.
  pinMode(13, OUTPUT);

  //inicjalization cc1000 module
  trx.init();
  trx.set_modem_mode(RTTY_MODE);
  trx.set_power(PA_VALUE_0DBM);
  trx.set_deviation(600);
  trx.set_bitrate(300);
  trx.set_frequency(432920000);
  //trx.set_frequency(432920000, VCO_AB, true);
  trx.set_trx_mode(TX_MODE);
  Serial.write(" test.\n");
  Serial.write("modem mode:");
  Serial.print(trx.get_modem_mode(), DEC);
  Serial.write(" power: ");
  Serial.print(trx.get_power(), DEC);
  Serial.write(" deviation: ");
  Serial.print(trx.get_deviation(), DEC);
  Serial.write(" bitrate: ");
  Serial.print(trx.get_bitrate(), DEC);
  Serial.write(" frequency: ");
  Serial.print(trx.get_frequency(), DEC);
  Serial.write("\n");
  //trx.set_trx_mode(PD_MODE);

}
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  txsetup();
  leaksetup();
  dhtsetup();
}


//////////////// FUNCTIONS OF SENSORS BELOW

bool checkleak()
{
  if (digitalRead(leakdigital) == LOW) {
    Serial.println("Digital value : wet"); 
    return true;
    delay (10);
  } else {
    Serial.println("Digital value : dry");
    return false;
  }
}

bool checkfire()
{
    //Read data and store it to variables hum and temp
    int hum = dht.readHumidity();
    int temp= dht.readTemperature();
    //Print temp and humidity values to serial monitor
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    delay(1000); ///// double check
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");

  if (digitalRead(temp) >= 70) {
    Serial.println("Fire on board"); 
    return true;
    delay (10);
  } else {
    Serial.println("No fire on board");
    return false;
  }
}



/////////////////////////RADIO + GPS FUNCTIONS BELOW
void txgpsproblem() {
  checkfire();
  checkleak();
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  String str = "";
  str = str + "$$$$$$$$ @@@@@@@@ ######## $$$$$$$$ "
        + "GPS not detected"
        + " Frequency: " + trx.get_frequency() + "Hz"
        + " $$$$$$$$\n";
    if (checkleak()==true){
    str = str + "Leak on board %#$$#$#$#$##$  ";
    }
    if (checkfire()==true){
    str = str + "Fire on board %#$$#$#$#$##$  ";
  }
  Serial.write((char*)str.c_str());
  trx.send_data(str);
  trx.set_trx_mode(TX_MODE);

  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(3000);
}
void TXgps()
{
  checkfire();
  checkleak();
  gps.f_get_position(&lat, &lon);
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
  String str = " ";
  str = str + "$$$$$$$$ @@@@@@@@ ######## $$$$$$$$" + "Latitude: " + String(lat, 6) + "Longitude: " + String(lon, 6);
  if (checkleak()==true){
    str = str + "Leak on board %#$$#$#$#$##$";
  }
  if (checkfire()==true){
    str = str + "Fire on board %#$$#$#$#$##$  ";
  }  
  Serial.write((char*)str.c_str());
  trx.send_data(str);
  trx.set_trx_mode(TX_MODE);
  Serial.println();
  delay(5000);
}
//////////////////////LOOP
void loop() {
  while (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
      TXgps();
      if ((millis() > 5000) && String(lon,6).length()<7 && String(lat,6).length()<7)
      {
        txgpsproblem();
        while (true);
      }
      delay(3000);
    }
  }
}

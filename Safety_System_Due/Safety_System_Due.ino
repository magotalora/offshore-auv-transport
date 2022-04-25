
/*Safety system module
   Note: at some point we need to move the GPS to the other module
   
                                  +---+   +---+
                   +----[PWR]-----|USB|---|USB|----+
                   |              +---+   +---+    |
                   |    GND/RST2 [ ][ ]     SCL[ ] |
                   |  MOSI2/SCK2 [ ][ ]     SDA[ ] |
                   |    5V/MISO2 [ ][ ]    AREF[ ] |
                   |                        GND[ ] |
                   | [ ]N/C                  13[ ]~|
                   | [ ]IOREF                12[ ]~|
                   | [ ]RST                  11[ ]~|
   Radio J2(4) VCC | [X]3V3   +----------+   10[ ]~|
           GPS VCC | [X]5v    | Arduino  |    9[ ]~|
   Radio J2(6) GND | [X]GND   |   Due    |    8[ ]~|
           GPS GND | [X]GND   +----------+         |
                   | [ ]Vin                   7[ ]~|
                   |                          6[ ]~|
          Radio PA | [X]A0                    5[ ]~|
          Radio PD | [X]A1                    4[ ]~|
          Radio PC | [X]A2               INT5/3[ ]~|
         Radio CLK | [X]A3               INT4/2[ ]~|
         Radio DIO | [X]A4                 TX>1[ ]~|
         Radio CHP | [X]A5                 RX<0[ ]~|
  Radio J2(5) RSSI | [X]A6                         |
                   | [ ]A7               TX3/14[ ] |
                   |                     RX3/15[ ] |
                   | [ ]A8               TX2/16[ ] | 
                   | [ ]A9               RX2/17[ ] |
                   | [ ]A10         TX1/INT3/18[X] | GPS RX
                   | [ ]A11         RX1/INT2/19[X] | GPS TX
                   | [ ]A12     I2C-SDA/INT1/20[ ] |
                   | [ ]A13     I2C-SCL/INT0/21[ ] |
                   | [ ]A14                        |
                   | [ ]A15                        |
                   |          RST SCK MISO         |
                   |     ICSP [ ] [ ] [ ]          |
                   |          [ ] [ ] [ ]          |
                   |          GND MOSI 5V          |
                   |                   ____________/
                    \_________________/     
*/

#include "cc1000.h" //Library for the Radio commmunications
#include <DHT.h> //Library for the temperature sensor
#include <TinyGPSPlus.h>

float firetemp = 70; //Maximum allowed temperature in the system
TinyGPSPlus gps;
Cc1000 trx;

const int leakdigital = 12; // leak pin
const int leakVCC = 13; // leak pin



#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


//////////////// SETUPS OF MODULES BELOW


void leaksetup(){
  pinMode(leakVCC, OUTPUT);
  digitalWrite(leakVCC, HIGH); 
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
    int temp = dht.readTemperature();
    //Print temp and humidity values to serial monitor
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");

  if (temp >= firetemp) {
    Serial.println("Fire on board"); 
    return true;
    delay (10);
  } else {
    Serial.println("No fire on board");
    return false;
  }
}



/////////////////////////RADIO + GPS FUNCTION
void TXgps()
{
  checkfire();
  checkleak();
  String str = " ";
  if (gps.location.isValid())
  {
  str = str + "$$$$$$$$ @@@@@@@@ ######## $$$$$$$$" + "Latitude: " + String(gps.location.lat(), 6) + "Longitude: " + String(gps.location.lng(), 6);
  } else 
  {
    str = str + "%$%$%$%$ Gps invalid ##@#@#@#";
  }
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
  delay(1000);
}
//////////////////////LOOP
void loop() {
  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
      TXgps();
      delay(1000);
    }

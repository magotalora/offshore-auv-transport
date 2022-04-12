#include "SoftwareSerial.h"

#include "cc1000.h"
#include <TinyGPS++.h>

int RXPin = 7;
int TXPin = 6;

TinyGPSPlus gps;
SoftwareSerial SerialGPS(RXPin, TXPin);

//create cc1000 trx object
Cc1000 trx;

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
  SerialGPS.begin(9600);
  txsetup();
}
// the loop function runs over and over again forever
void txgpsproblem() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  String str = "";
  str = str + "$$$$$$$$ @@@@@@@@ ######## $$$$$$$$ "
        + "GPS not detected"
        + " Modem mode: " + trx.get_modem_mode()
        + " Power: " + trx.get_power()
        + " Deviation: " + trx.get_deviation() + " Hz"
        + " Bitrate: " + trx.get_bitrate() + " bps"
        + " Frequency: " + trx.get_frequency() + " Hz"
        + " $$$$$$$$\n";
  Serial.write((char*)str.c_str());
  trx.send_data(str);
  trx.set_trx_mode(TX_MODE);

  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(3000);
}



void TXgps()
{
  String str = " ";
  if (gps.location.isValid())
  {
    str = str + "$$$$$$$$ @@@@@@@@ ######## $$$$$$$$"
          + "Latitude: " + (gps.location.lat(), 6)
          + "Longitude: " + (gps.location.lng(), 6)
          + "Altitude: " + gps.altitude.meters();
  }
  else
  {
    str = "$$$$$$$$$ @@@@@@@@@@ Location is not available";
  }
  Serial.write((char*)str.c_str());
  trx.send_data(str);
  trx.set_trx_mode(TX_MODE);
  Serial.println();
  delay(5000);
}

void loop() {
  while (SerialGPS.available() > 0)
    if (gps.encode(SerialGPS.read()))
      TXgps();
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    txgpsproblem();
    while (true);
  }
  delay(3000);
}

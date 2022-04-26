/*This file is part of a code test for the serial communication function between two Arduinos.
  This is the code for the receiver. We are testing for receiving a string of text, and turning it
  into the two float values for Latitude and Logitude of the GPS.
      
  Code uses Serial3, PINS 14(TX3) and 15(RX3). 
*/
float gpsLat=0;
float gpsLong=0;

void setup() {
  Serial.begin(9600); //Start communication with the computer
  Serial3.begin(115200); //Start communication with the other Arduino
}

void loop() {
     serialReadGPS();
}

void serialReadGPS() {
        //Variable to store the string to be received
        char message[31];
        //Read message from the other Arduino
        Serial3.readBytes(message,34);
        //Turns the string into a float starting after "Lat:"
        int index=String(message).indexOf("Lat:")+4;
        gpsLat=String(message).substring(index).toFloat();
        //Does the same for "Long:"
        index=String(message).indexOf("Long:")+5;
        gpsLong=String(message).substring(index).toFloat();

        /*For debugging
        Serial.println("-----Serial communication:-----");
        Serial.print("Message received:");
        Serial.println(message);
        Serial.print("gpsLat=");
        Serial.println(gpsLat,6);
        Serial.print("gpsLong=");
        Serial.println(gpsLong,6);
        Serial.println("-------------------------------");
        */
}

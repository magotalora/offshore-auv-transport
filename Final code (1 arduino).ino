 /*Safety system module
                                +-----+
         +----[PWR]-------------| USB |--+
         |    GND/RST2 [ ][ ]   +-----+  |
         |  MOSI2/SCK2 [ ][ ]     SCL[ ] |
         |    5V/MISO2 [ ][ ]     SDA[ ] |
         |                       AREF[ ] |
         |                        GND[ ] |
         | [ ]N/C                  13[ ]~|
         | [ ]IOREF                12[ ]~|
         | [ ]RST                  11[ ]~|
         | [ ]3V3   +----------+   10[ ]~|
         | [ ]5v    | Arduino  |    9[ ]~|
         | [ ]GND   |    DUE   |    8[ ]~|
         | [ ]GND   +----------+         |
         | [ ]Vin                   7[ ]~|
         |                          6[ ]~|
         | [ ]A0                    5[ ]~|
         | [ ]A1                    4[ ]~|
         | [ ]A2               INT5/3[ ]~|
         | [ ]A3               INT4/2[ ]~|
         | [ ]A4                 TX>1[ ]~|
         | [ ]A5                 RX<0[ ]~|
         | [ ]A6                         |
         | [ ]A7               TX3/14[ ] |
         |                     RX3/15[ ] |
         | [ ]A8               TX2/16[ ] |
         | [ ]A9               RX2/17[ ] |
         | [ ]A10         TX1/INT3/18[ ] |
         | [ ]A11         RX1/INT2/19[ ] |
         | [ ]A12     I2C-SDA/INT1/20[ ] |
         | [ ]A13     I2C-SCL/INT0/21[ ] |
         | [ ]A14                        |
         | [ ]A15                        |
         |          RST SCK MISO         |
         |     ICSP [ ] [ ] [ ]          |
         |          [ ] [ ] [ ]          |
         |          GND MOSI 5V          |
         |                               |
         |     2560          ____________/
          \_________________/
*/
//===================================================================================================================================================
// Libraries for navigation system below

#include <Wire.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
@@ -51,35 +6,21 @@
#include <Adafruit_LSM303DLH_Mag.h>
#include <Adafruit_Sensor.h>

//===================================================================================================================================================
// Libraries for navigation and safety system below

#include "cc1000.h" //Library for the Radio commmunications
#include <DHT.h> //Library for the temperature sensor
#include <TinyGPSPlus.h>

//===================================================================================================================================================
// global variables and other items used by navigation system below

float lat, lon;
//GPS
int year;
byte month, day, hour, minute, second, hundredths;
TinyGPS gps;

float currentLat,
      currentLong,
      targetLat = 56.846468,
      targetLong = 4.386233;
int distanceToTarget,            // current distance to target (current waypoint)
    originalDistanceToTarget;    // distance to original waypoing when we started navigating to it

//Servo
Servo fMotor, hMotor, vMotor1, vMotor2;

// Object avoidance distances (in inches)
#define SAFE_DISTANCE 4000
#define MID_DISTANCE 2500
#define STOP_DISTANCE 1000
#define TURN_CW 1
#define TURN_CCW 2
#define TURN_STRAIGHT 0
long sonarDistance, time;
enum directions {cw = TURN_CW, ccw = TURN_CCW, straight = TURN_STRAIGHT} ;
directions turnDirection = straight;
Servo yawMotor;
int pwmSignal;
bool once = false;

//Magnetometer
int check = 1;
@@ -88,51 +29,19 @@ int targetHeading;              // where we want to go to reach current waypoint
int currentHeading;             // where we are actually facing now
int headingError;               // signed (+/-) difference between targetHeading and currentHeading
float rotationAngle;
#define HEADING_TOLERANCE 5     // tolerance +/- (in degrees) within which we don't attempt to turn to intercept targetHeading

//GPS
float currentLat,
      currentLong,
      targetLat,
      targetLong;
int distanceToTarget,            // current distance to target (current waypoint)
    originalDistanceToTarget;    // distance to original waypoing when we started navigating to it

//Motors
#define FAST_SPEED 1900
#define NORMAL_SPEED 1750
#define TURN_SPEED 1675
#define SLOW_SPEED 1600
#define STOP 1500
int speed = NORMAL_SPEED;


//===================================================================================================================================================
// global variables and other items used by communication and safety system below

float firetemp = 70; //Maximum allowed temperature in the system
TinyGPSPlus gps;
Cc1000 trx;

const int leakdigital = 12; // leak pin
const int leakVCC = 13; // leak pin

#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
float headingDegrees;
String direction;
#define HEADING_TOLERANCE 10     // tolerance +/- (in degrees) within which we don't attempt to turn to intercept targetHeading
#define TURN_CW 1
#define TURN_CCW 2
#define TURN_STRAIGHT 0
enum directions {cw = TURN_CW, ccw = TURN_CCW, straight = TURN_STRAIGHT} ;
directions turnDirection = straight;

//===================================================================================================================================================
// Main function and loop
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  fMotor.attach(8);
  hMotor.attach(9);
  vMotor1.attach(10);
  vMotor2.attach(11);
  txsetup();
  leaksetup();
  dhtsetup();
  yawMotor.attach(8);

  if (!mag.begin()) {
    /* There was a problem detecting the LSM303 ... check your connections */
@@ -141,41 +50,41 @@ void setup() {
      ;
  }

  fMotor.writeMicroseconds(1500);
  hMotor.writeMicroseconds(1500);
  vMotor1.writeMicroseconds(1500);
  vMotor2.writeMicroseconds(1500); // send "stop" signal to ESC.
  yawMotor.writeMicroseconds(1500); // send "stop" signal to ESC.
  delay(7000); // delay to allow the ESC to recognize the stopped signal
}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
      TXgps();
      //gps.f_get_position(&lat, &lon);
      distanceToWaypoint();
      courseToWaypoint();
    }
    currentHeading = readCompass();    // get our current heading
    calcDesiredTurn();                // calculate how we would optimatally turn, without regard to obstacles
    if (check == 1) {
      initialOrientation();
    }
    // distance in front of us, move, and avoid obstacles as necessary
    checkSonar();
    moveAndAvoid();
  }
}

//===================================================================================================================================================
// Functions for navigation system below

void checkSonar(void)
int readCompass(void)
{
  time = pulseIn(4, HIGH);
  sonarDistance = (time / 2) / 29.1;
} // checkSonar()
  sensors_event_t event;
  mag.getEvent(&event);

  float Pi = 3.14159;
  // Calculate the angle of the vector y,x
  float heading = (atan2(event.magnetic.y, event.magnetic.x) * 180) / Pi;
  // Normalize to 0-360
  if (heading < 0) {
    heading = 360 + heading;
  }

  headingDegrees = heading;
  return ((int)headingDegrees);
}

int distanceToWaypoint()
{
@@ -216,32 +125,6 @@ int courseToWaypoint()
  return targetHeading;
}

int readCompass(void)
{
  sensors_event_t event;
  mag.getEvent(&event);

  float Pi = 3.14159;
  // Calculate the angle of the vector y,x
  float heading = (atan2(event.magnetic.y, event.magnetic.x) * 180) / Pi;

#define DEC_ANGLE 0.069
  heading += DEC_ANGLE;

  // Correct for when signs are reversed.
  if (heading < 0)
    heading += 2 * PI;

  // Check for wrap due to addition of declination.
  if (heading > 2 * PI)
    heading -= 2 * PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180 / M_PI;

  return ((int)headingDegrees);
}

void calcDesiredTurn(void)
{
  // calculate where we need to turn to head to destination
@@ -259,176 +142,37 @@ void calcDesiredTurn(void)
  }
  if (abs(rotationAngle) <= HEADING_TOLERANCE)      // if within tolerance, don't turn
    turnDirection = straight;
  else if (rotationAngle < 0)
  else if (rotationAngle < 0) {
    turnDirection = cw;
  else if (rotationAngle > 0)
    turnDirection = ccw;
  else
    turnDirection = straight;
}
void moveAndAvoid(void)
{
  if (sonarDistance >= SAFE_DISTANCE)       // no close objects in front of car
  {
    if (turnDirection == straight)
      fMotor.writeMicroseconds(FAST_SPEED);
    else {
      fMotor.writeMicroseconds(TURN_SPEED);
      if (turnDirection == cw) {
        hMotor.writeMicroseconds(map(abs(rotationAngle), 0, 180, 1500, 1900));
      }
      else if (turnDirection == ccw) {
        hMotor.writeMicroseconds(map(abs(rotationAngle), 0, 180, 1500, 1100));
      }
    }
    return;
    direction = "cw";
  }

  if (sonarDistance > MID_DISTANCE && sonarDistance < SAFE_DISTANCE)    // not yet time to turn, but slow down
  {
    if (turnDirection == straight)
      fMotor.writeMicroseconds(NORMAL_SPEED);
    else {
      fMotor.writeMicroseconds(TURN_SPEED);
      if (turnDirection == cw) {
        hMotor.writeMicroseconds(map(abs(rotationAngle), 0, 180, 1500, 1900));
      }
      else if (turnDirection == ccw) {
        hMotor.writeMicroseconds(map(abs(rotationAngle), 0, 180, 1500, 1100));
      }
    }
    return;
  }

  if (sonarDistance <  MID_DISTANCE && sonarDistance > STOP_DISTANCE)  // getting close, time to turn to avoid object
  {
    if (turnDirection == straight)
      fMotor.writeMicroseconds(SLOW_SPEED);
    else {
      fMotor.writeMicroseconds(TURN_SPEED);
      if (turnDirection == cw) {
        hMotor.writeMicroseconds(map(abs(rotationAngle), 0, 180, 1500, 1900));
      }
      else if (turnDirection == ccw) {
        hMotor.writeMicroseconds(map(abs(rotationAngle), 0, 180, 1500, 1100));
      }
    }

    return;
  else if (rotationAngle > 0) {
    turnDirection = ccw;
    direction = "ccw";
  }


  if (sonarDistance <  STOP_DISTANCE)          // too close, stop and back up
  {
    fMotor.writeMicroseconds(STOP);
    hMotor.writeMicroseconds(STOP);
    return;
  else {
    turnDirection = straight;
  }

}

void initialOrientation(void) {
  while (abs(rotationAngle) <= HEADING_TOLERANCE) {
void orientation(void) {
  if (abs(rotationAngle) > HEADING_TOLERANCE) {
    bool once = true;
    if (turnDirection == cw) {
      hMotor.writeMicroseconds(map(abs(rotationAngle), 0, 180, 1500, 1900));
      pwmSignal = map(abs(rotationAngle), 0, 180, 1600, 1900);
      yawMotor.writeMicroseconds(pwmSignal);
      Serial.println("Turning CW");
    }
    else if (turnDirection == ccw) {
      hMotor.writeMicroseconds(map(abs(rotationAngle), 0, 180, 1500, 1100));
      pwmSignal = map(abs(rotationAngle), 0, 180, 1400, 1100);
      yawMotor.writeMicroseconds(pwmSignal);
      Serial.println("Turning CCW");
    }
    Serial.println(pwmSignal);
  }
  check = 0;
}

//============================================================================================================================================
// Functions for communication and safety system below

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
  currentLat = gps.location.lat();
  currentLong = gps.location.lng();
  str = str + "$$$$$$$$ @@@@@@@@ ######## $$$$$$$$" + "Latitude: " + String(currentLat, 6) + "Longitude: " + String(currentLong, 6);
  } else 
  {
    str = str + "%$%$%$%$ Gps invalid ##@#@#@#";
  }
  if (checkleak()==true){
    str = str + "Leak on board %#$$#$#$#$##$";
  else if ((abs(rotationAngle) <= HEADING_TOLERANCE)) {
    yawMotor.writeMicroseconds(1500);
    delay(1000);
    once = false;
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

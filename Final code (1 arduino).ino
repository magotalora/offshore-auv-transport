#include <Wire.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <Servo.h>
#include <Adafruit_LSM303DLH_Mag.h>
#include <Adafruit_Sensor.h>

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
Servo yawMotor;
int pwmSignal;
bool once = false;

//Magnetometer
int check = 1;
Adafruit_LSM303DLH_Mag_Unified mag = Adafruit_LSM303DLH_Mag_Unified(12345);
int targetHeading;              // where we want to go to reach current waypoint
int currentHeading;             // where we are actually facing now
int headingError;               // signed (+/-) difference between targetHeading and currentHeading
float rotationAngle;
float headingDegrees;
String direction;
#define HEADING_TOLERANCE 10     // tolerance +/- (in degrees) within which we don't attempt to turn to intercept targetHeading
#define TURN_CW 1
#define TURN_CCW 2
#define TURN_STRAIGHT 0
enum directions {cw = TURN_CW, ccw = TURN_CCW, straight = TURN_STRAIGHT} ;
directions turnDirection = straight;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  yawMotor.attach(8);

  if (!mag.begin()) {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while (1)
      ;
  }

  yawMotor.writeMicroseconds(1500); // send "stop" signal to ESC.
  delay(7000); // delay to allow the ESC to recognize the stopped signal
}
void loop() {
  //while (Serial1.available() > 0) {
  //  if (gps.encode(Serial1.read())) {
  //    gps.f_get_position(&currentLat, &currentLong);
  currentLat = 25.491047;
  currentLong = 8.446002;
  distanceToWaypoint();
  courseToWaypoint();
  //}
  currentHeading = readCompass();    // get our current heading
  calcDesiredTurn();                // calculate how we would optimatally turn, without regard to obstacles
  Serial.println("Current Heading: " + String(headingDegrees) + "  Target Heading: " + String(targetHeading) + "  Distance: " + String(distanceToTarget) + "  Error: " + String(rotationAngle) + " Direction: " + direction);
  orientation();
  //}
}

int readCompass(void)
{
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
  float delta = radians(currentLong - targetLong);
  float sdlong = sin(delta);
  float cdlong = cos(delta);
  float lat1 = radians(currentLat);
  float lat2 = radians(targetLat);
  float slat1 = sin(lat1);
  float clat1 = cos(lat1);
  float slat2 = sin(lat2);
  float clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  distanceToTarget =  delta * 6372795;

  return distanceToTarget;
}

int courseToWaypoint()
{
  float dlon = radians(targetLong - currentLong);
  float cLat = radians(currentLat);
  float tLat = radians(targetLat);
  float a1 = sin(dlon) * cos(tLat);
  float a2 = sin(cLat) * cos(tLat) * cos(dlon);
  a2 = cos(cLat) * sin(tLat) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  targetHeading = degrees(a2);
  return targetHeading;
}

void calcDesiredTurn(void)
{
  // calculate where we need to turn to head to destination
  if (abs(currentHeading - targetHeading) > 180) {
    if ((currentHeading - targetHeading) > 0) {
      rotationAngle = 360 - (currentHeading - targetHeading);
      rotationAngle = -rotationAngle;
    }
    else {
      rotationAngle = 360 - abs(currentHeading - targetHeading);
    }
  }
  else {
    rotationAngle = currentHeading - targetHeading;
  }
  if (abs(rotationAngle) <= HEADING_TOLERANCE)      // if within tolerance, don't turn
    turnDirection = straight;
  else if (rotationAngle < 0) {
    turnDirection = cw;
    direction = "cw";
  }
  else if (rotationAngle > 0) {
    turnDirection = ccw;
    direction = "ccw";
  }
  else {
    turnDirection = straight;
  }
}

void orientation(void) {
  if (abs(rotationAngle) > HEADING_TOLERANCE) {
    bool once = true;
    if (turnDirection == cw) {
      pwmSignal = map(abs(rotationAngle), 0, 180, 1600, 1900);
      yawMotor.writeMicroseconds(pwmSignal);
      Serial.println("Turning CW");
    }
    else if (turnDirection == ccw) {
      pwmSignal = map(abs(rotationAngle), 0, 180, 1400, 1100);
      yawMotor.writeMicroseconds(pwmSignal);
      Serial.println("Turning CCW");
    }
    Serial.println(pwmSignal);
  }
  else if ((abs(rotationAngle) <= HEADING_TOLERANCE)) {
    yawMotor.writeMicroseconds(1500);
    delay(1000);
    once = false;
  }
}

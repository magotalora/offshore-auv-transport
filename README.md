# offshore-auv-transport
Atonomous navigation system for Arduino, different sensors (IMU, GPS, pressure, leak, temperature), motors, and a radio communication system. Second semester project for the Applied Industrial Electronics BSc at [Aalborg Universitet Esbjerg](https://www.esbjerg.aau.dk).

The project is divided into two subsystems:
* Safety System

Responsible of keeping the system safe by tracking temperature and presence of water, and also for via the radio with onshore.

* Navigation System

Takes de necessary decisions for the AUV to navigate, by reading the location and orientation, compute the route to the determined destination, and activating the five motors.

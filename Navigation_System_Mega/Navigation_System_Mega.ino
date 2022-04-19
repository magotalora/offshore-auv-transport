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
         | [ ]GND   |   Mega   |    8[ ]~|
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


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

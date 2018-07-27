/* Midi host tests
 * the microcontroller has to be connected to the MIDI host board over serial
 * the midi controller messages are changing colors of the rgbw led strip and the servo position
 * 
 * Public Domian
 * but don't mind a shout out :-)
 * bitluni
 */

#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

const int PIXELPIN = 10;
const int PIXELCOUNT = 8;

const int SERVOPIN = 6;

//setup strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELCOUNT, PIXELPIN, NEO_GRBW + NEO_KHZ800);

Servo servo0;

void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  Serial1.begin(31250);
  //initialize servo
  servo0.attach(SERVOPIN);
  
  strip.begin();
  strip.show();
}

//midi message buffer
int byteRead = 0;
int message[3] = {0};

//pixel colors buffer
long colors[PIXELCOUNT] = {0};

//changes a color component of a pixel and updates the strip
void setColor(int pixel, int component, int value)
{
  //sets the 8bit component of a 32bit pixel
  ((char*)&colors[pixel])[component] = value;
  //updates the stripe
  strip.setPixelColor(pixel, colors[pixel]);
  strip.show();
}

void loop() 
{
  //from MIDI to debug out
  if (Serial1.available()) 
  {
    int inByte = Serial1.read();
    //start of a message is indicated by the higes bit set
    if(inByte >= 0x80)
    {
      //message start
      message[0] = inByte;
      byteRead = 1;
    }
    else  //read in bytes of the message
      message[byteRead++] = inByte;
  }
  
  //decode a 3 byte message
  if(byteRead == 3)
  {
    Serial.print("Message: ");
    Serial.print(message[0] >> 4, HEX); Serial.print(" "); 
    Serial.print(message[0] & 0xf, HEX); Serial.print(" ");     
    Serial.print(message[1]); Serial.print(" ");
    Serial.println(message[2]);
    byteRead = 0;

    //controler change
    if((message[0] >> 4) == 0xB)
    {
      //the controller number indentifies what to change, values are always between 0 and 127 
      if(message[1] == 62)
        servo0.write(map(message[2], 0, 127, 0, 180));
      else
      if(message[1] >= 16 && message[1] < 32)
        setColor((message[1] - 16) / 4, message[1] & 3, message[2] * 2);
      else
      if(message[1] >= 46 && message[1] < 62)
        setColor((message[1] - 46) / 4 + 4, message[1] & 3, message[2] * 2);
    }
  }
}

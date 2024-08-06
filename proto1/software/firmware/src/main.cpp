/*
  Example code that will cycle through all possble PPS voltage starting at 4.2V
  Will print out current reading through the 10mOhm sense resistor and blink LED
*/

#include <Arduino.h>
#include <AP33772_PS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AP33772_PS usbpd(Wire1); // Automatically wire to Wire0,


bool state = 0;
int voltage = 3300;
int current = 3000;
int maxVoltage = 10000;
int minVoltage = 3300;
int maxCurrent = 3000;

int button_count0 = 0;
int button_count1 = 0;
int button_count2 = 0;
int button_count3 = 0;

int mode = 0;

/**
 * @brief Debug code to quickly check power supply profile PDOs
 * dump all PDOs profile into Serial port
 */
void printPDO_PPS()
{
    byte numPDO = usbpd.getRawCtrl()->getNumPDO();
    PDO_DATA_T* pdoData = usbpd.getRawCtrl()->getPdoData();
    for (byte i = 0; i < numPDO; i++)
    {
        if ((pdoData[i].byte3 & 0xF0) == 0xC0) // PPS PDO
        {
            display.print("[");
            display.print(i + 1); // PDO position start from 1
            display.print("]:");
            display.print((float)(pdoData[i].pps.minVoltage) * 100 / 1000);
            display.print("V~");
            display.print((float)(pdoData[i].pps.maxVoltage) * 100 / 1000);
            display.print("V@");
            display.print((float)(pdoData[i].pps.maxCurrent) * 50 / 1000);
            display.println("A");
            
            minVoltage = pdoData[i].pps.minVoltage * 100;
            maxVoltage = pdoData[i].pps.maxVoltage * 100;
            maxCurrent = pdoData[i].pps.maxCurrent * 50;
        }
    }
}

void setup()
{
  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();

  usbpd.setup(26, 27);

  pinMode(25, OUTPUT); //Built in LED
  pinMode(23, OUTPUT); //Load Switch
  pinMode(16, INPUT_PULLDOWN); //MOSFET

  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println("initializing...");
  display.display();

  Serial.begin(115200);

  usbpd.begin(); // Start pulling the PDOs from power supply

  digitalWrite(23, HIGH);

  // Clear the buffer
  display.clearDisplay();
  display.display();
}

void loop()
{
  char button[5] = "    ";

  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner

  PinStatus lbutton, rbutton, mbutton;
  lbutton = digitalRead(18);
  rbutton = digitalRead(22);
  mbutton = digitalRead(19);

  if (mode == 0) {
    // voltage edit mode

    if (rbutton == LOW) {
      button[2] = 'x';
      button_count0 += 1;
      if (button_count0 > 5 && voltage < maxVoltage) {
        voltage += 100;
      }
    } else {
      button[2] = 'o';
      button_count0 = 0;
    }
    if (lbutton == LOW) {
      button[0] = 'x';
      button_count1 += 1;
      if (button_count1 > 5 && voltage > minVoltage) {
        voltage -= 100;
      }
    } else {
      button[0] = 'o';
      button_count1 = 0;
    }
  } else {
    // current edit mode

    if (rbutton == LOW) {
      button[2] = 'x';
      button_count0 += 1;
      if (button_count0 > 5 && current < maxCurrent) {
        current += 20;
      }
    } else {
      button[2] = 'o';
      button_count0 = 0;
    }
    if (lbutton == LOW) {
      button[0] = 'x';
      button_count1 += 1;
      if (button_count1 > 5 && current > 0) {
        current -= 20;
      }
    } else {
      button[0] = 'o';
      button_count1 = 0;
    }
  }

  if ((lbutton == LOW) && (rbutton == LOW)) {
    button_count3 += 1;
    if (button_count3 > 5) {
      if (mode == 0) {
        mode = 1;
      } else {
        mode = 0;
      }
      button_count3 = 0;
    }
  } else {
    button_count3 = 0;
  }

  bool enable = false;
  if (mbutton == LOW) {
    button[1] = 'x';
    button_count2 += 1;
    if (button_count2 > 5) {
      usbpd.getOutput(enable);
      if (enable) {
        usbpd.setOutput(false);
      } else {
        usbpd.setOutput(true);
      }
      button_count2 = 0;
    }
  } else {
    button[1] = 'o';
    button_count2 = 0;
  }

  usbpd.setOperatingCurrent(current);
  usbpd.setVoltage(voltage);
  
  Serial.print(button);
  display.print(button);
  if (mode == 0) {
    Serial.print("[V]");
    display.print("[V]");
  } else {
    Serial.print("[C]");
    display.print("[C]");
  }
  usbpd.getOutput(enable);
  if (enable) {
    Serial.println("[ON]");
    display.println("[ON]");
  } else {
    Serial.println("[OFF]");
    display.println("[OFF]");
  }
  printPDO_PPS();
  Serial.print("W (V): ");
  display.print("W (V): ");
  Serial.print((float)voltage / 1000);
  display.print((float)voltage / 1000);
  Serial.print(":");
  display.print(":");
  Serial.println((float)minVoltage / 1000);
  display.println((float)minVoltage / 1000);
  Serial.print("W (A): ");                                                                                                                                                               
  display.print("W (A): ");
  Serial.println((float)current / 1000);
  display.println((float)current / 1000);

  Serial.print("R (V): ");
  display.print("R (V): ");
  Serial.println((float)usbpd.getRawCtrl()->readVoltage() / 1000);
  display.println((float)usbpd.getRawCtrl()->readVoltage() / 1000);
  Serial.print("R (A): ");                                                                                                                                                               
  display.print("R (A): ");
  int rcurrent;
  usbpd.getCurrent(rcurrent);
  Serial.println((float)rcurrent / 1000);
  display.println((float)rcurrent / 1000);

  display.display();
  delay(100);
}

#include <M5Stack.h>
#include "BP35A1.h"

BP35A1 *bp35a1;

void setup()
{
  M5.begin();

  delay(1000);
  bp35a1 = new BP35A1();
  
  // text print
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setTextSize(4);

  Serial.begin(115200);
  Serial2.begin(115200);
  delay(100);

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.clear(BLACK);
  M5.Lcd.println("BP35A1");
  bp35a1->testComm();
  M5.Lcd.println("testComm:OK");
  bp35a1->initComm();
  M5.Lcd.println("initComm:OK");
  bp35a1->activeScan();
  M5.Lcd.println("activeScan:OK");
}

void loop()
{
  uint32_t power;

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.clear(BLACK);
  M5.Lcd.println("BP35A1");

  if(bp35a1->getInstantaneousPower(&power) == true){
    Serial.println("getInstantaneousPower success");
    M5.Lcd.println("IPMV:" + String(power, DEC) + "[W]");
  }
  delay(30 * 1000);
  M5.update();
  
}


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
  M5.Lcd.setTextSize(3);

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
  integral_power_consumpution_t integral_power = {0};

  uint8_t collect_date = 0; //0:today
  if(bp35a1->setIntegralCollectDate(collect_date)){
    Serial.println("setIntegralCollectDate success");
  }

  if(bp35a1->getIntegralPowerConsumption(&integral_power)){
    Serial.println("getIntegralPowerConsumption success");
    M5.Lcd.println("DATE:" + String(integral_power.year) + ":" 
                           + String(integral_power.month) + ":"
                           + String(integral_power.day) + ":"
                           );
    M5.Lcd.println("     " + String(integral_power.hour) + ":" 
                           + String(integral_power.minute)
                           );

    M5.Lcd.println("IPC:" + String(integral_power.power_consumpution, DEC) + "[kWh]");
  }
  delay(5 * 60 * 1000);
  M5.update();
  
}


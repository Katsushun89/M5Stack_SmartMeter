#include <M5Stack.h>
#include "BP35A1.h"
#include <Wire.h>
#include <esp_wifi.h>
#include "bme280_i2c.h"
#include "Ambient.h"

#define PERIOD 300

#define DELAY_CONNECTION 100

#define SDA 21
#define SCL 22

BME280 bme280(BME280_I2C_ADDR_PRIM);

WiFiClient client;
Ambient ambient;
BP35A1 *bp35a1;

void setupBME280(void)
{
  Serial.println("setup BME280 start");

  pinMode(SDA, INPUT_PULLUP);
  pinMode(SCL, INPUT_PULLUP);
  Wire.begin(SDA, SCL);

  delay(100);
  bme280.begin(); // BME280の初期化

  Serial.println("setup BME280 end");
}

void setupBP35A1(void)
{
  Serial.println("setup BP35A1 start");
  delay(100);
  bp35a1 = new BP35A1();

  delay(100);
  Serial2.begin(115200);

  M5.Lcd.println("BP35A1");
  delay(100);
  M5.Lcd.println("");
  bp35a1->testComm();
  M5.Lcd.println("testComm:OK");
  bp35a1->initComm();
  M5.Lcd.println("initComm:OK");
  bp35a1->activeScan();
  M5.Lcd.println("activeScan:OK");
  
  Serial.println("setup BP35A1 end");
}

void setupWiFi(void)
{
    int ret, i;
    while ((ret = WiFi.status()) != WL_CONNECTED) {
        Serial.printf("> stat: %02x", ret);
        ret = WiFi.begin(ssid, password);  //  Wi-Fi APに接続
        i = 0;
        while ((ret = WiFi.status()) != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
            delay(DELAY_CONNECTION);
            if ((++i % (1000 / DELAY_CONNECTION)) == 0) {
                Serial.printf(" >stat: %02x", ret);
            }
            if (i > 10 * 1000 / DELAY_CONNECTION) { // 10秒経過してもDISCONNECTEDのままなら、再度begin()
                break;
            }
        }
    }
    Serial.print("WiFi connected\r\nIP address: ");
    Serial.println(WiFi.localIP());
}

void setupAmbient(void)
{
  ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化
}

void setup()
{
  M5.begin();
  // text print
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setTextSize(3);

  M5.Lcd.clear(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("SMART METER/TEMP");

  Serial.begin(115200);

  setupBME280();
  setupBP35A1();
  setupWiFi();
  setupAmbient();
}

void loop()
{
  int t = millis();

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.clear(BLACK);
  M5.Lcd.println("SMART METER/TEMP");

  integral_power_consumpution_t integral_power = {0};
  struct bme280_data bme_data;

  uint32_t instantaneuous_power = 0;
  if(bp35a1->getInstantaneousPower(&instantaneuous_power)){
    Serial.println("getInstantaneousPower success");
    M5.Lcd.println("IPMV:" + String(instantaneuous_power, DEC) + "[W]");
  }

  uint8_t collect_date = 0; //0:today
  if(bp35a1->setIntegralCollectDate(collect_date)){
    Serial.println("setIntegralCollectDate success");
  }

  if(bp35a1->getIntegralPowerConsumption(&integral_power)){
    char c_hour[5];
    snprintf(c_hour, sizeof(c_hour), "%02d", integral_power.hour);
    char c_min[5];
    snprintf(c_min, sizeof(c_min), "%02d", integral_power.minute);
    Serial.println("getIntegralPowerConsumption success");
    M5.Lcd.println(String(integral_power.year) + ":" 
                  + String(integral_power.month) + ":"
                  + String(integral_power.day) + " "
                  + String(c_hour) + ":" 
                  + String(c_min)
                  );

    M5.Lcd.println("IPC:" + String(integral_power.power_consumpution, DEC) + "[kWh]");
  }
  
  int8_t rslt = bme280.get_sensor_data(&bme_data);
  Serial.println("temp hum, press");
  Serial.printf("%0.2f, %0.2f, %0.2f\r\n", bme_data.temperature, bme_data.humidity, bme_data.pressure / 100);

  M5.Lcd.println("TEMP:"  + String(bme_data.temperature) + "[C]");
  M5.Lcd.println("HUMID:" + String(bme_data.humidity) + "[%]");

  // send BME280 data to ambient
  ambient.set(1, String(bme_data.temperature).c_str());
  ambient.set(2, String(bme_data.humidity).c_str());
  ambient.set(3, String(bme_data.pressure / 100).c_str());
  
  // send BP35A1 power to ambient
  ambient.set(4, String(integral_power.power_consumpution).c_str());
  ambient.set(5, String(instantaneuous_power).c_str());

  ambient.send();

  t = millis() - t;
  t = (t < PERIOD * 1000) ? (PERIOD * 1000 - t) : 1;
  delay(t);
  M5.update();
  
}


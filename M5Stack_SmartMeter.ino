#include <M5Stack.h>
#include "BP35A1.h"
#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "bme280_i2c.h"
#include "Ambient.h"
#include "electric_bill_calculation.h"

#define PERIOD 5 * 60
#define JST 3600* 9
#define DELAY_CONNECTION 100

#define SDA 21
#define SCL 22

BME280 bme280(BME280_I2C_ADDR_PRIM);

WiFiClient client;
Ambient ambient;
BP35A1 *bp35a1;
ElectricBillCalculation bill_calc;

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

void executeInitialCommBP35A1(void)
{
  time_t t = time(NULL);
  uint8_t collect_date = bill_calc.calcMeterReadingDiffDays(&t); //0:today
  if(bp35a1->setIntegralCollectDate(collect_date)){
    Serial.println("setIntegralCollectDate success");
  }else{
    ESP.restart();
  }
   
  integral_power_consumpution_t integral_power = {0}; 
  if(bp35a1->getIntegralPowerConsumption(&integral_power)){
    Serial.println("getIntegralPowerConsumption success");
  }else{
    ESP.restart(); 
  }

  integral_power_record_t integral_power_record = {0};
  if(bp35a1->getIntegralPowerRecord(&integral_power_record)){
    Serial.println("getIntegralPowerRecord success");
  }else{
    ESP.restart();
  }

  bill_calc.setMeterReadingPowerConsumption(&integral_power_record);
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
  M5.Lcd.println("SMART METER/ENV SENSER");

  Serial.begin(115200);

  setupBME280();
  setupBP35A1();
  setupWiFi();
  setupAmbient();
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");//after setupWifi
  //executeInitialCommBP35A1();
}

void printTIme(void)
{
  time_t t;
  struct tm *tm;
  static const char *wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
 
  t = time(NULL);
  tm = localtime(&t);
  Serial.printf(" %04d/%02d/%02d(%s) %02d:%02d:%02d\n",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
        wd[tm->tm_wday],
        tm->tm_hour, tm->tm_min, tm->tm_sec);
  char c_hour[5];
  snprintf(c_hour, sizeof(c_hour), "%02d", tm->tm_hour);
  char c_min[5];
  snprintf(c_min, sizeof(c_min), "%02d", tm->tm_min);
  char c_sec[5];
  snprintf(c_sec, sizeof(c_sec), "%02d", tm->tm_sec);
  
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(BLUE);
  M5.Lcd.println(String(tm->tm_year+1900, DEC) + "/"
               + String(tm->tm_mon+1) + "/"
               + String(tm->tm_mon+1) + "("
               + String(wd[tm->tm_wday]) + ")");

  M5.Lcd.setTextSize(2);
  M5.Lcd.println(String(tm->tm_hour) + ":"
               + String(tm->tm_min) + ":"
               + String(tm->tm_sec)
               );
 
}
void loop()
{
  int t = millis();

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.clear(BLACK);

  printTIme();

  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(GREEN);

  uint32_t instantaneuous_power = 0;
  integral_power_consumpution_t integral_power = {0};
  struct bme280_data bme_data;

  if(bp35a1->getInstantaneousPower(&instantaneuous_power)){
    Serial.println("getInstantaneousPower success");
    M5.Lcd.println("IPMV    :" + String(instantaneuous_power, DEC) + "[W]");
  }else{
    ESP.restart(); 
  }

  if(bp35a1->getIntegralPowerConsumption(&integral_power)){
    char c_hour[5];
    snprintf(c_hour, sizeof(c_hour), "%02d", integral_power.hour);
    char c_min[5];
    snprintf(c_min, sizeof(c_min), "%02d", integral_power.minute);
    Serial.println("getIntegralPowerConsumption success");
    /*
    M5.Lcd.println(String(integral_power.year) + ":" 
                  + String(integral_power.month) + ":"
                  + String(integral_power.day) + " "
                  + String(c_hour) + ":" 
                  + String(c_min)
                  );
    */
    M5.Lcd.println("CUR IPC :" + String(integral_power.power_consumpution, DEC) + "[kWh]");
  }else{
    ESP.restart(); 
  }

/*
  uint32_t power_consumption_this_month = bill_calc.calcThisMonthPowerConsumption(&integral_power);
  uint32_t bill = static_cast<uint32_t>(bill_calc.calcThisMonthElectricBill());
  M5.Lcd.println("1MTH IPC:" + String(power_consumption_this_month) + "[kWh]");
  M5.Lcd.println("BILL    :" + String(bill) + "[YEN]");
*/
  int8_t bme280_result = bme280.get_sensor_data(&bme_data);
  if(bme280_result >= 0){
    Serial.println("temp hum, press");
    Serial.printf("%0.2f, %0.2f, %0.2f\r\n", bme_data.temperature, bme_data.humidity, bme_data.pressure / 100);

    M5.Lcd.println("TEMP    :"  + String(bme_data.temperature) + "[C]");
    M5.Lcd.println("HUMID   :" + String(bme_data.humidity) + "[%]");
  }else{
    M5.Lcd.println("BME280 ERROR:" + String(bme280_result));
  }

  // set BME280 data to ambient
  ambient.set(1, String(bme_data.temperature).c_str());
  ambient.set(2, String(bme_data.humidity).c_str());
  ambient.set(3, String(bme_data.pressure / 100).c_str());
  
  // set BP35A1 power to ambient
  ambient.set(4, String(integral_power.power_consumpution).c_str());
  ambient.set(5, String(instantaneuous_power).c_str());

  // set this month power consumption and electric bill
//  ambient.set(6, String(power_consumption_this_month).c_str());
//  ambient.set(7, String(bill).c_str());

  ambient.send();

  t = millis() - t;
  t = (t < PERIOD * 1000) ? (PERIOD * 1000 - t) : 1;
  delay(t);
  M5.update();
  
}


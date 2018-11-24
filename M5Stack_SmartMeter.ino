//#include <M5Stack.h>

void setup()
{
//  M5.begin();

  delay(1000);
  // text print
//  M5.Lcd.fillScreen(BLACK);
//  M5.Lcd.setCursor(10, 10);
//  M5.Lcd.setTextColor(WHITE);
//  M5.Lcd.setTextSize(1);

//  M5.Lcd.setCursor(10, 10); M5.Lcd.print("BP35A1");
  Serial.begin(115200);
  Serial2.begin(115200);
  delay(100);
  Serial.print("send SKVER");
  
  //Serial2.write("SKVER\r\n");
  Serial2.println("SKVER\r\n");
  delay(100);

  if ( Serial2.available() > 0 ) {
    String str = Serial2.readStringUntil('\n');
    Serial.print(str.c_str());
  }
  if ( Serial2.available() > 0 ) {
    String str = Serial2.readStringUntil('\n');
    Serial.print(str.c_str());
  }
}

void resetWI_SUN() {
  digitalWrite(5, LOW); delay(5); digitalWrite(5, HIGH);
  Serial.setTimeout(300);
  String str;
  while(1) {
    str = Serial2.readStringUntil('\0');
    if(str.length() == 0) {
      Serial2.println("SKVER");
    }
    delay(10);
  }

  return;
}
void loop()
{


  
}


#include <Arduino.h>

#define Ver "2.2"
//#define HW "ESP8285"
#define HW "ESP8266-12"

#include <LEDPwm2.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <Timezone.h>
#include <TaskScheduler.h>
#include <U8g2lib.h>


ADC_MODE(ADC_VCC);
#define ROZMIAR_JSON_CFG_BUF 1210 // Rozmiar bufora dla zapisu konfiguracji json

//OLED SSD1306 I2C
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define EPOCH2000 946684800 // Roznica sekund miedzy 1970.01.01 a 2000.01.01
RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime dt;

// UTC 
TimeChangeRule utcRule = {"UTC", Last, Sun, Mar, 1, 0};     // UTC
Timezone UTC(utcRule);
// Western European Time (London, Belfast)
TimeChangeRule WEST = {"WEST", Last, Sun, Mar, 1, 60};      // Western European Time Summer Time
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
Timezone WE(WEST, GMT);
// Central European Time (Berlin, Frankfurt, Paris, Warsaw)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);
// Kalingrad Standard Time (KSK, does not observe DST)
TimeChangeRule ksk = {"KSK", Last, Sun, Mar, 1, 120};
Timezone KSK(ksk);
// Eastern European Time ()
TimeChangeRule EEST = {"EEST", Last, Sun, Mar, 2, 180};     // Eastern European Summer Time
TimeChangeRule EET = {"EET ", Last, Sun, Oct, 3, 120};      // Eastern European Standard Time
Timezone EE(EEST, EET);
// Moscow Standard Time (MSK, does not observe DST)
TimeChangeRule msk = {"MSK", Last, Sun, Mar, 1, 180};
Timezone MSK(msk);

Timezone *Timezones[6] = {
  &UTC,
  &WE,
  &CE,
  &KSK,
  &EE,
  &MSK
};

TimeChangeRule *tcr;
uint8_t SC = 2;

#define WIFIMAN_TIMEOUT 300
uint8_t MAC_array[6];
char MAC_char[13];
uint8_t WM = 1;

#define ResetPin 1    //Pin (D10/TXD0)
#define ilePWM 8      //Okreslić ilość wyjść PWM

// ---> PWM <---
#define PwmPin1 3     //Pin PWM (D9/RXD0)
#define PwmPin2 4     //Pin PWM (D2)
#define PwmPin3 5     //Pin PWM (D8)
#define PwmPin4 13    //Pin PWM (D7)
#define PwmPin5 12    //Pin PWM (D6)
#define PwmPin6 14    //Pin PWM (D5)
#define PwmPin7 15    //Pin PWM (D8)
#define PwmPin8 16    //Pin PWM (D0)

uint8_t PWM[ilePWM][8] = {
  { 10, 0, 20, 0, 60, 1, 100, 0 },
  { 10, 0, 20, 0, 60, 0, 100, 0 },
  { 10, 0, 20, 0, 60, 0, 100, 0 },
  { 10, 0, 20, 0, 60, 0, 100, 0 },
  { 10, 0, 20, 0, 60, 0, 100, 0 },
  { 10, 0, 20, 0, 60, 0, 100, 0 },
  { 10, 0, 20, 0, 60, 0, 100, 0 },
  { 10, 0, 20, 0, 60, 0, 100, 0 }
};

LEDPwm Swiatlo1(PwmPin1); //Obiekt LED sterujacy swiatlem
LEDPwm Swiatlo2(PwmPin2); //Obiekt LED sterujacy swiatlem
LEDPwm Swiatlo3(PwmPin3); //Obiekt LED sterujacy swiatlem
LEDPwm Swiatlo4(PwmPin4); //Obiekt LED sterujacy swiatlem
LEDPwm Swiatlo5(PwmPin5); //Obiekt LED sterujacy swiatlem
LEDPwm Swiatlo6(PwmPin6); //Obiekt LED sterujacy swiatlem
LEDPwm Swiatlo7(PwmPin7); //Obiekt LED sterujacy swiatlem
LEDPwm Swiatlo8(PwmPin8); //Obiekt LED sterujacy swiatlem

LEDPwm *Swiatlo[ilePWM] = {
  &Swiatlo1,
  &Swiatlo2,
  &Swiatlo3,
  &Swiatlo4,
  &Swiatlo5,
  &Swiatlo6,
  &Swiatlo7,
  &Swiatlo8
};

//---TaskScheduler-----------------------------------------
Scheduler runner;
void loop3();
Task l3(5*TASK_SECOND, TASK_FOREVER, &loop3, &runner, true);
void pisz_ekran();
Task tEkran(1*TASK_SECOND, TASK_FOREVER, &pisz_ekran, &runner, true);

void task1();
Task t1(3*TASK_SECOND, TASK_FOREVER, &task1, &runner, false);
void task2();
Task t2(3*TASK_SECOND, TASK_FOREVER, &task2, &runner, false);
void task3();
Task t3(3*TASK_SECOND, TASK_FOREVER, &task3, &runner, false);
void task4();
Task t4(3*TASK_SECOND, TASK_FOREVER, &task4, &runner, false);
void task5();
Task t5(3*TASK_SECOND, TASK_FOREVER, &task5, &runner, false);
void task6();
Task t6(3*TASK_SECOND, TASK_FOREVER, &task6, &runner, false);
void task7();
Task t7(3*TASK_SECOND, TASK_FOREVER, &task7, &runner, false);
void task8();
Task t8(3*TASK_SECOND, TASK_FOREVER, &task8, &runner, false);

Task *T[ilePWM] = {
  &t1,
  &t2,
  &t3,
  &t4,
  &t5,
  &t6,
  &t7,
  &t8
};

//---WiFi--------------------------------------------------
String hostname = "HdwaO_";
boolean rest=0;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

//---funkcje-----------------------------------------------
void pobierzCzas(uint8_t strefa) {
  dt = Rtc.GetDateTime();
  time_t local = Timezones[strefa]->toLocal(dt + EPOCH2000, &tcr);
  dt = local - EPOCH2000;
}

void zapiszCzas(uint8_t strefa) {
  time_t utc = Timezones[strefa]->toUTC(dt + EPOCH2000);
  dt = utc - EPOCH2000;
  Rtc.SetDateTime(dt);
}

uint32_t pobierzTeraz(){
  pobierzCzas(SC);
  uint32_t teraz = dt.Hour() * 60 + dt.Minute();
  return teraz;
}

void ustawPWM() {
  LEDPwm *_swiatlo;
  uint8_t i;

  uint32_t teraz = pobierzTeraz();

  for(i=0;i<ilePWM;i++){
    _swiatlo = Swiatlo[i];
    _swiatlo->ustaw(PWM[i][0], PWM[i][1], PWM[i][2], PWM[i][3], PWM[i][4], teraz, PWM[i][5], PWM[i][6], PWM[i][7]);
  }
}

bool ladujConfig() {
  uint8_t i,j;
  File filecfg = SPIFFS.open("/config.json", "r");

  if (!filecfg) {
    return false;
  }

  size_t size = filecfg.size();
  if (size > 1024) {
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  filecfg.readBytes(buf.get(), size);

  DynamicJsonDocument doc(ROZMIAR_JSON_CFG_BUF);
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    return false;
  }

  SC = doc["SC"];
  WM = doc["WM"];

  for (i = 0; i < ilePWM; i++) {
    for (j = 0; j < 8; j++) {
      PWM[i][j] = doc["PWM"][i][j];
    }
  }

  return true;
}

void zapiszConfig() {
  File filecfg;
  uint8_t i,j;
  DynamicJsonDocument doc(ROZMIAR_JSON_CFG_BUF);
  StaticJsonDocument<128> doc2;

  for (i = 0; i < ilePWM; i++) {
    doc2.clear();
    for (j = 0; j < 8; j++) {
      doc2.add(PWM[i][j]);
    }
    doc["PWM"].add(doc2);
  }

  doc["SC"] = SC;
  doc["WM"] = 1;

  filecfg = SPIFFS.open("/config.json", "w");
  serializeJson(doc, filecfg);
  filecfg.close();
  delay(100);
}

void resetwifi() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  rest = 1;
  delay(100);
}

void resetuj() {
  SPIFFS.remove("/config.json");
  resetwifi();
  rest = 1;
  delay(100);
}

void task1() {
  uint32_t teraz = pobierzTeraz();
  Swiatlo1.Update(teraz);
}

void task2() {
  uint32_t teraz = pobierzTeraz();
  Swiatlo2.Update(teraz);
}

void task3() {
  uint32_t teraz = pobierzTeraz();
  Swiatlo3.Update(teraz);
}

void task4() {
  uint32_t teraz = pobierzTeraz();
  Swiatlo4.Update(teraz);
}

void task5() {
  uint32_t teraz = pobierzTeraz();
  Swiatlo5.Update(teraz);
}

void task6() {
  uint32_t teraz = pobierzTeraz();
  Swiatlo6.Update(teraz);
}

void task7() {
  uint32_t teraz = pobierzTeraz();
  Swiatlo7.Update(teraz);
}

void task8() {
  uint32_t teraz = pobierzTeraz();
  Swiatlo8.Update(teraz);
}

uint16_t coiletask(uint8_t czas){
  uint16_t interwal;

  interwal = czas*3;
  interwal = floor(interwal/10);
  return interwal;
}

void ustaw_task(uint8_t i){

  if(PWM[i][5] == 1){
    uint16_t interwal = coiletask(PWM[i][4]);
    if(interwal != T[i]->getInterval()){
      T[i]->setInterval(interwal);
    }
    if(!T[i]->isEnabled()){
      T[i]->enable();
    }
  }else{
    T[i]->disable();
  }
}

int8_t jakoscWifi() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int16_t dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

void pisz_ekran(){
  String datownik, godzina, ip;
  uint8_t i,pwm;
  LEDPwm *_swiatlo;
  
  ip = WiFi.localIP().toString();
  datownik = String(dt.Year()) + "/";
  if(dt.Month()<10) datownik += "0";
  datownik += String(dt.Month()) + "/";
  if(dt.Day()<10) datownik += "0";
  datownik += String(dt.Day());
  godzina = "";
  if(dt.Hour()<10) godzina += " ";
  godzina += String(dt.Hour()) + ":";
  if(dt.Minute()<10) godzina += "0";
  godzina += String(dt.Minute());

  int8_t rssi = jakoscWifi();

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_profont12_tn);
    //IP
    u8g2.drawStr(0,10,ip.c_str());
    //datownik
    u8g2.drawStr(0,64,datownik.c_str());
    u8g2.drawStr(95,64,godzina.c_str());
    //wifi BOX
    if(rssi>=0) u8g2.drawBox(109,8,3,2);
    if(rssi>20) u8g2.drawBox(113,6,3,4);
    if(rssi>40) u8g2.drawBox(117,4,3,6);
    if(rssi>60) u8g2.drawBox(121,2,3,8);
    if(rssi>80) u8g2.drawBox(125,0,3,10);
    //PWM
    for(i = 0; i < ilePWM; i++){
      _swiatlo = Swiatlo[i];
      pwm = _swiatlo->pobierzPwm();
      if(pwm > 0){
        if(pwm > 9){
          pwm = _swiatlo->pobierzPwm()/3;
          pwm++;
        }else{
          pwm = 2;
        }
      }else{
        pwm = 1;
      }
      u8g2.drawBox(i*16+2,50-pwm,12,pwm);
    }
    //ramki
    u8g2.drawHLine(0,12,128);
    u8g2.drawHLine(0,53,128);
  } while (u8g2.nextPage());
}

//---json API----------------------------------------------
void root_json(){
  uint8_t i;
  String json;
  LEDPwm *_swiatlo;

  json = "{\"swiatlo\":[";
  for(i = 1; i <= ilePWM; i++){
    _swiatlo = Swiatlo[i-1];
    json += "{";
    json += "\"id\":\"" + String(i) + "\",";
    json += "\"wl\":\"" + String(PWM[i - 1][5]) + "\",";
    json += "\"pwm\":\"" + String(_swiatlo->pobierzPwm()) + "\",";
    json += "\"onh\":\"";
    if(PWM[i - 1][0] < 10) json += "0";
    json += String(PWM[i - 1][0]);
    json += ":";
    if(PWM[i - 1][1] < 10) json += "0";
    json += String(PWM[i - 1][1]);
    json += "\",";
    json += "\"offh\":\"";
    if(PWM[i - 1][2] < 10) json += "0";
    json += String(PWM[i - 1][2]);
    json += ":";
    if(PWM[i - 1][3] < 10) json += "0";
    json += String(PWM[i - 1][3]);
    json += "\",";
    json += "\"czas\":\"" + String(PWM[i-1][4]) + "\",";
    json += "\"max\":\"" + String(PWM[i-1][6]) + "\",";
    json += "\"odwrpwm\":\"" + String(PWM[i-1][7]) + "\"";
    json += "}";
    if(i < ilePWM) json += ",";
  }
  json += "],";
  json += "\"ilepwm\":" + String(ilePWM);
  json += "}";

  server.send(200, "application/json", json);
}

void ustawienia_json(){
  String json;

  pobierzCzas(SC);

  json = "{\"data\":\"";
  json += String(dt.Year());
  json += "/";
  if(dt.Month()<10) json += "0";
  json += String(dt.Month());
  json += "/";
  if(dt.Day()<10) json += "0";
  json += String(dt.Day());
  json += "\",";
  json += "\"godzina\":\"";
  if(dt.Hour()<10) json += "0";
  json += String(dt.Hour());
  json += ":";
  if(dt.Minute()<10) json += "0";
  json += String(dt.Minute());
  json += "\",";
  json += "\"strefa_czasowa\":\""+String(SC)+"\"";
  json += ",";
  json += "\"wifiman\":\"1\"";
  json += "}";

  server.send(200, "application/json", json);
}

void info_json(){
  uint8_t i;
  String json, hostname="HdwaO_";
  RtcTemperature temp = Rtc.GetTemperature();
  for(i = 8; i < 12; i++) hostname += String(MAC_char[i]);
  json = "{\"ver\":\""+String(Ver)+"\",\"ilepwm\":\""+String(ilePWM)+"\",\"mac\":\""+String(WiFi.macAddress())+"\",\"free_flash\":\""+String(ESP.getFreeSketchSpace())+"\",\"free_ram\":\""+String(ESP.getFreeHeap())+"\",\"vcc\":\""+String(ESP.getVcc())+"\",\"rtctemp\": \""+String(temp.AsFloatDegC())+"\",\"hostname\": \""+hostname+"\"}";

  server.send(200, "application/json", json);
}

void pwm_all_json() {
  uint8_t i,j=0;
  LEDPwm *_swiatlo;
  String bufS="{\"swiatlo\":[";
  for(i=0;i<ilePWM;i++){
    j++;
    _swiatlo = Swiatlo[i];
    if(i>0) bufS += ",";
    bufS += "{\"id\":" + String(j) + ",\"pwm\":" + String(_swiatlo->pobierzPwm()) + "}";
  }
  bufS += "]}";

  server.send(200, "application/json", bufS);
}

void onoff_json() {
  String bufS;
  uint8_t i,j;
  boolean val;
  LEDPwm *_swiatlo;

  bufS = server.arg("id");
  j = bufS.toInt();
  i = j - 1;
  bufS = server.arg("val");
  if(bufS == "true" ) bufS = "1";
  if(bufS == "false" ) bufS = "0";
  val = bufS.toInt();
  PWM[i][5] = val;
  _swiatlo = Swiatlo[i];

  uint32_t teraz = pobierzTeraz();

  _swiatlo->ustaw(PWM[i][0], PWM[i][1], PWM[i][2], PWM[i][3], PWM[i][4], teraz, PWM[i][5], PWM[i][6], PWM[i][7]);
  zapiszConfig();
  ustaw_task(i);

  bufS = "{\"pwm\":\"" + String(_swiatlo->pobierzPwm()) + "\"}";
  server.send(200, "application/json", bufS);
}

void konfigpwm_json() {
  uint8_t i,j,x;
  String bufS, id, typ, val;
  LEDPwm *_swiatlo;

  id = server.arg("id");
  typ = server.arg("typ");
  val = server.arg("val");
  i = id.toInt();
  i--;

  if(typ == "wlacz"){
    bufS = val.substring(0,2);
    j = 0;
    x = bufS.toInt();
    PWM[i][j] = x;
    bufS = val.substring(3);
    j = 1;
    x = bufS.toInt();
    PWM[i][j] = x;
  }else if(typ == "wylacz"){
    bufS = val.substring(0,2);
    j = 2;
    x = bufS.toInt();
    PWM[i][j] = x;
    bufS = val.substring(3);
    j = 3;
    x = bufS.toInt();
    PWM[i][j] = x;
  }else if(typ == "czas"){
    j = 4;
    x = val.toInt();
    PWM[i][j] = x;
    ustaw_task(i);
  }else if(typ == "maxpwm"){
    j = 6;
    x = val.toInt();
    PWM[i][j] = x;
  }else if(typ == "odwrpwm"){
    j = 7;
    if((val == "true")||(val == "1")){
      x = 1;
    }else{
      x = 0;
    }
    PWM[i][j] = x;
  }

  _swiatlo = Swiatlo[i];

  uint32_t teraz = pobierzTeraz();

  _swiatlo->ustaw(PWM[i][0], PWM[i][1], PWM[i][2], PWM[i][3], PWM[i][4], teraz, PWM[i][5], PWM[i][6], PWM[i][7]);
  zapiszConfig();

  bufS = "{\"pwm\":\"" + String(_swiatlo->pobierzPwm()) + "\",\"id\":\"" + id + "\"}";
  server.send(200, "application/json", bufS);
}

void zmienpwm_json() {
  uint8_t i,pwm;
  String bufS, id, val;
  LEDPwm *_swiatlo;

  id = server.arg("id");
  val = server.arg("val");
  i = id.toInt() - 1;
  pwm = val.toInt();
  _swiatlo = Swiatlo[i];
  _swiatlo->ustawPwm(pwm);

  bufS = "{\"pwm\":\"" + val + "\",\"id\":\"" + id + "\"}";
  server.send(200, "application/json", bufS);
}

void ustaw_data_godz_json(){
  uint16_t rok;
  uint8_t miesiac, dzien, godzina, minuta;
  String bufS;

  bufS = server.arg("rok");
  rok = bufS.toInt();
  bufS = server.arg("miesiac");
  miesiac = bufS.toInt();
  bufS = server.arg("dzien");
  dzien = bufS.toInt();
  bufS = server.arg("godzina");
  godzina = bufS.toInt();
  bufS = server.arg("minuta");
  minuta = bufS.toInt();

  dt = RtcDateTime(rok, miesiac, dzien, godzina, minuta, 0);
  zapiszCzas(SC);
  ustawPWM();

  bufS = "{\"rok\":\"" + String(rok) + "\",\"miesiac\":\"" + String(miesiac) + "\",\"dzien\":\"" + String(dzien) + "\",\"godzina\":\"" + String(godzina) + "\",\"minuta\":\"" + String(minuta) + "\"}";
  server.send(200, "application/json", bufS);
}

void ustaw_strefa_czasowa_json(){
  String bufS;

  bufS = server.arg("val");
  SC = bufS.toInt();
  pobierzCzas(SC);
  ustawPWM();
  zapiszConfig();
  
  bufS = "{\"strefa_czasowa\":\"" + String(SC) + "\"}";
  server.send(200, "application/json", bufS);
}

void resetwifi_json(){
  if(server.arg("resetwifi")=="reset"){
    resetwifi();
    rest = 1;
  }
  server.send(200, "application/json", "{\"reset\":\"ok\"}");
}
//---------------------------------------------------------
void loop3() {
  if(digitalRead(ResetPin) == LOW) resetuj();
  if(rest == 1){
    ESP.restart();
    delay(1000);
  }
}
//---------------------------------------------------------
void setup() {
  uint8_t i;
  String bufS;
  File filecfg;
 
  ESP.wdtEnable(10000);
  pinMode(ResetPin, INPUT_PULLUP);
  delay(1);

  //--odczyt/zapis konfiguracji
  SPIFFS.begin();
  if(!ladujConfig()){
    zapiszConfig();
  }

  WiFi.macAddress(MAC_array);
  for (i = 0; i < sizeof(MAC_array); ++i){
    sprintf(MAC_char,"%s%02x",MAC_char,MAC_array[i]);
  }
  for(i = 8; i < 12; i++) hostname += String(MAC_char[i]);
  WiFi.hostname(hostname);

  //--WiFi Manager
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(WIFIMAN_TIMEOUT);
  char* ssid = &hostname[0];
  delay(100);
  if (!wifiManager.autoConnect(ssid,"test12345")) {
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //--ustawienia RTC
  Wire.begin(0, 2); //(D3, D4)
  Rtc.Begin(); // Initialize DS3231
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid()) {
    dt = compiled;
    zapiszCzas(SC);
  }
  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }
  pobierzCzas(SC);
  if (dt < compiled) {
    dt = compiled;
    zapiszCzas(SC);
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  
  //--scheduler
  for(i = 0; i < ilePWM; i++){
    if(PWM[i][5] == 1){
      if(!T[i]->isEnabled()){
        T[i]->setInterval(coiletask(PWM[i][4]));
        T[i]->enable();
      }
    }else{
      T[i]->disable();
    }
  }

  //---OLED SSD1306 I2C
  u8g2.begin();
  
  //---serwer HTTP
  MDNS.begin(ssid);
  httpUpdater.setup(&server);

  server.on("/root.json", root_json);
  server.on("/ustawienia.json", ustawienia_json);
  server.on("/info.json", info_json);
  server.on("/pwm_all.json", pwm_all_json);
  server.on("/onoff.json", onoff_json);
  server.on("/konfigpwm.json", konfigpwm_json);
  server.on("/zmienpwm.json", zmienpwm_json);
  server.on("/ustaw_data_godz.json", ustaw_data_godz_json);
  server.on("/ustaw_strefa_czasowa.json", ustaw_strefa_czasowa_json);
  server.on("/resetwifi.json", resetwifi_json);
  server.serveStatic("/", SPIFFS, "/root.html", "max-age=86400");
  server.serveStatic("/js", SPIFFS, "/js", "max-age=86400");
  server.serveStatic("/css", SPIFFS, "/css", "max-age=86400");
  server.serveStatic("/fonts", SPIFFS, "/fonts", "max-age=86400");
  server.serveStatic("/config.json", SPIFFS, "/config.json", "max-age=86400");
  server.begin();
  MDNS.addService("http", "tcp", 80);

  ustawPWM();
}

void loop() {
  server.handleClient();
  delay(1);
  runner.execute();
  //pisz_ekran();
}

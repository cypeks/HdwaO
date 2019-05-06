#include <Arduino.h>

#define Ver "2.0"
#define HW "ESP8266-12"

#include <LEDPwm2.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <FS.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <Timezone.h>
#include <TaskScheduler.h>

ADC_MODE(ADC_VCC);

#define EPOCH2000 946684800 //  Roznica sekund miedzy 1970.01.01 a 2000.01.01
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

#define WIFIMAN_TIMEOUT 300
uint8_t MAC_array[6];
char MAC_char[13];

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

byte PWM[ilePWM][8] = {
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

void loop2();
void loop3();
Task l2(3000, TASK_FOREVER, &loop2);
Task l3(5000, TASK_FOREVER, &loop3);
Scheduler runner;

#define IU 4 //ilosc ustawien
String Ustawienia[IU];

String hostname = "HdwaO_";
boolean rest=0;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

//---funkcje-----------------------------------------------
void pobierzCzas(byte strefa) {
  dt = Rtc.GetDateTime();
  time_t local = Timezones[strefa]->toLocal(dt + EPOCH2000, &tcr);
  dt = local - EPOCH2000;
}

void zapiszCzas(byte strefa) {
  time_t utc = Timezones[strefa]->toUTC(dt + EPOCH2000);
  dt = utc - EPOCH2000;
  Rtc.SetDateTime(dt);
}

void ustawPWM() {
  unsigned int teraz;
  LEDPwm *_swiatlo;
  byte i;

  pobierzCzas(Ustawienia[3].toInt());
  teraz = dt.Hour() * 60 + dt.Minute();

  for(i=0;i<ilePWM;i++){
    _swiatlo = Swiatlo[i];
    _swiatlo->ustaw(PWM[i][0], PWM[i][1], PWM[i][2], PWM[i][3], PWM[i][4], teraz, PWM[i][5], PWM[i][6], PWM[i][7]);
  }
}

void zapiszPWM() {
  File filecfg;
  byte i,j;
  String bufS;

  bufS = "";
  for (i = 0; i < ilePWM; i++) {
    for (j = 0; j < 8; j++) {
      bufS += String(PWM[i][j]) + "|";
    }
  }
  filecfg = SPIFFS.open("/pwm.cfg", "w");
  filecfg.print(bufS);
  filecfg.close();
}

void zapiszUstawienia(){
  byte i;
  File filecfg;
  String bufS;

  for(i = 0; i < IU; i++){
    bufS += Ustawienia[i] + "|";
  }
  filecfg = SPIFFS.open("/ustawienia.cfg", "w");
  filecfg.print(bufS);
  filecfg.close();
}

void resetwifi() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  delay(10);
}

void resetuj() {
  SPIFFS.remove("/ustawienia.cfg");
  resetwifi();
  delay(100);
  rest = 1;
}
//---json API----------------------------------------------
void root_json(){
  byte i;
  String json;
  LEDPwm *_swiatlo;

  json = "{\"swiatlo\":[";
  for(i = 1; i <= ilePWM; i++){
    _swiatlo = Swiatlo[i-1];
    json += "{";
    json += "\"id\":\"" + String(i) + "\",";
    json += "\"wl\":\"" + String(PWM[i-1][5]) + "\",";
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

  pobierzCzas(Ustawienia[3].toInt());

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
  json += "\"strefa_czasowa\":\""+Ustawienia[3]+"\"";
  json += "}";

  server.send(200, "application/json", json);
}

void info_json(){
  byte i;
  String json, hostname="HdwaO_";
  RtcTemperature temp = Rtc.GetTemperature();
  for(i = 8; i < 12; i++) hostname += String(MAC_char[i]);
  json = "{\"ver\":\""+Ustawienia[0]+"\",\"ilepwm\":\""+Ustawienia[1]+"\",\"mac\":\""+String(WiFi.macAddress())+"\",\"free_flash\":\""+String(ESP.getFreeSketchSpace())+"\",\"free_ram\":\""+String(ESP.getFreeHeap())+"\",\"vcc\":\""+String(ESP.getVcc())+"\",\"rtctemp\": \""+String(temp.AsFloatDegC())+"\",\"hostname\": \""+hostname+"\"}";

  server.send(200, "application/json", json);
}

void pwm_all_json() {
  byte i,j=0;
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
  byte i,j;
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

  pobierzCzas(Ustawienia[3].toInt());

  unsigned int teraz = dt.Hour() * 60 + dt.Minute();
  _swiatlo->ustaw(PWM[i][0], PWM[i][1], PWM[i][2], PWM[i][3], PWM[i][4], teraz, PWM[i][5], PWM[i][6], PWM[i][7]);
  zapiszPWM();

  bufS = "{\"pwm\":\"" + String(_swiatlo->pobierzPwm()) + "\"}";
  server.send(200, "application/json", bufS);
}

void konfigpwm_json() {
  byte i,j,x;
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

  //dt = Rtc.GetDateTime();
  //dt += gmt;
  pobierzCzas(Ustawienia[3].toInt());
  unsigned int teraz = dt.Hour() * 60 + dt.Minute();

  _swiatlo->ustaw(PWM[i][0], PWM[i][1], PWM[i][2], PWM[i][3], PWM[i][4], teraz, PWM[i][5], PWM[i][6], PWM[i][7]);
  zapiszPWM();

  bufS = "{\"pwm\":\"" + String(_swiatlo->pobierzPwm()) + "\",\"id\":\"" + id + "\"}";
  server.send(200, "application/json", bufS);
}

void zmienpwm_json() {
  byte i,pwm;
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
  zapiszCzas(Ustawienia[3].toInt());
  ustawPWM();

  bufS = "{\"rok\":\"" + String(rok) + "\",\"miesiac\":\"" + String(miesiac) + "\",\"dzien\":\"" + String(dzien) + "\",\"godzina\":\"" + String(godzina) + "\",\"minuta\":\"" + String(minuta) + "\"}";
  server.send(200, "application/json", bufS);
}

void ustaw_strefa_czasowa_json(){
  String bufS;

  Ustawienia[3] = server.arg("val");
  pobierzCzas(Ustawienia[3].toInt());
  ustawPWM();

  bufS = "{\"strefa_czasowa\":\"" + Ustawienia[3] + "\"}";
  server.send(200, "application/json", bufS);
}

void resetwifi_json(){
  if(server.arg("resetwifi")=="reset"){
    resetwifi();
    rest = 1;
  }
  server.send(200, "application/json", "[{}]");
}
//---------------------------------------------------------
void loop2() {
  unsigned int teraz;

  pobierzCzas(Ustawienia[3].toInt());
  teraz = dt.Hour() * 60 + dt.Minute();

  Swiatlo1.Update(teraz);
  delay(1);
  Swiatlo2.Update(teraz);
  delay(1);
  Swiatlo3.Update(teraz);
  delay(1);
  Swiatlo4.Update(teraz);
  delay(1);
  Swiatlo5.Update(teraz);
  delay(1);
  Swiatlo6.Update(teraz);
  delay(1);
  Swiatlo7.Update(teraz);
  delay(1);
  Swiatlo8.Update(teraz);
  delay(1);
}

void loop3() {
  if(digitalRead(ResetPin) == LOW) resetuj();
  if(rest == 1){
    ESP.restart();
    delay(1000);
  }
}
//---------------------------------------------------------
void setup() {
  byte rst=1, i, j;
  String bufS;
  File filecfg;

  Ustawienia[0] = Ver; //wersja
  Ustawienia[1] = String(ilePWM); //iloscPWM
  Ustawienia[2] = HW; //sprzet
  Ustawienia[3] = "2"; //strefa czasowa

  ESP.wdtEnable(10000);
  pinMode(ResetPin, INPUT_PULLUP);
  delay(1);

  WiFi.macAddress(MAC_array);
  for (i = 0; i < sizeof(MAC_array); ++i){
    sprintf(MAC_char,"%s%02x",MAC_char,MAC_array[i]);
  }
  for(i = 8; i < 12; i++) hostname += String(MAC_char[i]);
  WiFi.hostname(hostname);

  //--WiFi Manager
  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setTimeout(WIFIMAN_TIMEOUT);
  char* ssid = &hostname[0];
  delay(100);
  wifiManager.autoConnect(ssid,"test12345");

  //--ustawienia RTC
  Wire.begin(0, 2); //(D3, D4)
  Rtc.Begin(); // Initialize DS3231
  //sprawdzGMT();
  //int gmt = GMT*3600;
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  //compiled -= gmt;

  if (!Rtc.IsDateTimeValid()) {
    //Rtc.SetDateTime(compiled);
    dt = compiled;
    zapiszCzas(Ustawienia[3].toInt());
  }
  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }
  //dt = Rtc.GetDateTime();
  //dt += gmt;
  pobierzCzas(Ustawienia[3].toInt());
  if (dt < compiled) {
    //Rtc.SetDateTime(compiled);
    dt = compiled;
    zapiszCzas(Ustawienia[3].toInt());
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  //--odczyt/zapis konfiguracji
  SPIFFS.begin();
  filecfg = SPIFFS.open("/ustawienia.cfg", "r");
  if (!filecfg) {
    rst = 0;
    zapiszUstawienia();
  } else {
    for(i = 0; i < IU; i++){
      Ustawienia[i] = filecfg.readStringUntil('|');
    }
    filecfg.close();
    if(Ustawienia[0] != Ver) rst = 0;
    if(Ustawienia[1].toInt() != ilePWM) rst = 0;
  }
  //rst = 0; //reset ustawien
  if (rst != 1) {
    Ustawienia[0] = Ver;
    Ustawienia[1] = String(ilePWM);
    Ustawienia[2] = HW;
    Ustawienia[3] = "2";
    zapiszUstawienia();
    bufS = "";
    for (i = 0; i < ilePWM; i++) {
      for (j = 0; j < 8; j++) {
        bufS += String(PWM[i][j]) + "|";
      }
    }
    filecfg = SPIFFS.open("/pwm.cfg", "w");
    filecfg.print(bufS);
    filecfg.close();
    //wifiManager.resetSettings();
  } else {
    filecfg = SPIFFS.open("/pwm.cfg", "r");
    for (i = 0; i < ilePWM; i++) {
      for (j = 0; j < 8; j++) {
        bufS = filecfg.readStringUntil('|');
        PWM[i][j] = bufS.toInt();
      }
    }
    filecfg.close();
    //--scheduler
    runner.init();
    runner.addTask(l2);
    runner.addTask(l3);
    l2.enable();
    l3.enable();
  }

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
  server.on("/resetWifi.json", resetwifi_json);
  server.serveStatic("/", SPIFFS, "/root.html", "max-age=86400");
  server.serveStatic("/js", SPIFFS, "/js", "max-age=86400");
  server.serveStatic("/css", SPIFFS, "/css", "max-age=86400");
  server.serveStatic("/fonts", SPIFFS, "/fonts", "max-age=86400");
  server.begin();
  MDNS.addService("http", "tcp", 80);

  ustawPWM();
}

void loop() {
  server.handleClient();
  delay(1);
  runner.execute();
}

/*
	LEDPwm - biblioteka odpowiadajaca za obsluge switu i zmierzchu oswietlenia LED w akwarium
	ver. 2.5 - 2019.04.01
	(C) by Cyprian Samorajski

	Status:
		0 - nie zmieniaj PWM
		1 - zwiekszaj PWM
		2 - zmniejszaj PWM
*/

#include <LEDPwm2.h>
#include <Arduino.h>
//#if defined(ESP8266)
#include <pgmspace.h>
//#else
//#include <avr/pgmspace.h>
//#endif

static const uint16_t pwmtab_norm[] PROGMEM = {
		0,
		2,3,4,5,6,7,8,9,10,11,
		13,15,18,21,24,27,30,34,37,41,
		46,50,55,59,65,70,75,81,87,93,
		99,105,112,119,126,133,141,148,156,164,
		173,181,190,199,208,217,227,236,246,257,
		267,277,288,299,310,322,333,345,257,369,
		382,394,407,420,433,447,460,474,488,502,
		517,531,546,561,577,592,608,624,640,656,
		672,689,706,723,740,758,776,793,812,830,
		848,867,886,905,925,944,964,984,1004,1023 };
static const uint16_t pwmtab_rew[] PROGMEM = {
		1023,
		1021,1020,1019,1018,1017,1016,1015,1014,1013,1012,
		1010,1008,1005,1002,999,996,993,989,986,982,
		977,973,968,964,958,953,948,942,936,930,
		924,918,911,904,897,890,882,875,867,859,
		850,842,833,824,815,806,796,787,777,766,
		756,746,735,724,713,701,690,678,766,654,
		641,629,616,603,590,576,563,549,535,521,
		506,492,477,462,446,431,415,399,383,367,
		351,334,317,300,283,265,247,230,211,193,
		175,156,137,118,98,79,59,39,19,0 };

LEDPwm::LEDPwm(uint8_t pin)
  {
    ledPin = pin;
    pinMode(ledPin, OUTPUT);
    ostatniMillis = 0;
    Status = 0;
  }

  void LEDPwm::ustaw(uint8_t onh, uint8_t onm, uint8_t offh, uint8_t offm, uint8_t time, uint32_t teraz, bool active, uint8_t max, bool rewers){
	uint32_t t;
	uint16_t off_end;
	uint16_t on_end;
	bool night = 0;

	Activ = active;
	Status = 0;
	Pwm = 0;
	if(max < 0 || max > 100){
		Max = 100;
	}else{
		Max = max;
	}
	Rewers = rewers;

  if(active == 1){
    On = onh*60+onm;
		on_end = On + time;
    off_end = offh*60+offm;
		Off = off_end;

		if(On > Off) night = 1;
		if(Off < time) Off += 1440;
		Off -= time;

    Interwal = time*60000;
    Interwal = Interwal/100;
		if(Max < 100){
			Interwal = Interwal/Max;
			Interwal = Interwal*100;
		}
		uint8_t i = byte(Interwal/1000);

    Status = 0;
		Pwm = 0;
		yield();
			if(On < teraz && on_end > teraz){
				t = teraz-On;
				t *= 60;
				if(t <= 0) t=1;
				Pwm = byte(t/i);
				Status = 1;
			}else if(Off < teraz && off_end >= teraz){
				t = off_end - teraz;
				t *= 60;
				if(t <= 0) t=1;
				Pwm = byte(t/i);
				Status = 2;
			}else if(night == 0){ //dzienny nie przechodzi przez 0
				if(On < teraz && off_end > teraz){
					Pwm = 100;
				}else{
					Pwm = 0;
				}
			}else{ //nocny może przechodzić przez 0
				if(On > teraz && off_end <= teraz){ //wyłączony
					Pwm = 0;
				}else{ //włączony
					if(on_end > 1439 && teraz < 251) teraz = teraz + 1440;
					if(Off > off_end && teraz < 251){
						off_end = off_end + 1440;
						teraz = teraz + 1440;
					}else if(Off > off_end){
						off_end = off_end + 1440;
					}
					if(On < teraz && on_end > teraz){
						t = teraz-On;
						t *= 60;
						if(t <= 0) t=1;
						Pwm = byte(t/i);
						Status = 1;
					}else if(Off < teraz && off_end >= teraz){
						t = off_end - teraz;
						t *= 60;
						if(t <= 0) t=1;
						Pwm = byte(t/i);
						Status = 2;
					}else{
						Pwm = 100;
					}

				}
			}
  }
    if(Pwm > Max) Pwm = Max;
    ustawPwm(Pwm);
  }

  void LEDPwm::ustawPwm(uint8_t pwm)
  {
	uint16_t p;

	if(Activ == 1)
	{
		if(Rewers == 0){
			//p = pgm_read_word(&pwmtab_norm[pwm]);
			p = pgm_read_word(pwmtab_norm+pwm);
		}else{
			//p = pgm_read_word(&pwmtab_rew[pwm]);
			p = pgm_read_word(pwmtab_rew+pwm);
		}
		yield();
	}else{
		p = 0;
	}

	analogWrite(ledPin,p);
	Pwm = pwm;
  }

  uint8_t LEDPwm::pobierzPwm()
  {
    return Pwm;
  }

  uint8_t LEDPwm::pobierzMax()
  {
    return Max;
  }

  uint8_t LEDPwm::pobierzRewers()
  {
    return Rewers;
  }

  void LEDPwm::Update(uint16_t teraz)
  {
   if(Activ == 1)
   {
    if(ostatniMillis > millis()) ostatniMillis = millis();

    if(On == (teraz) && Status != 1) Status = 1;
    if(Off == (teraz) && Status != 2) Status = 2;
    if((Status == 1)&&((millis()-ostatniMillis)>Interwal))
    {
			if(Pwm >= Max){
				Status=0;
			}else{
				Pwm++;
				ustawPwm(Pwm);
				ostatniMillis = millis();
			}
    }
    if((Status == 2)&&((millis()-ostatniMillis)>Interwal))
    {
			if(Pwm == 0){
				Status=0;
			}else{
				Pwm--;
				ustawPwm(Pwm);
				ostatniMillis = millis();
			}
    }
   }
  }

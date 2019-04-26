/*
	LEDPwm - biblioteka odpowiadajaca za obsluge switu i zmierzchu oswietlenia LED w akwarium
	ver. 2.5
	(C) by Cyprian Samorajski
*/

#ifndef LEDPwm_h
#define LEDPwm_h

#include <Arduino.h>

class LEDPwm
{
	public:
		LEDPwm(uint8_t pin);
		void ustaw(uint8_t onh, uint8_t onm, uint8_t offh, uint8_t offm, uint8_t time, unsigned int teraz, bool active, uint8_t max, bool rewers);
		void ustawPwm(uint8_t pwm);
		uint8_t pobierzPwm();
		uint8_t pobierzMax();
		uint8_t pobierzRewers();
		void Update(uint16_t teraz);
		const String wersja = "2.5";

	private:
		uint8_t ledPin; // the number of the LED pin
		uint8_t Pwm;
		uint8_t Status;
		uint16_t On;
		uint16_t Off;
		uint32_t Interwal;
		uint32_t ostatniMillis;
		bool Activ;
		uint8_t Max;
		bool Rewers;
};

#endif

# HdwaO

Do kompilacji kodu użyto środowiska VSCode + Plaformio.

Wymagane biblioteki:
- lib/LEDPwm2 - biblioteka autorska
- lib/ESP8266HTTPUpdateServer - dostosowana do potrzeb projektu biblioteka z projektu: https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266HTTPUpdateServer
- https://github.com/PaulStoffregen/Time
- https://github.com/JChristensen/Timezone
- https://github.com/Makuna/Rtc
- https://github.com/arkhipenko/TaskScheduler
- https://github.com/tzapu/WiFiManager
- https://github.com/olikraus/u8g2

Obrazy flash oraz spiffs gotowe do wgrania dla ESP8266 w wersji 4MB znajdziecie tutaj:
https://github.com/cypeks/HdwaO/tree/master/bin

aby wgrać firmware oraz spiffs można skorzystać z esptool (https://github.com/espressif/esptool):
```
esptool.py --baud 115200 write_flash 0x00000000 firmware_8266-4M1.bin
```
```
esptool.py --baud 115200 write_flash 0x00300000 spiffs_8266-4M1.bin
```
Dokładam wszelkich starań aby oprogramowanie było funkcjonalne i bezpieczne, jednak nie biorę odpowiedzialności za działanie niezgodne z oczekiwaniami oraz za szkody powstałe w wyniku użytkowania niniejszego oprogramowania.

![HdwaO_1](/img/https://raw.githubusercontent.com/cypeks/HdwaO/dev/img/20191027185854.jpg)

##Historia zmian:

###2.2:
- aktualizacja jquery v3.4.1
- aktualizacja bootstrap v4.4.1
- optymalizacja kodu
- naprawiono problem z resetem ustawień WiFi
- dodano obsługę wyświetlacza OLED SSD1306 I2C
###2.1:
- plik konfiguracyjny w formacie JSON
- aktualizacja biblioteki ESP8266HTTPUpdateServer
- poprawiono kilka drobnych błędów

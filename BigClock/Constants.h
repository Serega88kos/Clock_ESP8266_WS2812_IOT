#include <Arduino.h>
#define VF "Serega88kos/BigClock@2.7"  // версия прошивки
//////////// НАСТРОЙКИ ////////////
//   питание платы от БП не выше 5В
//   DS3231        SDA=>D2 SCL=>D1 питание с 5В БП или с 3.3В
//   BMP/BME 280   SDA=>D2 SCL=>D1 питание 3.3В
//   DS18B20 питание 5В, если уличный без модуля ставим резистор между питанием и сигнальным на 4.7К

#define ONE_SENSORS_DS D5  // PIN датчика ds18b20
//#define COLOR_ORDER GRB    // тип ленты, поменять порядок GRB на свой тип, если неправильные цвета
#define LED_PIN 6          // PIN дата от ленты, подключать через резистор 330Ом

//Настройки для DFPlayer
#define MP3_RX_PIN 2               //GPIO2/D4 к DFPlayer Mini TX
#define MP3_TX_PIN 16              //GPIO16/D0 к DFPlayer Mini RX
#define MP3_SERIAL_SPEED 9600      //DFPlayer Mini поддерживает только 9600 бод
#define MP3_SERIAL_BUFFER_SIZE 32  //размер последовательного буфера программного обеспечения в байтах, для отправки 8 байтов вам нужен 11-байтовый буфер (стартовый байт + 8 битов данных + байт четности + стоп-байт = 11 байтов)
#define MP3_SERIAL_TIMEOUT 350     //среднее время ожидания ответа DFPlayer для чипа GD3200B 350 мс..400 мс

//   если в схеме один фоторезистор не модулем, то вешаем резистор от 1К до 10К. Если у вас высокая освещенность, то 1К, если слабая, то до 10К
//   один вывод фоторезистора на пин A0, или любой другой, второй на GND. Между GND и питанием ставим резистор
#define BRG_PIN A0  // PIN фоторезистора

//Настройки точки доступа, IP 192.168.4.1
#define ssidAP "BigClock"
#define passAP "administrator"  // не менее 8 символов

struct Wifi {
  char ssid[32] = "";              // SSID
  char pass[32] = "";              // пароль
  int gmt = 3;                     // часовой пояс, 3 для МСК
  char host[32] = "pool.ntp.org";  // NTP сервер
};
Wifi w;

struct Clck {
  bool rtc_check = 0;                     // 1 - есть модуль RTC, 0 - нет
  uint8_t change_color = 0;               // смена цвета ( 0 - никогда, 1 - раз в минуту, 2 - каждые десять минут, 3 - каждый час, 4 - каждые десять часов)
  bool prs = 0;                           // 0 - не показываем символ давления, 1 - показать
  bool hmd = 1;                           // 0 - не показываем символ влажности, 1 - показать
  bool symbol = 0;                        // 0 - не показывать первый ноль в часах, 1 - показать
  bool mode_sec = 1;                      // режим мигания секунд, 0 - 1р/с, 1 - 2р/с
  uint8_t mode_color = 0;                  // режимы цветов)
  bool htu21d = 0;                        // модуль htu21d
  uint8_t myMods[9] = { 0, 1, 2, 3, 4 };  // режимы
  uint8_t myTime[9] = { 2, 2, 2, 2, 2 };  // время
  int counter = 4;                        // счетчик
  uint8_t led_color = 2;                   // основной цвет из таблицы
  uint8_t LEDS_IN_SEGMENT = 4;            // кол-во СД в сегменте
  uint8_t DOTS_NUM = 2;                   // кол-во СД в точках
  uint8_t DOT_TEMP = 0;                   // только если есть дополнительный последний СД для точки десятичной температуры = 1
  bool dotDate = 0;                       // точка для даты
  bool dotInv = 0;                        // сменить точку
  uint8_t COLOR_ORDER = 0;                // тип ленты 0 = GRB, 1 = RGB
};
Clck c;

struct Other {
  float cor_tempH = 0;      // корректировка показаний датчика комнатной температуры
  float cor_tempS = 0;      // корректировка показаний датчика уличной температуры
  int cor_pres = 0;         // корректировка показаний датчика давления
  int cor_hum = 0;          // корректировка показаний датчика влажности
  bool type_brg = 0;        // выбрать тип датчика, 0 - аналог, 1 - цифровой
  bool auto_brg = 1;        // автоматическая подстройка яркости от уровня внешнего освещения (1 - включить, 0 - выключить)
  int min_brg = 10;         // минимальная яркость (0 - 255)
  int max_brg = 200;        // максимальная яркость (0 - 255)
  int brg = 10;             // как часто проверять изменение по датчику освещенности в сек
  bool min_max = false;     // инверсия фотодатчика
  bool night_mode = 0;      // ночной режим
  uint8_t start_night = 0;  // начало ночного режима в часах
  uint8_t stop_night = 0;   // окончание ночного режима в часах
  int night_brg = 10;       // яркость ночного режима
  uint8_t night_color = 2;   // цвет ночного режима
  bool night_time = 0;      // ночной режим
};
Other o;

//   при отправке больше одного запроса в 5 минут, получите бан по IP, либо получите повышенные привелегии за помощь проекту, тогда ограничения изменяются :)
//   регистрирумся на narodmon.ru, загружаем настроенный скетч, в меню профиль, отладка по IP копируем Mac
//   снова профиль, мои датчики, вводим Mac и настраиваем датчики по усмотрению и радуемся их работе :)
struct Monitor {
  bool Enable = false;  // включаем мониторинг, иначе false
  int delay = 300;      // как часто отправлять значения датчиков на мониторинг, минимум 5 минут, значение в секундах
  bool tempH = false;   // включить отправку показаний комнатной температуры
  bool tempS = false;   // включить отправку показаний уличной температуры
  bool pres = false;    // включить отправку показаний давления
  bool hum = false;     // включить отправку показаний влажности
};
Monitor nm;

struct DFP {
  bool status_kuku = false;     // включаем модуль DFP
  uint8_t start_kuku = 0;       // время работы в часах, начало
  uint8_t stop_kuku = 0;        // время работы в часах, окончание
  bool kuku_mp3_check = false;  // режим кукушка
  bool vrem_mp3_check = false;  // режим оповещения времени
  int grom_mp3 = 0;             // громкость 0-30
  uint8_t golos = 0;            // голосовой пакет
  uint8_t board = 0;            // тип платы
  //0 - DFPLAYER_MINI (DFPlayer Mini module, MP3-TF-16P module, FN-M16P module, YX5200 chip, YX5300 chip, JL AAxxxx chip)
  //1 - DFPLAYER_FN_X10P (FN-M10P module, FN-S10P module, FN6100 chip)
  //2 -  DFPLAYER_HW_247A (HW-247A module, GD3200B chip)
  //3 - DFPLAYER_NO_CHECKSUM (no checksum calculation (not recomended for MCU without external crystal oscillator))
};
DFP dfp;
/////////////////// КОНЕЦ НАСТРОЕК /////////////////
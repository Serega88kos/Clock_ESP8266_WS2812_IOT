//////////// Функции опросов датчиков и их преобразования
void ReadingSensors() {
  //////////Дом. темп.
  if (c.dsHome == 1) FtempH = bmp280.readTemperature() + o.cor_tempH;
  if (c.dsHome == 2) {
    htu.readTick();
    FtempH = htu.getTemperature() + o.cor_tempH;
  }
  if (c.dsHome == 3) FtempH = dht.readTemperature() + o.cor_tempH;
  if (c.dsHome == 4) FtempH = aht.readTemperature() + o.cor_tempH;
  //////////Ул. темп.
  if (c.dsStreet == 1) {
    if (ds.readTemp()) FtempS = ds.getTemp() + o.cor_tempS;
  }
  if (c.dsStreet == 2) {
    if (availableTempRX()) {
      if (getAddrRX() == addrRadDS[c.radioAddrDS]) FtempS = getTempRX() + o.cor_tempS;
      else getTempRX();
    }
  }
  //////////Давление
  if (c.dsPrs == 1) Fpres = pressureToMmHg(bmp280.readPressure()) + o.cor_pres;
  //////////Влажность
  if (c.dsHum == 1) hum = bmp280.readHumidity() + o.cor_hum;
  if (c.dsHum == 2) {
    htu.readTick();
    hum = htu.getHumidity() + o.cor_hum;
  }
  if (c.dsHum == 3) hum = dht.readHumidity() + o.cor_hum;
  if (c.dsHum == 4) hum = aht.readHumidity() + o.cor_hum;

  if (s.mode_udp == 1) readUDP();
  if (s.mode_udp == 2) sendUDP();
}

void TempHomeToArray() {  // вывод температуры с датчика BMP/BME280 на экран
  if (s.DOT_TEMP == 1) {
    leds[NUM_LEDS - 1] = CRGB::Black;
  }
  tempH = FtempH;
  //Serial.println((String)tempH + " | " + FtempH);
  Dots(!Dot);
  DigitTempH(digits[10], segment_4);  // символ градуса
  int digit = abs(tempH % 10);
  DigitTempH(digits[digit], segment_3);
  digit = tempH / 10;
  if (digit == 0) {
    DigitTempH(digits[12], segment_2);  // если впереди ноль, то выключаем его
  } else {
    DigitTempH(digits[digit], segment_2);  // иначе показываем как есть
    DigitTempH(digits[12], segment_1);     // отключаем 1 сегмент
  }
}

void TempStreetToArray() {  // вывод уличной температуры на экран
  tempS = FtempS;
  //Serial.println((String)tempS + " | " + FtempS);
  Dots(!Dot);
  if (s.DOT_TEMP == 1) {
    if (c.mode_color == 1) {
      leds[NUM_LEDS - 1] = ColorTable[rand() % 16];
    } else if (c.mode_color == 0) {
      leds[NUM_LEDS - 1] = ledColor;
    }
    int a = FtempS * 10;                   //25.43 -> 254
    int digit = abs(a % 10);               //254 -> 4
    DigitTempS(digits[digit], segment_4);  // символ градуса
    digit = abs((a % 100) / 10);           // 254 -> 54 -> 5
    DigitTempS(digits[digit], segment_3);
    digit = abs(a / 100);                               // 254 -> 2
    if (digit == 0) DigitTempS(digits[12], segment_2);  // если впереди ноль, то выключаем его
    else
      DigitTempS(digits[digit], segment_2);              // иначе показываем как есть
    if (tempS <= -1) DigitTempS(digits[13], segment_1);  // если < или = -1, то показываем -
    else
      DigitTempS(digits[12], segment_1);  // иначе выключаем 1 сегмент
  } else {
    DigitTempS(digits[10], segment_4);  // символ градуса
    int digit = abs(tempS % 10);
    DigitTempS(digits[digit], segment_3);
    digit = abs(tempS / 10);
    if (digit == 0) DigitTempS(digits[12], segment_2);  // если впереди ноль, то выключаем его
    else
      DigitTempS(digits[digit], segment_2);  // иначе показываем как есть
    if (tempS <= -1) {                       // если < или = -1
      if (tempS <= -10) {                    // если < или = -10
        DigitTempS(digits[13], segment_1);   // то показываем - на 1 сегменте
      } else {                               // если от -9 до -1
        DigitTempS(digits[12], segment_1);   // то показываем выключаем 1 сегмент
        DigitTempS(digits[13], segment_2);   // и показываем - на 2 сегменте
      }
    } else {
      DigitTempS(digits[12], segment_1);  // иначе выключаем 1 сегмент
    }
  }
}

void PressToArray() {  // вывод давления на экран с датчика BMP/BME280
  if (s.DOT_TEMP == 1) {
    leds[NUM_LEDS - 1] = CRGB::Black;
  }
  pres = Fpres;
  //Serial.println(pres);
  Dots(!Dot);
  int digit = pres % 10;
  Digit(digits[digit], segment_4);
  digit = pres % 100 / 10;
  Digit(digits[digit], segment_3);
  digit = pres / 100;
  Digit(digits[digit], segment_2);
  if (c.prs) {
    Digit(digits[14], segment_1);  // показываем символ P
  } else {
    Digit(digits[12], segment_1);  // отключаем первый сегмент
  }
}

void HumToArray() {  // вывод влажности с датчика BME280 на экран
  if (s.DOT_TEMP == 1) {
    leds[NUM_LEDS - 1] = CRGB::Black;
  }
  Dots(!Dot);
  if (c.hmd) {
    Digit(digits[16], segment_4);  // символ %  включен
    Digit(digits[15], segment_3);
  } else {
    Digit(digits[12], segment_4);  // символ % выключен
    Digit(digits[12], segment_3);
  }
  int digit = hum % 10;
  Digit(digits[digit], segment_2);  // 2 сегмент
  digit = hum / 10;
  Digit(digits[digit], segment_1);  // 1 сегмент
}

void DateToArray() {
  if (!c.dotDate) {
    Dots(!Dot);
  }
  if (c.dotDate) {
    if (c.dotInv) {
      leds[s.LEDS_IN_SEGMENT * 14] = ledColor;
    } else {
      leds[s.LEDS_IN_SEGMENT * 14 + 1] = ledColor;
    }
  }
  int digit = day % 10;
  Digit(digits[digit], segment_2);  // 2 сегмент
  digit = day / 10;
  Digit(digits[digit], segment_1);  // 1 сегмент
  digit = month % 10;
  Digit(digits[digit], segment_4);  // 4 сегмент
  digit = month / 10;
  Digit(digits[digit], segment_3);  // 3 сегмент
}
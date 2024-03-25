//////////// Функция WiFi и RTC
void wifi_connected() {
  WiFi.mode(WIFI_STA);
  byte tries = 60;
  WiFi.hostname("BigClock");
  WiFi.begin(w.ssid, w.pass);
  while (--tries && WiFi.status() != WL_CONNECTED) {
    Serial.print F(".");
    int digit = tries % 10;
    Digit(digits[digit], segment_4);  // 4 сегмент
    digit = tries / 10;
    Digit(digits[digit], segment_3);  // 3 сегмент
    delay(1000);
  }
  if (WiFi.status() != WL_CONNECTED) {
    // Если не удалось подключиться запускаем в режиме AP
    IPAddress apIP(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    Serial.println F("");
    Serial.print F("WiFi up ACCESS POINT: ");
    Serial.println(ssidAP);
    Serial.print F("Start Settings IP: ");
    Serial.println(apIP);
    // Отключаем WIFI
    WiFi.disconnect();
    // Меняем режим на режим точки доступа
    WiFi.mode(WIFI_AP);
    // Задаем настройки сети
    WiFi.softAPConfig(apIP, apIP, subnet);
    // Включаем WIFI в режиме точки доступа с именем и паролем
    // хранящихся в переменных _ssidAP _passwordAP
    WiFi.softAP(ssidAP, passAP);
    hub.onBuild(build);  // подключаем билдер
    hub.begin();         // запускаем систему
  } else {
    Serial.println F("");
    Serial.println F("WiFi запущен");
    Serial.print("IP адрес: ");
    Serial.println(WiFi.localIP());
    ntp.setGMT(w.gmt);
    ntp.setHost(w.host);
    ntp.begin();
    hub.onBuild(build);     // подключаем билдер
    hub.begin();            // запускаем систему
  }
}

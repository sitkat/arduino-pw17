#include <NTPClient.h>
#include <WiFiUdp.h>
/*******************************************************************
    Телеграм Бот для модуля WeMos D1 R2 c МК ESP8266.
    Предназначен для вкл./выкл. светодиода на плате.
 *******************************************************************/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Имя и пароль вашей сети Wifi
#define WIFI_SSID "SitKat"
#define WIFI_PASSWORD "12345678"
// Телеграм Бот Токен, можно получить у бота @BotFather в Телеграмм
#define BOT_TOKEN "5397503447:AAF-Mn1SbGqdrIBpc9h2QUJgByVg4_WbzLE"

bool toogle = false;
const unsigned long BOT_MTBS = 1000; // Через сколько времени проверять сообщения

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // Последнее время сканирования сообщения

const int ledPin = LED_BUILTIN;
int ledStatus = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");


void handleNewMessages(int numNewMessages)
{
  Serial.print("Обработка нового сообщения ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/toogle")
    {
      int botMess = bot.getUpdates(bot.last_message_received);
      toogle = true;
      bot.sendMessage(chat_id, "Переключение светодиода", "");
      for (;;) {
        if (toogle) {
          digitalWrite(ledPin, LOW); // Включить светодиод на плате
          delay(1000);
          digitalWrite(ledPin, HIGH);
          delay(1000);
          if (botMess != bot.getUpdates(bot.last_message_received + 1)) {
            break;
          }
        }
      }
      text = bot.messages[i].text;
    }
    if (text == "/datetime")
    {
      toogle = false;
      time_t epochTime = timeClient.getEpochTime();
      struct tm *ptm = gmtime ((time_t *)&epochTime);
      int monthDay = ptm->tm_mday;
      int currentMonth = ptm->tm_mon+1;
      int currentYear = ptm->tm_year+1900;
      
      int _sec = timeClient.getSeconds();
      int _min = timeClient.getMinutes();
      int _hour = timeClient.getHours();
      bot.sendMessage(chat_id, "Дата и время: " + String(currentYear)+ "." + String(currentMonth)+ "." + String(monthDay) + "   " 
      + String(_hour) + ":" + String(_min) + ":" + String(_sec));
    }

    if (text == "/ledon")
    {
      toogle = false;
      digitalWrite(ledPin, LOW); // Включить светодиод на плате
      ledStatus = 1;
      bot.sendMessage(chat_id, "Светодиод включен - ON", "");
    }

    if (text == "/ledoff")
    {
      toogle = false;
      ledStatus = 0;
      digitalWrite(ledPin, HIGH); // Выключить светодиод на плате
      bot.sendMessage(chat_id, "Светодиод выключен - OFF", "");
    }

    if (text == "/status")
    {
      toogle = false;
      if (ledStatus)
      {
        bot.sendMessage(chat_id, "Светодиод включен - ON", "");
      }
      else
      {
        bot.sendMessage(chat_id, "Светодиод выключен - OFF", "");
      }
    }

    if (text == "/start")
    {
      toogle = false;
      String welcome = "Привет, я Телеграм Бот. Я умею преключать светодиод и выводить время на модуле WeMos D2 R1 " + from_name + ".\n";
      welcome += "А это перечень команд, которые я пока знаю.\n\n";
      welcome += "/ledon : Переключает светодиод в состояние ON\n";
      welcome += "/ledoff : Переключает светодиод в состояние OFF\n";
      welcome += "/status : Возвращает текущее состояние светодиода\n";
      welcome += "/toogle : Переключает светодиод до тех пор пока не будет иной команды\n";
      welcome += "/datetime : Выводит текущую дату и время\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}


void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(ledPin, OUTPUT); // Настраиваем пин ledPin на выход
  delay(10);
  digitalWrite(ledPin, HIGH); // По умолчанию светодиод выключен

  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Подключение к сети Wifi SSID: ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  timeClient.begin();
  timeClient.setTimeOffset(10800);
  Serial.print("\nWiFi подключен. IP адрес: ");
  Serial.println(WiFi.localIP());

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Время подключения: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}

void loop()
{
  timeClient.update();
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("Получен ответ");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}

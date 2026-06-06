#include <Arduino.h>
#include <WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define WIFI_SSID "wifi_name"
#define WIFI_PASS "wifi_password"
#define APP_KEY "app_key_API"
#define APP_SECRET "app_secret"
#define SWITCH_ID "switch_id"

// ==========================================
// CONFIGURATION INFRAROUGE
// ==========================================
const uint16_t kIrLed = 4; // Ta LED émettrice et transistor sur GPIO 4
IRsend irsend(kIrLed);

// Les codes scannés
const uint16_t addr = 0x4004;
const uint64_t codePower = 0x538BC81;
const uint64_t codeCD = 0x5505005;
const uint64_t codeTitre7 = 0x5386855;

// Fonction appelée par Google Home
bool onPowerState(const String &deviceId, bool &state)
{
  Serial.printf("Google Home a demandé l'état : %s\r\n", state ? "ON" : "OFF");

  if (state)
  {
    // ---- SCÉNARIO ALLUMAGE ----
    Serial.println("1. Envoi [POWER]...");
    irsend.sendPanasonic(addr, codePower);

    // On attend que la chaîne démarre complètement (3.5 secondes)
    Serial.println("Attente du démarrage (3.5s)...");
    delay(5000);

    Serial.println("2. Envoi [CD]...");
    irsend.sendPanasonic(addr, codeCD);

    // On laisse le temps au lecteur de passer en mode CD (1 seconde)
    Serial.println("Attente du mode CD (1s)...");
    delay(6000);

    Serial.println("3. Envoi [Titre 7]...");
    irsend.sendPanasonic(addr, codeTitre7);

    Serial.println("Séquence terminée !");
  }
  else
  {
    // ---- SCÉNARIO EXTINCTION ----
    Serial.println("Extinction -> Envoi [POWER]...");
    irsend.sendPanasonic(addr, codePower);
  }

  return true;
}

void setup()
{
  Serial.begin(115200);

  // Démarrage LED IR
  irsend.begin();

  // Connexion Wi-Fi
  Serial.printf("\r\nConnexion au Wi-Fi %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\r\nWi-Fi connecté !");

  // Configuration SinricPro
  SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
  mySwitch.onPowerState(onPowerState);

  SinricPro.onConnected([]()
                        { Serial.println("Connecté à SinricPro !"); });
  SinricPro.onDisconnected([]()
                           { Serial.println("Déconnecté..."); });

  SinricPro.begin(APP_KEY, APP_SECRET);
}

void loop()
{
  SinricPro.handle();
}
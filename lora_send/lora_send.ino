#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 4
#define dio0 2

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  Serial.println("LoRa Node Starting...");

  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6))
  { // match this on both nodes
    Serial.println("LoRa init failed!");
    while (1)
      ;
  }

  Serial.println("LoRa Initialized!");
}

void loop()
{
  // ---- SEND ----
  LoRa.beginPacket();
  LoRa.print("Hello bro");
  LoRa.endPacket();

  Serial.println("Sent packet");
  delay(2000);

  // ---- RECEIVE ----
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    Serial.print("Received: ");

    while (LoRa.available())
    {
      Serial.print((char)LoRa.read());
    }

    Serial.print(" | RSSI: ");
    Serial.println(LoRa.packetRssi());
  }

  delay(1000);
}
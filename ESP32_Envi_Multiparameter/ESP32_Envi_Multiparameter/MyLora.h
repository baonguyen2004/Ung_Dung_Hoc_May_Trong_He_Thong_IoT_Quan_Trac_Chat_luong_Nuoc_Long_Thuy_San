#ifndef MY_LORA_H
#define MY_LORA_H

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// ====== DEFAULT CONFIG ======
#define LORA_DEFAULT_SCK    18
#define LORA_DEFAULT_MISO   19
#define LORA_DEFAULT_MOSI   23
#define LORA_DEFAULT_SS     5
#define LORA_DEFAULT_RST    17
#define LORA_DEFAULT_DIO0   4
#define LORA_DEFAULT_FREQ   433E6

class MyLoRa {
  private:
    long frequency;
    int sck, miso, mosi, ss, rst, dio0;

  public:
    MyLoRa() {
      frequency = LORA_DEFAULT_FREQ;

      sck  = LORA_DEFAULT_SCK;
      miso = LORA_DEFAULT_MISO;
      mosi = LORA_DEFAULT_MOSI;
      ss   = LORA_DEFAULT_SS;
      rst  = LORA_DEFAULT_RST;
      dio0 = LORA_DEFAULT_DIO0;
    }

    MyLoRa(long freq, int _sck, int _miso, int _mosi, int _ss, int _rst, int _dio0) {
      frequency = freq;

      sck  = _sck;
      miso = _miso;
      mosi = _mosi;
      ss   = _ss;
      rst  = _rst;
      dio0 = _dio0;
    }

    void begin() {
      SPI.begin(sck, miso, mosi, ss);

      LoRa.setPins(ss, rst, dio0);
      LoRa.setSPIFrequency(20000000);
      LoRa.setTxPower(20);

      if (!LoRa.begin(frequency)) {
        Serial.println("LoRa init failed!");
        while (1);
      }

      Serial.println("LoRa OK");
    }

    void send(String payload) {
      LoRa.beginPacket();
      LoRa.print(payload);
      LoRa.endPacket();
    }
};

#endif
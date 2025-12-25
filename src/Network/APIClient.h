#ifndef APICLIENT_H
#define APICLIENT_H

#include "../Config.h"
#include "../Sequence/SequenceQueue.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <functional>

class APIClient {
private:
  SequenceQueue &queue;
  unsigned long lastCheckTime;
  unsigned long checkInterval;

  // Boot Logic State
  String bootStatus;
  bool initialFetchDone;
  bool isConnected;
  int bootState; // 0=Wait WiFi, 1=Connected Wait, 2=Fetching, 3=Done Wait

  using StatusCallback = std::function<void(const String &)>;
  StatusCallback statusCallback;

  void updateStatus(String msg) {
    bootStatus = msg;
    if (statusCallback) {
      statusCallback(msg);
    }
  }

  void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
      isConnected = true;
      return;
    }

    // This is non-blocking (starts connection process)
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    updateStatus("Connecting to wifi...");
  }

  bool fetchSequences(const char *messageType) {
    if (WiFi.status() != WL_CONNECTED) {
      updateStatus("Error: WiFi lost!");
      return false;
    }

    updateStatus("Building JSON...");

    // Build JSON Payload
    StaticJsonDocument<200> doc;
    doc["message"] = messageType;
    String requestBody;
    serializeJson(doc, requestBody);

    // Choose Client based on Protocol
    if (String(API_URL).startsWith("https")) {
      std::unique_ptr<BearSSL::WiFiClientSecure> client(
          new BearSSL::WiFiClientSecure);
      client->setInsecure();
      return executeRequest(*client, requestBody);
    } else {
      std::unique_ptr<WiFiClient> client(new WiFiClient);
      return executeRequest(*client, requestBody);
    }
  }

  bool executeRequest(WiFiClient &client, String &requestBody) {
    HTTPClient http;
    updateStatus("Connecting API...");
    Serial.print("Sending API Request: ");
    Serial.println(API_URL);

    if (http.begin(client, API_URL)) {
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", String("Bearer ") + API_TOKEN);

      updateStatus("Post " + WiFi.localIP().toString() + "..");
      int httpCode = http.POST(requestBody);

      if (httpCode > 0) {
        Serial.printf("HTTP Code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
          updateStatus("Reading data...");
          String payload = http.getString();
          updateStatus("Parsing JSON...");
          parseResponse(payload);
          http.end();
          return true;
        } else {
          updateStatus("HTTP Err: " + String(httpCode));
        }
      } else {
        updateStatus("Fail(" + String(httpCode) +
                     "): " + http.errorToString(httpCode));
      }
      http.end();
    } else {
      updateStatus("Conn Failed!");
    }
    return false;
  }

  bool parseResponse(String &json) {
    // Expected usage:
    // [{"expression": "...", ...}, ...]
    // Adjust size based on expected complexity
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      updateStatus("JSON Err: " + String(error.c_str()));
      return false;
    }

    JsonArray array = doc.as<JsonArray>();
    for (JsonObject v : array) {
      SequenceStep step;
      step.expression = v["Expression"].as<String>();
      step.beepDuration = v["BuzzerDuration"].as<float>();
      step.text = v["Text"].as<String>();
      step.displayDuration = v["DisplayDuration"].as<float>();

      queue.add(step);
    }
    Serial.printf("Added %d steps to queue.\n", array.size());
    return true;
  }

public:
  APIClient(SequenceQueue &_queue)
      : queue(_queue), lastCheckTime(0), checkInterval(10000),
        initialFetchDone(false), isConnected(false), bootState(0),
        bootStatus("Booting...") {}

  void setStatusCallback(StatusCallback cb) { statusCallback = cb; }

  void begin() {
    Serial.begin(115200);
    connectWiFi();
  }

  String getBootStatus() { return bootStatus; }

  bool isBootComplete() { return initialFetchDone; }

  void update() {
    unsigned long now = millis();

    // Ensure WiFi is connected
    // Ensure WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
      if (isConnected) {
        updateStatus("WiFi Lost!");
        isConnected = false;
      }
      return;
    } else {
      if (!isConnected) {
        isConnected = true;
        updateStatus("IP: " + WiFi.localIP().toString());
      }
    }

    // 1. Initial Boot Fetch Logic
    if (!initialFetchDone) {
      // Simple state machine for display purposes
      if (bootState == 0) {
        if (isConnected) {
          bootState = 1;
          // Give user time to see "Connected to wifi!"
          lastCheckTime = now;
        }
      } else if (bootState == 1) {
        // Wait 1 second
        if (now - lastCheckTime > 1000) {
          updateStatus("connecting to API..");
          bootState = 2;
        }
      } else if (bootState == 2) {
        // Trigger Fetch
        if (fetchSequences(MSG_BOOT)) {
          updateStatus("Connected to API!");
          bootState = 3;
          lastCheckTime = millis();
        } else {
          // Stay in this state but wait a bit before retrying
          // The updateStatus inside fetchSequences will show the error
          // We can add a delay here or just rely on the next loop
          delay(2000); // Wait 2s before retry so user can read error
          lastCheckTime = millis();
        }
      } else if (bootState == 3) {
        // Wait 1 second to show "Connected to API!"
        if (now - lastCheckTime > 1000) {
          initialFetchDone = true;
        }
      }
      return;
    }

    // 2. Regular Fetch
    if (now - lastCheckTime >= checkInterval) {
      // Only fetch if queue is low
      if (queue.size() < 5) {
        Serial.println("Queue low, fetching more...");
        fetchSequences(MSG_REPEAT);
      }
      lastCheckTime = now;
    }
  }
};

#endif

//
// This is part of the VEdirecthexMQTT project
//
// https://github.com/Smartsmurf/VEdirecthexMQTT
// 
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

#include "html.h"
#include "preferences.h"
#include "vedirect.h"

WebServer server(80);

void startWebServer() {

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getWiFiMqttPage());
  });

/*
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "<html><body><h1>ESP32 Webserver is up and running!</h1></body></html>");
  });
*/

  server.on("/save", HTTP_POST, []() {
    ssid = server.arg("ssid");
    password = server.arg("password");
    mqttServer = server.arg("mqtt_server");
    mqttTopic = server.arg("mqtt_topic");
    mqttDevicename = server.arg("mqtt_devicename");
    mqttUser = server.arg("mqtt_user");
    mqttPass = server.arg("mqtt_pass");

    prefs.begin("config", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.putString("mqtt_server", mqttServer);
    prefs.putString("mqtt_topic", mqttTopic);
    prefs.putString("mqtt_devicename", mqttDevicename);
    prefs.putString("mqtt_user", mqttUser);
    prefs.putString("mqtt_pass", mqttPass);
    prefs.end();

    server.send(200, "text/html", "<html><body><h2>Saved. Rebooting...</h2></body></html>");
    delay(1000);
    ESP.restart();
  });

  server.on("/serial", HTTP_GET, []() {
    server.send(200, "text/html", getSerialConfigPage());
  });

  server.on("/serial/save", HTTP_POST, []() {
    rxPin = server.arg("rx_pin").toInt();
    txPin = server.arg("tx_pin").toInt();
//    rtsPin = server.arg("rts_pin").toInt();
//    batteryCount = server.arg("batt_count").toInt();

    prefs.begin("config", false);
    prefs.putInt("rx_pin", rxPin);
    prefs.putInt("tx_pin", txPin);
//    prefs.putInt("rts_pin", rtsPin);
//    prefs.putInt("batt_count", batteryCount);
    for (int i = 0; i < NUM_VE_HEX_REGISTERS; i++) {
      String key = "mqtt_subscription_" + String(i);
      String val = server.arg(key);
      VEregisters[i].mqtt_subscription = val;
      prefs.putString(key.c_str(), val);
    }
    prefs.end();

    server.send(200, "text/html", "<html><body><h2>Saved. Rebooting...</h2></body></html>");
    delay(1000);
    server.sendHeader("Location", "/", true);
    server.send(303);  // 303 See Other
    delay(1000);
    ESP.restart();
  });

  server.on("/update", HTTP_GET, []() {
      server.send(200, "text/html", getUpdatePage());    
  });

  server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "Update FEHLGESCHLAGEN" : "Update erfolgreich. Neustart...");
        delay(1000);
        ESP.restart();
        }, []() {
        HTTPUpload& upload = server.upload();

        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("OTA start: %s\n", upload.filename.c_str());
            if (!Update.begin()) Update.printError(Serial);
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
            Serial.printf("OTA success: %u bytes\n", upload.totalSize);
            } else {
            Update.printError(Serial);
            }
        }
  });

  server.begin();
  Serial.println("Webserver started.");
}

// AP fallback for initial setup
void startConfigPortal() {
  Serial.println("[WEB] starting config AP.");
  WiFi.softAP("VEdirecthexMQTT_Config", "12345678");
  Serial.print("[WEB] AP IP: ");
  Serial.println(WiFi.softAPIP());
  startWebServer();
}

// HTML: WLAN + MQTT
String getWiFiMqttPage() {
  
  String html = "<html><body><h2>Netzwerk & MQTT</h2>";
  html += "<form action=\"/save\" method=\"POST\">";
  html += "SSID: <input name=\"ssid\" value=\"" + ssid + "\"><br>";
  html += "Passwort: <input name=\"password\" type=\"password\" value=\"" + password + "\"><br>";
  html += "MQTT Server: <input name=\"mqtt\" value=\"" + mqttServer + "\"><br>";
  html += "MQTT Port: <input name=\"mqtt_port\" type=\"number\" value=\"" + String(mqttPort) + "\"><br>";
  html += "MQTT User: <input name=\"mqtt_user\" value=\"" + mqttUser + "\"><br>";
  html += "MQTT Passwort: <input name=\"mqtt_pass\" type=\"password\" value=\"" + mqttPass + "\"><br>";
  html += "MQTT Topic: <input name=\"mqtt_topic\" value=\"" + mqttTopic + "\"><br>";
  html += "MQTT Devicename: <input name=\"mqtt_devicename\" value=\"" + mqttDevicename + "\"><br>";
  html += "<input type=\"submit\" value=\"Save\">";
  html += "</form><br><a href=\"/serial\">More settings</a>";
  html += "</body></html>";
  return html;
}

String getSerialConfigPage() {
  String html = "<html><body><h2>Serielle Pins & MQTT Subscriptions</h2>";
  html += "<form action=\"/serial/save\" method=\"POST\">";
  
  html += "RX Pin: <input name=\"rx_pin\" type=\"number\" value=\"" + String(rxPin) + "\"><br>";
  html += "TX Pin: <input name=\"tx_pin\" type=\"number\" value=\"" + String(txPin) + "\"><br>";

  html += "<h3>MQTT Subscriptions</h3>";
  for (int i = 0; i < NUM_VE_HEX_REGISTERS; i++) {
    html += "Register <b>" + String(VEregisters[i].register_name) + "</b>: ";
    html += "<input name=\"mqtt_subscription_" + String(i) + "\" type=\"text\" value=\"" + VEregisters[i].mqtt_subscription + "\"><br>";
  }

  html += "<input type=\"submit\" value=\"Save\">";
  html += "</form><br><a href=\"/\">Back</a>";
  html += "</body></html>";
  return html;
}

String getUpdatePage() {
  return R"rawliteral(
<html><body>
  <h2>Firmware Update</h2>
  <form method="POST" action="/update" enctype="multipart/form-data">
    <input type="file" name="update"><br><br>
    <input type="submit" value="Begin update">
  </form>
  <br><a href="/">Back</a>
</body></html>
)rawliteral";
}

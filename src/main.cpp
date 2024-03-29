#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Replace with your information
#define DEVICE_NAME "DefaultName"
#define WIFI_SSID "default"
#define WIFI_PSK "default"

WiFiClient client;

// IPIFY API
HTTPClient http;
const String publicIPCall = "https://api.ipify.org/";

// Duck DNS
const String duckApiToken = "Your Duck DNS API Token Here";
const String domainName = "Your Duck DNS URL Here";
const String fqdn = domainName + ".duckdns.org";
const String duckDNSuri = "https://www.duckdns.org/update?domains=";

// Discord web-hook
const String webhook = "Optional, but put your discord webhook here if you want discord alerts";
bool using_discord = true; //Change to false if not using discord

Preferences prefs;

// Global variables
String publicIP;

// PublicIP check timer, in millis
const int public_interval = 10000;
long unsigned int public_timer = millis();

// Keep track of failures in case you want to do something if things fail too much
int failure_counter = 0;

// Sets the DDNS to the input IP address
String setDDNS(String ipAddress)
{
  // create the uri for the call from the variables
  String uri = duckDNSuri + domainName;
  uri += "&token=" + duckApiToken;
  uri += "&ip=" + ipAddress;

  Serial.println("Setting IP on DuckDNS...");
  Serial.println("Calling: " + uri);

  // Send GET
  http.begin(uri);
  int httpCode = http.GET();

  // Read response
  String response = http.getString();

  if (httpCode != 200)
  {
    return "failure";
  }
  else
  {
    return response;
  }
}

// Calls out via API to get the current public address
String getPublicIP()
{
  Serial.println("Getting Public IP...");

  http.begin(publicIPCall);
  int httpCode = http.GET();

  if (httpCode == 200)
  {
    Serial.println("Success!");
    String response = http.getString();
    http.end();
    return response;
  }
  else
  {
    http.end();
    Serial.println("ERROR! Recieved a " +  String(httpCode));
    return "failure";
  }
}

// Takes a message and sends it to the Discord webhook provided
void sendDiscordMessage(String message) 
{

  // Skip if not using discord webhooks.
  if (!using_discord){
    return;
  }

  // Create JSON
  DynamicJsonDocument doc(JSON_OBJECT_SIZE(1024));
  doc["content"] = message;
  // Feel free to modify these if you want a differernt name/image
  doc["username"] = "HoboDDNS";
  doc["avatar_url"] = "http://www.duckdns.org/img/ducky_icon.png";
  String serialJSON;
  serializeJsonPretty(doc, serialJSON);

  // Send POST
  http.begin(webhook);
  http.addHeader("Content-Type","application/json");
  http.POST(serialJSON);
}

void setup() 
{

  // Initiate serial connection
  Serial.begin(9600);
  Serial.println("Initializing...");
  
  // Establish WiFi Connection
  Serial.println();
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  Serial.print("\nConnecting to WiFI");

  //wait for Wifi to connect
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi Connected! IP is: ");
  Serial.print(WiFi.localIP());
  Serial.println();
  
  // Load flash storage, see if we had an IP before
  prefs.begin("wonder", false);
  publicIP = prefs.getString("publicIP");

  // If not found, create it and update discord
  if (publicIP == String())
  {
    Serial.println("IP not found in storage, generating...");
    publicIP = getPublicIP();
    prefs.putString("publicIP", publicIP);
  }

  // Hello World!
  String message = String(DEVICE_NAME) + " is now online";
  sendDiscordMessage(message);
}
 
void loop() 
{
  // Check if its time to look at public IP
  if (public_timer - millis() >= public_interval)
  {
    Serial.println();
    Serial.println("Checking public IP...");
    String currentPubIP = getPublicIP();

    // If the call fails, increment the counter and try again
    if (currentPubIP == "failure")
    {
      failure_counter++;

      // After 15 minutes of failure, reboot
      if (failure_counter >= 90)
      {
        Serial.println("Something broke");
        ESP.restart();
      }
    }

    // If the call is successful and the new IP does not match the old, change it
    else if (currentPubIP != publicIP)
    {
      // Reset the failure counter as it didnt fail.
      failure_counter = 0;

      Serial.print("IP change detected! IP is now: ");
      Serial.print(currentPubIP);
      Serial.println();

      // Save it in Flash
      prefs.putString("publicIP", currentPubIP);

      // Update DuckDNS
      String response = setDDNS(currentPubIP);

      if (response == "OK")
      {
        Serial.println("IP updated");
        
        // Notify Discord
        String message = String(DEVICE_NAME) + " changed IPv4 address. DNS updated to: ";
        message += currentPubIP;
        sendDiscordMessage(message);
      }
      else
      {
        Serial.println("Failed to update DDNS");
        sendDiscordMessage(String("Failed to update DuckDNS for" + String(DEVICE_NAME)));
      }

      // Save and write out the new IP
      publicIP = currentPubIP;
    }
    Serial.println("Public IP is currently: " + publicIP);
    Serial.println("Resetting check timer...");
    
    // Reset timer to current millis
    public_timer = millis();
  }
}
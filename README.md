# HoboDDNS
An Arudiono DDNS client to allow for constant updating of your public IP to a DuckDNS domain.

## Hardware
This code is written with an ESP32 Devkit V1 in mind. You can get a million of them from AliExpress

I also have printed a really nice case for all mine, check it out here:
https://www.printables.com/model/50035-esp32-devkit-v1-chunky-case

## Setup
1. Sign up and create a domain on http://www.duckdns.org/.
- Be sure to keep track of your API Token, as well as the name of the domain you made.
2. Setup and install either Arduino Studio, or PlatformIO on VSCode
- I use PlatformIO, so the setup will be focused on that.
3. Create a PlatformIO project with an ESP32 dev board as the target DEVICE_NAME
4. Copy the code from src into the src PlatformIO created.
5. Change the items in the Configuration section
6. Upload and test!

## Configuration
The script should work as is, you just need to provide a bit of information:
* `DEVICE_NAME`: This is the name that shows up in the alert
* `WIFI_SSID`: The SSID of your local WiFi
* `WIFI_PSK`: The PreShared Key, or your WiFi Pasword
* `duckAPiToken`: The API token on your login page of duckDNS
* `domainName`: The target domain you want to update
* `webhook`: The Discord webhook of the channel you want notifications on

If you dont want to integrate with Discord, just delete all the calls to sendDiscordMessage().

## Disclaimer
This code was stripped of a lot of the things I use it for to give a more barebones example.
I encourage you to make changes and improvements as you see fit.
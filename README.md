![WifiSnare](https://github.com/user-attachments/assets/23dad4db-6bda-4d47-b5af-698ed5451e18)

# WifiSnare
It capture 2.4GHz WiFi passwords using Evil-Twin attack.

# Key Features
- Simple Setup.
- Set SSID easily for attack.
- LED Status for Right and Wrong Passwords.
- Download captured password on the system in a `.txt` file.

# Supported Board
- It supports NodeMCU ESP8266 Boards only.
- It supports 2.4GHz frequency only.
- If possible, use those NodeMCU ESP8266 boards which contain `CP2102` driver chipset.

# Setup
1. Download Arduino IDE from [here](https://www.arduino.cc/en/software) according to your Operating System.
2. Install it.
3. Go to `File` → `Preferences` → `Additional Boards Manager URLs`.
4. Paste the following link :
   
   ```
   https://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
5. Click on `OK`.
6. Go to `Tools` → `Board` → `Board Manager`.
7. Wait for sometimes and search `esp8266` by `ESP8266 Community`.
8. Simply install it.
9. Wait for sometime and after that it is installed.
10. Restart the Arduino IDE.
11. Done!

# Install
## Via Arduino IDE
1. Download or Clone the Repository.
2. Open the folder and just double click on `WifiSnare.ino` file.
3. It opens in Arduino IDE.
4. Compile the code.
5. Select the correct board from the `Tools` → `Board` → `ESP8266 Boards`.
   - It is generally `NodeMCU 1.0 (ESP-12E Module)`.
6. Select the correct port number of that board.
7. Upload the code.
## Via ESP8266 Flasher
1. Download the NodeMCU ESP8266 Flasher from [here](https://github.com/nodemcu/nodemcu-flasher) according to your operating system.
2. Download the `.bin` file from [here](https://github.com/wirebits/WifiSnare/releases/download/v1.0/WifiSnare.bin).
3. Open NodeMCU ESP8266 Flasher.
4. Click on `Advanced` Tab.
5. Click on `Restore Default` button.
6. Click on `Config` Tab.
   - It show `INTERNAL://NODEMCU`.
7. Click on ![image](https://github.com/user-attachments/assets/1540d7e8-514a-4e60-a29d-3019699868df) in front of `INTERNAL://NODEMCU`.
8. Select the `WifiSnare.bin` file.
9. Click on `Operation` Tab.
10. Click on `Flash(F)` button.
12. Wait for sometimes and when completed, press `RST` button.
# Run the Script
1. After uploading wait 1-2 minutes and after that an Access Point is created named `WiFiSnare` whose password is `wifisnare`.
2. Connect to it.
3. After few seconds, a page automatically opens where ask to enter the exact SSID name to start attack on it.
4. Click on `Start Capture` button.
5. After that, it disconnects the access point and as well as an open access point created with the same SSID name.
6. Connect to that open WiFi.
7. It show a page where it ask for password.
8. Enter the password and click on `Sign Up` button.
    - If password is wrong, then led of the board blink `2` times and back to the password page to enter password again.
    - If password is right, then led of the board blink `3` times and after `2` seconds it close that open access point and restart the `WiFiSnare` access point.
9. Connnct again to that `WiFiSnare` access point.
10. At the bottom, it shows the password of that SSID and a download button to save the password to the Phone/PC/Laptop in a `.txt` file.
11. Also, to attack on some other SSID, type the exact name and click on `Start Capture` button.

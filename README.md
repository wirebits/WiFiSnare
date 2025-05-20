![WifiSnare](https://github.com/user-attachments/assets/23dad4db-6bda-4d47-b5af-698ed5451e18)

# WiFiSnare
It capture 2.4GHz WiFi passwords using Evil-Twin attack.

# Key Features
- Simple Setup.
- Select network easily for eviltwin attack.
- It deauthenticates network parallelly.
- LED Status for Right and Wrong Passwords.
- Download captured password on the system in a `.txt` file.

# Supported Board
- It supports microcontroller boards which contain `ESP8266` chipset only.
- It supports 2.4GHz frequency only.
- If possible, use those ESP8266 microcontroller boards which contain `CP2102` driver chipset.

# Setup
1. Download Arduino IDE from [here](https://www.arduino.cc/en/software) according to your Operating System.
2. Install it.
3. Go to `File` → `Preferences` → `Additional Boards Manager URLs`.
4. Paste the following link :
   
   ```
   https://raw.githubusercontent.com/SpacehuhnTech/arduino/main/package_spacehuhn_index.json
   ```
5. Click on `OK`.
6. Go to `Tools` → `Board` → `Board Manager`.
7. Wait for sometimes and search `deauther` by `Spacehuhn Technologies`.
8. Simply install it.
9. Wait for sometime and after that it is installed.
10. Restart the Arduino IDE.
11. Done!

# Install
## Via Arduino IDE
1. Download or Clone the Repository.
2. Open the folder and just double click on `WiFiSnare.ino` file.
3. It opens in Arduino IDE.
4. Compile the code.
5. Select the correct board from the `Tools` → `Board` → `Deauther ESP8266 Boards`.
   - It is `NodeMCU`.
6. Select the correct port number of that board.
7. Upload the code.
## Via ESP8266 Flasher
1. Download the NodeMCU ESP8266 Flasher from [here](https://github.com/nodemcu/nodemcu-flasher) according to your operating system.
2. Download the `.bin` file from [here]().
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
3. After few seconds, a page automatically opens where it show a table contain nearby wifi networks.
4. Select the network want to attack.
5. Click on `Start Deauth` button to start deauthentication attack on selected network.
6. Click on `Start EvilTwin` button to start eviltwin attack on selected network.
7. After that, it disconnects the access point and as well as an open access point created with the selected network SSID name.
8. Connect to that open WiFi.
9. It show a page where it ask for password.
10. Enter the password and click on `Continue` button.
    - If password is wrong, then led of the board blink `2` times and back to the password page to enter password again.
    - If password is right, then led of the board blink `3` times and after `3` seconds it stops deauthentication attack, close that open access point and restart the `WiFiSnare` access point.
11. Connnct again to that `WiFiSnare` access point.
12. At the bottom, it shows the password of that SSID and a download button to save the password to the Phone/PC/Laptop in a `.txt` file.
13. Also, to attack on some other SSID, just select the network want to attack and repeat steps from `5` to `12`.

# ESP32-S3 Supermini device with ST7789 TFT display and connected BME280 sensor

Getting started with a **BME280** environment sensor connected to an **ESP32-S3 Supermini** device and a TFT display with driver chip **ST7789**.

This is the accompanying repository for my article "**Getting started with a BME280 environment sensor connected to an ESP32-S3 Supermini device and ST7789 TFT display**": https://medium.com/@androidcrypto/260347aa499c.

My display is a 2.0 inch large TFT display with 240 x 320 pixels.

## Settings for the display specific setup file

I'm using a display specific setup file for the combination ESP32-S3 Supermini with TFT display driver ST7789. This file contains e.g. the display driver, size and pins for the ESP32-device. If your display has a different size please change the height and width accordingly. 

### Copy a file to the User_Setups folder

Please copy the file

    Setup703_S3_SM_ST7789_240x320.h

to the **User_Setups** folder.

### Edit the User_Setup_Select.h file

You need to place the following line in the root folder's "**User_Setup_Select.h**" file

    #include <User_Setups/Setup703_S3_SM_ST7789_240x320.h> // ESP32-S3 Supermini, 40 MHz

Second: please comment all other "#include..." entries like this, especially the "//#include <User_Setup.h>" entry.

````
// Example User_Setup files are stored in the "User_Setups" folder. These can be used
// unmodified or adapted for a particular hardware configuration.
#ifndef USER_SETUP_LOADED //  Lets PlatformIO users define settings in
                          //  platformio.ini, see notes in "Tools" folder.
///////////////////////////////////////////////////////
//   User configuration selection lines are below    //
///////////////////////////////////////////////////////
// Only ONE line below should be uncommented to define your setup.  Add extra lines and files as needed.
//#include <User_Setup.h>                       // Default setup is root library folder
// Setup file in folder Arduino/libraries (updates will not overwrite your setups)
#include <User_Setups/Setup703_S3_SM_ST7789_240x320.h> // ESP32-S3 Supermini, 40 MHz
````

## Important note

You need to modify the display library TFT_eSPI to get the code to work. Please find instructions on how to do this in my forked TFT_eSPI repository here on GitHub: [https://github.com/AndroidCrypto/TFT_eSPI](https://github.com/AndroidCrypto/TFT_eSPI).

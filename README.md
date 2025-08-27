# Weather Data Logger

A C program for Raspberry Pi to fetch weather data from WeatherAPI, parse JSON, and log to a CSV file.

Setup

 - Install dependencies: sudo apt update && sudo apt install libcurl4-openssl-dev

- Download cJSON: wget https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.c

- Compile: gcc -o weather_logger main.c cJSON.c -lcurl

- Run: ./weather_logger


## Requirements

- Raspberry Pi with Raspberry Pi OS

- WeatherAPI key (sign up at https://www.weatherapi.com)

- Internet connection

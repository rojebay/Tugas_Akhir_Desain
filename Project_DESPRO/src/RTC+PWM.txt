#include <Arduino.h>
#include <uRTCLib.h>
#include <Wire.h>
#include <time.h>

// Pin Definitions
#define WHITE_LED_PIN 12
#define YELLOW_LED_PIN 14

// PWM Configuration
#define PWM_CHANNEL_WHITE 0
#define PWM_CHANNEL_YELLOW 1
#define PWM_FREQUENCY 100
#define PWM_RESOLUTION 8

bool first_time_run = false;

tm current_time;

// Initialize RTC object (DS1307)
uRTCLib rtc(0x68);    //, URTCLIB_MODEL_DS3231); // Default I2C address for DS1307

#define ONE_HOUR 3600

    /** Angular degree, expressed in arc seconds */
#define ONE_DEGREE 3600

    /** One day, expressed in seconds */
#define ONE_DAY 86400

// extracts 1..4 characters from a string and interprets it as a decimal value
#define CONV_STR2DEC_1(str, i)  (str[i]>'0'?str[i]-'0':0)
#define CONV_STR2DEC_2(str, i)  (CONV_STR2DEC_1(str, i)*10 + str[i+1]-'0')
#define CONV_STR2DEC_3(str, i)  (CONV_STR2DEC_2(str, i)*10 + str[i+2]-'0')
#define CONV_STR2DEC_4(str, i)  (CONV_STR2DEC_3(str, i)*10 + str[i+3]-'0')

// Custom "glue logic" to convert the day of week name to a usable number
#define GET_DOW(str, i)        (str[i]=='S' && str[i+1]=='u' && str[i+2]=='n' ? 1 :     \
                                str[i]=='M' && str[i+1]=='o' && str[i+2]=='n' ? 2 :     \
                                str[i]=='T' && str[i+1]=='u' && str[i+2]=='e' ? 3 :     \
                                str[i]=='W' && str[i+1]=='e' && str[i+2]=='d' ? 4 :     \
                                str[i]=='T' && str[i+1]=='h' && str[i+2]=='r' ? 5 :     \
                                str[i]=='F' && str[i+1]=='r' && str[i+2]=='i' ? 6 :     \
                                str[i]=='S' && str[i+1]=='a' && str[i+2]=='t' ? 7 : 0)

// Custom "glue logic" to convert the month name to a usable number
#define GET_MONTH(str, i)      (str[i]=='J' && str[i+1]=='a' && str[i+2]=='n' ? 1 :     \
                                str[i]=='F' && str[i+1]=='e' && str[i+2]=='b' ? 2 :     \
                                str[i]=='M' && str[i+1]=='a' && str[i+2]=='r' ? 3 :     \
                                str[i]=='A' && str[i+1]=='p' && str[i+2]=='r' ? 4 :     \
                                str[i]=='M' && str[i+1]=='a' && str[i+2]=='y' ? 5 :     \
                                str[i]=='J' && str[i+1]=='u' && str[i+2]=='n' ? 6 :     \
                                str[i]=='J' && str[i+1]=='u' && str[i+2]=='l' ? 7 :     \
                                str[i]=='A' && str[i+1]=='u' && str[i+2]=='g' ? 8 :     \
                                str[i]=='S' && str[i+1]=='e' && str[i+2]=='p' ? 9 :     \
                                str[i]=='O' && str[i+1]=='c' && str[i+2]=='t' ? 10 :    \
                                str[i]=='N' && str[i+1]=='o' && str[i+2]=='v' ? 11 :    \
                                str[i]=='D' && str[i+1]=='e' && str[i+2]=='c' ? 12 : 0)

// extract the information from the time string given by __TIME__ and __DATE__
#define __TIME_SECONDS__        CONV_STR2DEC_2(__TIME__, 6)
#define __TIME_MINUTES__        CONV_STR2DEC_2(__TIME__, 3)
#define __TIME_HOURS__          CONV_STR2DEC_2(__TIME__, 0)
#define __TIME_DOW__            GET_DOW(__TIMESTAMP__, 0)
#define __TIME_DAYS__           CONV_STR2DEC_2(__DATE__, 4)
#define __TIME_MONTH__          GET_MONTH(__DATE__, 0)
#define __TIME_YEARS__          CONV_STR2DEC_2(__DATE__, 9)

constexpr auto COMPILE_HASH = __TIME_SECONDS__ + __TIME_MINUTES__ + __TIME_HOURS__ + __TIME_DOW__ + __TIME_DAYS__ + __TIME_MONTH__ + __TIME_YEARS__;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Check if RTC is working
    if (rtc.refresh() == false) {
        Serial.println(F("RTC tidak terdeteksi!"));
        while (true) {
            delay(100);
        }
    }

    ledcSetup(PWM_CHANNEL_WHITE, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_YELLOW, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(WHITE_LED_PIN, PWM_CHANNEL_WHITE);
    ledcAttachPin(YELLOW_LED_PIN, PWM_CHANNEL_YELLOW);

    // Print compile time untuk debugging
    Serial.println(F("Waktu Kompilasi:"));
    Serial.printf("Tanggal: %02d/%02d/%02d\n", __TIME_DAYS__, __TIME_MONTH__, __TIME_YEARS__);
    Serial.printf("Waktu: %02d:%02d:%02d\n", __TIME_HOURS__, __TIME_MINUTES__, __TIME_SECONDS__);

    // Force set waktu saat pertama kali atau RTC kehilangan daya
    if (first_time_run || rtc.lostPower() || rtc.getEOSCFlag()) {
        Serial.println(F("Setting waktu RTC..."));
        
        rtc.set(
            __TIME_SECONDS__,   // Detik
            __TIME_MINUTES__,   // Menit
            __TIME_HOURS__,     // Jam
            __TIME_DOW__,       // Hari (1-7)
            __TIME_DAYS__,      // Tanggal
            __TIME_MONTH__,     // Bulan
            __TIME_YEARS__      // Tahun
        );
        
        if (rtc.lostPower()) {
            rtc.lostPowerClear();
        }
        
        Serial.println(F("RTC berhasil diset!"));
    }

    // Verifikasi waktu RTC
    rtc.refresh();
    Serial.println(F("Waktu RTC saat ini:"));
    Serial.printf("Tanggal: %02d/%02d/%02d\n", rtc.day(), rtc.month(), rtc.year());
    Serial.printf("Waktu: %02d:%02d:%02d\n", rtc.hour(), rtc.minute(), rtc.second());
}

void loop() {
    // Refresh RTC data
    rtc.refresh();

    // Create time structure for calibration
    
    // Calibrate time values
    current_time.tm_year = rtc.year() + 100;    // RTC years since 2000, tm_year expects years since 1900
    current_time.tm_mon = rtc.month() - 1;      // RTC month is 1-12, tm_mon expects 0-11
    current_time.tm_mday = rtc.day();           // Day of month (1-31)
    current_time.tm_hour = rtc.hour();          // Hour (0-23)
    current_time.tm_min = rtc.minute();         // Minute (0-59)
    current_time.tm_sec = rtc.second();         // Second (0-59)

    // Print formatted date and time
    Serial.printf("Date: %02d/%02d/20%02d\n", 
        current_time.tm_mday,
        current_time.tm_mon + 1,
        current_time.tm_year - 100);
    
    Serial.printf("Time: %02d:%02d:%02d\n",
        current_time.tm_hour,
        current_time.tm_min,
        current_time.tm_sec);

    // auto now_time = mktime(&current_time);
    

    // for(int i = 0; i <= 8; i++) {
    //     ledcWrite(PWM_CHANNEL_WHITE, i);
    //     Serial.printf("White LED Brightness: %d\n", i);
    //     delay(500);
    // }
    // // White LED fade out
    // for(int i = 8; i >= 0; i--) {
    //     ledcWrite(PWM_CHANNEL_WHITE, i);
    //     Serial.printf("White LED Brightness: %d\n", i);
    //     delay(500);
    // }

    // Yellow LED fade in
    for(int i = 0; i <= 11; i++) {
        ledcWrite(PWM_CHANNEL_YELLOW, i);
        ledcWrite(PWM_CHANNEL_WHITE, i);
        Serial.printf("White LED Brightness: %d\n", i);
        delay(1000);
    }
    // Yellow LED fade out
    for(int i = 11; i >= 0; i--) {
        ledcWrite(PWM_CHANNEL_YELLOW, i);
        ledcWrite(PWM_CHANNEL_WHITE, i);
        Serial.printf("White LED Brightness: %d\n", i);
        delay(1000);
    }

    delay(1000); // Update every second
}
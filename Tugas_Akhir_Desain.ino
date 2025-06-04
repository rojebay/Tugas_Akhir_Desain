#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Wire.h>
#include "RTClib.h"

// Inisialisasi RTC
RTC_DS3231 rtc;

// Inisialisasi DFPlayer Mini
SoftwareSerial mySoftwareSerial(17, 16); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// Variabel untuk menyimpan waktu terakhir pemutaran
int lastPlayedHour = -1;

void setup() {
  Serial.begin(9600);
  mySoftwareSerial.begin(9600);
  
  // Inisialisasi RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Jika RTC kehilangan daya, atur waktu ke waktu kompilasi
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Inisialisasi DFPlayer Mini
  Serial.println(F("Initializing DFPlayer..."));
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(20); // Set volume (0-30)
}

void loop() {
  DateTime now = rtc.now();
  int currentHour = now.hour();
  
  // Cek jika jam berubah dan sesuai dengan waktu yang diinginkan
  if (currentHour != lastPlayedHour) {
    if (currentHour == 8) { // Jam 08:00 pagi
      playTrack("alarm.mp3");
      lastPlayedHour = currentHour;
    } 
    else if (currentHour == 22) { // Jam 22:00 malam
      playTrack("musik-tidur.mp3");
      lastPlayedHour = currentHour;
    }
  }
  
  delay(1000); // Cek setiap detik
}

void playTrack(const char* filename) {
  // Pada DFPlayer Mini, kita memutar berdasarkan nomor track atau nama file
  // Versi ini mengasumsikan Anda menggunakan nomor track
  
  // Jika Anda ingin menggunakan nama file, Anda perlu mengimplementasikan
  // sistem mapping nama file ke nomor track atau menggunakan library khusus
  
  Serial.print("Playing: ");
  Serial.println(filename);
  
  // Contoh: jika alarm.mp3 adalah track 1 dan musik-tidur.mp3 adalah track 2
  if (strcmp(filename, "alarm.mp3") == 0) {
    myDFPlayer.play(1);
  } else if (strcmp(filename, "musik-tidur.mp3") == 0) {
    myDFPlayer.play(2);
  }
  
  // Atau jika Anda ingin menghentikan pemutaran sebelumnya
  // myDFPlayer.stop();
  // delay(100);
  // kemudian play track baru
}
#include "CTBot.h"
CTBot mybot;

// Konfigurasi koneksi WiFi
const char* ssid = "Febriyandy2_";
const char* pass = "12345678";

// Token bot Telegram dan ID chat untuk tujuan pengiriman pesan
const char* token = "6024356760:AAFuCPbuSUHBqs4CaW-5HSPoeIBe9QQhx-k";
const long chatId = 868397214;

// Pin untuk sensor hujan dan sensor cahaya
const int pinSensorHujan = 35;
const int pinSensorCahaya = 34;

// Pin untuk kontrol motor
int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 14;

// Konfigurasi PWM untuk mengontrol kecepatan motor
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 170;

// Variabel untuk melacak status jemuran
bool JemuranKeluar = false;

// Variabel untuk menyimpan nilai sensor sebelumnya
int lastSensorHujan = 0;
int lastSensorCahaya = 0;

// Fungsi untuk mengirim pesan melalui Telegram
void sendTelegramMessage(String message) {
  mybot.sendMessage(chatId, message);
}

void setup() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  // Konfigurasi PWM untuk motor
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcWrite(pwmChannel, dutyCycle);

  Serial.begin(9600);
  Serial.println("Mulai telegram bot");

  // Menghubungkan bot ke jaringan WiFi
  mybot.wifiConnect(ssid, pass);
  mybot.setTelegramToken(token);

  // Memeriksa koneksi ke server Telegram
  if (mybot.testConnection())
    Serial.println("Koneksi Berhasil");
  else
    Serial.println("Koneksi gagal");
}

// Fungsi untuk menggerakkan motor
void GerakMotor(bool jemuranKeluar, int duration) {
  if (jemuranKeluar) {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
  } else {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
  }

  // Mengatur kecepatan motor dengan PWM
  for (dutyCycle = 250; dutyCycle < 255; dutyCycle++) {
    ledcWrite(pwmChannel, dutyCycle);
    delay(20);
  }

  // Menggerakkan motor selama durasi yang ditentukan (dalam detik)
  delay(duration * 130);

  // Menghentikan motor setelah durasi berakhir
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
}

void loop() {
 
  int nilaiSensorHujan = analogRead(pinSensorHujan);
  int nilaiSensorCahaya = analogRead(pinSensorCahaya);

  // Menampilkan nilai sensor hujan dan cahaya pada Serial Monitor
  Serial.print("Nilai Sensor Hujan: ");
  Serial.println(nilaiSensorHujan);
  Serial.print("Nilai Sensor Cahaya: ");
  Serial.println(nilaiSensorCahaya);


  if (nilaiSensorHujan != lastSensorHujan || nilaiSensorCahaya != lastSensorCahaya) {
    String message;

    if (nilaiSensorHujan >= 3900 && nilaiSensorCahaya < 400) {
      if (!JemuranKeluar) {
        GerakMotor(true, 2);
        JemuranKeluar = true;
        message = "Cuaca Cerah dan Jemuran Keluar";
        sendTelegramMessage(message);
      }
    } else if (nilaiSensorHujan <= 3900 && nilaiSensorCahaya < 400) {
      if (JemuranKeluar) {
        GerakMotor(false, 2);
        JemuranKeluar = false;
        message = "Cuaca Hujan Panas dan Jemuran Masuk";
        sendTelegramMessage(message);
      }
    } else if (nilaiSensorHujan <= 3900 && nilaiSensorCahaya > 400) {
      if (JemuranKeluar) {
        GerakMotor(false, 2); 
        JemuranKeluar = false;
        message = "Cuaca Hujan dan Jemuran Masuk";
        sendTelegramMessage(message);
      }
    } else if (nilaiSensorHujan >= 3900 && nilaiSensorCahaya > 400) {
      if (JemuranKeluar) {
        GerakMotor(false, 2); 
        JemuranKeluar = false;
        message = "Cuaca Mendung / Sudah Malam dan Jemuran Masuk";
        sendTelegramMessage(message);
      }
    }

    // Menyimpan nilai sensor terbaru
    lastSensorHujan = nilaiSensorHujan;
    lastSensorCahaya = nilaiSensorCahaya;
  }

  // Menerima perintah dari Telegram
  TBMessage tMessage;
  if (mybot.getNewMessage(tMessage)) {
    if (tMessage.text.equalsIgnoreCase("/cuaca")) {
      String cuaca;
      if (nilaiSensorHujan >= 3900 && nilaiSensorCahaya < 400) {
        cuaca = "Cerah";
      } else if (nilaiSensorHujan <= 3900 && nilaiSensorCahaya < 400) {
        cuaca = "Hujan Panas";
      } else if (nilaiSensorHujan <= 3900 && nilaiSensorCahaya > 400) {
        cuaca = "Hujan";
      } else if (nilaiSensorHujan >= 3900 && nilaiSensorCahaya > 400) {
        cuaca = "Mendung";
      }

      String message = "Kondisi Cuaca: " + cuaca;
      sendTelegramMessage(message);
    } else if (tMessage.text.equalsIgnoreCase("/jemuran")) {
      sendTelegramMessage(JemuranKeluar ? "Jemuran Berada di Luar" : "Jemuran Berada di Dalam");
    } else if (tMessage.text.equalsIgnoreCase("/keluarkan")) {
      if (!JemuranKeluar) {
        GerakMotor(true, 2); 
        JemuranKeluar = true;
        sendTelegramMessage("Jemuran Keluar");
      }
    } else if (tMessage.text.equalsIgnoreCase("/masukkan")) {
      if (JemuranKeluar) {
        GerakMotor(false, 2); 
        JemuranKeluar = false;
        sendTelegramMessage("Jemuran Masuk");
      }
    }
  }

  delay(1000); // Menunda pembacaan selama 1 detik
}

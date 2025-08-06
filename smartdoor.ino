#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN 5
#define SS_PIN 10
#define BUZZER_PIN 8
#define SERVO_PIN 7
MFRC522 rfid(SS_PIN, RST_PIN);
Servo myServo;

String authorizedUIDs[] = {
  "b7 6c 6a 05"
};
int numberOfAuthorizedUids = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);

bool buzzing = false;
int melody[] = {
  659, 622, 659, 784, 784, 880, 784, 659,
  698, 659, 622, 659, 784, 784, 880, 784, 659,
  784, 880, 988, 988, 880, 784, 659, 784,
  698, 659, 622, 659, 784, 784, 880, 784, 659
};
int noteDurations[] = {
  300, 150, 150, 300, 150, 150, 300, 300,
  300, 500, 150, 150, 300, 150, 150, 300, 300,
  200, 200, 300, 150, 150, 300, 200, 200,
  300, 200, 150, 150, 300, 150, 150, 300, 500
};
int melodyLength = sizeof(melody) / sizeof(melody[0]);

int melodyIndex = 0;
unsigned long buzzerPreviousMillis = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  myServo.attach(SERVO_PIN);
  myServo.write(50);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("Sistem siap. Scan kartumu.");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String scannedUID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      scannedUID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) scannedUID += " ";
    }

    // Serial.print("UID Terdeteksi: ");
    // Serial.println(scannedUID);
    // rfid.PICC_HaltA();

    bool isAuthorized = false;
    for (int i = 0; i < numberOfAuthorizedUids; i++) {
      if (scannedUID.equalsIgnoreCase(authorizedUIDs[i])) {
        isAuthorized = true;
        break;
      }
    }

    if (isAuthorized) {
      if (buzzing) {
        buzzing = false;
        digitalWrite(BUZZER_PIN, LOW);
        Serial.println("Alarm disabled!");
      } else {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);

        myServo.write(90);
        Serial.println("Authorized!");
        delay(6000);
        myServo.write(50);
      }
    } else {
      buzzing = true;
    }

    rfid.PICC_HaltA();
  }

  if (buzzing) {
    unsigned long currentMillis = millis();

    if (currentMillis - buzzerPreviousMillis >= noteDurations[melodyIndex]) {
      buzzerPreviousMillis = currentMillis;

      int note = melody[melodyIndex];
      if (note == 0) {
        noTone(BUZZER_PIN);
      } else {
        tone(BUZZER_PIN, note, noteDurations[melodyIndex]);
      }

      melodyIndex++;
      if (melodyIndex >= melodyLength) {
        melodyIndex = 0;
      }
    }
  } else {
    noTone(BUZZER_PIN);
    melodyIndex = 0;
  }
}

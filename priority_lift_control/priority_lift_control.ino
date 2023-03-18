#include <Adafruit_Fingerprint.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Servo.h>

#define RST_PIN         9          // MFRC522 reset pin
#define SS_PIN          10         // MFRC522 slave select pin
#define LED_PIN         6          // LED connected to digital pin 6
#define SERVO_PIN       8          // Servo motor connected to digital pin 8
#define FINGERPRINT_RX  7          // Fingerprint sensor RX pin (connect to TX pin on Arduino)
#define FINGERPRINT_TX  6          // Fingerprint sensor TX pin (connect to RX pin on Arduino)
#define FINGERPRINT_ID  1234       // Authorized fingerprint ID
#define NO_AUTH_ID      0          // Fingerprint ID for non-authorized users
#define NUM_AUTH_CARDS  3          // Number of authorized RFID cards
#define AUTH_CARDS      {           \
  {0x12, 0x34, 0x56, 0x78},        \
  {0x98, 0x76, 0x54, 0x32},        \
  {0xAB, 0xCD, 0xEF, 0x01}         \
}
#define CARD_SIZE       4          // Size of RFID card UID

MFRC522 rfid(SS_PIN, RST_PIN);      // Create MFRC522 instance
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1); // Create fingerprint instance
Servo servo;                        // Create servo instance

byte auth_cards[NUM_AUTH_CARDS][CARD_SIZE] = AUTH_CARDS;

bool check_card(byte *card_uid);
bool check_fingerprint(uint8_t id);

void setup() {
  Serial.begin(9600);
  while (!Serial) {} // Wait until serial is ready - Leonardo/Micro
  Serial1.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  rfid.PCD_Init(); // Initialize MFRC522 RFID reader
  finger.begin(57600); // Initialize fingerprint sensor
}

void loop() {
  // Wait for card or fingerprint scan
  while (!rfid.PICC_IsNewCardPresent() && !finger.getImage()) {}
  bool has_card = rfid.PICC_IsNewCardPresent();
  
  if (has_card) {
    // Get card UID
    byte card_uid[CARD_SIZE];
    if (!rfid.PICC_ReadCardSerial()) {
      return;
    }
    memcpy(card_uid, rfid.uid.uidByte, CARD_SIZE);
    rfid.PICC_HaltA();
    
    // Check if card is authorized
    if (!check_card(card_uid)) {
      Serial.println("Access denied");
      return;
    }
  } else {
    // Check if fingerprint is authorized
    if (!check_fingerprint(FINGERPRINT_ID)) {
      Serial.println("Access denied");
      return;
    }
  }
  
  // Move the elevator
  Serial.println("Elevator moving...");
  digitalWrite(LED_PIN, HIGH);
  servo.write(90);
  delay(2000);
  servo.write(0);
  digitalWrite(LED_PIN, LOW);
}

bool check_card(byte *card_uid) {
  for (int i = 0; i < NUM_AUTH_CARDS; i++) {
    if (memcmp(auth_cards[i], card_uid, CARD_SIZE) == 0) {
      return true;
    }
  }
  return false;
}

bool check_fingerprint(uint8_t id) {
  // Check if fingerprint sensor is available
  if (finger.verifyPassword()) {
    // Search fingerprint database for match
    uint

//Libraries

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <Servo.h>
#include <stdint.h>
#include <string.h>

#define RST_PIN 9
#define SS_PIN 10
#define BUZZER 5

MFRC522 mfrc522(SS_PIN, RST_PIN);

//Global variables
unsigned long uid;
unsigned long null = 00000000;
int pos;
Servo myservo;
bool unlocked = false;

void setup() {
  // Empty authorized users and reset count
  /*int g;
  for(g = 8; g < 1024; g += 1){
    EEPROM.write(g,0);
  }
  EEPROM.write(1023,1); */


// Set up connection
  Serial.begin(9600);
  
// Set up servo motor
  myservo.attach(6);
  myservo.write(135);

// Set up storage
  unsigned long ownerTag = 3225510982;
  EEPROM.put(0,ownerTag);
  
  // Start when the serial connection is established
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(BUZZER, OUTPUT);
}

void loop() {

  if(Serial.available() > 0){
    // Retrieve received data
    String data;
    data = Serial.readString();

    // Decrypt the data
    String decrypted = caesar_decrypt(data,1);

    // Convert string to hex
    unsigned long hex = strtoul(decrypted.c_str(), nullptr, 16);

    // Notify the user it has worked and add it to the EEPROM
    buzz();
    addEEPROM(hex);

    // Empty the buffer
    Serial.end();
    Serial.begin(9600);
    
  }

  // New tag is detected
  if(mfrc522.PICC_IsNewCardPresent()) {
    
    if (mfrc522.PICC_ReadCardSerial()) {
      
      uid = getID();
      int i;
      
      // Check EEPROM for uid
      for(i=0;i < count*8; i = i + 8){
        unsigned long uidlst;
        EEPROM.get(i,uidlst);
  
        if(uid == uidlst){
          // Lock or unlock based on status
          if(unlocked){
            lockServo();
            unlocked = false;
          } else {
            openServo();
            unlocked = true;
          }
        return;
        }
      }

      // Alarm if not known uid
      for(i=0;i < 5; i = i+1){
        buzz();
      }    
    } 
  }
}

void buzz() {
  tone(BUZZER, 5000); //Set the voltage to high and makes a noise
  delay(200);//Waits for 1000 milliseconds
  noTone(BUZZER);//Sets the voltage to low and makes no noise
  delay(200);//Waits for 1000 milliseconds
}

void lockServo(){
   // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(135);              // tell servo to go to position in variable 'pos'
    delay(500);                       // waits 15ms for the servo to reach the position
}
void openServo(){                    // waits 15ms for the servo to reach the position
   // goes from 180 degrees to 0 degrees
    myservo.write(0);              // tell servo to go to position in variable 'pos'
    delay(500);                       // waits 15ms for the servo to reach the position
  
}

// UID in Byte array to unsigned long
unsigned long getID(){
  unsigned long hex_num = 0;

  // For every byte in the byte array
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    // Shift 8 bits left
    hex_num <<= 8;
    // Append the byte to hex_num
    hex_num |= mfrc522.uid.uidByte[i];
  }

  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}

String caesar_decrypt(String text, int shift) {
  String result = "";

  // Iterate through every char in text
  for (char character : text) {
    if (isAlpha(character)) {
      // Store whether the character was upper or lower
      bool isUpper = isUpperCase(character);
      character = toLowerCase(character);
      int charCode = character - 'a';

      // Decipher using caesar
      charCode = (charCode - shift + 26) % 26 + 'a';

      // Convert back to upper if needed
      if (isUpper) {
        character = toupper(charCode);
      } else {
        character = charCode;
      }
    } else if (isDigit(character)) {
      int digit = character - '0';

      // Decipher using caesar
      digit = (digit + shift) % 10 + '0';
      character = digit;
    }

    result += character;
  }

  return result;
}

void addEEPROM(unsigned long uid){
  int i = isAdded(uid);
  int count = EEPROM.read(1023);
  if (i > 0){ // uid exists in EEPROM at i, remove uid
    
    // If the uid is somewhere in the middle, move all others
    if (i/8 < count){
      int j;
      unsigned long temp;
      for(j = i/8; j < count-1; j += 1){
        EEPROM.get((j+1)*8,temp);
        EEPROM.put(j*8,temp);   
      }

      // erase last one
      EEPROM.put((j)*8,null);

    } else {

      // erase last one
      EEPROM.put(i,null);
    }

    // Count -1
    EEPROM.write(1023,count-1);

  } else if (i == -1) { // uid DNE in EEPROM, add uid
    EEPROM.put(count*8,uid);

    // Count +1
    EEPROM.write(1023,count+1);
    Serial.println('tag added!');
  }
  
}

// Checks whether uid is in EEPROM and returns the EEPROM address
int isAdded(unsigned long uid){
  int count = EEPROM.read(1023);
  int i;
  
  for(i=0;i < 8*count; i = i + 8){
    unsigned long uidlst;
    EEPROM.get(i,uidlst);

    if(uid == uidlst){
      return i;
    }
  }
  return -1;
}
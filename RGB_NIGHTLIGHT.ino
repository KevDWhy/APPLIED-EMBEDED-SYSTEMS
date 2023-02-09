#include <EEPROM.h>

const int rLED = 11;
const int gLED = 10;
const int bLED = 9;

const int button = 2;

bool lastButton = false;
bool currentButton = false;

bool lastOSRInput = false;

long transitionAt = millis();

int ledState = 0;

const int minRand = 2;
const int maxRand = 15;

String incoming = "";

bool newNumber = false;
bool setValid = true;

byte values[3];

void setup() {
  // put your setup code here, to run once:
  pinMode(rLED, OUTPUT);
  pinMode(gLED, OUTPUT);
  pinMode(bLED, OUTPUT);
  pinMode(button, INPUT);
  Serial.begin(9600);
  randomSeed(analogRead(A0));
}

bool debounce(long debounceTime){
  currentButton = digitalRead(button);
  if(currentButton && !lastButton){
    transitionAt = millis();
  }
  lastButton = currentButton;
  return currentButton && ((millis() - debounceTime) >= transitionAt);
}

bool OSR(bool input){
  bool tmp = input && !lastOSRInput;
  lastOSRInput = input;
  return tmp;
}

int updateLED(int mode){
  switch(mode){
    case 0:// OFF
      Serial.println("OFF");
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, LOW);
      digitalWrite(bLED, LOW);
      mode = 1;
      break;
    case 1:// RED
      Serial.println("RED");
      digitalWrite(rLED, HIGH);
      digitalWrite(gLED, LOW);
      digitalWrite(bLED, LOW);
      mode = 2;
      break;
    case 2:// GREEN
      Serial.println("GREEN");
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, HIGH);
      digitalWrite(bLED, LOW);
      mode = 3;
      break;
    case 3:// BLUE
      Serial.println("BLUE");
      digitalWrite(rLED, LOW);
      digitalWrite(gLED, LOW);
      digitalWrite(bLED, HIGH);
      mode = 4;
      break;
    case 4:// PURPLE
      Serial.println("PURPLE");
      analogWrite(rLED, 127);
      analogWrite(gLED, 0);
      analogWrite(bLED, 127);
      mode = 5;
      break;
    case 5:// TEAL
      Serial.println("TEAL");
      analogWrite(rLED, 0);
      analogWrite(gLED, 127);
      analogWrite(bLED, 127);
      mode = 6;
      break;
    case 6:// ORANGE
      Serial.println("ORANGE");
      analogWrite(rLED, 127);
      analogWrite(gLED, 127);
      analogWrite(bLED, 0);
      mode = 7;
      break;
    case 7:// WHITE
      Serial.println("WHITE");
      analogWrite(rLED, 170);
      analogWrite(gLED, 170);
      analogWrite(bLED, 170);
      mode = 8;
      break;
    case 8:// EEPROM COLOR
      Serial.println("EEPROM COLOR");
      analogWrite(rLED, EEPROM.read(0));
      analogWrite(gLED, EEPROM.read(1));
      analogWrite(bLED, EEPROM.read(2));
      mode = 9;
      break;
    case 9:// RANDOM COLOR
      Serial.println("RANDOM COLOR");
      int tmp = random(minRand, maxRand);
      analogWrite(rLED, tmp*tmp);
      tmp = random(minRand, maxRand);
      analogWrite(gLED, tmp*tmp);
      tmp = random(minRand, maxRand);
      analogWrite(bLED, tmp*tmp);
      mode = 0;
      break;
  }
  return mode;
}

void handleSerial(){
  if(Serial.available() > 0){
    incoming.concat((char)Serial.read());
  }
  if(incoming.endsWith("\n")){
    newNumber = true;
    int valuesParsed = 0;
    setValid = true;
    for(int i = 0; i < incoming.length(); i++){
      if(isDigit(incoming.charAt(i)) && newNumber){
        values[valuesParsed] = incoming.substring(i).toInt();
        newNumber = false;
      }
      if(incoming.charAt(i) == ','){
        newNumber = true;
        valuesParsed++;
        if(valuesParsed > 2){
          Serial.println("Too many parameters!");
          setValid = false;
          break;
        }
      }
    }
    if(setValid){
      if(valuesParsed < 2){
        Serial.println("Too few parameters!");
      }else{
        EEPROM.write(0, values[0]);
        EEPROM.write(1, values[1]);
        EEPROM.write(2, values[2]);
        Serial.println("New color stored!");
        if(ledState == 9)
          updateLED(8);
      }
    }
    incoming = "";
  }
}

void loop() {
  if(OSR(debounce(50))){
    ledState = updateLED(ledState);
  }
  handleSerial();
}

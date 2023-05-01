// Battery Charge / Discharge testing code - created 04/03/2023 by Brian Gardner

#include <LiquidCrystal.h>

int ps_relay = 2; //This is the pin that drives the relay for the charging power supply
int load_relay = 3; //This is the pin that drives the load for discharging the battery
int batteryPin = A0; //This identifies the pin used for reading battery voltage 
int state = 0; //This sets the state of the machine (three) 0 = charging the battery, 1 = wait and check voltage after charging, 2 = discharge testing, 3 = test complete
int batteryValue; //This is where we temporarily store the value from the A0 line that reads the battery voltage
float voltage;  //This is where we will temporarily store the battery voltage during each iteration of the test
float maths1;  //This is a floating point variable used for temprary math functions
float maths2;  //This is a floating point variable used for temprary math functions
float loadOhms = 2.6; //This is the resistance value of your load in ohms
float watts = 0; //This is where we will track how many watts have been dissapated by the load
int seconds = 0; //This is where we will track how long the test has been running
int testDuration = 0;

long prevMillis = 0;

bool earlyFailure = false;

char phases[4][10] = {"charging ","rest test","discharge","done     "};

LiquidCrystal LCD(7,8,9,10,11,12);

void setup() {
  pinMode(ps_relay, OUTPUT); //Tells the IDE that the ps_relay pin is an output
  pinMode(load_relay, OUTPUT); //Tells the IDE that the load_relay pin is an output
  digitalWrite(ps_relay, HIGH); //turn off relays (active low)
  digitalWrite(load_relay, HIGH);
  Serial.begin(9600);
  LCD.begin(16,2);
  LCD.clear();
  prevMillis = millis();
}

void loop() {
  if(millis() - prevMillis >= 1000){
    prevMillis = millis();
    
    Serial.print("Current State = ");
    Serial.println(state);
  
    LCD.setCursor(0,0);
    LCD.print(phases[state]);
    
    batteryValue = analogRead(batteryPin);  //Read the battery voltage
    voltage = batteryValue * 0.049;  //calculate battery voltage (0.0049 * 10)
  
    switch(state){
      case 0: // charge
        Serial.print("Charging - State = ");
        Serial.println(state);  //Lets see what state we're in
        Serial.println("Connecting the charger...");
    
        digitalWrite(ps_relay, LOW); //if state = 0 then we're chraging the battery from the PS
    
        Serial.print("Charging Voltage = ");
    
        Serial.println(voltage);  //Tell the world about that voltage

        LCD.setCursor(8,1);
        LCD.print("V:      ");
        LCD.setCursor(10,1);
        LCD.print(voltage);
    
        if (voltage >= 14.4) {  //If the battery voltage is greater than or equal to 14.4V then
    
          Serial.println("Disconnecting the charger");
          digitalWrite(load_relay, HIGH);  //Just making sure the load is disconnected
          digitalWrite(ps_relay, HIGH); //Disconnect the battery and then
          state = 1;  //Change the state to 'resting' and exit the loop
          seconds = -1;// reset time tracking for next phase (-1 because this increments at the end of the loop)
        }
      break;
      case 1: // rest test
        Serial.print("Resting the battery - State = ");
        Serial.println(state);  //Lets see what state we're in
        
        LCD.setCursor(8,1);
        LCD.print("V:      ");
        LCD.setCursor(10,1);
        LCD.print(voltage);
        
        if(seconds >= 30){
    
          Serial.print("Resting Voltage = ");
          Serial.println(voltage);  //Tell the world about that voltage
          if (voltage <= 12.5) {  //If the battery voltage is less than or equal to 12.5V then 
            Serial.println("Early Failure");  //Tell the world it failed and then
            earlyFailure = true;
            state = 3;
          } else {
            state = 2;  //The battery is full enough to run the discharge loop, set the state variable to 2, discharge testing
            seconds = -1;// reset time tracking for next phase (-1 because this increments at the end of the loop)
            Serial.println(state);  //Lets see what state we're in
          }
        }
      break;
      case 2: // discharge testing
        digitalWrite(ps_relay, HIGH);  //Make sure the charger is disconnected
        digitalWrite(load_relay, LOW);  //Turn on the load
    
        Serial.println("Load connected - Battery Discharging");
        
        Serial.print("Voltage = ");
        Serial.println(voltage);  //Tell the world about that voltage
        
        maths1 = voltage * voltage;  //Part of calculating watts is squaring the voltage
        maths2 = maths1 / loadOhms; //calulate how many watts we've dissapated if nothing changed for one hour
        maths1 = maths2 / 3600; //But, we're checking power dissipation every second, so divide that by 3600
        watts = watts + maths1; //We add the power dissipated in the last one second to whatever we had before, accumulating tracked dissipated power
        
        LCD.setCursor(0,1);
        LCD.print("Wh:     ");
        LCD.setCursor(3,1);
        LCD.print(watts);
        
        LCD.setCursor(8,1);
        LCD.print("V:      ");
        LCD.setCursor(10,1);
        LCD.print(voltage);
        
        if (voltage <= 12.5) {  //If the battery voltage is less than or equal to 12.5V then 
            
          state = 3;  //Test complete
          digitalWrite(ps_relay, HIGH);  //Make sure the charger is disconnected
          digitalWrite(load_relay, HIGH);  //Turn on the load
        
          Serial.println(state);  //Lets see what state we're in    Serial.println("TEST COMLPETE");
        
          watts = watts + watts;  // 12.5 volts is about %50 discharged so we'll double the watts before telling the user how many watts the battery capacity is

          testDuration = seconds; // store how long the test took
          
          Serial.print("Watts = ");
          Serial.println(watts);  //Display how many watts of capacity the battery has
          Serial.print("Seconds = ");
          Serial.println(seconds);  //Display how many seconds it took to get there
        } 
      break;
      case 3: // end of test, display results
        if(earlyFailure){
          LCD.setCursor(0,1);
          LCD.print("Charge not held!");
        }else{
          LCD.setCursor(6,0);
          LCD.print("Time:     ");
          LCD.setCursor(11,0);
          LCD.print(testDuration);
          LCD.print(" S");
          
          LCD.setCursor(0,1);
          LCD.print("Est. Wh:        ");
          LCD.setCursor(8,1);
          LCD.print(watts);
        }
        while(true); // we don't need to keep updating the LCD and state machine
      break;
    }
    seconds++; //Increment seconds by 1
  }
}

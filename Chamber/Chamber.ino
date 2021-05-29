/*
 * Intention is to cycle between the maximum and minimum temperatures
 * as quickly as possible
 */


#include <OneWire.h>
#include <DallasTemperature.h>

// Shut off the system if we get any values outside the expected normal startup range
#define TEMPERATURE_SHUTOFF_MIN_F 50
#define TEMPERATURE_SHUTOFF_MAX_F 170
// Thermal cycling range
#define TEMPERATURE_TARGET_MIN_F 130
#define TEMPERATURE_TARGET_MAX_F 160
// Due to residual heat from the immersion heater, this is about the
// amount of overshoot that is measured. So cut the heater off early
#define TEMPERATURE_OVERSHOOT_MAX_F 20
boolean temperatureGoingUp = true;

#define ULTRASONIC_ATOMIZER_ON_PERIOD_SECS ((unsigned long)30)
#define ULTRASONIC_ATOMIZER_OFF_PERIOD_SECS ((unsigned long)60)
unsigned long atomizerNextOnTimeMillis = 0;
unsigned long atomizerNextOffTimeMillis = (ULTRASONIC_ATOMIZER_ON_PERIOD_SECS * 1000);
boolean atomizerOn = true;

#define IMMERSION_HEATER_PIN 4
#define ULTRASONIC_ATOMIZER_PIN 3
// Pins 2 and 1 on controller are unused
#define EXTERNAL_FAN_PIN 7

// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 6
// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  
// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

void FanTurnOn(void) {
  // Assume off state
  // Press button twice
  Serial.println("Turning on fan!");
  for (int i = 0; i < 2; i++) {
    digitalWrite(EXTERNAL_FAN_PIN, LOW);
    delay(100);
    digitalWrite(EXTERNAL_FAN_PIN, HIGH);
    delay(300);
  }
}

void FanTurnOff(void) {
  // Assume on state
  // Press button once
  Serial.println("Turning off fan!");
  for (int i = 0; i < 1; i++) {
    digitalWrite(EXTERNAL_FAN_PIN, LOW);
    delay(100);
    digitalWrite(EXTERNAL_FAN_PIN, HIGH);
    delay(300);
  }
}

void setup(void)
{
  sensors.begin();  // Start up the temperature sensor library
  Serial.begin(9600);

  pinMode(IMMERSION_HEATER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ATOMIZER_PIN, OUTPUT);
  pinMode(EXTERNAL_FAN_PIN, OUTPUT);
}

float temperatureF = 0;
void loop(void)
{ 
  // Send the command to get temperatures
  sensors.requestTemperatures();
  temperatureF = (sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0;
  Serial.print(temperatureF);
  Serial.println("F");
  // If no temperature detected or it is out of range,
  // shut things down and stop execution
  if (temperatureF > TEMPERATURE_SHUTOFF_MAX_F || temperatureF < TEMPERATURE_SHUTOFF_MIN_F) {
    Serial.println("Erroneous values, shutting down");
    Serial.println("Immersion heater off!");
    digitalWrite(IMMERSION_HEATER_PIN, HIGH);
    while(1);
    
  }

  if (temperatureGoingUp == true && temperatureF >= (TEMPERATURE_TARGET_MAX_F-TEMPERATURE_OVERSHOOT_MAX_F)) {
    // Decrease temperature
    temperatureGoingUp = false;
    FanTurnOn();
    Serial.println("Immersion heater off!");
    digitalWrite(IMMERSION_HEATER_PIN, HIGH);
  } else if (temperatureGoingUp == false && temperatureF <= TEMPERATURE_TARGET_MIN_F) {
    // Increase temperature
    temperatureGoingUp = true;
    FanTurnOff();
    Serial.println("Immersion heater on!");
    digitalWrite(IMMERSION_HEATER_PIN, LOW);
    // Force atomizer on to keep the water flowing around
    Serial.println("Atomizer on!");
    digitalWrite(ULTRASONIC_ATOMIZER_PIN, LOW);
    
  }

  // Sporadically turn on atomizer
  // Don't turn off atomizer while heating up
  if (atomizerOn == true && millis() >= atomizerNextOffTimeMillis && temperatureGoingUp != true) {
    // Turn off atomizer
    atomizerOn = false;
    atomizerNextOnTimeMillis = atomizerNextOffTimeMillis + (ULTRASONIC_ATOMIZER_OFF_PERIOD_SECS * 1000);
    Serial.print("Atomizer off!");
    Serial.print(atomizerNextOffTimeMillis);
    Serial.print(",");
    Serial.println(atomizerNextOnTimeMillis);
    digitalWrite(ULTRASONIC_ATOMIZER_PIN, HIGH);
  } else if (atomizerOn == false && millis() >= atomizerNextOnTimeMillis) {
    // Turn on atomizer
    atomizerOn = true;
    atomizerNextOffTimeMillis = atomizerNextOnTimeMillis + (ULTRASONIC_ATOMIZER_ON_PERIOD_SECS * 1000);
    Serial.print("Atomizer on!");
    Serial.print(atomizerNextOffTimeMillis);
    Serial.print(",");
    Serial.println(atomizerNextOnTimeMillis);
    digitalWrite(ULTRASONIC_ATOMIZER_PIN, LOW);
  }
  
  delay(500);
}

#include <LiquidCrystal_I2C.h>
#include<Wire.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
int pumpDuration = 0;

byte water[8] ={B01110, B00100, B10100, B11110, B11111, B10001, B00000, B00001};
byte light[8] = {B01010, B01110, B11111, B01110, B01010, B00000, B00000, B00000};
byte celsius[8] = { B10000, B00111, B01000, B01000, B01000, B00111, B00000, B00000};

// pin setting

int pin_moisture_sensor = A0;
int pin_temperature_sensor = A1;  
int pin_light_sensor = A2; 



int pin_light_button = 9;
int pin_select_button = 10;
int pin_mode_button = 11;



int pin_light_bulb = 2;
int pin_water_motor = 4;

int pin_potentiometer = A4;





// setting value(for automatic mode)
int setting_temperature = 20;
int setting_moisture = 50;
int setting_light = 50;
int setting_watering_duration = 5;


// current value
float current_temperature;
float current_moisture;
float current_light;


//state value
boolean light_on= false;
int current_mode = 0;  //(0:automatic, 1: passive,  2: setting);
int value_selector = 0; //(0: light,    1: moisture, 2: pump duration)
int device_selector = 0; //(0: light, 1: pump)

//
int current_time;
int watering_start_time;
boolean motor_on = false;
boolean watering_enable = true;

void setup() {
  lcd.begin();

  lcd.createChar(1,water); //pump duration
  lcd.createChar(2, light);
  lcd.createChar(3, celsius);
  
  pinMode(pin_temperature_sensor, INPUT);
  pinMode(pin_moisture_sensor, INPUT);
  pinMode(pin_light_sensor, INPUT);
  
  pinMode(pin_light_button, INPUT_PULLUP);
  pinMode(pin_select_button, INPUT_PULLUP);
  pinMode(pin_mode_button, INPUT_PULLUP);
  

  pinMode(pin_potentiometer, INPUT);
  
  pinMode(pin_light_bulb, OUTPUT);
  pinMode(pin_water_motor, OUTPUT);
  turn_off_light();
  turn_off_motor();
  Serial.begin(9600);
}

//mode change
void modeCheck(){
  if(!digitalRead(pin_mode_button)){
    current_mode++;
  }
  if(current_mode > 2){
    current_mode = 0;
  }
}


// sensor value reading function
float readingTemperature(){

  int reading = analogRead(pin_temperature_sensor);
  float voltage = reading * 5.0 / 1023;
  float temperature = voltage * 100;
    Serial.println(temperature);  return temperature;
}

float readingMoisture(){
  int reading = analogRead(pin_moisture_sensor);
  float voltage = reading * 5.0 / 1023;
  float moisture = (5.0 - voltage) /5 * 100;
  return moisture;
}

float readingLight(){
  int reading = analogRead(pin_light_sensor);
  float voltage = 5- (reading * 5.0 / 1023);
  float light = voltage / 5 * 100;
  return light;
}

//check current value for automatic mode

boolean checkMoisture(float moisture){
  return (moisture <= setting_moisture);
}

boolean checkLight(float light){
  return (light <= setting_light);
}



// control function
void turn_on_motor(){
  digitalWrite(pin_water_motor, LOW);
  motor_on = true;
}

void turn_off_motor(){
  digitalWrite(pin_water_motor, HIGH);
  motor_on = false;
}

void turn_on_light(){
  digitalWrite(pin_light_bulb, LOW);
  light_on = true;
}

void turn_off_light(){
  digitalWrite(pin_light_bulb, HIGH);
  light_on = false;
}


void display_status(float temp, float humi, float light, int pump, int mode) {
  lcd.clear();
  if(mode == 0){
    lcd.setCursor(0, 0); lcd.print("  <Automatic Mode>");
  }
  else{
    lcd.setCursor(0, 0); lcd.print("   <Manual Mode>");
  }
  lcd.setCursor(1, 1); lcd.print("Temp:"); printDigits(temp); lcd.write(3); //add temp status
  lcd.setCursor(1, 2); lcd.print("Humi:"); printDigits(humi); lcd.print("%"); //add humidity status 
  lcd.setCursor(1, 3); lcd.print("Light:"); printDigits(light);  //add ON/OFF print light status
  if(mode == 1 && device_selector == 0){
    lcd.setCursor(0,3); lcd.print("!");
  }
  if(mode ==1 && device_selector == 1){
    lcd.setCursor(0,2); lcd.print("!");
  }
  if(light_on){
    displayLightOn();
  }
  if(watering_enable){
    displayWateringEnable();
  }
}
void display_setting(float humi, float light, int pump, int num) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("   <Setting Mode>");
  lcd.setCursor(1, 1); lcd.print("Humidity:"); printDigits(humi); lcd.print("%"); //add humidity status 
  lcd.setCursor(1, 2); lcd.print("   Light:"); printDigits(light);  //add ON/OFF print light status
  lcd.setCursor(1, 3); lcd.print("Duration:"); printDigits(pump); lcd.print("sec");      //pump/29 no!
  if(num == 1){
    lcd.setCursor(0,1); lcd.print("!");
  }
  else if(num == 2){
    lcd.setCursor(0,2); lcd.print("!");
  }
  else if(num == 3){
    lcd.setCursor(0,3); lcd.print("!");
  }
  else{}
}

void displayWateringEnable(){
  lcd.setCursor(19,1);
  lcd.write(1);
}
void displayLightOn(){
  lcd.setCursor(17,1);
  lcd.write(2);
}


void printDigits(float digits) {
  if (digits < 10)
  lcd.print('0');
  lcd.print(digits);
}

void wateringCheck(){
  if(!watering_enable){
    if(current_time - watering_start_time >= 10 * 1000 + setting_watering_duration * 1000){
      watering_enable = true; 
    }
  }
}
void wateringStop(){
  if(current_time - watering_start_time >= setting_watering_duration * 1000){
    turn_off_motor();
  }
}

void loop() {
  current_time = millis();
  current_temperature = readingTemperature();
  current_moisture = readingMoisture();
  current_light = readingLight();
  float temp = readingTemperature();
  Serial.print("Current temp: ");
  Serial.println(current_temperature);
  Serial.print("Current moisture: ");
  Serial.println(current_moisture);
  Serial.print("Current light: ");
  Serial.println(current_light);
  wateringCheck();
  if(motor_on){
    wateringStop();
  }
  modeCheck();
  //automatic mode
  if(current_mode == 0){
    value_selector = 0; // initialize setting choice;
    
    Serial.println("auto");
    display_status(current_temperature, current_moisture, current_light, setting_watering_duration, 0);
    if(checkMoisture(current_moisture)){
      if(watering_enable){
        turn_on_motor();
        watering_start_time = millis();
        watering_enable = false;
      }
    }
    //check light
    if(checkLight(current_light)){
      turn_on_light();
    }
    else{
      turn_off_light();
    }
  }


  // passive mode
  else if(current_mode == 1){
    Serial.println("passive");

    display_status(current_temperature, current_moisture, current_light, setting_watering_duration, 1);
    //light control

    if(!digitalRead(pin_select_button)){
      device_selector++;
      if(device_selector > 1){
        device_selector = 0;
      }
    }
    if(!digitalRead(pin_light_button)){
      if(device_selector == 0){
        Serial.println("Light button!!!");
        if(light_on) turn_off_light();    
        else turn_on_light();
      }
      else{
        Serial.println("motor button!!!");
        if(watering_enable){
          turn_on_motor();
          watering_start_time = millis();
          watering_enable = false;
        }
      }
     }
    }


  //setting mode
  else{

    Serial.println("setting");
    int reading = analogRead(pin_potentiometer);
    Serial.print("potentiometer: ");
    Serial.println(reading);
    int digit = map(reading, 0, 1023, 0, 100);
    if(!digitalRead(pin_select_button)){
      Serial.println("button!!!");
      value_selector++;
      if(value_selector > 3) value_selector = 0;
      delay(500);
    }
      Serial.print("selector: ");
      Serial.println(value_selector);
    if(value_selector == 0) {
      display_setting(setting_moisture, setting_light, setting_watering_duration, 0);
    }
    else if(value_selector == 1 ){   //moisture change
      setting_moisture = digit;
      display_setting(setting_moisture, setting_light, setting_watering_duration, 1);
    }
    else if(value_selector == 2){
      setting_light = digit;
      display_setting(setting_moisture, setting_light, setting_watering_duration, 2);
    }
    else{
      setting_watering_duration = digit/2.0;
      display_setting(setting_moisture, setting_light, setting_watering_duration, 3);
    }
   }
  delay(500);
}

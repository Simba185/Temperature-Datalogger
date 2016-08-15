/*********************

Temperature Datalogger by Tristan Redish

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

int set_delay = 25;
int sample_rate = 1;
int record_mode = 0;
int Setup = 25;

void setup() {
  // Debugging output
  Serial.begin(9600);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  // Print a message to the LCD. We track how long it takes since
  // this library has been optimized a bit and we're proud of it :)
    lcd.setBacklight(WHITE);
    lcd.print("Temp. DataLogger");
    lcd.setCursor(1, 1); 
    lcd.print("By: Tristan R.");
    delay (3000); 
    lcd.clear(); 
}

uint8_t i=0;
void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 0);
  
   if (Setup < 30 ) {
   Serial.println(" Temp. DataLogger By: Tristan R.");
   Serial.println(" Data is: Temperature D, DXX, Temperature E, EXX, Temperature F, FXX, Temperature G, GXX");
   Serial.print(" Sample Rate: ");
   Serial.print(sample_rate);
   Serial.println(" Sec ");
   Setup = Setup + 25;
   }
   
     
   float temperature_1 = 0.0;   // stores the calculated temperature 1
    int sample_1;                // counts through ADC samples 1
    float ten_samples_1 = 0.0;   // stores sum of 10 samples 1
    
    float temperature_2 = 0.0;   // stores the calculated temperature 2
    int sample_2;                // counts through ADC samples 2
    float ten_samples_2 = 0.0;   // stores sum of 10 sample 2
  
   float temperature_3 = 0.0;   // stores the calculated temperature 1
    int sample_3;                // counts through ADC samples 1
    float ten_samples_3 = 0.0;   // stores sum of 10 samples 1
  
  float temperature_4 = 0.0;   // stores the calculated temperature 1
    int sample_4;                // counts through ADC samples 1
    float ten_samples_4 = 0.0;   // stores sum of 10 samples 1  
   
    // take 10 samples from the MCP9700 0
    for (sample_1 = 0; sample_1 < 10; sample_1++) {
        // convert A0 value to temperature 1
        temperature_1 = ((float)analogRead(A0) * 5.0 / 1024.0) - 0.5;
        temperature_1 = temperature_1 / 0.01;
        // sample every 0.1 seconds
        delay(set_delay);
        // sum of all samples
        ten_samples_1 = ten_samples_1 + temperature_1;
    }
    
     for (sample_2 = 0; sample_2 < 10; sample_2++) {
        // convert A1 value to temperature 2
        temperature_2 = ((float)analogRead(A1) * 5.0 / 1024.0) - 0.5;
        temperature_2 = temperature_2 / 0.01;
        // sample every 0.1 seconds
        delay(set_delay);
        // sum of all samples
        ten_samples_2 = ten_samples_2 + temperature_2;
    }
    
    for (sample_3 = 0; sample_3 < 10; sample_3++) {
        // convert A2 value to temperature 3
        temperature_3 = ((float)analogRead(A2) * 5.0 / 1024.0) - 0.5;
        temperature_3 = temperature_3 / 0.01;
        // sample every 0.1 seconds
        delay(set_delay);
        // sum of all samples
        ten_samples_3 = ten_samples_3 + temperature_3;
    }
    
    for (sample_4 = 0; sample_4 < 10; sample_4++) {
        // convert A3 value to temperature 4
        temperature_4 = ((float)analogRead(A3) * 5.0 / 1024.0) - 0.5;
        temperature_4 = temperature_4 / 0.01;
        // sample every 0.1 seconds
        delay(set_delay);
        // sum of all samples
        ten_samples_4 = ten_samples_4 + temperature_4;
    }
    
    // get the average value of 10 temperatures 1
    temperature_1 = ten_samples_1 / 10.0;
    // get the average value of 10 temperatures 2
    temperature_2 = ten_samples_2 / 10.0;
    // get the average value of 10 temperatures 3
    temperature_3 = ten_samples_3 / 10.0;
    // get the average value of 10 temperatures 4
    temperature_4 = ten_samples_4 / 10.0;
    
     uint8_t buttons = lcd.readButtons();


    if (record_mode) {                     
    Serial.print(" Temperature D");    // send temperature 1 out of serial port
    Serial.print(", ");
    Serial.print(temperature_1);
    Serial.print(", ");
    Serial.print("Temperature E");  // send temperature 2 out of serial port
    Serial.print(", ");
    Serial.print(temperature_2);
    Serial.print(", ");
    Serial.print("Temperature F");  // send temperature 2 out of serial port
    Serial.print(", ");
    Serial.print(temperature_3);
    Serial.print(", ");
    Serial.print("Temperature G");  // send temperature 2 out of serial port
    Serial.print(", ");
    Serial.println(temperature_4);
    }
    
    lcd.print("D ");
    lcd.print(temperature_1,1);
    lcd.print("C");  

    lcd.setCursor(0,1);
    lcd.print("E ");
    lcd.print(temperature_2,1);
    lcd.print("C");
    
    lcd.setCursor(8,0);
    lcd.print("F ");
    lcd.print(temperature_3,1);
    lcd.print("C");
    
    lcd.setCursor(8,1);
    lcd.print("G ");
    lcd.print(temperature_4,1);
    lcd.print("C");
    
    
    ten_samples_1 = 0.0;
    ten_samples_2 = 0.0;
    ten_samples_3 = 0.0;
    ten_samples_4 = 0.0;
    
  if (buttons) {
     if (buttons & BUTTON_UP) {
     lcd.clear();
     set_delay = set_delay + 25;
     sample_rate = sample_rate + 1;     
     lcd.print("Sample Rate:");
     lcd.setCursor(0,1);
     lcd.print(sample_rate);
     lcd.print(" Sec ");
     Serial.print(" Sample Rate: ");
     Serial.print(sample_rate);
     Serial.println(" Sec ");
     delay (1500);
     lcd.clear();
    }
    if (buttons & BUTTON_DOWN && (set_delay > 1)) {
     lcd.clear();
     set_delay = set_delay - 25;
     sample_rate = sample_rate - 1;     
     lcd.print("Sample Rate:");
     lcd.setCursor(0,1);
     lcd.print(sample_rate);
     lcd.print(" Sec ");
     Serial.print(" Sample Rate: ");
     Serial.print(sample_rate);
     Serial.println(" Sec ");
     delay (1500);
     lcd.clear();
    }
    if (buttons & BUTTON_LEFT) {
     lcd.setBacklight(WHITE);
     lcd.clear();
     lcd.print("Idle Mode");
     Serial.println(" Idle Mode ");
     record_mode = 0;
     delay (1500);
     lcd.clear();
    }
    if (buttons & BUTTON_RIGHT) {
     lcd.setBacklight(WHITE);
     lcd.clear();
     lcd.print("Idle Mode");
     Serial.println(" Idle Mode ");
     record_mode = 0;
     delay (1500);
     lcd.clear();
    }
    if (buttons & BUTTON_SELECT) {
     lcd.setBacklight(RED);
     lcd.clear();
     lcd.print("Record Mode");
     Serial.println(" Record Mode ");
     record_mode = 1;
     delay (1500);
     lcd.clear();
    }  
     
  }   
}
   

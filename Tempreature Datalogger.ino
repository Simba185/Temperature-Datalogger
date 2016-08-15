//4CH Datalogger by Tristan Redish

#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7
// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
//#define LOG_INTERVAL 1000 // mills between entries (reduce to take more/faster data)
// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()
#define ECHO_TO_SERIAL 1 // echo data to serial port
#define WAIT_TO_START 0 // Wait for serial input in setup()
// the digital pins that connect to the LEDs
#define redLEDpin 2
#define greenLEDpin 3
// The analog pins that connect to the sensors
#define tempDPin 0 // analog 1
#define tempEPin 1 // analog 1
#define tempFPin 2 // analog 1
#define tempGPin 3 // analog 1
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

Sd2Card card;
SdVolume volume;
SdFile root;

int LOG_INTERVAL = 1000; // mills between entries 
int set_delay = 2;
float sample_rate = 1.0;
int record_mode = 0;

RTC_DS1307 RTC; // define the Real Time Clock object
// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;
// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);
  while(1);
}

void errorlcd(char *str)
{
  lcd.clear(); 
  lcd.setBacklight(RED);
  lcd.print("Please insert SD card");
  lcd.setCursor(0, 1);
  lcd.print(str);
  Serial.print(F("Error: please insert SD card"));
  digitalWrite(redLEDpin, HIGH);
  while(1);
}

void errorlcd2(char *str)
{
  lcd.clear(); 
  lcd.setBacklight(RED);
  lcd.print("Error:");
  lcd.setCursor(0, 1);
  lcd.print(str);
  while(1);
}

uint8_t i=0;

void setup(void)
{
  Serial.begin(9600);
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);

  // Print a message to the LCD. We track how long it takes since
  // this library has been optimized a bit and we're proud of it :)
  lcd.setBacklight(WHITE);
  lcd.print("Temp. DataLogger");
  lcd.setCursor(1, 1); 
  lcd.print("By: Tristan R.");
  delay (2000); 
  lcd.clear();

  // initialize the SD card
  Serial.print(F("Initializing SD card..."));
  lcd.print("Init. SD card?");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  // see if the card is present and can be initialized:
  delay(500);
  if (!SD.begin(chipSelect)) {
    errorlcd("card and reboot."); // error line if no SD card or failed init.
  }
  //if (!SD.begin(chipSelect)) {
  //lcd.print("SD card failed!"); // error line if no SD card or failed init.
  //}
  Serial.print(F("card initialized."));
  lcd.setCursor(0, 1);
  lcd.print("SD Card Init!");
  delay (2000);
  lcd.clear();
  while (!card.init(SPI_HALF_SPEED, chipSelect)) {
  } 
  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    return;
  }
  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  lcd.clear();
  lcd.print("Volume is:FAT");
  lcd.print(volume.fatType(), DEC);
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  Serial.print("Volume size (Mbytes):");
  volumesize /= 1048576;
  Serial.println(volumesize);
  Serial.println();
  lcd.setCursor(0, 1);
  lcd.print("Size (MB):");
  lcd.print(volumesize);
  delay (2000);
  lcd.clear();

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break; // leave the loop!
    }
  }
  if (! logfile) {
    errorlcd2("Cant create file");
  }
  Serial.print("Logging to: ");
  Serial.println(filename);
  lcd.print("Logging to:");
  lcd.setCursor(0, 1);
  lcd.print(filename);
  delay (2000);
  lcd.clear();
  //lcd.print("Format:");
  //lcd.setCursor(0, 1);
  //lcd.print("st,datetime,temp");

  // connect to RTC
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
    lcd.clear();
    lcd.print("RTC failed");
    delay (1000);
    lcd.clear();
#endif //ECHO_TO_SERIAL
  }
  logfile.println(F("stamp,datetime,tempD C,tempE C,tempF C,tempG C,sample rate(Sec)"));

#if ECHO_TO_SERIAL
  Serial.println(F("stamp,datetime,tempD C,tempE C,tempF C,tempG C,sample rate(Sec)"));
#endif //ECHO_TO_SERIAL
}

void loop(void)
{
  DateTime now;
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  digitalWrite(greenLEDpin, HIGH);
  // log milliseconds since starting
  uint32_t m = millis();

  uint8_t buttons = lcd.readButtons();
  

  // fetch the time
  now = RTC.now();
  if (record_mode){
    // log time
    logfile.print(now.unixtime()); // seconds since 1/1/1970
    logfile.print(", ");
    logfile.print('"');
    logfile.print(now.year(), DEC);
    logfile.print("/");
    logfile.print(now.month(), DEC);
    logfile.print("/");
    logfile.print(now.day(), DEC);
    logfile.print(" ");
    logfile.print(now.hour(), DEC);
    logfile.print(":");
    logfile.print(now.minute(), DEC);
    logfile.print(":");
    logfile.print(now.second(), DEC);
    logfile.print('"');
  }
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 1/1/1970
  Serial.print(", ");
  Serial.print('"');
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print('"');
#endif //ECHO_TO_SERIAL

  analogRead(tempDPin);
  delay(set_delay);
  int tempReadingD = analogRead(tempDPin);
  // converting that reading to voltage, for 3.3v arduino use 3.3, for 5.0, use 5.0
  float voltageD = tempReadingD * 5.00 / 1024;
  float temperatureDC = (voltageD - 0.5) * 100 ;
  float temperatureDF = (temperatureDC * 9 / 5) + 32;

  analogRead(tempEPin);
  delay(set_delay);
  int tempReadingE = analogRead(tempEPin);
  // converting that reading to voltage, for 3.3v arduino use 3.3, for 5.0, use 5.0
  float voltageE = tempReadingE * 5.00 / 1024;
  float temperatureEC = (voltageE - 0.5) * 100 ;
  float temperatureEF = (temperatureEC * 9 / 5) + 32;

  analogRead(tempFPin);
  delay(set_delay);
  int tempReadingF = analogRead(tempFPin);
  // converting that reading to voltage, for 3.3v arduino use 3.3, for 5.0, use 5.0
  float voltageF = tempReadingF * 5.00 / 1024;
  float temperatureFC = (voltageF - 0.5) * 100 ;
  float temperatureFF = (temperatureFC * 9 / 5) + 32;

  analogRead(tempGPin);
  delay(set_delay);
  int tempReadingG = analogRead(tempGPin);
  // converting that reading to voltage, for 3.3v arduino use 3.3, for 5.0, use 5.0
  float voltageG = tempReadingG * 5.00 / 1024;
  float temperatureGC = (voltageG - 0.5) * 100 ;
  float temperatureGF = (temperatureGC * 9 / 5) + 32;

  lcd.setCursor(0, 0);
  lcd.print("D ");
  lcd.print(temperatureDC,1);
  lcd.print("C");

  lcd.setCursor(0,1);
  lcd.print("E ");
  lcd.print(temperatureEC,1);
  lcd.print("C");

  lcd.setCursor(8,0);
  lcd.print("F ");
  lcd.print(temperatureFC,1);
  lcd.print("C");

  lcd.setCursor(8,1);
  lcd.print("G ");
  lcd.print(temperatureGC,1);
  lcd.print("C");

  if (record_mode){
    logfile.print(", ");
    logfile.print(temperatureDC);
    logfile.print(", ");
    logfile.print(temperatureEC);
    logfile.print(", ");
    logfile.print(temperatureFC);
    logfile.print(", ");
    logfile.print(temperatureGC);
    logfile.print(", ");
    logfile.print(sample_rate);
  }

#if ECHO_TO_SERIAL
  Serial.print(", ");
  Serial.print(temperatureDC);
  Serial.print(", ");
  Serial.print(temperatureEC);
  Serial.print(", ");
  Serial.print(temperatureFC);
  Serial.print(", ");
  Serial.print(temperatureGC);
  Serial.print(", ");
  Serial.print(sample_rate);
#endif //ECHO_TO_SERIAL
  if (record_mode){
    logfile.println();
  }
#if ECHO_TO_SERIAL
  Serial.println();
#endif // ECHO_TO_SERIAL
  digitalWrite(greenLEDpin, LOW);
  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);

  if (buttons) {
    if (buttons & BUTTON_UP) {
      lcd.clear();
      LOG_INTERVAL = LOG_INTERVAL + 500;
      sample_rate = sample_rate + 0.5;     
      lcd.print("Sample Rate:");
      lcd.setCursor(0,1);
      lcd.print(sample_rate,1);
      lcd.print(" Sec ");
      Serial.print(" Sample Rate: ");
      Serial.print(LOG_INTERVAL);
      Serial.println(" ms ");
      delay (1500);
      lcd.clear();
    }
    if (buttons & BUTTON_DOWN && (LOG_INTERVAL > 500)) {
      lcd.clear();
      LOG_INTERVAL = LOG_INTERVAL - 500;
      sample_rate = sample_rate - 0.5;     
      lcd.print("Sample Rate:");
      lcd.setCursor(0,1);
      lcd.print(sample_rate,1);
      lcd.print(" Sec ");
      Serial.print(" Sample Rate: ");
      Serial.print(LOG_INTERVAL);
      Serial.println(" ms ");
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



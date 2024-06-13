#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <espnow.h>
#include <macAddress.h>

//Firmware Version
const String version = "0.0.1";

//Initialize LCD Display
LiquidCrystal_I2C lcd(0x27,20,4);
const int screenOnDuration = 300000; //5 Minutes
unsigned long screenSaverRunTime = 0;
char keyConcat[32];

float KeyTimer = 0; //Timer for avoiding multiple keypresses at a time
bool keyPressed = false; //status which keeps track if a key was pressed

void lcdDisplay(String, String, String, String);
void ScreenSaver(void);
void KeyPad(void);
void ManageData(char);
void Append(char* s, char c);


uint8_t broadcastAddress[] = REMOTEMAC;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  String a;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    lcdDisplay("Current Time: ", "<time>", "Sent Data Status: ", "Delivery success");
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
    lcdDisplay("Current Time: ", "<time>", "Sent Data Status:" , "Delivery FAIL");
  }
}
 



void setup()
{
  Wire.begin(D1,D2);
  lcd.init();

  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP8266 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  
  Serial.write("Connected");
  lcdDisplay(
    "Bingo Display Board",
    "I am the 'Send Unit'",
    "Firmware " + version,
    "By: Eko Illius"
  );
 delay(2000);

 WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
       return;
  }
 
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}



void lcdDisplay(String line1 = "", String line2 = "", String line3 = "", String line4= "" )
{
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  lcd.setCursor(0, 2);
  lcd.print(line3);
  lcd.setCursor(0, 3);
  lcd.print(line4);
  screenSaverRunTime = millis();
}

void ScreenSaver(void)
{
  if((millis() - screenSaverRunTime) > screenOnDuration){
    screenSaverRunTime = 0;
    lcd.clear();
    lcd.noBacklight();
  }
}

void SendData(String data){
   myData.a = data;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}

void loop() {
    KeyPad();
   ScreenSaver();
 }

 void KeyPad()
{
  if (keyPressed == true)
  {
    if (millis() - KeyTimer > 350)
    {
      KeyTimer = millis();
      keyPressed = false;
    }
    else
    {
      //do nothing
    }
  }
  else
  {
    Wire.beginTransmission(0x20);
      Wire.write(B11101111);
    Wire.endTransmission();
    
    Wire.requestFrom(0x20, 1);
    switch (Wire.read())
    {
      case 238: //Key D
      ManageData('D');
      keyPressed = true;
      break;
  
      case 237: //Key #
      ManageData('#');
      keyPressed = true;
      break;
     
      case 235: //Key 0
      ManageData('0');
      keyPressed = true;
      break;
     
      case 231: //Key *
      ManageData('*');
      keyPressed = true;
      break;
    }
    
    
    Wire.beginTransmission(0x20);
      Wire.write(B11011111);
    Wire.endTransmission();

    Wire.requestFrom(0x20, 1);
    switch (Wire.read())
    {
      case 222: //Key C
        ManageData('C');
        keyPressed = true;
      break;

      case 221: //Key 9
        ManageData('9');
        keyPressed = true;
      break;

      case 219: //Key 8
        ManageData('8');
        keyPressed = true;
      break;

      case 215: //Key 7
        ManageData('7');
        keyPressed = true;
      break;
    }
    
    Wire.beginTransmission(0x20);
      Wire.write(B10111111);
    Wire.endTransmission();

    Wire.requestFrom(0x20, 1);
    switch (Wire.read())
    {
      case 190: //Key B
        ManageData('B');
        keyPressed = true;
      break;

      case 189: //Key 6
        ManageData('6');
        keyPressed = true;
      break;

      case 187: //Key 5
        ManageData('5');
        keyPressed = true;
      break;

      case 183: //Key 4
        ManageData('4');
        keyPressed = true;
      break;
    }
    
    Wire.beginTransmission(0x20);
    Wire.write(B01111111);
    Wire.endTransmission();

    Wire.requestFrom(0x20, 1);
    switch (Wire.read())
    {
      case 126: //Key A
        ManageData('A');
        keyPressed = true;
      break;

      case 125: //Key 3
      ManageData('3');
      keyPressed = true;
      break;

      case 123: //Key 2
        ManageData('2');
        keyPressed = true;
      break;

      case 119: //Key 1
        ManageData('1');
        keyPressed = true;
      break;
    }
    KeyTimer = millis();
  }
}

void ManageData(char key){

 switch (key)
 {
   case '*' :
    memset(keyConcat, 0, 32);
    Serial.println("Cleared");
    lcdDisplay("Current Time: ", "<time>", "Data Cleared: ", keyConcat);
  break;

  case '#' :
    Serial.print("Sending: ");
    Serial.println(keyConcat);
    SendData(keyConcat);
    memset(keyConcat, 0, 32);
    lcdDisplay("Current Time: ", "<time>", "Sent Data: ", keyConcat);
  break;

 default:
    Serial.write("Key: " + key);
    char tmpKey[3] = {key};
    strcat(keyConcat, tmpKey );
    Serial.println(keyConcat);
    lcdDisplay("Current Time: ", "<time>", "Data To Send: ", keyConcat);
  break;
 }
}
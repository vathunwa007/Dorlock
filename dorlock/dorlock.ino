#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <Keypad.h>


Servo servo1;
BlynkTimer timer;

#include <DHT.h>
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#include <SimpleTimer.h>
#define SET_PIN 0


char mqtt_server[40]= "bstick-board.com";
char mqtt_port[6] = "9443";
char blynk_token[34] = "0c0f5095af7c4e5fa789c5a2c1664a86";

//ประกาศตัวแปร
//----------------------------------

//---------------------------------
const int Relay0 = D0;
const int Relay1 = D1;

int blynkIsDownCount=0;

// Set web server port number to 80
WiFiServer server(80);
// Variable to store the HTTP request
//---------------------------------------------------------------------------keypad setting-------------
#define Password_Lenght 4 // Give enough room for six chars + NULL char

int pos = 0;    // variable to store the servo position

char Data[Password_Lenght]; // 6 is the number of chars it can hold + the null char = 7
char Master[Password_Lenght] = "007";
byte data_count = 0, master_count = 0;
bool Pass_is_good;
char customKey;
//-------------------------------------------keypad setting-----------------------------------------
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
bool door = true;

byte rowPins[ROWS] = {D2, D3, D4, D5}; //เชื่อมต่อกับ pinouts แถวของปุ่มกด
byte colPins[COLS] = {D6, D7, D8}; //เชื่อมต่อกับ pinouts คอลัมน์ของปุ่มกด

Keypad customKeypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); //เริ่มต้นอินสแตนซ์ของคลาส NewKeypad
String header;
//----------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
pinMode(SET_PIN, INPUT_PULLUP);
//-----------------------------------------------ไวไฟเมเนเจอร์ตั้งค่า---------------------------------------
      WiFiManagerParameter custom_blynk_token("Blynk", "Blynk", blynk_token, 32);
  WiFiManagerParameter custom_mqtt_server("mqtt_server", "mqtt_server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("mqtt_port", "mqtt_port", mqtt_port,6);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.addParameter(&custom_blynk_token);
wifiManager.addParameter(&custom_mqtt_server);
wifiManager.addParameter(&custom_mqtt_port);
 
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AUNWAIOT");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
 Serial.println("Connected.....🙂");
   Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();

  Blynk.config(custom_blynk_token.getValue());
  Blynk.config(custom_mqtt_server.getValue());
  Blynk.config(custom_mqtt_port.getValue());

 if (digitalRead(SET_PIN) == LOW) {
    wifiManager.resetSettings();
  }
//----------------------------------------------------------------------------------------------------------- 
  pinMode(Relay0,OUTPUT); // NODEMCU PIN D0
  pinMode(A0, INPUT);
  
  
Blynk.config(blynk_token, "bstick-board.com", 8080);
// Set Defult Relay Status
  digitalWrite(Relay0, LOW);
  
  
  servo1.attach(D1); //servopin D1
  servo1.write(0);
  delay(500);

  ServoOpen();
   Serial.println(" Arduino Door");

   Serial.println("--Look project--");
  delay(3000);
 Serial.println(" Enter Password");
}

void loop()
{ 
 
   //ส่วนการทำงานของแอพบิ้งค์
  Blynk.run();
  timer.run(); // run BlynkTimer
  //-------------------------------------------------------------------
  if (door == 0)
  {
    customKey = customKeypad.getKey();
   
    if (customKey == '*')

    {
     
      ServoClose();
      Serial.println("  Door is close");
      delay(3000);
      door = 1;
    }
  }

  else Open();
  
  
}
void clearData()
{
  while (data_count != 0)
  { // This can be used for any array size,
    Data[data_count--] = 0; //clear array for new data
  }
  return;
}

void ServoOpen()
{
  for (pos = 180; pos >= 0; pos -= 5) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo1.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);              // waits 15ms for the servo to reach the position
    digitalWrite(D0,LOW);   
  }
}

void ServoClose()
{
  for (pos = 0; pos <= 180; pos += 5) { // goes from 180 degrees to 0 degrees
    servo1.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);
    digitalWrite(D0,HIGH);// waits 15ms for the servo to reach the position
  }
}

void Open()
{
  
 
  
  customKey = customKeypad.getKey();
  if (customKey) // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
  {
    Data[data_count] = customKey; // store char into data array
    
    Serial.println(Data[data_count]); // print char at said cursor
    data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
  }

  if (data_count == Password_Lenght - 1) // if the array index is equal to the number of expected chars, compare data to master
  {
    if (!strcmp(Data, Master)) // equal to (strcmp(Data, Master) == 0)
    {
     
      ServoOpen();
      Serial.println("  Door is Open");
      door = 0;
    }
    else
    {
     
      Serial.println("  Wrong Password");
      delay(1000);
      door = 1;
      digitalWrite(D0,LOW);
      delay(500);
      digitalWrite(D0,HIGH);
      delay(500);
      digitalWrite(D0,LOW);
      delay(500);
      digitalWrite(D0,HIGH);
      delay(500);
    }
    clearData();
  }
}
void reconnecting()
{
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
    Blynk.syncAll();
  }

}
BLYNK_WRITE(V1){
  ServoOpen();
  door = 0;
  }
BLYNK_WRITE(V2){
  ServoClose();
  door = 1;
}

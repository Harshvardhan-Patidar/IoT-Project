#include <Wire.h>
#include <ThingSpeak.h>
#include <WiFi.h>
#include <HTTPClient.h>


const char* ssid = "HARSH";   // your network SSID (name) 
const char* password = "harsh@1234";   // your network password

const String accountSID = "ACa3110760528872b0d38890c5355f149a"; // Your Twilio Account SID
const String authToken = "4e58d91615e3544ab3be39fcbadcb505"; // Your Twilio Auth Token
const String fromNumber = "+14404901486"; // Your Twilio phone number
const String toNumber = "+919893538777"; // The recipient's phone number

WiFiClient  client;

unsigned long myChannelNumber = 2740653;
const char * myWriteAPIKey = "VQ361GGV40A0TZY5";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;


const int s0 = 5;
const int s1 = 19;
const int s2 = 4;
const int s3 = 2;
const int out = 18;

int red = 0;
int green = 0;
int blue = 0;

const int trigPin = 21;  
const int echoPin = 15; 
long duration;          
float distance;        

int previousFuelLevel = 0;        
int fuelLevelThreshold = 35;     
float ultrasonicThreshold = 2.5;     
int currentFuelLevel = 0;        

unsigned long previousFuelLevelTime = 0;
unsigned long previousUltrasonicTime = 0;
const unsigned long fuelLevelInterval = 5000;   
const unsigned long ultrasonicInterval = 500;  

void setup() {
  Serial.begin(115200);
   WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client); 
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  digitalWrite(s0, HIGH);
  digitalWrite(s1, HIGH);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // Serial.begin(9600);
    previousFuelLevel = analogRead(34); 

}

void loop() {
   if ((millis() - lastTime) > timerDelay) {
    
    // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(1000);     
      } 
      Serial.println("\nConnected.");
    }
int y=red+green+blue;
    
    int x = ThingSpeak.writeField(myChannelNumber, 1, currentFuelLevel, myWriteAPIKey);
   
    int x2 = ThingSpeak.writeField(myChannelNumber, 2, y, myWriteAPIKey);
   

  color();     
  Serial.print("R =");
  Serial.print(red, DEC);
  Serial.print(" G = ");
  Serial.print(green, DEC);
  Serial.print(" B = ");
  Serial.print(blue, DEC);
  Serial.print("\t");

 petroldetect();  

  Serial.println();

  levelDetect();  

  // delay(3000);
   lastTime = millis();
  }
}




void color() {
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s3, HIGH);
  blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s2, HIGH);
  green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
}

void petroldetect(){
if (red < 50 && green < 50 && blue < 50) {
    Serial.println("Clear/Pure Petrol");
} 
else if (red > green && red > blue && red < 100 && green < 50 && blue < 50) {
    Serial.println("Yellow Petrol"); 
    sendSMS("Impure Petrol"); 
} 
else if (blue > green && blue > red && blue > 50 && blue < 150) {
    Serial.println("Pale Blue Petrol");
    sendSMS("Impure Petrol");  
} 
else if (green > red && green > blue && green < 100 && red < 50 && blue < 50) {
    Serial.println("Greenish Petrol"); 
    sendSMS("Impure Petrol"); 
} 
else if (red > green && red > blue && red > 100 && green < 100 && blue < 100) {
    sendSMS("Impure Petrol");  
}

else if (red < 40 && green < 40 && blue < 40) {
    Serial.println("Possible Kerosene Mixture");
    sendSMS("Impure Petrol");   
} 
else if ((red < 80 && green < 80 && blue < 80) && ((red + green + blue) < 200)) {
    Serial.println("Kerosene Mixture Detected");
    sendSMS("Impure Petrol");   
} 

else if ((red > green && red > blue) && red < 80 && green < 50 && blue < 50) {
    Serial.println("Ethanol Mixture Detected - Light Yellow");
    sendSMS("Impure Petrol");   
} 
else if ((red < 60 && green < 60 && blue < 60) && ((red + green + blue) > 50 && (red + green + blue) < 180)) {
    Serial.println("Ethanol Mixture Detected - Color Dilution");
    sendSMS("Impure Petrol");   
} 
else {
    Serial.println("Unknown or High Ethanol Presence");
    sendSMS("Impure Petrol");   
}
}

void levelDetect(){
      unsigned long currentTime = millis();

    if (currentTime - previousUltrasonicTime >= ultrasonicInterval) {
        previousUltrasonicTime = currentTime;
        
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        duration = pulseIn(echoPin, HIGH);
        distance = duration * 0.0344 / 2; 

        Serial.print("Ultrasonic Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
    }

    if (currentTime - previousFuelLevelTime >= fuelLevelInterval) {
        previousFuelLevelTime = currentTime;

        currentFuelLevel = analogRead(34);

        Serial.print("Fuel Level Sensor Value: ");
        Serial.println(currentFuelLevel);

        if (distance > ultrasonicThreshold) {
            Serial.println("ALERT: Possible Fuel Theft or Leak Detected!");
            sendSMS("Theft Detected");
        }

        previousFuelLevel = currentFuelLevel;
}
}

void sendSMS(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.twilio.com/2010-04-01/Accounts/" + accountSID + "/Messages.json";
    
    http.begin(url);
    http.setAuthorization(accountSID.c_str(), authToken.c_str());
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String payload = "From=" + fromNumber + "&To=" + toNumber + "&Body=" + message;
    
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
}
}
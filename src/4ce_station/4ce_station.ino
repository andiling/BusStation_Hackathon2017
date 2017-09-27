#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
 
const char* ssid = "CityHack2017";
const char* password = "Ci5yh@6K2017";

//http://3b7d1823.ngrok.io/api/stats/station/59c53bc37e0003ed59f86dd3/lcd
//const char *  host      = "3b7d1823.ngrok.io";
const char *  host      = "4ceteam.tremend.ro";
const char * stationID  = "59c53bc37e0003ed59f86dd3";
String  url             = (String)"/api/stats/station/" + stationID +"/lcd";
//String  url             = (String)"/api/stats";
const int httpPort      = 80;
const unsigned long HTTP_TIMEOUT = 1000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response
int gDummySeconds = 10*60;  //10 minutes
WiFiClient  client;

// local server ??
WiFiServer server(80);

int ledPin = LED_BUILTIN; // GPIO13

LiquidCrystal_I2C lcd(0x3F, 16, 3);

// The type of data that we want to extract from the page
struct UsefulData {
  char bus[32];
  char eta_seconds[32];
};

UsefulData dataArray[10];

void print_lcd(char *line0, char * line1, char * line2, char * line3)
{
  lcd.clear();
  // 1st line
  lcd.setCursor(0,0);
  lcd.print(line0);

  // 2nd Line
  lcd.setCursor(0,1);
  lcd.print(line1);

  // 3rd Line
  lcd.setCursor(0,2);
  lcd.print(line2);

  // 4th Line
  lcd.setCursor(0,3);    
  lcd.print(line3);  
}


void print_station2(char *station)
{
  lcd.clear();
  // 1st line
  lcd.setCursor(2,0);
  lcd.print(station);

  for (int i = 0 ; i < 3; i++)
  {

    //dummy station -> update
    if ( i == 2 ) 
    {
      strcpy(dataArray[i].bus, "111");
      itoa(gDummySeconds,dataArray[i].eta_seconds, 10);
      //sprintf(dataArray[index].eta_seconds,"%d", gDummySeconds);
    }
    
    if ( strcmp(dataArray[i].bus, "") )
    {
      lcd.setCursor(2,i+1);
      lcd.print(" ");
        for ( int j = strlen(dataArray[i].bus); j < 3; j ++) lcd.print(" ");
        
      lcd.print(dataArray[i].bus);
      lcd.print("  in ");
      
      
      int min = atoi(dataArray[i].eta_seconds) / 60;

      if (min == 0) 
      {
        lcd.print("station"); 
      }
      else
      {
        if (min < 10) lcd.print(" ");        
        lcd.print( min);
        lcd.print(" min");      
      }            
    }
    else
    {
      lcd.setCursor(2,i+1);
      lcd.print("   ");
    }
  }
/*
  // 2nd Line
  if ( strcmp(dataArray[0].bus, "") )
  {
    lcd.setCursor(2,1);
    lcd.print("[");
    lcd.print(dataArray[0].bus);
    lcd.print("] in ");
    lcd.print(dataArray[0].eta_seconds);
    lcd.print("s");    
  }


  // 3rd Line
  if ( strcmp(dataArray[1].bus, "") )
  {
    lcd.setCursor(2,2);
    lcd.print("[");
    lcd.print(dataArray[1].bus);
    lcd.print("] in ");
    lcd.print(dataArray[1].eta_seconds);
    lcd.print("s");    
  }
  
  // 4th Line
  if ( strcmp(dataArray[2].bus, "") )
  {
    lcd.setCursor(2,3);    
    lcd.print("[");
    lcd.print(dataArray[2].bus);
    lcd.print("] in ");
    lcd.print(dataArray[2].eta_seconds); 
    lcd.print("s");
  }
*/
}


// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  //Serial.println("skipResponseHeaders");
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    Serial.println("No response or invalid response!");
  }

  return ok;
}

bool just_parse(String line, int index)
{
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // See https://bblanchon.github.io/ArduinoJson/assistant/
  const size_t BUFFER_SIZE =
      JSON_OBJECT_SIZE(8)    // the root object has 8 elements
      + JSON_OBJECT_SIZE(2)  // the "bus" object has 2 elements
      + JSON_OBJECT_SIZE(5)  // the "station" object has 5 elements      
      + MAX_CONTENT_SIZE;    // additional space for strings


  line = "{" + line + "}";
  //Serial.println(" ---- " + line + " ----");
  

  // Allocate a temporary memory pool
  DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);
  JsonObject& root = jsonBuffer.parseObject(line);
  
  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    return false;
  }

  // Here were copy the strings we're interested in  
  strcpy(dataArray[index].bus, root["bus"]);
  strcpy(dataArray[index].eta_seconds, root["eta_seconds"]);

  //printUserData(&uData);
  
  // It's not mandatory to make a copy, you could just use the pointers
  // Since, they are pointing inside the "content" buffer, so you need to make
  // sure it's still in memory when you read the 
  return true;
}
 


/*
[{"_id":"59c579467e0003ed59f888b0",
  "bus":{
      "_id":"59c50a4a50d69044c5b8ceee",
      "line":"133"
  },
  "station":{
      "_id":"59c53bc37e0003ed59f86dd3",
      "long":26.11446,
      "lat":44.41201,
      "route_point_id":23,
      "name":"PIATA TIMPURI NOI"
   },
  "eta_seconds":356,
  "eta_meters":488,
  "updatedAt":"2017-09-22T20:58:12.332Z",
  "__v":0,
  "createdAt":"2017-09-22T20:57:42.811Z"
  } 
  ]*/

bool readReponseContent() {  
  // cleanup
  memset(dataArray[0].bus,0, sizeof(dataArray[2].bus));
  memset(dataArray[1].bus,0, sizeof(dataArray[2].bus));
  memset(dataArray[2].bus,0, sizeof(dataArray[2].bus));

  memset(dataArray[0].eta_seconds,0, sizeof(dataArray[2].eta_seconds));
  memset(dataArray[1].eta_seconds,0, sizeof(dataArray[2].eta_seconds));
  memset(dataArray[2].eta_seconds,0, sizeof(dataArray[2].eta_seconds));
  

  String line = client.readStringUntil('\r');
  Serial.println(line.length()); 
  line.remove(0,1);
  line.remove(line.length()-1,1);
  
  //line = "{{\"bus\":\"133\",\"eta_seconds\":356},{\"bus\":\"124\",\"eta_seconds\":222},{\"bus\":\"22\",\"eta_seconds\":333}}";  
    Serial.println(line);  

  char char_line[200] ;
  line.toCharArray(char_line, sizeof(char_line) ); //line.length());

  Serial.println(char_line);

  int idx = 0;
  char *tok = strtok( char_line, "}{");
  while (tok) {      
      if ( strlen(tok) > 1 ) {
        //Serial.println(tok);  
        just_parse(tok, idx++);
      }
      tok = strtok(NULL, "}{"); // Note: NULL, not IncomingString
  }


  return true;
}
 

bool get_json()
{
  // Test JSON
  // http://3b7d1823.ngrok.io/api/buses
   client.print("GET " + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +                  
                   "Content-Type: application/json\r\n" + 
                   "Connection: close\r\n\r\n");
      delay(500);

  return true;
}
 
void setup() {
  Serial.begin(115200);
  delay(10);
 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.print("initialize the LCD\n");
  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  //lcd.print("444444CE Team rulz!");
  
  //// test print
  print_lcd("    - 4CE Team -","  ","    CityHack2017"," TIMPURI NOI Square");

  
  //EEPROM.begin(512);
  EEPROM.begin(1024);

  //Serial.print("EEPROM length ");
  //uint16_t ll = EEPROM.length;
  //Serial.println(EEPROM.length);

 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);  
 
  WiFi.begin(ssid, password);

  int toSaveToEEPROM = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    toSaveToEEPROM ++;
  }

  // read last value  
  byte high = EEPROM.read(0); //read the first half
  byte low = EEPROM.read(1); //read the second half
  int last = (high << 8) + low;  
  Serial.print("LAST value= ");
  Serial.println(last);


  // save to EEPROM
  EEPROM.write(0, highByte(toSaveToEEPROM)); //write the first half
  EEPROM.write(1, lowByte(toSaveToEEPROM)); //write the second half

  EEPROM.commit();
  Serial.print("NEW value= ");
  Serial.println(toSaveToEEPROM);


   //WIFI
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

 
 // Use WiFiClient class to create TCP connections         
/*  if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
  }*/ 
}

// Open connection to the HTTP server
bool connect() {
  Serial.print("Connect to ");
  Serial.println(host);

  bool ok = client.connect(host, httpPort);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
}

// Pause for a 1 minute
void wait() {
  Serial.println("Wait 10 seconds");
  delay(10000);
}
 
void loop() {
  if (connect()) {
    if (get_json() && skipResponseHeaders()) {
      UsefulData uData;
      if (readReponseContent()) {
        //printUserData();
        print_station2(" TIMPURI NOI ");
      }
    }
  }
  disconnect();
  wait();


 
  get_json();
  gDummySeconds -= 60;
  if (gDummySeconds <0) gDummySeconds = 10*60;
  delay(10000);

  digitalWrite(ledPin, !digitalRead(ledPin) );   
  
 
}
 

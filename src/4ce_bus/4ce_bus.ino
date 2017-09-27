#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <string.h>

static const u1_t PROGMEM APPEUI[8]={ 0x55, 0x6F, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
static const u1_t PROGMEM DEVEUI[8]={ 0x16, 0x0F, 0x54, 0x19, 0x69, 0x3A, 0x75, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
static const u1_t PROGMEM APPKEY[16] = { 0x46, 0xE5, 0x4C, 0x1A, 0x9B, 0xDD, 0x36, 0x96, 0xD1, 0x28, 0xC7, 0x8D, 0x25, 0x07, 0xBC, 0x62 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static uint8_t mydata[] = “Hello, world!“;
static osjob_t sendjob;
uint8_t bresult[10];

const unsigned TX_INTERVAL = 15;

TinyGPS gps;
SoftwareSerial ss(5, 6);

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN,
  //.rst = 9,
  .rst = LMIC_UNUSED_PIN,
  .dio = {8, 7, LMIC_UNUSED_PIN},
};

typedef union
{
 float number;
 uint32_t w;
 uint8_t bytes[4];
} FLOATUNION_t;


void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(“: “);
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F(“EV_SCAN_TIMEOUT”));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F(“EV_BEACON_FOUND”));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F(“EV_BEACON_MISSED”));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F(“EV_BEACON_TRACKED”));
            break;
        case EV_JOINING:
            Serial.println(F(“EV_JOINING”));
            break;
        case EV_JOINED:
            Serial.println(F(“EV_JOINED”));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F(“EV_RFU1”));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F(“EV_JOIN_FAILED”));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F(“EV_REJOIN_FAILED”));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F(“EV_TXCOMPLETE (includes waiting for RX windows)“));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F(“Received ack”));
            if (LMIC.dataLen) {
              Serial.println(F(“Received “));
              Serial.println(LMIC.dataLen);
              Serial.println(F(” bytes of payload”));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F(“EV_LOST_TSYNC”));
            break;
        case EV_RESET:
            Serial.println(F(“EV_RESET”));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F(“EV_RXCOMPLETE”));
            break;
        case EV_LINK_DEAD:
            Serial.println(F(“EV_LINK_DEAD”));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F(“EV_LINK_ALIVE”));
            break;
         default:
            Serial.println(F(“Unknown event”));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F(“OP_TXRXPEND, not sending”));
    } else {
        float flat, flon;
  unsigned short sats;
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      //Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    sats = gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites();
    }
    else
    {
    flat = -1.000000;
    flon = -1.000000;
    }

    FLOATUNION_t union1;
    FLOATUNION_t union2;
    union1.number = flat;
    union2.number = flon;
	
	uint8_t bresult[10];
    
	// bus id
	bresult[0] = 0;
    bresult[1] = 12;
    
	for(int i = 0; i < 4; i++)
    {
      bresult[2+i] = union1.bytes[i];
      bresult[6+i] = union2.bytes[i];
    }
	
	union1.w = (uint32_t)(flat*100000);
	union2.w = (uint32_t)(flon*100000);
	
	
	//TODO - update this part of the code with lat&lon as uint32_t	
	// manual fix for bytes order
	bresult[2] = union1.bytes[2];
	bresult[3] = union1.bytes[3];
	bresult[4] = union1.bytes[0];
	bresult[5] = union1.bytes[1];
	bresult[6] = union2.bytes[2];
	bresult[7] = union2.bytes[3];
	bresult[8] = union2.bytes[0];
	bresult[9] = union2.bytes[1];

      
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, bresult, sizeof(bresult), 0);
        Serial.println(F(“Packet queued”));
    }

}

void setup() {
    Serial.begin(115200);
    Serial.println(F(“Starting”));
    ss.begin(9600); 
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
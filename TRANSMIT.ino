#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>

// Define LoRa parameters
#define RF_FREQUENCY 433000000 // Hz
#define TX_OUTPUT_POWER 20     // dBm
#define LORA_BANDWIDTH 0       // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR 12// [SF7..SF12]
#define LORA_CODINGRATE 4      // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 16 // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 10  // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 128 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle = true;

static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);

// DHT22 SENSOR DECLARATION START
#define DHTPIN 6        // Pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22 (AM2302) or AM2305

DHT dht(DHTPIN, DHTTYPE);
// DHT22 SENSOR DECLARATION ENDS

void setup() {
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

    txNumber = 0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    // Initialize the DHT22 sensor
    dht.begin();
}

void loop() {
    // DHT22 SENSOR input and calculation
    float externalTemperature = dht.readTemperature();
    float externalHumidity = dht.readHumidity();
    if (isnan(externalTemperature) || isnan(externalHumidity)) {
        Serial.println("Failed to read from DHT sensor!");
        externalTemperature = 0.0;
        externalHumidity = 0.0;
    }

    // Debugging Information
    Serial.print("External Payload Temperature: ");
    Serial.println(externalTemperature);
    Serial.print("External Humidity: ");
    Serial.println(externalHumidity);

    if (lora_idle == true) {
        txNumber += 0.01;
        sprintf(txpacket, "Ext Temp: %0.2f C, Ext Humidity: %0.2f %%, Packet: %0.2f", 
                externalTemperature, externalHumidity, txNumber);  // Start a package
    
        Serial.printf("\r\nSending packet \"%s\" , length %d\r\n", txpacket, strlen(txpacket));

        Radio.Send((uint8_t *)txpacket, strlen(txpacket)); // Send the package out    
        lora_idle = false;
    }
    Radio.IrqProcess();
    
    delay(2000); // Delay for 2 seconds between each reading
}

void OnTxDone(void) {
    Serial.println("TX done......");
    lora_idle = true;
}

void OnTxTimeout(void) {
    Radio.Sleep();
    Serial.println("TX Timeout......");
    lora_idle = true;
}

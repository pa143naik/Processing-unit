#include "LoRaWan_APP.h"
#include "Arduino.h"

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

static RadioEvents_t RadioEvents;

int16_t txNumber;

int16_t rssi, rxSize;

bool lora_idle = true;

void setup() {
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
    
    txNumber = 0;
    rssi = 0;
  
    RadioEvents.RxDone = OnRxDone;
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
}

void loop() {
    if (lora_idle) {
        lora_idle = false;
        Serial.println("into RX mode");
        Radio.Rx(0);
    }
    Radio.IrqProcess();
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    rssi = rssi;
    rxSize = size;
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';
    Radio.Sleep();
    Serial.printf("\r\nreceived packet \"%s\" with rssi %d, length %d\r\n", rxpacket, rssi, rxSize);
    
    // Parse the received data
    float extTemp, extHumidity, packetNumber;
    sscanf(rxpacket, "Ext Temp: %f C, Ext Humidity: %f %%, Packet: %f", 
           &extTemp, &extHumidity, &packetNumber);
    
    // Display parsed data
    Serial.printf("Parsed Data:\n");
    Serial.printf("External Temperature: %0.2f C\n", extTemp);
    Serial.printf("External Humidity: %0.2f %%\n", extHumidity);
    Serial.printf("Packet Number: %0.2f\n", packetNumber);

    // Send the parsed data to the serial port in CSV format
    Serial.printf("%0.2f,%0.2f,%0.2f\n", extTemp, extHumidity, packetNumber);
    
    lora_idle = true;
}

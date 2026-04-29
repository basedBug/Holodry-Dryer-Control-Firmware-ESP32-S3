#include "commsManager.h"

/*
	If it's wanted to use the other USB port onboard (the one that isnt used by default for uploading 
	the code or debugging), use Serial0, or just grab its pins. Otherwise just use any other pins or 
	UARTS.

	The second USB port onboard named "COM" (besides the "USB" one) is connected to UART0, in other
	words, Serial0, which has pins RX->GPIO_44, TX->GPIO_43.
*/
HardwareSerial CommsSerial(Serial0); // Repurpose UART0 for external serial comms

void commsManagerTask(void *pvParameters)
{
	Serial.println("[commsManager] Task started");

    // Init
    initCommsManager();

	/*
		Just to keep the task alive
		Cant just delete the task, the lost references would break the system
	*/
	while (true)
	{
        receiveFromSerialComms();
        receiveFromSysStateManager();

		vTaskDelay(pdTICKS_TO_MS(1));
	}
}

void initCommsManager()
{
    Serial.println("[commsManager] Initializing Serial comms to master controller");
    
    CommsSerial.begin(
        EXTERNAL_COMMS_SERIAL_BAUD_RATE,        // Baudrate
        SERIAL_8N1,                             // UART config, SERIAL_8N1 is the default config
        EXTERNAL_COMMS_UART_SERIAL_RX_PIN,    // RX pin
        EXTERNAL_COMMS_UART_SERIAL_TX_PIN     // TX pin
    );

    Serial.println("[commsManager] Serial comms to master controller initialized");
}

void receiveFromSerialComms()
{
    // Check if theres data available to be read
    if (CommsSerial.available())
    {
        // Read the JSON document from the serial port
        JsonDocument doc_rx;
        DeserializationError error = deserializeJson(doc_rx, CommsSerial);
        if (error) 
        {
            Serial.printf("[commsManager] Error: JSON parse error: %s \n", error.c_str());

            /*
                If deserializeJson() returns an error (such as NoMemory), any subsequent call to 
                deserializeJson() will return InvalidInput. 
                Indeed, deserializeJson() stops reading as soon as it encounters an error, so the remainder
                of the document is still in the serial buffer.

                The solution is to flush the serial buffer any time an error is detected
            */
            while (Serial1.available() > 0)
            {
                Serial1.read();
            }

            return;
        } 
        
        // Send received JSON doc to its destination
        sendToSysStateManager(doc_rx);
    }
}

void sendToSerialComms(JsonDocument &doc)
{
    serializeJson(doc, CommsSerial);
}

void receiveFromSysStateManager()
{
	static char rxJsonMsgBuffer[MAX_MSG_SIZE];

	size_t receivedBytes = xMessageBufferReceive(
		xSysStateManagerToCommsManagerMsgBuffer,	// Target message buffer handle
		rxJsonMsgBuffer,							// Pointer to the buffer for the received message
		sizeof(rxJsonMsgBuffer), 					// Length of the buffer for the received message
		pdMS_TO_TICKS(0)							// Max time this task should be in the Blocked state
													// waiting for a message, if there buffer is empty
	);
	
	if (receivedBytes > 0)
	{
		// Parse and process the JSON
		JsonDocument rx_doc;
		
		DeserializationError error = deserializeJson(rx_doc, rxJsonMsgBuffer, receivedBytes);
		if (error)
		{
			Serial.printf("[commsManager] Error: JSON parse error: %s \n", error.c_str());
			return;
		}

		// Print contents into serial
		//JsonHandlers::printJsonContents(rx_doc);
		
        // Send the JSON doc to its destination
		sendToSerialComms(rx_doc);
	}
}

bool sendToSysStateManager(JsonDocument &doc)
{
	// Maybe (FUTURE) replace the buffer with thread-safe allocation?
	static char txJsonMsgBuffer[MAX_MSG_SIZE];
	const size_t len = measureJson(doc);
	if (len == 0) 
	{
		Serial.println("[commsManager] Warning: Tried to send JSON message of size 0");
		return false;
	}
	if (len > sizeof(txJsonMsgBuffer))
	{
		Serial.printf("[commsManager] Warning: JSON message %u bigger than message buffer %u, dropping JSON \n",
			len, 
			sizeof(txJsonMsgBuffer)
		);
		return false;
	}

	serializeJson(doc, txJsonMsgBuffer, len);

	size_t sentBytes = xMessageBufferSend(
		xCommsManagerToSysStateManagerMsgBuffer,	// Target message buffer handle
		txJsonMsgBuffer,							// Pointer to data being sent
		len, 										// Length of the message
		pdMS_TO_TICKS(0)							// Max time this task should be the in Blocked state
													// for enough space in the buffer, if there's 
													// insufficient space when the call is made
	);

	if (sentBytes != len) {
		Serial.println("[commsManager] Warning: Message buffer to commsManager full, message dropped");
		return false;
	}

	Serial.printf("[commsManager] Sent JSON message of size: %u \n", sentBytes);
	return true;
}
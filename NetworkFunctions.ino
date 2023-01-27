/**
 * @brief The purpose of this file is to hold network-related functions which are device-agnostic.
 * This is not realistic because of the presence of onReceiveCallback.
 * Ideally, this file could be used by an ESP32, ESP8266, or similar boards.
 * Because memory capacity varies wildly from device to device, buffer sizes are declared as variables in the entry-point file.
 */
#include "LilyGo_T-Display_SHT40_APDS9960.h"

/**
 * @brief mqttCallback() will process incoming messages on subscribed topics.
 * { "command": "publishTelemetry" }
 * { "command": "changeTelemetryInterval", "value": 10000 }
 * ToDo: Add more commands for the board to react to.
 */
void mqttCallback( char *topic, byte *payload, unsigned int length )
{
	callbackCount++;
	Serial.printf( "\nMessage arrived on Topic: '%s'\n", topic );

	StaticJsonDocument<JSON_DOC_SIZE> callbackJsonDoc;
	deserializeJson( callbackJsonDoc, payload, length );

	const char *command = callbackJsonDoc["command"];
	Serial.printf( "Processing command '%s'.\n", command );
	if( strcmp( command, "publishTelemetry" ) == 0 )
	{
		Serial.println( "Reading and publishing sensor values." );
		// Poll the sensor.
		readTelemetry();
		// Publish the sensor readings.
		publishTelemetry();
		Serial.println( "Readings have been published." );
	}
	else if( strcmp( command, "changeTelemetryInterval" ) == 0 )
	{
		unsigned long tempValue = callbackJsonDoc["value"];
		// Only update the value if it is greater than 4 seconds.  This prevents a seconds vs. milliseconds confusion.
		if( tempValue > 4000 )
			telemetryPublishInterval = tempValue;
		Serial.printf( "MQTT publish interval has been updated to %lu\n", telemetryPublishInterval );
		lastTelemetryPublishTime = 0;
	}
	else if( strcmp( command, "publishStatus" ) == 0 )
		Serial.println( "publishStatus is not yet implemented." );
	else
		Serial.printf( "Unknown command '%s'\n", command );
} // End of the mqttCallback() function.

/**
 * @brief configureOTA() will configure and initiate Over The Air (OTA) updates for this device.
 *
 */
void configureOTA()
{
	Serial.println( "Configuring OTA." );

#ifdef ESP8266
	// The ESP8266 hostname defaults to esp8266-[ChipID].
	// The ESP8266 port defaults to 8266.
	// ArduinoOTA.setPort( 8266 );
	// Authentication is disabled by default.
	// ArduinoOTA.setPassword( ( const char * ) "admin" );
#else
	// The ESP32 hostname defaults to esp32-[MAC].
	// The ESP32 port defaults to 3232.
	// ArduinoOTA.setPort( 3232 );
	// Authentication is disabled by default.
	// ArduinoOTA.setPassword( "admin" );
	// Password can be set with it's md5 value as well.
	// MD5( admin ) = 21232f297a57a5a743894a0e4a801fc3.
	// ArduinoOTA.setPasswordHash( "21232f297a57a5a743894a0e4a801fc3" );
#endif

	// ArduinoOTA is a class-defined object.
	ArduinoOTA.setHostname( HOST_NAME );

	Serial.printf( "Using hostname '%s'\n", HOST_NAME );

	String type = "filesystem"; // SPIFFS
	if( ArduinoOTA.getCommand() == U_FLASH )
		type = "sketch";

	// Configure the OTA callbacks.
	// Port defaults to 8266.
	// ArduinoOTA.setPort( 8266 );

	// Hostname defaults to esp8266-[ChipID].
	// ArduinoOTA.setHostname( "myesp8266" );

	// No authentication by default.
	// ArduinoOTA.setPassword( "admin" );

	// Password can be set with it's md5 value as well.
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3.
	// ArduinoOTA.setPasswordHash( "21232f297a57a5a743894a0e4a801fc3" );

	ArduinoOTA.onStart( []() {
								  String type;
								  if( ArduinoOTA.getCommand() == U_FLASH )
									  type = "sketch";
								  else // U_SPIFFS
									  type = "filesystem";

								  // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
								  Serial.println( "Start updating " + type ); } );
	ArduinoOTA.onEnd( []() { Serial.println( "\nEnd" ); } );
	ArduinoOTA.onProgress( []( unsigned int progress, unsigned int total ) { Serial.printf( "Progress: %u%%\r", ( progress / ( total / 100 ) ) ); } );
	ArduinoOTA.onError( []( ota_error_t error ) {
								  Serial.printf( "Error[%u]: ", error );
								  if( error == OTA_AUTH_ERROR ) Serial.println( "Auth Failed" );
								  else if( error == OTA_BEGIN_ERROR ) Serial.println( "Begin Failed" );
								  else if( error == OTA_CONNECT_ERROR ) Serial.println( "Connect Failed" );
								  else if( error == OTA_RECEIVE_ERROR ) Serial.println( "Receive Failed" );
								  else if( error == OTA_END_ERROR ) Serial.println( "End Failed" ); } );

	// Start listening for OTA commands.
	ArduinoOTA.begin();

	Serial.println( "OTA is configured and ready." );
} // End of the configureOTA() function.

/*
 * checkForSSID() is used by wifiMultiConnect() to avoid attempting to connect to SSIDs which are not in range.
 * Returns 1 if 'ssidName' can be found.
 * Returns 0 if 'ssidName' cannot be found.
 */
int checkForSSID( const char *ssidName )
{
	byte networkCount = WiFi.scanNetworks();
	if( networkCount == 0 )
		Serial.println( "No WiFi SSIDs are in range!" );
	else
	{
		Serial.printf( "%d networks found.\n", networkCount );
		for( int i = 0; i < networkCount; ++i )
		{
			// Check to see if this SSID matches the parameter.
			if( strcmp( ssidName, WiFi.SSID( i ).c_str() ) == 0 )
				return 1;
			// Alternately, the String compareTo() function can be used like this: if( WiFi.SSID( i ).compareTo( ssidName ) == 0 )
		}
	}
	Serial.printf( "SSID '%s' was not found!\n", ssidName );
	return 0;
} // End of checkForSSID() function.

/**
 * @brief wifiConnect() will connect to a SSID.
 */
void wifiConnect()
{
	long time = millis();
	if( lastWifiConnectTime == 0 || ( time > wifiCoolDownInterval && ( time - wifiCoolDownInterval ) > lastWifiConnectTime ) )
	{
		int ssidCount = checkForSSID( wifiSsid );
		if( ssidCount == 0 )
		{
			Serial.printf( "SSID '%s' is not in range!\n", wifiSsid );
			digitalWrite( BACKLIGHT, 0 ); // Turn the LED off to show that Wi-Fi has no chance of connecting.
		}
		else
		{
			wifiConnectCount++;
			// Turn the LED off to show Wi-Fi is not connected.
			digitalWrite( BACKLIGHT, 0 );

			Serial.printf( "Attempting to connect to Wi-Fi SSID '%s'", wifiSsid );
			WiFi.mode( WIFI_STA );
			WiFi.config( INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE );
			const char *hostName = macAddress;
			WiFi.setHostname( hostName );
			WiFi.begin( wifiSsid, wifiPassword );

			unsigned long wifiConnectionStartTime = millis();

			// Loop until connected, or until wifiConnectionTimeout.
			while( WiFi.status() != WL_CONNECTED && ( millis() - wifiConnectionStartTime < wifiConnectionTimeout ) )
			{
				Serial.print( "." );
				delay( 1000 );
			}
			Serial.println( "" );

			if( WiFi.status() == WL_CONNECTED )
			{
				// Print that Wi-Fi has connected.
				Serial.println( "\nWi-Fi connection established!" );
				snprintf( ipAddress, 16, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
				// Turn the LED on to show that Wi-Fi is connected.
				digitalWrite( BACKLIGHT, 1 );
				return;
			}
			else
				Serial.println( "Wi-Fi failed to connect in the timeout period.\n" );
		}
		lastWifiConnectTime = millis();
	}
} // End of the wifiConnect() function.

/**
 * @brief mqttConnect() will connect to the MQTT broker.
 */
void mqttConnect()
{
	long time = millis();
	// This block prevents MQTT from connecting more than every mqttCoolDownInterval milliseconds.
	// Connect the first time.  Avoid subtraction overflow.  Connect after cool down.
	if( lastMqttConnectionTime == 0 || ( time > mqttCoolDownInterval && ( time - mqttCoolDownInterval ) > lastMqttConnectionTime ) )
	{
		mqttConnectCount++;
		digitalWrite( BACKLIGHT, 0 );
		Serial.printf( "Connecting to broker at %s:%d using client ID '%s'...\n", mqttBroker, mqttPort, macAddress );
		mqttClient.setServer( mqttBroker, mqttPort );
		mqttClient.setCallback( mqttCallback );

		// Connect to the broker, using the MAC address for a MQTT client ID.
		if( mqttClient.connect( macAddress ) )
		{
			Serial.println( "Connected to MQTT Broker." );
			mqttClient.subscribe( MQTT_COMMAND_TOPIC );
			digitalWrite( BACKLIGHT, 1 );
		}
		else
		{
			int mqttStateCode = mqttClient.state();
			char buffer[29];
			lookupMQTTCode( mqttStateCode, buffer );
			Serial.printf( "MQTT state: %s\n", buffer );
			Serial.printf( "MQTT state code: %d\n", mqttStateCode );

			// This block increments the broker connection "cooldown" time by 10 seconds after every failed connection, and resets it once it is over 2 minutes.
			if( mqttCoolDownInterval > 120000 )
				mqttCoolDownInterval = 0;
			mqttCoolDownInterval += 10000;
		}

		lastMqttConnectionTime = millis();
	}
} // End of the mqttConnect() function.

/**
 * @brief publishTelemetry() will publish basic device information to the MQTT broker.
 */
void publishTelemetry()
{
	float averageTempC = averageArray( sht40TempCArray );
	float tempF = cToF( averageTempC );

	float averageHumidity = averageArray( sht40HumidityArray );
	char mqttStatsString[JSON_DOC_SIZE];
	// Create a JSON Document on the stack.
	StaticJsonDocument<JSON_DOC_SIZE> statsJsonDoc;
	// Add data: SKETCH_NAME, macAddress, ipAddress, rssi, publishCount
	statsJsonDoc["sketch"] = __FILE__;
	statsJsonDoc["mac"] = macAddress;
	statsJsonDoc["ip"] = ipAddress;
	statsJsonDoc["rssi"] = rssi;
	statsJsonDoc["publishCount"] = publishCount;
	statsJsonDoc["tempC"] = averageTempC;
	statsJsonDoc["tempF"] = tempF;
	statsJsonDoc["latestTempC"] = sht40TempCArray[0];
	statsJsonDoc["humidity"] = averageHumidity;
	statsJsonDoc["latestHumidity"] = sht40HumidityArray[0];

	// Serialize statsJsonDoc into mqttStatsString, with indentation and line breaks.
	serializeJsonPretty( statsJsonDoc, mqttStatsString );

	Serial.printf( "Publishing stats to the '%s' topic.\n", MQTT_STATS_TOPIC );

	if( mqttClient.publish( MQTT_STATS_TOPIC, mqttStatsString ) )
	{
		Serial.printf( "Published to broker '%s:%d' on topic '%s'.\n", mqttBroker, mqttPort, MQTT_STATS_TOPIC );
		Serial.println( mqttStatsString );
	}
	else
	{
		Serial.println( "\n---------------" );
		Serial.println( "Publish failed!" );
		Serial.println( "---------------\n" );
	}

	publishCount++;
	char topicBuffer[256] = "";
	char valueBuffer[25] = "";

	snprintf( valueBuffer, 25, "%f", averageArray( sht40TempCArray ) );
	if( mqttClient.publish( topicBuffer, valueBuffer ) )
		Serial.printf( "Published '%s' to '%s'\n", valueBuffer, TEMP_C_TOPIC );
	else
		Serial.printf( "Failed to publish '%s' to '%s'\n", valueBuffer, TEMP_C_TOPIC );

	snprintf( valueBuffer, 25, "%f", cToF( averageArray( sht40TempCArray ) ) );
	if( mqttClient.publish( topicBuffer, valueBuffer ) )
		Serial.printf( "Published '%s' to '%s'\n", valueBuffer, TEMP_F_TOPIC );
	else
		Serial.printf( "Failed to publish '%s' to '%s'\n", valueBuffer, TEMP_F_TOPIC );

	snprintf( valueBuffer, 25, "%f", averageArray( sht40HumidityArray ) );
	if( mqttClient.publish( topicBuffer, valueBuffer ) )
		Serial.printf( "Published '%s' to '%s'\n", valueBuffer, HUMIDITY_TOPIC );
	else
		Serial.printf( "Failed to publish '%s' to '%s'\n", valueBuffer, HUMIDITY_TOPIC );

	snprintf( valueBuffer, 25, "%ld", rssi );
	if( mqttClient.publish( topicBuffer, valueBuffer ) )
		Serial.printf( "Published '%s' to '%s'\n", valueBuffer, RSSI_TOPIC );
	else
		Serial.printf( "Failed to publish '%s' to '%s'\n", valueBuffer, RSSI_TOPIC );

	snprintf( valueBuffer, 25, "%s", macAddress );
	if( mqttClient.publish( topicBuffer, valueBuffer ) )
		Serial.printf( "Published '%s' to '%s'\n", valueBuffer, MAC_TOPIC );
	else
		Serial.printf( "Failed to publish '%s' to '%s'\n", valueBuffer, MAC_TOPIC );

	snprintf( valueBuffer, 25, "%s", ipAddress );
	if( mqttClient.publish( topicBuffer, valueBuffer ) )
		Serial.printf( "Published '%s' to '%s'\n", valueBuffer, IP_TOPIC );
	else
		Serial.printf( "Failed to publish '%s' to '%s'\n", valueBuffer, IP_TOPIC );

	//	snprintf( valueBuffer, 25, "%u", wifiConnectCount );
	//	Serial.printf( "Published '%s' to '%s'\n", valueBuffer, wifiCountTopic );
	//	mqttClient.publish( topicBuffer, valueBuffer );
	//
	//	snprintf( valueBuffer, 25, "%lu", wifiCoolDown );
	//	Serial.printf( "Published '%s' to '%s'\n", valueBuffer, wifiCoolDownTopic );
	//	mqttClient.publish( topicBuffer, valueBuffer );
	//
	//	snprintf( valueBuffer, 25, "%u", mqttConnectCount );
	//	Serial.printf( "Published '%s' to '%s'\n", valueBuffer, mqttCountTopic );
	//	mqttClient.publish( topicBuffer, valueBuffer );
	//
	//	snprintf( valueBuffer, 25, "%lu", mqttCoolDown );
	//	Serial.printf( "Published '%s' to '%s'\n", valueBuffer, mqttCoolDownTopic );
	//	mqttClient.publish( topicBuffer, valueBuffer );

	snprintf( valueBuffer, 25, "%lu", publishCount );
	if( mqttClient.publish( topicBuffer, valueBuffer ) )
		Serial.printf( "Published '%s' to '%s'\n", valueBuffer, PUBLISH_COUNT_TOPIC );
	else
		Serial.printf( "Failed to publish '%s' to '%s'\n", valueBuffer, PUBLISH_COUNT_TOPIC );
} // End of publishTelemetry() function.

/**
 * @brief lookupWifiCode() will return the string for an integer code.
 */
void lookupWifiCode( int code, char *buffer )
{
	switch( code )
	{
		case 0:
			snprintf( buffer, 26, "%s", "Idle" );
			break;
		case 1:
			snprintf( buffer, 26, "%s", "No SSID" );
			break;
		case 2:
			snprintf( buffer, 26, "%s", "Scan completed" );
			break;
		case 3:
			snprintf( buffer, 26, "%s", "Connected" );
			break;
		case 4:
			snprintf( buffer, 26, "%s", "Connection failed" );
			break;
		case 5:
			snprintf( buffer, 26, "%s", "Connection lost" );
			break;
		case 6:
			snprintf( buffer, 26, "%s", "Disconnected" );
			break;
		default:
			snprintf( buffer, 26, "%s", "Unknown Wi-Fi status code" );
	}
} // End of lookupWifiCode() function.

/**
 * @brief lookupMQTTCode() will return the string for an integer state code.
 */
void lookupMQTTCode( int code, char *buffer )
{
	switch( code )
	{
		case -4:
			snprintf( buffer, 29, "%s", "Connection timeout" );
			break;
		case -3:
			snprintf( buffer, 29, "%s", "Connection lost" );
			break;
		case -2:
			snprintf( buffer, 29, "%s", "Connect failed" );
			break;
		case -1:
			snprintf( buffer, 29, "%s", "Disconnected" );
			break;
		case 0:
			snprintf( buffer, 29, "%s", "Connected" );
			break;
		case 1:
			snprintf( buffer, 29, "%s", "Bad protocol" );
			break;
		case 2:
			snprintf( buffer, 29, "%s", "Bad client ID" );
			break;
		case 3:
			snprintf( buffer, 29, "%s", "Unavailable" );
			break;
		case 4:
			snprintf( buffer, 29, "%s", "Bad credentials" );
			break;
		case 5:
			snprintf( buffer, 29, "%s", "Unauthorized" );
			break;
		default:
			snprintf( buffer, 29, "%s", "Unknown MQTT state code" );
	}
} // End of lookupMQTTCode() function.

/**
 * @brief This program will read the temperature and humidity from a SHT40 sensor, and light levels from an APDS9960 lux sensor.
 * Values will be read every 15 seconds (configurable), and the current reading will be averaged with the previous two.
 * The temperature and humidity sensor is from Adafruit: https://www.adafruit.com/products/4885
 * The light sensor is also from Adafruit: https://www.adafruit.com/product/3595
 */

#include "Wire.h"
#include "Adafruit_SHT4x.h"
#include "Adafruit_APDS9960.h"

#define LED_BUILTIN 38

#define OVERRIDE_WIRE									// Commend out this line to use the default SCL and SDA GPIOs.
#ifdef OVERRIDE_WIRE
const byte sdaGPIO = 43;							// Use this to set the SDA GPIO if your board uses a non-standard GPIOs for the I2C bus.
const byte sclGPIO = 44;							// Use this to set the SCL GPIO if your board uses a non-standard GPIOs for the I2C bus.
#endif


unsigned long lastTelemetryPollTime = 0;			// The last time sensors were polled.
unsigned long lastTelemetryProcessTime = 0;		// The last time sensor data was acted on.
unsigned long lastTelemetryPrintTime = 0;			// The last time sensor data was printed.
unsigned int telemetryPollInterval = 15000;		// How long to wait between sensor polling.
unsigned int telemetryProcessInterval = 200;	// How long to wait between sensor processing.
unsigned int telemetryPrintInterval = 5000;		// How long to wait between sensor printing.
float tempArray[] = { 0, 0, 0 };							// An array to hold the 3 most recent temperature values.
float humidityArray[] = { 0, 0, 0 };					// An array to hold the 3 most recent humidity values.


Adafruit_SHT4x sht4 = Adafruit_SHT4x();
Adafruit_APDS9960 apds;


void setup() 
{
  delay( 1000 );
  Serial.begin( 115200 );

  if( !Serial )
    delay( 1000 );
  
  pinMode( LED_BUILTIN, OUTPUT );

#ifdef OVERRIDE_WIRE
	// Override the default I2C GPIOs.
	Wire.begin( sdaGPIO, sclGPIO );
#else
	// Use default I2C GPIOs.
	 Wire.begin();
#endif

  Serial.println( "Adafruit SHT4x test" );
  if( !sht4.begin() ) 
  {
    Serial.println( "Couldn't find SHT4x" );
    while( 1 )
      delay( 1 );
  }
  Serial.println( "Found SHT4x sensor" );
  Serial.print( "Serial number 0x" );
  Serial.println( sht4.readSerial(), HEX );

  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision( SHT4X_HIGH_PRECISION );
  switch( sht4.getPrecision() ) 
  {
     case SHT4X_HIGH_PRECISION: 
       Serial.println( "High precision" );
       break;
     case SHT4X_MED_PRECISION: 
       Serial.println( "Med precision" );
       break;
     case SHT4X_LOW_PRECISION: 
       Serial.println( "Low precision" );
       break;
  }

  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater( SHT4X_NO_HEATER );
  switch ( sht4.getHeater() ) 
  {
     case SHT4X_NO_HEATER: 
       Serial.println( "No heater" );
       break;
     case SHT4X_HIGH_HEATER_1S: 
       Serial.println( "High heat for 1 second" );
       break;
     case SHT4X_HIGH_HEATER_100MS: 
       Serial.println( "High heat for 0.1 second" );
       break;
     case SHT4X_MED_HEATER_1S: 
       Serial.println( "Medium heat for 1 second" );
       break;
     case SHT4X_MED_HEATER_100MS: 
       Serial.println( "Medium heat for 0.1 second" );
       break;
     case SHT4X_LOW_HEATER_1S: 
       Serial.println( "Low heat for 1 second" );
       break;
     case SHT4X_LOW_HEATER_100MS: 
       Serial.println( "Low heat for 0.1 second" );
       break;
  }
  if( !apds.begin() )
    Serial.println( "Failed to initialize device! Please check your wiring." );
  else Serial.println( "Device initialized!" );

  //enable color sensign mode
  apds.enableColor( true );

  Serial.println( "Setup has completed." );  
} // End of setup() function.


/**
 * @brief readTelemetry() will manipulate the settingArray[] like a FIFO queue, by popping the head value off, and adding a new value to the tail.
 */
void readTelemetry()
{
  sensors_event_t humidity;
  sensors_event_t temp;
  sht4.getEvent (&humidity, &temp );// populate temp and humidity objects with fresh data

  tempArray[0] = tempArray[1];
  tempArray[1] = tempArray[2];
  tempArray[2] = temp.temperature;

  humidityArray[0] = humidityArray[1];
  humidityArray[1] = humidityArray[2];
  humidityArray[2] = humidity.relative_humidity;
}


void printTelemetry()
{
  Serial.printf( "Current temp: %.3f degrees C\n", tempArray[2] );
  float tempC = ( tempArray[0] + tempArray[1] + tempArray[2] ) / 3.0;
  Serial.printf( "Average: %.3f degrees C\n", tempC );
  float tempF = ( tempC * 1.8 ) + 32;
  Serial.printf( "Average: %.3f degrees F\n", tempF );

  Serial.printf( "Current humidity: %.3f %% rH\n", humidityArray[2] );
  float humidity = ( humidityArray[0] + humidityArray[1] + humidityArray[2] ) / 3.0;
  Serial.printf( "Average: %.3f %% rH\n", humidity );
}


void readColors()
{
  // Variables to store the color data.
  uint16_t r, g, b, c;
  
  // Wait for color data to be ready.
  while( !apds.colorDataReady() )
    delay( 5 );

  // Get color data and print the different channels.
  apds.getColorData( &r, &g, &b, &c );
  Serial.print( "red: " );
  Serial.print( r );
  
  Serial.print( " green: " );
  Serial.print( g );
  
  Serial.print( " blue: " );
  Serial.print( b );
  
  Serial.print( " clear: " );
  Serial.println( c );
  Serial.println();
} // End of readColors() functino.


/**
 * @brief toggleLED() will change the state of the LED.
 * This function does not manage any timings.
 */
void toggleLED()
{
	if( digitalRead( LED_BUILTIN ) != 1 )
		digitalWrite( LED_BUILTIN, 1 );
	else
		digitalWrite( LED_BUILTIN, 0 );
} // End of toggleLED() function.


void loop() 
{
  unsigned long time = millis();
	// Poll the first time.  Avoid subtraction overflow.  Poll every interval.
	if( lastTelemetryPollTime == 0 || ( ( time > telemetryPollInterval ) && ( time - telemetryPollInterval ) > lastTelemetryPollTime ) )
	{
		readTelemetry();
		lastTelemetryPollTime = millis();

    printTelemetry();
    readColors();
    toggleLED();
	}
} // End of loop() function.

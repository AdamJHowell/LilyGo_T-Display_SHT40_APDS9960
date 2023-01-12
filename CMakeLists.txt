cmake_minimum_required( VERSION 3.24 )
project( LilyGo_T_Display_SHT40_APDS9960 )

set( CMAKE_CXX_STANDARD 14 )

include_directories( .
		C:/Code/Arduino/libraries/Adafruit_SHT4x_Library
		C:/Code/Arduino/libraries/Adafruit_Unified_Sensor
		C:/Code/Arduino/libraries/Adafruit_APDS9960_Library
		C:/Code/Arduino/libraries/Adafruit_BusIO
		)

add_executable( LilyGo_T_Display_SHT40_APDS9960
		./LilyGo_T-Display_SHT40_APDS9960.ino
		./LilyGo_T-Display_SHT40_APDS9960.h
		./NetworkFunctions.ino
		./privateInfo.h
		C:/Code/Arduino/libraries/Adafruit_SHT4x_Library/Adafruit_SHT4x.h
		C:/Code/Arduino/libraries/Adafruit_APDS9960_Library/Adafruit_APDS9960.h
		C:/Code/Arduino/libraries/Adafruit_Unified_Sensor/Adafruit_Sensor.h )

set_target_properties( LilyGo_T_Display_SHT40_APDS9960 PROPERTIES LINKER_LANGUAGE CXX )

target_include_directories( LilyGo_T_Display_SHT40_APDS9960 PRIVATE ${/Code/Arduino/libraries/Adafruit_SHT4x_Library} )

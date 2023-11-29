// IR line sensors
#define LINE_LEFT_PIN 34
#define LINE_LEFT_POWER_PIN 18
#define LINE_RIGHT_PIN 35
#define LINE_RIGHT_POWER_PIN 19
#define LINE_REFLECTION_MULTIPLIER 2  // defines how high the threshold is between detecting ring surface and outside line, ignoring ambient light



/***********
Header Files
************/



/***************
Global Variables
****************/
// calibrated values above which the sensors have detected lines
int g_line_left_threshold, g_line_right_threshold;
// to store readings from the ultrasonic sensors
struct Sensors {
	float object_front = 400.0;
	float object_left = 400.0;
	float object_right = 400.0;
	bool line_left = false;
	bool line_right = false;
} g_sensor_readings;  // the variable name

/******************
Function Prototypes
*******************/
void reverse_escape(void);
void opponent_check(void);
void turn_opponent(void);
void search_opponent(void);
void update_motor(void);
bool calibrate_line_sensors(void);


/*********
Setup Code
**********/
void setup() {
	// put your setup code here, to run once:
  Serial.begin(115200);

	bool success;

	// set sensor pin modes
	pinMode(LINE_LEFT_POWER_PIN, OUTPUT);
	pinMode(LINE_RIGHT_POWER_PIN, OUTPUT);

	while(true) {
		// calibrate the line sensors for the current ambient lighting, then switch them on
		success = calibrate_line_sensors();
		// if the sensors calibrated then switch them on, otherwise repeat the calibration
		if(success == true) {
			digitalWrite(LINE_LEFT_POWER_PIN, HIGH);
			digitalWrite(LINE_RIGHT_POWER_PIN, HIGH);
			break;
		}
	}
  Serial.println("Setup completed");
}



/********
Main Code
*********/

void loop() {
	// put your main code here, to run repeatedly:

	// check for lines
	line_check();
	Serial.println("Line left:");
	Serial.println(g_sensor_readings.line_left);
	Serial.println("Line right:");
	Serial.println(g_sensor_readings.line_right);
	Serial.println("**********");
  delay(1000);
}


void line_check() {
	// check for lines
	if(analogRead(LINE_LEFT_PIN) >= g_line_left_threshold) {
		g_sensor_readings.line_left = true;
	} else {
		g_sensor_readings.line_left = false;
	}
	if(analogRead(LINE_RIGHT_PIN) >= g_line_right_threshold) {
		g_sensor_readings.line_right = true;
	} else {
		g_sensor_readings.line_right = false;
	}
}


bool calibrate_line_sensors() {
	int on_value_left = 0, on_value_right = 0, off_value_left = 0, off_value_right = 0, total_readings = 5;
	int ambient_light_left, ambient_light_right, ring_surface_left, ring_surface_right;
	
	// take light off readings
	digitalWrite(LINE_LEFT_POWER_PIN, LOW);
	digitalWrite(LINE_RIGHT_POWER_PIN, LOW);
	delayMicroseconds(500);
	for(int i=0; i<total_readings; i++){
		// take a reading from the left sensor, light off
		off_value_left += analogRead(LINE_LEFT_PIN);
		// take a reading from the right sensor, light off
		off_value_right += analogRead(LINE_RIGHT_PIN);
		delayMicroseconds(50);
	}
	// take light on readings
	digitalWrite(LINE_LEFT_POWER_PIN, HIGH);
	digitalWrite(LINE_RIGHT_POWER_PIN, HIGH);
	delayMicroseconds(500);
	for(int i=0; i<total_readings; i++){
		// take a reading from the left sensor, light off
		on_value_left += analogRead(LINE_LEFT_PIN);
		// take a reading from the right sensor, light off
		on_value_right += analogRead(LINE_RIGHT_PIN);
		delayMicroseconds(50);
	}

	// calculate thresholds
	ambient_light_left = off_value_left / total_readings;
	ambient_light_right = off_value_right / total_readings;
	ring_surface_left = on_value_left / total_readings;
	ring_surface_right = on_value_right / total_readings;
	g_line_left_threshold = ceil((LINE_REFLECTION_MULTIPLIER * (ring_surface_left - ambient_light_left)) + ambient_light_left);
	g_line_right_threshold = ceil((LINE_REFLECTION_MULTIPLIER * (ring_surface_right - ambient_light_right)) + ambient_light_right);

	// return true if sucessful
	if((g_line_left_threshold > 1) && (g_line_right_threshold > 1)) {
		return true;
	} else {
		return false;
	}
}

/*
**************************************
**  B.O.B. (simple behaviour mode)  **
**************************************
* Barely Operable Bot
*
* Authors:
* Dave Riley
* Raghav Kejriwal
* Loïc Zammit
* 
* Code guidelines:
* 	- variable names should be all lowercase using underscores, struct or class names CamelCase, global variables prefixed with g_
* 	- use spaces around operators, apart from in arguments
* 	- first open brace { on same line, closing brace } a line by itself
* 	- keep things simple and readable
* 
* NB: With our board, may have to hold 'BOOT' Switch when uploading sketch
*/


/**********
Definitions
***********/
// left motor
#define LMOTOR_PWM_PIN   3
#define LMOTOR_DIR_PIN_1 4 
#define LMOTOR_DIR_PIN_2 6
#define LMOTOR_CORRECTION_FACTOR 1  // must be <=1
#if LMOTOR_CORRECTION_FACTOR > 1
	#error Motor correction factors must be less than or equal to 1
#endif
// right motor
#define RMOTOR_PWM_PIN   5
#define RMOTOR_DIR_PIN_1 7 
#define RMOTOR_DIR_PIN_2 8
#define RMOTOR_CORRECTION_FACTOR 1  // must be <=1
#if RMOTOR_CORRECTION_FACTOR > 1
	#error Motor correction factors must be less than or equal to 1
#endif
// both motors
#define MOTORMIN 60  // to account for deadband of motors, set based on motor with largest deadband
#define TURNING_FACTOR_FAST 50  // amount of PWM to decrease the turning wheel by when driving quickly
// IR line sensors
#define LINE_LEFT_PIN A0
#define LINE_LEFT_POWER_PIN 12
#define LINE_RIGHT_PIN A1
#define LINE_RIGHT_POWER_PIN 13
#define LINE_REFLECTION_MULTIPLIER 2  // defines how high the threshold is between detecting ring surface and outside line, ignoring ambient light
// object sensors
#define RING_SIZE 77


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
// stores commands for motor power and steering
struct MotorCommands {
	bool turn_left;
	bool turn_right;
	int motor_power;  // negative for reverse
} g_motor_commands;  // the variable name

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
	bool success;

	// set inbuilt LED pin (D13) as output & switch on to show that setup in progress
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	// set motor pin modes
	pinMode(LMOTOR_DIR_PIN_1, OUTPUT);
	pinMode(LMOTOR_DIR_PIN_2, OUTPUT);
	pinMode(LMOTOR_PWM_PIN, OUTPUT); 
	pinMode(RMOTOR_DIR_PIN_1, OUTPUT);
	pinMode(RMOTOR_DIR_PIN_2, OUTPUT);
	pinMode(RMOTOR_PWM_PIN, OUTPUT); 

	// set sensor pin modes
	pinMode(LINE_LEFT_POWER_PIN, OUTPUT);
	pinMode(LINE_RIGHT_POWER_PIN, OUTPUT);

	// set motors in forward direction
	digitalWrite(LMOTOR_DIR_PIN_1, LOW);
	digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
	digitalWrite(RMOTOR_DIR_PIN_1, LOW);
	digitalWrite(RMOTOR_DIR_PIN_2, HIGH);

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

	// blink LED to show setup completed sucessfully
	for(int i=0; i<3; i++) {
		digitalWrite(LED_BUILTIN, LOW);
		delay(100);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(100);
		digitalWrite(LED_BUILTIN, LOW);
	}
}



/********
Main Code
*********/

void loop() {
	// put your main code here, to run repeatedly:

	// check for lines
	line_check();

	// move away if lines detected
	if(g_sensor_readings.line_left && g_sensor_readings.line_right) {
		reverse_escape();
	} else if(g_sensor_readings.line_left) {
		g_motor_commands.turn_right = true;
		g_motor_commands.turn_left = false;
	} else if(g_sensor_readings.line_right) {
		g_motor_commands.turn_right = false;
		g_motor_commands.turn_left = true;
	}
	update_motor();

	// check for opponents
	// TODO

	// face opponent and then attack if within range & if no lines detected
	// TODO

	// search for opponent if not detected in range & if no lines detected
	// TODO
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


void reverse_escape(){
	// does a quick reverse, a turn, then returns control of motor and steering to normal
}


void opponent_check() {
	// updates sensor readings in the global variable
}


void turn_opponent() {
	//turn through steer fx until minimum distance on front sensor to opponent is found, and then power fx
}


void search_opponent() {
	//steer to find opponent, only go to this fx if opponent not detected in opponent_check
}


void update_motor() {
	// logic to update output values to motor based on global variable values
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
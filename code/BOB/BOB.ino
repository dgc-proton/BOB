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
*
* ERROR CODES BUILTIN LED: (REPEAT ERROR CODES 10 TIMES IN 1 SEC INTERVALS (?) USING for(j) LOOP, AND for(i) LOOP FOR ERROR CODE) 
* 1: . . . . . : Issue with update_motor, g_motor_commands has conflicting/erroneous values. Either turn is not within [-1, 1] or direction says reverse, but turn is not set to straight (0) {while this is possible to implement, it is rather confusing}. Hopefully code logic should never bring such a situation.
*/


/**********
Definitions
***********/
// left motor
#define LMOTOR_PWM_PIN   13
#define LMOTOR_DIR_PIN_1 12 
#define LMOTOR_DIR_PIN_2 14
#define LMOTOR_CORRECTION_FACTOR 1  // must be <=1
#if LMOTOR_CORRECTION_FACTOR > 1
	#error Motor correction factors must be less than or equal to 1
#endif
// right motor
#define RMOTOR_PWM_PIN   15
#define RMOTOR_DIR_PIN_1 2 
#define RMOTOR_DIR_PIN_2 4
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
	int turn; // -1 for left; 1 for right; 0 for straight.
	bool forward; // true for forward, false for reverse. Can only be in reverse if turn is set to 0, or straight! Not allowing a reverse turning possibility, since doing an on the spot turn anyways.
	int motor_power;  // negative for reverse
} g_motor_commands;  // the variable name
/******************
Function Prototypes
*******************/
void line_check(void);
void reverse_escape(void);
void single_side_escape(void);
void search_attack(void);
void update_motor(void);
void turn_left(void);
void turn_right(void);
void dir_forward(void);
void dir_reverse(void);
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
	dir_forward();

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
		turn_right()
	} else if(g_sensor_readings.line_right) {
		turn_left
	}
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


void reverse_escape() {
	// does a quick reverse, a turn, then returns control of motor and steering to normal
	// sets motors to reverse direction, delays to make sure it gets somewhere before checking for next loop iteration. completes a line check and iteration of loop done if line check still returns positive
	do {
		g_motor_commands.motor_power = 255;
		dir_reverse();
		// revers set to high speed so can counter forward momentum if needed. wip
		delay(100);
		line_check();
	} while(g_sensor_readings.line_left && g_sensor_readings.line_right);
	// does right side turn, slow speed to reduce slip, completes line check, iteration only takes place if either line is no longer seen. if both lines are seen, recurses reverse_escape().
	// How does it know its facing forwards, or should it go to opponent check as soon as line isnt seen?
	// do {
		g_motor_commands.motor_power = 50;
		turn_right();
		line_check();
		if(g_sensor_readings.line_left && g_sensor_readings.line_right) {
			reverse_escape();
		}
	// } while((g_sensor_readings.line_left && !g_sensor_readings.line_right) || (!g_sensor_readings.line_left && g_sensor_readings.line_right));
	// wip if needed here or in do while loop
	// if(g_sensor_readings.line_left && g_sensor_readings.line_right) {
	//		reverse_escape();
	// }
}

void single_side_escape() {
	// escape when line is detected only on single side
}

void search_attack() {
	// searches for the opponent, if opponent found will accelerate full speed at them
}


void update_motor() {
	// logic to update output values to motor based on global variable values
	// Sets directions for both motors. true value means forward, false value means reverse
	if(g_motor_commands.turn == -1 && g_motor_commands.forward == true) {
		digitalWrite(LMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(LMOTOR_DIR_PIN_2, LOW);
		digitalWrite(RMOTOR_DIR_PIN_1, LOW);
		digitalWrite(RMOTOR_DIR_PIN_2, HIGH);
	} else if(g_motor_commands.turn == 1 && g_motor_commands.forward == true) {
		digitalWrite(LMOTOR_DIR_PIN_1, LOW);
		digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_2, LOW);
	} else if(g_motor_commands.turn == 0 && g_motor_commands.forward == true) {
		digitalWrite(LMOTOR_DIR_PIN_1, LOW);
		digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_1, LOW);
		digitalWrite(RMOTOR_DIR_PIN_2, HIGH);
	} else if(g_motor_commands.turn == 0 && g_motor_commands.forward == false) {
		digitalWrite(LMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(LMOTOR_DIR_PIN_2, LOW);
		digitalWrite(RMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_2, LOW);
	}
	// ERROR CODE 1
	else {
		for (j = 0; j < 10; j++) {
			for (i = 0; i < 5; i++) {
				digitalWrite(LED_BUILTIN, LOW);
				delay (50);
				digitalWrite(LED_BUILTIN, HIGH);
				delay(50);
				digitalWrite(LED_BUILTIN, LOW);
			}
			delay(1000);
		}
	
	digitalWrite(LMOTOR_PWM_PIN, g_motor_commands.motor_power);
	digitalWrite(RMOTOR_PWM_PIN, g_motor_commands.motor_power);
}

void turn_left() {
	g_motor_commands.turn = -1;
	g_motor_commands.forward = true;
	update_motor();
}

void turn_right() {
	g_motor_commands.turn = 1;
	g_motor_commands.forward = true;
	update_motor();
}

void dir_forward() {
	g_motor_commands.turn = 0;
	g_motor_commands.forward = true;
	update_motor();
}

void dir_reverse() {
	g_motor_commands.turn = 0;
	g_motor_commands.forward = false;
	update_motor();
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

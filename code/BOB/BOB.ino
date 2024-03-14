/*
**************************************
**  B.O.B. - Barely Operational Bot  **
**************************************
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
* ERROR CODES BUILTIN LED: (REPEAT ERROR CODES 10 TIMES IN 2 SEC INTERVALS (?))
* 1: . . . . . : Issue with update_motor, g_motor_commands has conflicting/erroneous values. Either turn is not within [-1, 1] or direction says reverse, but turn is not set to straight (0) {while this is possible to implement, it is rather confusing}. Hopefully code logic should never bring such a situation.
*/

/**********
Definitions
***********/

// left motor
#define LMOTOR_PWM_PIN   13
#define LMOTOR_DIR_PIN_1 12 
#define LMOTOR_DIR_PIN_2 14
const float LMOTOR_CORRECTION_FACTOR = 0.5; // must be <=1
const float LMOTOR_TURN_CORRECTION_FACTOR = 1; // motors slower in reverse

#if LMOTOR_CORRECTION_FACTOR > 1
	#error Motor correction factors must be less than or equal to 1
#endif

// right motor
#define RMOTOR_PWM_PIN   15
#define RMOTOR_DIR_PIN_1 2 
#define RMOTOR_DIR_PIN_2 4
const float RMOTOR_CORRECTION_FACTOR = 0.5; // must be <=1
const float RMOTOR_TURN_CORRECTION_FACTOR = 1; // motors slower in reverse

#if RMOTOR_CORRECTION_FACTOR > 1
	#error Motor correction factors must be less than or equal to 1
#endif

// both motors
#define MOTORMIN 60  // to account for deadband of motors, set based on motor with largest deadband
#define TURNING_FACTOR_FAST 50  // amount of PWM to decrease the turning wheel by when driving quickly

// IR line sensors
#define LINE_LEFT_PIN 27
#define LINE_LEFT_POWER_PIN 26
#define LINE_RIGHT_PIN 25
#define LINE_RIGHT_POWER_PIN 33
#define LINE_REFLECTION_MULTIPLIER 2  // defines how high the threshold is between detecting ring surface and outside line, ignoring ambient light

//Object sensors
#define RING_SIZE 770

#define LOBJSENSOR_TRIG 16
#define LOBJSENSOR_ECHO 17

#define ROBJSENSOR_TRIG 5
#define ROBJSENSOR_ECHO 18

// Built-in LED
#define LED_BUILTIN 2

/***************
Global Variables
****************/
// calibrated values above which the sensors have detected lines
int g_line_left_threshold, g_line_right_threshold;

//Stores readings from the ultrasonic and infrared sensors
struct Sensors {
	float object_left = 4000.0;
	float object_right = 4000.0;
	float last_seen_right = true;
	bool line_left = false;
	bool line_right = false;
} g_sensor_readings;  // the structure name

// stores commands for motor power and steering
struct MotorCommands {
	int turn; // -1 for left; 1 for right; 0 for straight.
	bool forward; // true for forward, false for reverse. Can only be in reverse if turn is set to 0, or straight! Not allowing a reverse turning possibility, since doing an on the spot turn anyways.
	int motor_power;  // negative for reverse
} g_motor_commands;  // the structure name

/******************
Function Prototypes
*******************/
void line_check(void);
void reverse_escape(void);
void search_attack(void);
void update_object_sensors(void);
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
  Serial.begin(115200); //Starts serial output

	bool success;

	//Set built-in LED pin (D13) as output & switch on to show that setup in progress
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	//Set motors pin modes
	pinMode(LMOTOR_DIR_PIN_1, OUTPUT);
	pinMode(LMOTOR_DIR_PIN_2, OUTPUT);
	pinMode(LMOTOR_PWM_PIN, OUTPUT); 
	pinMode(RMOTOR_DIR_PIN_1, OUTPUT);
	pinMode(RMOTOR_DIR_PIN_2, OUTPUT);
	pinMode(RMOTOR_PWM_PIN, OUTPUT);
	
	//Set ultrasonic sensor pin modes
	pinMode(LOBJSENSOR_TRIG, OUTPUT);
	pinMode(LOBJSENSOR_ECHO, INPUT);
	pinMode(ROBJSENSOR_TRIG, OUTPUT);
	pinMode(ROBJSENSOR_ECHO, INPUT);

	//Set line sensors pin mode
	pinMode(LINE_LEFT_POWER_PIN, OUTPUT);
	pinMode(LINE_RIGHT_POWER_PIN, OUTPUT);
	pinMode(LINE_LEFT_PIN, INPUT);
	pinMode(LINE_RIGHT_PIN, INPUT);

	//Set motors in forward direction
	dir_forward();

	while(true) {
		//Calibrates the line sensors for the current ambient lighting, then switch them on
		success = calibrate_line_sensors();
		//If the sensors calibrated successfully, then switch them on, otherwise repeat the calibration
		if(success == true) {
			digitalWrite(LINE_LEFT_POWER_PIN, HIGH);
			digitalWrite(LINE_RIGHT_POWER_PIN, HIGH);
			break;
		}
	}

	//Blinks LED to show setup completed sucessfully
	for(int i=0; i<3; i++) {
		digitalWrite(LED_BUILTIN, LOW);
		delay(500);
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500);
		digitalWrite(LED_BUILTIN, LOW);
	}
}


/********
Main Code
*********/

void loop() { //Main Control loop	
	line_check(); //Checks for lines

	 //Bound detection
	if(g_sensor_readings.line_left && g_sensor_readings.line_right) {
    //If both lines detected in the front, reverse
		Serial.println("Rev Escape");		
		reverse_escape();
	} else if(g_sensor_readings.line_left) {
    //If one line detected in the left, turn right
		Serial.println("Right Turn Side Escape");		
		turn_right();
	} else if(g_sensor_readings.line_right) {
    //If one line detected on the right, turn left
		Serial.println("Left Turn Side Escape");
		turn_left();
	} else {
    //If no bounds detected, search for opponent
		Serial.println("Updated Object Sensors");
		update_object_sensors();
		Serial.println("Search Attack");
		search_attack();
	}
}


void line_check() { //Checks for lines
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

void update_object_sensors() {
	// Updates the global struct with readings from the object sensors. If sight of
	// the enemy is lost, also updates the history of direction enemy was last seen in

	// Ultrasonics output HIGH pulse for the amount of time it takes the waves to reflect back
	// pulseIn measures that, and distance in mm is then calculated
	
  //Left utrasonic sensor
  digitalWrite(LOBJSENSOR_TRIG, LOW);
	delayMicroseconds(2);
	digitalWrite(LOBJSENSOR_TRIG, HIGH);
	delayMicroseconds(10);
	digitalWrite(LOBJSENSOR_TRIG, LOW);
	int Lduration = pulseIn(LOBJSENSOR_ECHO, HIGH);
	
  //Right ultrasonic sensor
	digitalWrite(ROBJSENSOR_TRIG, LOW);
	delayMicroseconds(2);
	digitalWrite(ROBJSENSOR_TRIG, HIGH);
	delayMicroseconds(10);
	digitalWrite(ROBJSENSOR_TRIG, LOW);
	int Rduration = pulseIn(ROBJSENSOR_ECHO, HIGH);
	
	float Ldistance = Lduration * 0.034/2;
	float Rdistance = Rduration * 0.034/2;
	
	// Updates history of direction enemy was last seen in
	if(min(Ldistance, Rdistance) > RING_SIZE && min(g_sensor_readings.object_left, g_sensor_readings.object_right) < RING_SIZE) {
		float left_right = g_sensor_readings.object_left - g_sensor_readings.object_right;
		if(left_right >= 0) {
			g_sensor_readings.last_seen_right = true;
		} else if(left_right < 0) {
			g_sensor_readings.last_seen_right = false;
		}
	}
	g_sensor_readings.object_left = Ldistance;
	g_sensor_readings.object_right = Rdistance;

	return;
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

	return;
}


void search_attack() {// Searches for the opponent, if opponent found to be in range will accelerate full speed at them

	if(min(g_sensor_readings.object_left, g_sensor_readings.object_right) > RING_SIZE){
		// the enemy is not in range, turn in the direction they were last seen
		if(g_sensor_readings.last_seen_right){
			g_motor_commands.motor_power = 60;
			turn_right();
			return;
		}else{
			g_motor_commands.motor_power = 60;
			turn_left();
			return;
		}
	}

	// If we haven't returned to the caller yet then the enemy is in range
	float left_minus_right = g_sensor_readings.object_left - g_sensor_readings.object_right;
	if(abs(left_minus_right) < 10){
		// If the enemy is pretty much in front of us, CHARGE!
		g_motor_commands.motor_power = 255;
		dir_forward();
		return;
	}

	// If we haven't charged then we need to turn to face the enemy better
	if(left_minus_right > 10){
		g_motor_commands.motor_power = 60;
		turn_right();
		return;
	} else {
		g_motor_commands.motor_power = 60;
		turn_left();
		return;
	}
}


void update_motor() {
	// logic to update output values to motor based on global variable values
	// Sets directions for both motors. true value means forward, false value means reverse
	int LMOTORPWR = 0;
	int RMOTORPWR = 0;

	if(g_motor_commands.turn == -1 && g_motor_commands.forward == true) {
		digitalWrite(LMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(LMOTOR_DIR_PIN_2, LOW);
		digitalWrite(RMOTOR_DIR_PIN_1, LOW);
		digitalWrite(RMOTOR_DIR_PIN_2, HIGH);
		LMOTORPWR = g_motor_commands.motor_power * LMOTOR_CORRECTION_FACTOR;
		RMOTORPWR = g_motor_commands.motor_power * RMOTOR_CORRECTION_FACTOR * RMOTOR_TURN_CORRECTION_FACTOR;
	} else if(g_motor_commands.turn == 1 && g_motor_commands.forward == true) {
		digitalWrite(LMOTOR_DIR_PIN_1, LOW);
		digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_2, LOW);
		LMOTORPWR = g_motor_commands.motor_power * LMOTOR_CORRECTION_FACTOR * LMOTOR_TURN_CORRECTION_FACTOR;
		RMOTORPWR = g_motor_commands.motor_power * LMOTOR_CORRECTION_FACTOR;
	} else if(g_motor_commands.turn == 0 && g_motor_commands.forward == true) {
		digitalWrite(LMOTOR_DIR_PIN_1, LOW);
		digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_1, LOW);
		digitalWrite(RMOTOR_DIR_PIN_2, HIGH);
		LMOTORPWR = g_motor_commands.motor_power * LMOTOR_CORRECTION_FACTOR;
		RMOTORPWR = g_motor_commands.motor_power * RMOTOR_CORRECTION_FACTOR;
	} else if(g_motor_commands.turn == 0 && g_motor_commands.forward == false) {
		digitalWrite(LMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(LMOTOR_DIR_PIN_2, LOW);
		digitalWrite(RMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_2, LOW);
		LMOTORPWR = g_motor_commands.motor_power * LMOTOR_CORRECTION_FACTOR;// + MOTORMIN;
		RMOTORPWR = g_motor_commands.motor_power * RMOTOR_CORRECTION_FACTOR;// + MOTORMIN;
	}
	// ERROR CODE 1
	else {
		for (int j = 0; j < 10; j++) {
			for (int i = 0; i < 5; i++) {
				digitalWrite(LED_BUILTIN, LOW);
				delay (500);
				digitalWrite(LED_BUILTIN, HIGH);
				delay(500);
				digitalWrite(LED_BUILTIN, LOW);
			}
			delay(2000);
		}
	}

  if (max(LMOTORPWR, RMOTORPWR) < 255 - MOTORMIN) {
      LMOTORPWR = LMOTORPWR + MOTORMIN;
      RMOTORPWR = RMOTORPWR + MOTORMIN;
  }

  Serial.print("Left Motor Power: ");
  Serial.println(LMOTORPWR);
  Serial.print("Right Motor Power: ");
  Serial.println(RMOTORPWR);
	
  analogWrite(LMOTOR_PWM_PIN, LMOTORPWR);
	analogWrite(RMOTOR_PWM_PIN, RMOTORPWR);
	
  return;
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

	//Calculate thresholds
	ambient_light_left = off_value_left / total_readings;
	ambient_light_right = off_value_right / total_readings;
	ring_surface_left = on_value_left / total_readings;
	ring_surface_right = on_value_right / total_readings;
	g_line_left_threshold = ceil((LINE_REFLECTION_MULTIPLIER * (ring_surface_left - ambient_light_left)) + ambient_light_left);
	g_line_right_threshold = ceil((LINE_REFLECTION_MULTIPLIER * (ring_surface_right - ambient_light_right)) + ambient_light_right);

	return (g_line_left_threshold > 1) && (g_line_right_threshold > 1); 	// return true if sucessful
}
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
#define LMOTOR_DIR_PIN_2 12  // swapped the pin definitions around because motors were running in reverese
#define LMOTOR_DIR_PIN_1 14
const float LMOTOR_CORRECTION_FACTOR = 0.35; // must be <=1
const float LMOTOR_TURN_CORRECTION_FACTOR = 1; // motors slower in reverse

#if LMOTOR_CORRECTION_FACTOR > 1
	#error Motor correction factors must be less than or equal to 1
#endif

// right motor
#define RMOTOR_PWM_PIN   15
#define RMOTOR_DIR_PIN_2 2  // swapped the pin definitions around because motors were running in reverese
#define RMOTOR_DIR_PIN_1 4
const float RMOTOR_CORRECTION_FACTOR = 0.35; // must be <=1
const float RMOTOR_TURN_CORRECTION_FACTOR = 1; // motors slower in reverse

#if RMOTOR_CORRECTION_FACTOR > 1
	#error Motor correction factors must be less than or equal to 1
#endif


// IR line sensors
#define LINE_LEFT_PIN 27
#define LINE_LEFT_POWER_PIN 26
#define LINE_RIGHT_PIN 25
#define LINE_RIGHT_POWER_PIN 33
#define LINE_LEFT_THRESHOLD 150  // based on manual callibration
#define LINE_RIGHT_THRESHOLD 150  // based on manual callibration

//Object sensors
#define RING_SIZE 770
#define OBJ_SENSOR_ALLOWABLE_DIFF 100

#define LOBJSENSOR_TRIG 16
#define LOBJSENSOR_ECHO 17

#define ROBJSENSOR_TRIG 5
#define ROBJSENSOR_ECHO 18

// Built-in LED
#define LED_BUILTIN 2


// Timings and power for turning etc
#define TURN_TIMING_MICROS 150
#define TURN_POWER 70
#define REVERSE_TIMING_MICROS 1500


/***************
Global Variables
****************/

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
	bool l_motor_dir = 1; // 1 = fwd, 0 = rev
	bool r_motor_dir = 1; // 1 = fwd, 0 = rev
	int l_motor_power = 0;
	int r_motor_power = 0;
} g_motor_commands;  // the structure name

/******************
Function Prototypes
*******************/
void line_check(void);
void update_object_sensors(void);
void update_motor(void);
void stop_motors(void);

/*********
Setup Code
**********/
void setup() {
  Serial.begin(115200); //Starts serial output

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

	//Power on to the line sensors
	digitalWrite(LINE_LEFT_POWER_PIN, HIGH);
	digitalWrite(LINE_RIGHT_POWER_PIN, HIGH);

	//Set motors in forward direction
	dir_forward();

	// warmup &  update sensors
	for(int i=0; i<5; i++){
		line_check();
		update_object_sensors();
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

	while true{
		// check for lines
		line_check();
		if(g_sensor_readings.line_left && g_sensor_readings.line_right) {
			// if both lines are sensed: reverse then turn
			stop_motors();
			g_motor_commands.l_motor_dir = 0;
			g_motor_commands.r_motor_dir = 0;
			g_motor_commands.l_motor_power = 120;
			g_motor_commands.r_motor_power = 120;
			update_motor();
			delayMicroseconds(REVERSE_TIMING_MICROS);
			stop_motors();
			g_motor_commands.l_motor_dir = 1;
			g_motor_commands.l_motor_power = TURN_POWER;
			g_motor_commands.r_motor_power = TURN_POWER;
			update_motor();
			delayMicroseconds(TURN_TIMING_MICROS);
			continue;
		} else if(g_sensor_readings.line_left) {
			// if line only detected on the left then turn right
			stop_motors();
			g_motor_commands.l_motor_dir = 1;
			g_motor_commands.r_motor_dir = 0;
			g_motor_commands.l_motor_power = TURN_POWER;
			g_motor_commands.r_motor_power = TURN_POWER;
			update_motor();
			delayMicroseconds(TURN_TIMING_MICROS);
			continue;
		} else if(g_sensor_readings.line_right) {
			// if line only detected on the right then turn left
			stop_motors();
			g_motor_commands.l_motor_dir = 0;
			g_motor_commands.r_motor_dir = 1;
			g_motor_commands.l_motor_power = TURN_POWER;
			g_motor_commands.r_motor_power = TURN_POWER;
			update_motor();
			delayMicroseconds(TURN_TIMING_MICROS);
			continue;
		}
    //If no lines detected, search for opponent
		update_object_sensors();
		// if the enemy is not in range, turn in the direction they were last seen
		if(min(g_sensor_readings.object_left, g_sensor_readings.object_right) > RING_SIZE){
			if(g_sensor_readings.last_seen_right){
				stop_motors();
				g_motor_commands.l_motor_dir = 1;
				g_motor_commands.r_motor_dir = 0;
				g_motor_commands.l_motor_power = TURN_POWER;
				g_motor_commands.r_motor_power = TURN_POWER;
				update_motor();
				delayMicroseconds(TURN_TIMING_MICROS);
				stop_motor();
				continue;
			}else{
				stop_motors();
				g_motor_commands.l_motor_dir = 0;
				g_motor_commands.r_motor_dir = 1;
				g_motor_commands.l_motor_power = TURN_POWER;
				g_motor_commands.r_motor_power = TURN_POWER;
				update_motor();
				delayMicroseconds(TURN_TIMING_MICROS);
				stop_motor();
				continue;
			}
		}

		// If the enemy is in range and they are ahead of us then charge
		float left_minus_right = g_sensor_readings.object_left - g_sensor_readings.object_right;
		if(abs(left_minus_right) < OBJ_SENSOR_ALLOWABLE_DIFF){
			// If the enemy is pretty much in front of us, CHARGE!
			g_motor_commands.l_motor_dir = 1;
			g_motor_commands.r_motor_dir = 1;
			g_motor_commands.l_motor_power = 255;
			g_motor_commands.r_motor_power = 255;
			update_motor();
			return;
		}

		// If the enemy is in range but we aren't facing them then turn to face them
		if(left_minus_right > OBJ_SENSOR_ALLOWABLE_DIFF){
			stop_motors();
			g_motor_commands.l_motor_dir = 1;
			g_motor_commands.r_motor_dir = 0;
			g_motor_commands.l_motor_power = TURN_POWER;
			g_motor_commands.r_motor_power = TURN_POWER;
			update_motor();
			delayMicroseconds(TURN_TIMING_MICROS);
			stop_motor();
			return;
		} else {
			stop_motors();
			g_motor_commands.l_motor_dir = 0;
			g_motor_commands.r_motor_dir = 1;
			g_motor_commands.l_motor_power = TURN_POWER;
			g_motor_commands.r_motor_power = TURN_POWER;
			update_motor();
			delayMicroseconds(TURN_TIMING_MICROS);
			stop_motor();
			return;
		}
	}


  // debugging
  /*
  Serial.print("object left: ");
  Serial.println(g_sensor_readings.object_left);
  Serial.print("object right: ");
  Serial.println(g_sensor_readings.object_right);
  Serial.print("last seen right?: ");
  Serial.println(g_sensor_readings.last_seen_right);
  Serial.print("line left: ");
  Serial.println(g_sensor_readings.line_left);
  Serial.print("line right: ");
  Serial.println(g_sensor_readings.line_right);
  Serial.print("turn (-1 left, 0 straight, 1 right): ");
  Serial.println(g_motor_commands.turn);
  Serial.print("forward? ");
  Serial.println(g_motor_commands.forward);
  Serial.print("motor power: ");
  Serial.println(g_motor_commands.motor_power);
  Serial.println();
  delay(1000);
  */
}


void stop_motors(){
	g_motor_commands.l_motor_power = 0;
	g_motor_commands.r_motor_power = 0;
	update_motors();
}


void line_check() { 
	//Update values for lines sensed from the QRE1113 **DIGITAL** breakout board sensors 
  //Lower numbers mean more refleacive, more than 3000 means nothing was reflected
  //(testing on black tape and white masking tape suggest anything below 150 is a white line)
  pinMode(LINE_LEFT_PIN, OUTPUT);
  digitalWrite(LINE_LEFT_PIN, HIGH);  
  delayMicroseconds(10);
  pinMode(LINE_LEFT_PIN, INPUT);
  long time = micros();
  //time how long the input is HIGH, but quit after 3ms as nothing happens after that
  while (digitalRead(LINE_LEFT_PIN) == HIGH && micros() - time < 3000);
  int diff = micros() - time;
	if(diff < LINE_LEFT_THRESHOLD){
		g_sensor_readings.line_left = true;
	} else {
		g_sensor_readings.line_left = false;
	}

  pinMode(LINE_RIGHT_PIN, OUTPUT);
  digitalWrite(LINE_RIGHT_PIN, HIGH);  
  delayMicroseconds(10);
  pinMode(LINE_RIGHT_PIN, INPUT);
  time = micros();
  //time how long the input is HIGH, but quit after 3ms as nothing happens after that
  while (digitalRead(LINE_RIGHT_PIN) == HIGH && micros() - time < 3000);
  diff = micros() - time;
	if(diff < LINE_RIGHT_THRESHOLD){
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


void update_motor() {
	// update direction of left motor
	if(g_motor_commands.l_wheel_dir = 1){
		digitalWrite(LMOTOR_DIR_PIN_1, LOW);
		digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
	}else{
		digitalWrite(LMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(LMOTOR_DIR_PIN_2, LOW);
	}
	// update direction of right motor
	if(g_motor_commands.r_wheel_dir = 1){
		digitalWrite(RMOTOR_DIR_PIN_1, LOW);
		digitalWrite(RMOTOR_DIR_PIN_2, HIGH);
	}else{
		digitalWrite(RMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_2, LOW);
	}
	// update power to motors
  analogWrite(LMOTOR_PWM_PIN, g_motor_commands.l_motor_power * LMOTOR_CORRECTION_FACTOR);
  analogWrite(RMOTOR_PWM_PIN, g_motor_commands.r_motor_power * RMOTOR_CORRECTION_FACTOR);

  return;
}

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
#define LMOTOR_DIR_PIN_1 12  // swapped the pin definitions around because motors were running in reverese
#define LMOTOR_DIR_PIN_2 14
const float LMOTOR_CORRECTION_FACTOR = 1; // must be <=1
const float LMOTOR_TURN_CORRECTION_FACTOR = 0.2; // motors slower in reverse

#if LMOTOR_CORRECTION_FACTOR > 1
	#error Motor correction factors must be less than or equal to 1
#endif

// right motor
#define RMOTOR_PWM_PIN   15
#define RMOTOR_DIR_PIN_2 32  // swapped the pin definitions around because motors were running in reverese
#define RMOTOR_DIR_PIN_1 4
const float RMOTOR_CORRECTION_FACTOR = 1; // must be <=1
const float RMOTOR_TURN_CORRECTION_FACTOR = 0.2; // motors slower in reverse

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
#define LINE_LEFT_THRESHOLD 2500  // based on manual callibration
#define LINE_RIGHT_THRESHOLD 2500  // based on manual callibration

//Object sensors
#define RING_SIZE 600

#define LOBJSENSOR_TRIG 16
#define LOBJSENSOR_ECHO 17

#define ROBJSENSOR_TRIG 5
#define ROBJSENSOR_ECHO 18

// Built-in LED
#define LED_BUILTIN 2

// START BUTTON
#define START_BUTTON 19

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
	int turn = 0; // -1 for left; 1 for right; 0 for straight.
	bool forward = true; // true for forward, false for reverse. Can only be in reverse if turn is set to 0, or straight! Not allowing a reverse turning possibility, since doing an on the spot turn anyways.
	int motor_power;
} g_motor_commands;  // the structure name

/******************
Function Prototypes
*******************/
void line_check(void);
void reverse_escape(void);
void single_side_escape(void);
void search_attack(void);
void update_object_sensors(void);
void update_motor(void);
void turn_left(void);
void turn_right(void);
void dir_forward(void);
void dir_reverse(void);

/*********
Setup Code
**********/
void setup() {
    Serial.begin(115200); //Starts serial output
    analogWrite(LMOTOR_PWM_PIN, 0);
    analogWrite(RMOTOR_PWM_PIN, 0);
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

    // Start Button
    pinMode(START_BUTTON, INPUT_PULLUP);

	//Power on to the line sensors
	digitalWrite(LINE_LEFT_POWER_PIN, HIGH);
	digitalWrite(LINE_RIGHT_POWER_PIN, HIGH);

	//Set motors in forward direction
	dir_forward();

    for (int i=0; i<5; i++) {
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

    while(true) {
        bool start_check = digitalRead(START_BUTTON);
        if (start_check == false) {
            break;
        }
    }

    g_motor_commands.motor_power = 80;
    update_motor();
    delay(400);
    g_motor_commands.motor_power = 0;
    update_motor();
}


/********
Main Code
*********/

void loop() { //Main Control loop	
	line_check(); //Checks for lines
    //update_object_sensors(); //debugging
	 //Bound detection
	if(g_sensor_readings.line_left && g_sensor_readings.line_right) {
    //If both lines detected in the front, reverse	
        //Serial.println("Reverse Escape");
		reverse_escape();
	} else if(g_sensor_readings.line_left) {
    //If one line detected in the left, turn right
        //Serial.println("Turning Right");
        left_side_escape();
	} else if(g_sensor_readings.line_right) {
    //If one line detected on the right, turn left
        //Serial.println("Turning Left");
        right_side_escape();
	} else {
    //If no bounds detected, search for opponent
		update_object_sensors();
        //Serial.println("Search Attack");
		search_attack();
	}

  // debugging
  
  /*Serial.print("object left: ");
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
  delay(5000);*/
  
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
  //Serial.print("Left:");
  Serial.println(diff);
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
  //Serial.print("Right:");
  Serial.println(diff);
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
	
	float Ldistance = Lduration * 0.34/2;
	float Rdistance = Rduration * 0.34/2;
	
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
		// revers set to high speed so can counter forward momentum if needed.
		delay(500);
		line_check();
	} while(g_sensor_readings.line_left && g_sensor_readings.line_right);
	if(g_sensor_readings.last_seen_right == true) {
		g_motor_commands.motor_power = 60;
		turn_right();
	} else {
		g_motor_commands.motor_power = 60;
		turn_left();
	}
	delay(200);
	g_motor_commands.motor_power = 0;
	dir_forward();
	return;
}

void left_side_escape() {
    do {
		g_motor_commands.motor_power = 100;
		dir_reverse();
		// revers set to high speed so can counter forward momentum if needed.
		delay(150);
		line_check();
	} while(g_sensor_readings.line_left && g_sensor_readings.line_right);
    g_motor_commands.motor_power = 60;
    turn_right();
    delay(200);
    return;
}

void right_side_escape() {
    do {
		g_motor_commands.motor_power = 100;
		dir_reverse();
		// revers set to high speed so can counter forward momentum if needed.
		delay(150);
		line_check();
	} while(g_sensor_readings.line_left && g_sensor_readings.line_right);
    g_motor_commands.motor_power = 60;
    turn_left();
    delay(200);
    return;
}


void search_attack() {// Searches for the opponent, if opponent found to be in range will accelerate full speed at them

	if(min(g_sensor_readings.object_left, g_sensor_readings.object_right) > RING_SIZE){
		// the enemy is not in range, turn in the direction they were last seen
		if(g_sensor_readings.last_seen_right){
			g_motor_commands.motor_power = 60;
			turn_right();
            delay(200);
            g_motor_commands.motor_power = 0;
            dir_forward();
            //Serial.println("Stopped");
            //delay(1000);
			return;
		}else{
			g_motor_commands.motor_power = 60;
			turn_left();
            delay(200);
            g_motor_commands.motor_power = 0;
            dir_forward();
            //Serial.println("Stopped");
            //delay(1000);
			return;
		}
	}

	// If we haven't returned to the caller yet then the enemy is in range
	float left_minus_right = g_sensor_readings.object_left - g_sensor_readings.object_right;
	if(abs(left_minus_right) < 100){
		// If the enemy is pretty much in front of us, CHARGE!
		g_motor_commands.motor_power = 255;
		dir_forward();
		return;
	}

	// If we haven't charged then we need to turn to face the enemy better
	if(left_minus_right > 100){
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
        // Left Turn
		digitalWrite(LMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(LMOTOR_DIR_PIN_2, LOW);
		digitalWrite(RMOTOR_DIR_PIN_1, LOW);
		digitalWrite(RMOTOR_DIR_PIN_2, HIGH);
		LMOTORPWR = g_motor_commands.motor_power * LMOTOR_CORRECTION_FACTOR * LMOTOR_TURN_CORRECTION_FACTOR;
		RMOTORPWR = g_motor_commands.motor_power * RMOTOR_CORRECTION_FACTOR * RMOTOR_TURN_CORRECTION_FACTOR;
	} else if(g_motor_commands.turn == 1 && g_motor_commands.forward == true) {
        // Right Turn
		digitalWrite(LMOTOR_DIR_PIN_1, LOW);
		digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_2, LOW);
		LMOTORPWR = g_motor_commands.motor_power * LMOTOR_CORRECTION_FACTOR * LMOTOR_TURN_CORRECTION_FACTOR;
		RMOTORPWR = g_motor_commands.motor_power * RMOTOR_CORRECTION_FACTOR * RMOTOR_TURN_CORRECTION_FACTOR;
	} else if(g_motor_commands.turn == 0 && g_motor_commands.forward == true) {
        // Forwards
		digitalWrite(LMOTOR_DIR_PIN_1, LOW);
		digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_1, LOW);
		digitalWrite(RMOTOR_DIR_PIN_2, HIGH);
		LMOTORPWR = g_motor_commands.motor_power * LMOTOR_CORRECTION_FACTOR;
		RMOTORPWR = g_motor_commands.motor_power * RMOTOR_CORRECTION_FACTOR;
	} else if(g_motor_commands.turn == 0 && g_motor_commands.forward == false) {
        // Reverse
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
    
    if (LMOTORPWR != 0 && RMOTORPWR !=0 && max(LMOTORPWR, RMOTORPWR) < 255 - MOTORMIN) {
        LMOTORPWR = LMOTORPWR + MOTORMIN;
        RMOTORPWR = RMOTORPWR + MOTORMIN;
    }
	
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

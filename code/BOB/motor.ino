#define LMOTOR_PWM_PIN 13
#define LMOTOR_DIR_PIN_1 12
#define LMOTOR_DIR_PIN_2 14
#define LMOTOR_CORRECTION_FACTOR 1

#define RMOTOR_PWM_PIN 32
#define RMOTOR_DIR_PIN_1 35
#define RMOTOR_DIR_PIN_2 34
#define RMOTOR_CORRECTION_FACTOR 1

int motorPower;
bool rmotor_dir;
bool lmotor_dir;

void direction_change() {
	if(rmotor_dir) {
		digitalWrite(RMOTOR_DIR_PIN_1, LOW);
		digitalWrite(RMOTOR_DIR_PIN_2, HIGH);
	}
	else {
		digitalWrite(RMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(RMOTOR_DIR_PIN_2, LOW);
	}
	if(lmotor_dir) {
		digitalWrite(LMOTOR_DIR_PIN_1, LOW);
		digitalWrite(LMOTOR_DIR_PIN_2, HIGH);
	}
	else {
		digitalWrite(LMOTOR_DIR_PIN_1, HIGH);
		digitalWrite(LMOTOR_DIR_PIN_2, LOW);
	}
}

void turn_right() {
	rmotor_dir = true;
	lmotor_dir = false;
	direction_change();
}

void turn_left() {
	rmotor_dir = false;
	lmotor_dir = true;
	direction_change();
}

void setup() {
	Serial.begin(9600);
	//Motor Output Pins
	pinMode (LMOTOR_DIR_PIN_1, OUTPUT);
	pinMode (LMOTOR_DIR_PIN_2, OUTPUT);
	pinMode (LMOTOR_PWM_PIN, OUTPUT);
	pinMode (RMOTOR_DIR_PIN_1, OUTPUT);
	pinMode (RMOTOR_DIR_PIN_2, OUTPUT);
	pinMode (RMOTOR_PWM_PIN, OUTPUT);
	//Motor Direction Forwards
	digitalWrite (LMOTOR_DIR_PIN_1, LOW);
	digitalWrite (LMOTOR_DIR_PIN_2, HIGH);
	digitalWrite (RMOTOR_DIR_PIN_1, LOW);
	digitalWrite (RMOTOR_DIR_PIN_2, HIGH);
	
}

void loop(){
	//High Speed LMotor Forwards
	motorPower = 255;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay (2000);
	//Low Speed LMotor Forwards
	motorPower = 50;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay (2000);
	//Stop LMotor
	motorPower = 0;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay(2000);
	
	//High Speed RMotor Forwards
	motorPower = 255;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay (2000);
	//Low Speed RMotor Forwards
	motorPower = 50;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay (2000);
	//Stop RMotor
	motorPower = 0;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay(2000);
	
	//Reverse Directions
	digitalWrite (LMOTOR_DIR_PIN_1, HIGH);
	digitalWrite (LMOTOR_DIR_PIN_2, LOW);
	digitalWrite (RMOTOR_DIR_PIN_1, HIGH);
	digitalWrite (RMOTOR_DIR_PIN_2, LOW);
	
	//Same routine as forwards
	motorPower = 255;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay (2000);
	motorPower = 50;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay (2000);
	motorPower = 0;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay(2000);
	
	motorPower = 255;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay (2000);
	motorPower = 50;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay (2000);
	motorPower = 0;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay(2000);
	
	turn_right();

	motorPower = 255;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	
	motorPower = 255;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	
	delay(2000);
	
	
	turn_left();
	
	motorPower = 255;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	
	motorPower = 255;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	
	delay(2000);
}
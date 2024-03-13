#define LMOTOR_PWM_PIN 13
#define LMOTOR_DIR_PIN_1 14
#define LMOTOR_DIR_PIN_2 12
#define LMOTOR_CORRECTION_FACTOR 1

#define RMOTOR_PWM_PIN 15
#define RMOTOR_DIR_PIN_1 2
#define RMOTOR_DIR_PIN_2 4
#define RMOTOR_CORRECTION_FACTOR 1

#define DELAY_TIME 5000

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
	rmotor_dir = false;
	lmotor_dir = true;
	direction_change();
}

void turn_left() {
	rmotor_dir = true;
	lmotor_dir = false;
	direction_change();
}

void setup() {
	Serial.begin(115200);
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
  digitalWrite (LMOTOR_DIR_PIN_1, LOW);
	digitalWrite (LMOTOR_DIR_PIN_2, HIGH);
	digitalWrite (RMOTOR_DIR_PIN_1, LOW);
	digitalWrite (RMOTOR_DIR_PIN_2, HIGH);
	//High Speed LMotor Forwards
  Serial.println("High Speed LMotor Forwards");
	motorPower = 255;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay (DELAY_TIME);
	//Low Speed LMotor Forwards
  Serial.println("Low Speed LMotor Forwards");
	motorPower = 50;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay (DELAY_TIME);
	//Stop LMotor
  Serial.println("Stop LMotor");
	motorPower = 0;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay(DELAY_TIME);
	
	//High Speed RMotor Forwards
  Serial.println("High Speed RMotor Forwards");
	motorPower = 255;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay (DELAY_TIME);
	//Low Speed RMotor Forwards
  Serial.println("Low Speed RMotor Forwards");
	motorPower = 50;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay (DELAY_TIME);
	//Stop RMotor
  Serial.println("Stop RMotor");
	motorPower = 0;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay(DELAY_TIME);
	
	//Reverse Directions
	digitalWrite (LMOTOR_DIR_PIN_1, HIGH);
	digitalWrite (LMOTOR_DIR_PIN_2, LOW);
	digitalWrite (RMOTOR_DIR_PIN_1, HIGH);
	digitalWrite (RMOTOR_DIR_PIN_2, LOW);
	
	//Same routine as forwards
  Serial.println("High Speed LMotor Backwards");
	motorPower = 255;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay (DELAY_TIME);
  Serial.println("Low Speed LMotor Backwards");
	motorPower = 50;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay (DELAY_TIME);
  Serial.println("Stop LMotor");
	motorPower = 0;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	delay(DELAY_TIME);
	
  Serial.println("High Speed RMotor Backwards");
	motorPower = 255;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay (DELAY_TIME);
  Serial.println("Low Speed RMotor Backwards");
	motorPower = 50;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay (DELAY_TIME);
  Serial.println("Stop RMotor");
	motorPower = 0;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	delay(DELAY_TIME);
	
  Serial.println("Turn Right Routine");
	turn_right();

  Serial.println("Stop Both Motor");
	motorPower = 0;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
  analogWrite (RMOTOR_PWM_PIN, motorPower);

  Serial.println("Set L & R Motors to Max");
	motorPower = 255;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	
	motorPower = 255;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	
	delay(DELAY_TIME);
	
  Serial.println("Stop Both Motor");
	motorPower = 0;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
  analogWrite (RMOTOR_PWM_PIN, motorPower);

	Serial.println("Turn Left Routine");
	turn_left();

  Serial.println("Set L & R Motors to Max");
	motorPower = 255;
	analogWrite (LMOTOR_PWM_PIN, motorPower);
	
	motorPower = 255;
	analogWrite (RMOTOR_PWM_PIN, motorPower);
	
	delay(DELAY_TIME);

  Serial.println("Stop Both Motors");
  motorPower = 0;
  analogWrite (RMOTOR_PWM_PIN, motorPower);
  analogWrite (LMOTOR_PWM_PIN, motorPower);

  delay(DELAY_TIME);

}
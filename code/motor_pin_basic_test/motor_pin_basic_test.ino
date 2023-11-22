#define LMOTOR_PWM_PIN 13
#define LMOTOR_DIR_PIN_1 12
#define LMOTOR_DIR_PIN_2 14

#define RMOTOR_PWM_PIN 32
#define RMOTOR_DIR_PIN_1 35
#define RMOTOR_DIR_PIN_2 34

#define DELAY_TIME 10000

void setup() {
  // put your setup code here, to run once:
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

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Pin 13, LMOTOR_PWM_PIN, high");
  analogWrite(LMOTOR_PWM_PIN, 255);

  delay(DELAY_TIME);

  Serial.println("Pin 12, LMOTOR_DIR_PIN_1, high");
  analogWrite(LMOTOR_DIR_PIN_1, HIGH);

  delay(DELAY_TIME);

  Serial.println("Pin 14, LMOTOR_DIR_PIN_2, high");
  analogWrite(LMOTOR_DIR_PIN_2, HIGH);

  delay(DELAY_TIME);



  Serial.println("Pin 32, RMOTOR_PWM_PIN, high");
  analogWrite(LMOTOR_PWM_PIN, 255);

  delay(DELAY_TIME);

  Serial.println("Pin 35, RMOTOR_DIR_PIN_1, high");
  analogWrite(RMOTOR_DIR_PIN_1, HIGH);

  delay(DELAY_TIME);

  Serial.println("Pin 34, RMOTOR_DIR_PIN_2, high");
  analogWrite(RMOTOR_DIR_PIN_2, HIGH);
}

/*

Ultrasonic Sensor Test

***********
Definitions
***********/

#define RING_SIZE 77

#define OBJSENSOR1_TRIG
#define OBJSENSOR1_ECHO

#define OBJSENSOR2_TRIG
#define OBJSENSOR2_ECHO

void setup() {
	pinMode(OBJSENSOR1_TRIG, OUTPUT);
	pinMode(OBJSENSOR2_TRIG, OUTPUT);
	pinMode(OBJSENSOR1_ECHO, INPUT);
	pinMode(OBJSENSOR2_ECHO, INPUT);
	
	digitalWrite(OBJSENSOR1_TRIG, LOW);
	digitalWrite(OBJSENSOR2_TRIG, LOW);
	delayMicroseconds(2);
	Serial.begin(115200);
}

void loop() {
	digitalWrite(OBJSENSOR1_TRIG, LOW);
	digitalWrite(OBJSENSOR2_TRIG, LOW);
	delayMicroseconds(2);
	
	digitalWrite(OBJSENSOR1_TRIG, HIGH);
	digitalWrite(OBJSENSOR2_TRIG, HIGH);
	delayMicroseconds(10);
	
	digitalWrite(OBJSENSOR1_TRIG, LOW);
	digitalWrite(OBJSENSOR2_TRIG, LOW);
	
	int Lduration = pulseIn(OBJSENSOR1_ECHO, HIGH);
	int Rduration = pulseIn(OBJSENSOR2_ECHO, HIGH);
	
	float Ldistance = Lduration * 0.034/2;
	float Rdistance = Rduration * 0.034/2;
	
	Serial.println(Ldistance, Rdistance);
	delay(1000);
}
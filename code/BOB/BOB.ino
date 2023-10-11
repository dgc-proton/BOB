void setup() {
  // put your setup code here, to run once:
	if health_check() == False {
		//LED?
	}
}

void loop() {
  // put your main code here, to run repeatedly:
	//linecheck fx
	//check return of linecheck if statement to steeraway fx
	//opponent_check return struct
	//if opponent detected go to turn_opponent fx else go to lpm_steer fx
	//power
}

bool health_check() {
	//Startup Health Check
}

int *line_check() {
	//IR Check, return array containing bool ir1 and ir2 that will go to steer away fx
}

void steer_away(){
	//skip through if fx in loop if line_check returns false
	//otherwise steer away or reverse and steer if needed likely through steer and power fx
}

struct opponent_check() {
	//if sensor hits within range of ring, return struct containing all snesors and their readings
}

void turn_opponent() {
	//turn through steer fx until minimum distance on front sensor to opponent is found, and then power fx
}

void lpm_steer() {
	//steer to find opponent, only go to this fx if opponent not detected in opponent_check
}
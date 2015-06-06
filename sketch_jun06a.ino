#include <Max3421e.h>
#include <Max3421e_constants.h>
#include <Usb.h>

#include <AndroidAccessory.h>
#include <SparkFunISL29125.h>
#include <Servo.h>
#include <Wire.h>

SFE_ISL29125 RGB_sensor;
unsigned int red, blue, green, red1, blue1, green1;
byte readdata[100];        //statetable data
int readsize;


struct State {
	int current_state;              // 0~n
	int input;                      //0,1
	int next_state;                 // 0~n
	int change;                     // 0,1
	int displace;                   // 0(L), 1(R)
};
struct State states[100];
int currentState;
int terminate;
Servo servo1, servo2;
AndroidAccessory acc("ph", "Turing_Machine", "lego turing machine", "1.0", " ", "0000000012345678");

int check() {
	red = RGB_sensor.readRed();
	green = RGB_sensor.readGreen();
	blue = RGB_sensor.readBlue();

	Serial.print("color value : ");
	Serial.println(red + green + blue);

	if (red + green + blue > 500) {
		return 0;
	} else return 1;
}

int FindState() {
	int chk = check();
	int i = 0;
	while (true) {
		if ((states[i].current_state == currentState) && (states[i].input == chk))
			break;
		else
			i++;
	}
	return i;
}

int moving() {
	int num = FindState();
	Serial.println("chang flag");
	if (states[num].input != states[num].change) {
		if (states[num].change == 0) {
			servo2.writeMicroseconds(1470);
			delay(462);
		} else {
			servo2.writeMicroseconds(1530);
			delay(612);
		}
		servo2.writeMicroseconds(1500);
                
	}

	Serial.println("moving tape");
	if (states[num].displace == 0) {                      // if grey
		servo1.writeMicroseconds(1550);
		delay(4660);
                
	} else {
		servo1.writeMicroseconds(1450);
		delay(4660);

	}
	servo1.writeMicroseconds(1500);
	delay(400);

	return num;
}

void ChangeState(int num) {
	currentState = states[num].next_state;
	Serial.print("current state : ");
	Serial.println(currentState);
}

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	Serial.println("Start");
	if (RGB_sensor.init()) {
		Serial.println("RGB Sensor Initialization Successful\n\r");
	}
	servo1.attach(3); // moving rail
	servo2.attach(6); // moving RGBsensor right
	acc.powerOn();
}


State state[100];
void loop() {
	// put your main code here, to run repeatedly:
	if (acc.isConnected()) {
		Serial.println("connected");
		readsize = acc.read(readdata, sizeof(readdata), 1);
                Serial.println(readsize);
		if (readsize > 0) {
			int i = 0;
			Serial.print(readdata[0]);
			Serial.print(", ");
			Serial.print(readdata[1]);
			Serial.print(", ");
			Serial.print(readdata[2]);
			Serial.println("");
			for (int j = 3; j < (10 * readdata[0]) + 3; j += 5) {
				state[i].current_state = readdata[j];
				state[i].input = readdata[j + 1];
				state[i].next_state = readdata[j + 2];
				state[i].change = readdata[j + 3];
				state[i].displace = readdata[j + 4];
				states[i] = state[i];
//				Serial.print(state[i].current_state);
//				Serial.print(", ");
//				Serial.print(state[i].input);
//				Serial.print(", ");
//				Serial.print(state[i].next_state);
//				Serial.print(", ");
//				Serial.print(state[i].change);
//				Serial.print(", ");
//				Serial.print(state[i].displace);
//				Serial.print(", ");
//				Serial.println("");
				i++;
			}
			currentState = readdata[1];
			terminate = readdata[2];
			
			Serial.println("before while");
			while (true) {
				if (currentState == terminate)
					break;
				Serial.println("before moving");
				int n = moving();

				Serial.print("moving : ");
				Serial.println(n);

				ChangeState(n);
			}
		}

	}
	delay(500);
}

#include <Servo.h>
#include <Max3421e.h>
#include <Max3421e_constants.h>
#include <Usb.h>

#include <AndroidAccessory.h>
#include <Wire.h>

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
AndroidAccessory acc("ph", "Turing_Machine", "lego turing machine", "1.0", " ", "0000000012345678");

Servo servo_rail, servo_head;
const byte pin_sensor1 = 9;
const byte pin_sensor2 = 2;
const byte pin_head_sensor = 10;
const byte pin_rail = 3;
const byte pin_head = 6;



int readQD(int pin) {
	//Returns value from the QRE1113 
	//Lower numbers mean more refleacive
	//More than 3000 means nothing was reflected.
	pinMode(pin, OUTPUT);
	digitalWrite(pin, HIGH);
	delayMicroseconds(10);
	pinMode(pin, INPUT);

	long time = micros();

	//time how long the input is HIGH, but quit after 3ms as nothing happens after that
	while (digitalRead(pin) == HIGH && micros() - time < 3000);
	int diff = micros() - time;

	return diff;
}



void rail_move(int speed) {
	int sensor1_val = readQD(pin_sensor1);
	int sensor2_val = readQD(pin_sensor2);
	boolean bool_sensor1 = sensor1_val < 1000 ? 1 : 0;
	boolean bool_sensor2 = sensor2_val < 1000 ? 1 : 0;


	//Serial.print("rail first while : ");
	//Serial.print(bool_sensor1);
	//Serial.print(" ");
	//Serial.print(bool_sensor2);
	//Serial.print(" ");
	//Serial.println(speed);
	//Serial.println("first while");
	while (bool_sensor1 | bool_sensor2) {
		sensor1_val = readQD(pin_sensor1);
		sensor2_val = readQD(pin_sensor2);
	//Serial.print(bool_sensor1);
	//Serial.print(" ");
	//Serial.print(bool_sensor2);
	//Serial.print(" ");
		bool_sensor1 = sensor1_val < 1000 ? 1 : 0;
		bool_sensor2 = sensor2_val < 1000 ? 1 : 0;
		servo_rail.writeMicroseconds(speed);
		delay(100);
	}

	//Serial.print("rail second while : ");
	///Serial.print(bool_sensor1);
	//Serial.print(" ");
	//Serial.print(bool_sensor2);
	//Serial.print(" ");
	//Serial.println(speed);
	//Serial.println("second while");
	while (!(bool_sensor1 | bool_sensor2)) {
		sensor1_val = readQD(pin_sensor1);
		sensor2_val = readQD(pin_sensor2);
		bool_sensor1 = sensor1_val < 1000 ? 1 : 0;
		bool_sensor2 = sensor2_val < 1000 ? 1 : 0;
		servo_rail.writeMicroseconds(speed);
		delay(100);
	}

	delay(500);
	servo_rail.writeMicroseconds(1500);
}

void head_move(int speed) {
	int headVal = readQD(pin_head_sensor);
	boolean head_bool = headVal < 1000 ? 1 : 0;

	//Serial.println(speed);

	//Serial.print("head first while : ");
	//Serial.print(headVal);
	//Serial.print(" ");
	//Serial.println(head_bool);
	//Serial.println("head first while");
	while (head_bool) {
		headVal = readQD(pin_head_sensor);
		head_bool = headVal < 1000 ? 1 : 0;
		servo_head.writeMicroseconds(speed);
		delay(20);
	}

	//Serial.print("head second while : ");
	//Serial.print(headVal);
	//Serial.print(" ");
	//Serial.println(head_bool);
	//Serial.println("head second while");
	while (!head_bool) {
		headVal = readQD(pin_head_sensor);
		head_bool = headVal < 1000 ? 1 : 0;
		servo_head.writeMicroseconds(speed);
		delay(20);
	}

	servo_head.writeMicroseconds(1500);
}



int check() {
	int diff = readQD(pin_sensor1);

	//Serial.print("diff value : ");
	//Serial.println(diff);

	if (diff > 1000) {
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
	//Serial.println("chang flag");
	if (states[num].input != states[num].change) {
		if (states[num].change == 0) {
			head_move(1480);

		} else {
			head_move(1520);
		}
	}

	if (states[num].displace == 0) {                      // if grey
		rail_move(1700);

	} else {
		rail_move(1300);
	}

	return num;
}

void ChangeState(int num) {
	currentState = states[num].next_state;
	//Serial.print("current state : ");
	//Serial.println(currentState);
}

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	Serial.println("Start");

	servo_rail.attach(pin_rail);
	servo_head.attach(pin_head);

	servo_rail.writeMicroseconds(1500);
	servo_head.writeMicroseconds(1500);
	delay(100);
	acc.powerOn();
}


State state[100];
void loop() {
	// put your main code here, to run repeatedly:
	if (acc.isConnected()) {
		//Serial.println("connected");
		readsize = acc.read(readdata, sizeof(readdata), 1);
		Serial.println(readsize);
		if (readsize > 0) {
			rail_move(1600);
			int i = 0;
			for (int j = 3; j < (10 * readdata[0]) + 3; j += 5) {
				state[i].current_state = readdata[j];
				state[i].input = readdata[j + 1];
				state[i].next_state = readdata[j + 2];
				state[i].change = readdata[j + 3];
				state[i].displace = readdata[j + 4];
				states[i] = state[i];
				//Serial.print(state[i].current_state);
				//Serial.print(", ");
				//Serial.print(state[i].input);
				//Serial.print(", ");
				//Serial.print(state[i].next_state);
				//Serial.print(", ");
				//Serial.print(state[i].change);
				//Serial.print(", ");
				//Serial.print(state[i].displace);
				//Serial.print(", ");
				//Serial.println("");
				i++;
			}
			currentState = readdata[1];
			terminate = readdata[2];

			//Serial.println("before while");
			while (true) {
				if (currentState == terminate)
					break;
				//Serial.println("before moving");
				int n = moving();
				//Serial.print("moving : ");
				//Serial.println(n);
				//delay(500);

				ChangeState(n);
			}
		}

	}
	delay(500);
}

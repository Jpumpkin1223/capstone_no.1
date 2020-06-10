#include <Stepper.h>
int state = 0;
int button_pin[4] = {10, 11, 12, 13};
int step_count = 0;
int onstage_3 = 2;

//////////these lines are for voltage check/////////////
float contact_coefficient = 0.96;
float dropoff_coefficient = 0.99;
float start_voltage;

///////these lines are for debouncing buttons///////////
boolean last_button_state[4] = {0, 0, 0, 0};
boolean button_state[4] = {0, 0, 0, 0};
boolean button_queue[4] = {0, 0, 0, 0};
unsigned long last_debounce[4] = {0, 0, 0, 0};
const int debounce_delay = 50;

///////////these lines are for step motor////////////////
const int stepsPerRevolution = 200; //200step = 360degree
Stepper myStepper(stepsPerRevolution, 4, 5, 6, 7);
int one_step = -2; // 2step = 0.04mm in z axis

////////////Activate state N ////////////////////////////
int scheduler(int);
int state0();
int state1();
int state2();
int state3();
int state4();
void debouncing_button(int);

void setup() {
  
  Serial.begin(115200);
  
  ////////////Activate input buttons//////////////////////
  for (int i = 1; i < 4; i++) {
    pinMode(button_pin[i], INPUT);
  }
  
  ////////////State 4 current cut/////////////////////////
  pinMode(onstage_3, OUTPUT);
  
  ////////////Set the RPM of Step motor///////////////////
  myStepper.setSpeed(60);     //set the motor speed in 60RPM
}

void loop() {
  for (int i = 0; i < 4; i++) {
    debouncing_button(i);
  }
  if (scheduler(state) == 1) {
    state++;
    if (state == 5) {
      state = 0;
    }
  }
  delay(20);
}

int scheduler(int i) {
  int done = 0;
  switch (i) {
    case 0:
      done = state0();
      break;
    case 1:
      done = state1();
      break;
    case 2:
      done = state2();
      break;
    case 3:
      done = state3();
      break;
    case 4:
      done = state4();
      break;
    default:
      break;
  }
  return done;
}


int state0() { 

  float voltage = analogRead(A0) * 25.00 / 1024.00;

  if (digitalRead(11) == LOW) {
    
    myStepper.step(-500);      //go down 1cm
    delay(50);
    return 0;
  }


  else if (digitalRead(12) == LOW) {
    myStepper.step(500);        //go up 1cm
    delay(50);
    return 0;
  }

  else if (digitalRead(10) == LOW) {
    start_voltage = analogRead(A0) * 25.00 / 1024.00;
    Serial.println(start_voltage, 4);
    Serial.println("STATE1");
    return 1;
  }

  else {
    myStepper.step(0);
    delay(50);
    return 0;
  }

}

int state1() {

  float voltage1 = analogRead(A0) * 25.00 / 1024.00; 
  
  if ((voltage1 < 2.48) && (voltage1 > 0.5)) {
    Serial.println("STATE2");
    return 1;
  }
  
  else {
    myStepper.step(one_step);
    step_count++; 
    delay(100);
    return 0;
  }

}

int state2() {
  
  myStepper.step(one_step * 50);
  step_count += 50;
  delay(50);

  Serial.println("STATE3");
  return 1;

}

int state3() {

  float voltage3 = analogRead(A0) * 25.00 / 1024.00;
  
  if (voltage3 > 2.47) { 
    myStepper.step(0);
    digitalWrite(onstage_3, HIGH);
    Serial.println("STATE4");
    return 1; 
  }
  
  else {
    myStepper.step(0);
    return 0;
  }

}

int state4() {

  myStepper.step(-(one_step) * 50);
  delay(50);
  step_count -= 50;

  while (step_count > 0) {
    myStepper.step(-one_step);
    step_count--; 
    delay(50);
  }
  digitalWrite(onstage_3, LOW);
  Serial.println("STATE0");
  return 1;
}

/////////////Debouncing Function//////////////////
void debouncing_button(int i) {
  int reading = digitalRead(button_pin[i]);
  if (reading != last_button_state[i])
    last_debounce[i] = millis();

  if ((millis() - last_debounce[i]) > debounce_delay) { 
    if (reading != button_state[i]) {
      button_state[i] = reading;
      if (button_state[i] == LOW) { 
        button_queue[i] = HIGH;
        Serial.print(i + 1);
        Serial.println(" button pressed");
      }
    }
  }
  last_button_state[i] = reading;
}

////////////////////////////////////////////////////////
///////////////////////// 6V  /// 4V ///////////////////
#include <Stepper.h>
int state = 0; //0: 시작 전 대기상태, 1: 내려가고있는 중, 2: 2미리 더 담그는 중 3: 정지 후 대기, 4: 엣칭 작업이 끝나고 올라가는 중
int button_pin[4] = {10, 11, 12, 13};
int step_count = 0;
////////////////////////////전압 차단//////////////////////
int onstage_3 = 2;

//////////these lines are for voltage check/////////////
float contact_coefficient = 0.98; //TODO -> 컨택 비율 찾아야함. 이 비율 이하면 접촉했다고 판단함.
float dropoff_coefficient = 0.98; //TODO -> 드롭옾 비율 찾아야함. 이 비율 이상이면 완료되었다고 판단함.
float start_voltage; //start 버튼 눌렀을때의 voltage 기록함.


///////these lines are for debouncing buttons///////////
boolean last_button_state[4] = {0, 0, 0, 0};
boolean button_state[4] = {0, 0, 0, 0};
boolean button_queue[4] = {0, 0, 0, 0};
unsigned long last_debounce[4] = {0, 0, 0, 0};
const int debounce_delay = 50;


///////////these lines are for step motor////////////////
const int stepsPerRevolution = 200;  // 42각 모터사용 200step 한바퀴 => 4mm 이동
Stepper myStepper(stepsPerRevolution, 4, 5, 6, 7);//4,5,6,7번핀 모터구동핀으로 사용
int one_step = -2; // 2step = 0.04mm

///////////these line is for PWM control/////////////////
unsigned int PWM_duty_ratio;

/////////////////////////////////////////////////////////
int scheduler(int);
int state0();
int state1();
int state2();
int state3();
int state4();
void debouncing_button(int);

void setup() {

  Serial.begin(115200);

  //////////////Activate input buttons//////////////////////
  for (int i = 1; i < 4; i++) { //버튼 핀 4개 인풋 활성화
    pinMode(button_pin[i], INPUT);
  }

  //////////////////////////State 4 current cut/////////////
  pinMode(onstage_3, OUTPUT);

  //////////////Set the RPM of Step motor///////////////////
  myStepper.setSpeed(60);//스텝모터 60rpm으로 지정
  /*
    //////////////Control PWM of the voltage//////////////////
    pinMode   (3, OUTPUT);
    TCCR0A  = (1 << WGM01) | (1 << WGM00); // FastPWM mode
    TCCR0A |= (1 << COM0A1);        // non-inverting
    TCCR0B  = (1 << CS02) | (1 << CS00); // prescaler 1024
    TIMSK0 |= (1 << TOIE0);
    OCR0A   = 50;
    PWM_duty_ratio = OCR0A * 100 / 256;
    sei();
  */

}

void loop() {
  for (int i = 0; i < 4; i++) {//버튼 눌렸는지 체크하는 부분
    debouncing_button(i);
  }
  float voltage = analogRead(A0) * 25.00 / 1024.00; //볼티지 센서 측정 전압값
  Serial.println(voltage, 4);
  if (scheduler(state) == 1) { //state 에 따라 작업을 할당하는 부분, 작업 끝나면 state +1
    state++;

    if (state == 5) {
      state = 0;
    }
  }
  delay(20);

}

int scheduler(int i) { //현재 state에 따라 알맞은 작업 할당
  int done = 0; //  state 작업이 다 끝났는지 확인하는 변수
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
int state0() { // state 0 : 시작하기 전에 대기하는 state

  //Serial.println("STATE0");
  float voltage = analogRead(A0) * 25.00 / 1024.00; //볼티지 센서 측정 전압값

  if (digitalRead(10) == LOW) { //버튼 눌려있으면
    start_voltage = analogRead(A0) * 25.00 / 1024.00; //현재 볼트 확인
    Serial.println(start_voltage, 4);
    Serial.println("STATE1");
    return 1; //state 벗어나기 위해 1 return
  }

  else { //버튼 안눌렸으면 버튼 눌릴 것 대기
    Serial.println(voltage, 4);
    myStepper.step(0);
    delay(50);
    return 0;
  }

}

int state1() { // state 1 : 내려가는 부분

  float voltage1 = analogRead(A0) * 25.00 / 1024.00; //현재 볼트 확인
  if ((voltage1 < start_voltage * contact_coefficient) && (voltage1 > 0.5)) { //볼트가 컨택보다 작아지면 접촉했겠죠?
    Serial.println("STATE2");
    return 1; //state 벗어나기 위해 1 return
  }
  else { //접촉하기 전이면
    myStepper.step(one_step);
    Serial.println(voltage1, 4);
    step_count++; //내려간거 기록
    delay(50); //TODO 내려가는 스텝에 맞게 시간 조정
    return 0;
  }

}

int state2() { // state 2 : 2미리 내려가는 중

  for (int i = 0; i < 50; i++) {
    myStepper.step(one_step);
    step_count++; //내려간거 기록
    delay(50); //TODO 내려가는 스텝에 맞게 시간 조정
  }
  Serial.println("STATE3");
  return 1; //state 벗어나기 위해 1 return

}

int state3() { // state 3 : 엣칭완료 기다리는 중

  float voltage3 = analogRead(A0) * 25.00 / 1024.00; //현재 볼트 확인
  if (voltage3 > start_voltage * dropoff_coefficient) { //볼트가 초기볼트로 돌아오면 끊긴 것, dropoff_coefficient로 safety margin
    myStepper.step(0);
    digitalWrite(onstage_3, HIGH);
    Serial.println("STATE4");
    return 1; //state 벗어나기 위해 1 return
  }
  else { //아니면 그냥 기다리기
    myStepper.step(0);
    return 0;
  }

}
int state4() { // state 4 : 올라가는 중

  while (step_count > 0) {
    myStepper.step(-one_step);
    step_count--; //올라간거 기록
    delay(50); //TODO 내려가는 스텝에 맞게 시간 조정
  }
  Serial.println("STATE0");
  return 1; //state 벗어나기 위해 1 return
}

///////////// 버튼 인식 부분 (debouncing) ////////////////
void debouncing_button(int i) {
  int reading = digitalRead(button_pin[i]);
  if (reading != last_button_state[i])  //스위치의 이전과 지금 상태가 다르면
    last_debounce[i] = millis();   //초를 기록합니다.

  if ((millis() - last_debounce[i]) > debounce_delay) { //스위치 상태가 debounce_delay 이상 같으면
    if (reading != button_state[i]) { //state와 상태가 다를경우 기록
      button_state[i] = reading;
      if (button_state[i] == LOW) { //LOW일때 작업 할당 -> pull-up 저항있음
        button_queue[i] = HIGH;
        Serial.print(i + 1); //몇번째(1번 부터) 버튼 눌렸는지 체크하는
        Serial.println(" button pressed");
      }
    }
  }
  last_button_state[i] = reading;
}

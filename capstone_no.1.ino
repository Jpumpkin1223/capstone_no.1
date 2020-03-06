//
int state = 0; //0: 시작 전 대기상태, 1: 내려가고있는 중, 2: 2미리 더 담그는 중 3: 정지 후 대기, 4: 엣칭 작업이 끝나고 올라가는 중
int button_pin[4] = {10, 11, 12, 13};
int step_count = 0;
//////////these lines are for voltage check/////////////
float contact_coefficient = 0.65; //TODO -> 컨택 비율 찾아야함. 이 비율 이하면 접촉했다고 판단함.
float dropoff_coefficient = 0.9; //TODO -> 드롭옾 비율 찾아야함. 이 비율 이상이면 완료되었다고 판단함.
float start_voltage; //start 버튼 눌렀을때의 voltage 기록함.
///////these lines are for debouncing buttons///////////
boolean last_button_state[4] = {0, 0, 0, 0};
boolean button_state[4] = {0, 0, 0, 0};
unsigned long last_debounce[4] = {0, 0, 0, 0};
const int debounce_delay = 50;
/////////////////////////////////////////////////////////
int scheduler(int i)
int state1();
int state2();
int state3();
int state4();
void debouncing_button(int);


void setup() {
  Serial.begin(115200);
  for (int i = 1; i < 4; i++) { //버튼 핀 4개 인풋 활성화
    pinMode(button_pin[i], INPUT);
  }
}

void loop() {
  for (int i = 0; i < 4; i++) {//버튼 눌렸는지 체크하는 부분
    debouncing_button(i);
  }

  float voltage = analogRead(A0) * 25 / 1023; //볼티지 센서 측정 전압값
  Serial.println(voltage);
  delay(200);
  scheduler(state);
  /* switch (state) {
     case label1:
       // statements
       break;
     case label2:
       // statements
       break;
     default:
       // statements
       break;
    }*/

}
int scheduler(int i) { //현재 state에 따라 알맞은 작업 할당
  int done = 0;
    switch (i) {
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
int state1() { // state 1 : 내려가는 부분
  float voltage = analogRead(A0) * 25 / 1023; //현재 볼트 확인
  if (voltage < start_voltage * contact_coefficient) { //볼트가 컨택보다 작아지면 접촉했겠죠?
    return 1; //state 벗어나기 위해 1 return

  }
  else { //접촉하기 전이면
    // TODO 한스텝 내려가는 동작
    step_count++; //내려간거 기록
    delay(10); //TODO 내려가는 스텝에 맞게 시간 조정
    return 0;
  }

}

int state2() { // state 2 : 2미리 내려가는 중
  for (i=0; i<100; i++){// TODO 내려가야하는 step 수 계산
    // TODO 내려가는 코드
    step_count++; //내려간거 기록
    delay(10); //TODO 내려가는 스텝에 맞게 시간 조정
  }
  return 1; //state 벗어나기 위해 1 return

}

int state3() { // state 3 : 엣칭완료 기다리는 중
  float voltage = analogRead(A0) * 25 / 1023; //현재 볼트 확인
  if (voltage > start_voltage * dropoff_coefficient) { //볼트가 초기볼트로 돌아오면 끊긴 것, dropoff_coefficient로 safety margin
    return 1; //state 벗어나기 위해 1 return

  }
  else {
    delay(100);
    return 0;
  }

}
int state4() { // state 4 : 올라가는 중
  while(step_count > 0){
    // TODO 내려가는 코드
    step_count--; //올라간거 기록
    delay(10); //TODO 내려가는 스텝에 맞게 시간 조정
  }
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
        Serial.print(i + 1); //몇번째(1번 부터) 버튼 눌렸는지 체크하는
        Serial.println(" button pressed");
      }
    }
  }
  last_button_state[i] = reading;
}

int state = 0; //0: 시작 전 대기상태, 1: 내려가고있는 중, 2: 정지 후 대기, 3: 다 되서 올라가는 중
int button_pin[4] = {10, 11, 12, 13};
///////these lines are for debouncing buttons///////////
boolean last_button_state[4] = {0, 0, 0, 0};
boolean button_state[4] = {0, 0, 0, 0};
unsigned long last_debounce[4] = {0, 0, 0, 0};
const int debounce_delay = 50;
/////////////////////////////////////////////////////////


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

///////////// 버튼 인식 부분 (debouncing) ////////////////
void debouncing_button(int i) {
  int reading = digitalRead(button_pin[i]);
  if (reading != last_button_state[i])  //스위치의 이전과 지금 상태가 다르면
    last_debounce[i] = millis();   //초를 기록합니다.

  if ((millis() - last_debounce[i]) > debounce_delay) { //스위치 상태가 debounce_delay 이상 같으면
    if (reading != button_state[i]) { //state와 상태가 다를경우 기록
      button_state[i] = reading;
      if (button_state[i] == LOW) { //LOW일때 작업 할당 -> pull-up 저항있음
        Serial.print(i + 1);
        Serial.println(" button pressed");
      }
    }
  }
  last_button_state[i] = reading;
}

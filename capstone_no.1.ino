int state=0; //0: 시작 전 대기상태, 1: 내려가고있는 중, 2: 정지 후 대기, 3: 다 되서 올라가는 중
int switch_pin[3]={7,8,9};


void setup() {
  // put your setup code here, to run once:
  for(i=1;i<3;i++){//스위치 핀 3개 인풋 활성화
    pinMode(switch_pin[i], INPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
  case label1:
    // statements
    break;
  case label2:
    // statements
    break;
  default:
    // statements
    break;
}

}

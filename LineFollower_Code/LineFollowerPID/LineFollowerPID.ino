#define A0 14
#define A1 27
#define A2 26
#define A3 25
#define A4 33
#define A5 32
#define A6 35
#define A7 34
#define AI1 0
#define AI2 2
#define BI1 4
#define BI2 16
#define PWMA 15
#define PWMB 17

int InArr[8] = {A0,A1,A2,A3,A4,A5,A6,A7};
int SArr[8];
int threshold[8];
int error = 0;
int lasterror = 0;
int correction = 0;
int weights[8] = {-4,-3,-2,-1,1,2,3,4};
int activeIR = 0;
bool inversionFlag = false;
int kp = 0;
int kd = 0;
int Lspeed;
int Rspeed;
int basespeed = 100;
int maxspeed = 200;
int minspeed = 40;

void rotate(){
  digitalWrite(AI1,HIGH);
  digitalWrite(AI2,LOW);
  digitalWrite(BI1,LOW);
  digitalWrite(BI2,HIGH);
  analogWrite(PWMA,100);
  analogWrite(PWMB,100);
}

void stopmotors(){
  digitalWrite(AI1,LOW);
  digitalWrite(AI2,LOW);
  digitalWrite(BI1,LOW);
  digitalWrite(BI2,LOW);
  analogWrite(PWMA,0);
  analogWrite(PWMB,0);
}

void calibrate(){
  int minVal[8] = {analogRead(A0),analogRead(A1),analogRead(A2),analogRead(A3),analogRead(A4),analogRead(A5),analogRead(A6),analogRead(A7)};
  int maxVal[8] = {analogRead(A0),analogRead(A1),analogRead(A2),analogRead(A3),analogRead(A4),analogRead(A5),analogRead(A6),analogRead(A7)};

  for(int i = 0; i < 200; i++){
    for(int j = 0; j < 8; j++){
      int temp = analogRead(InArr[j]);
      if(temp < minVal[j]){
        minVal[j] = temp;
      }
      if(temp > maxVal[j]){
        maxVal[j] = temp;
      }
    }
    rotate();
    delay(50);
  }

  for(int i = 0; i < 8; i++){
    threshold[i] = (minVal[i] + maxVal[i])/2;
  }
  stopmotors();
  delay(3000);
}

void sensorhl(){
  if(!inversionFlag){
    for(int i = 0; i < 8; i++){
      if(analogRead(InArr[i]) >= threshold[i]){
        SArr[i] = 1;
      }
      else{
        SArr[i] = 0;
      }
    }
  }
  else{
    for(int i = 0; i < 8; i++){
      if(analogRead(InArr[i]) < threshold[i]){
        SArr[i] = 1;
      }
      else{
        SArr[i] = 0;
      }
    }
  }
}

void blackwhite(){
  if(SArr[0] == 1 && SArr[1] == 1 && SArr[6] == 1 && SArr[7] == 1){
    inversionFlag = true;
  }
  else if(SArr[0] == 0 && SArr[1] == 0 && SArr[6] == 0 && SArr[7] == 0){
    inversionFlag = false;
  }
}

void motor(int Lspeed,int Rspeed){
  Lspeed = constrain(Lspeed,-maxspeed,maxspeed);
  Rspeed = constrain(Rspeed,-maxspeed,maxspeed);

  if(Rspeed >= 0){
    digitalWrite(AI1,HIGH);
    digitalWrite(AI2,LOW);
  }
  else{
    digitalWrite(AI1,LOW);
    digitalWrite(AI2,HIGH);
    Rspeed = -Rspeed;
  }

  if(Lspeed >= 0){
    digitalWrite(BI1,HIGH);
    digitalWrite(BI2,LOW);
  }
  else{
    digitalWrite(BI1,LOW);
    digitalWrite(BI2,HIGH);
    Lspeed = -Lspeed;
  }

  analogWrite(PWMA,Rspeed);
  analogWrite(PWMB,Lspeed);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
  pinMode(A5,INPUT);
  pinMode(A6,INPUT);
  pinMode(A7,INPUT);

  pinMode(AI1,OUTPUT);
  pinMode(AI2,OUTPUT);
  pinMode(BI1,OUTPUT);
  pinMode(BI2,OUTPUT);
  pinMode(PWMA,OUTPUT);
  pinMode(PWMB,OUTPUT);

  calibrate();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  activeIR = 0;
  error = 0;
  sensorhl();
  blackwhite();

  for(int i = 0; i < 8; i++){
    if(SArr[i] == 1){
      error = error + weights[i];
      activeIR++;
    }
  }

  if(activeIR == 0){
    rotate(); //searchline
  }

  int P = error;
  int D = error - lasterror;

  correction = (kp*P) + (kd*D);

  Lspeed = basespeed - correction;
  Rspeed = basespeed + correction;

  if(Lspeed > 0){
    if(Lspeed < 40){
      Lspeed = 40;
    }
  }
  else if(Lspeed < 0){
    if(Lspeed > -40){
      Lspeed = -40;
    }
  }

  if(Rspeed > 0){
    if(Rspeed < 40){
      Rspeed = 40;
    }
  }
  else if(Rspeed < 0){
    if(Rspeed > -40){
      Rspeed = -40;
    }
  }

  motor(Lspeed,Rspeed);
  lasterror = error;
}

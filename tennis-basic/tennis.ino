#define L1 2
#define L2 3
#define L3 4
#define L4 5
#define L5 6
#define L6 7
#define L7 8
#define L8 9
#define S1 11
#define S2 12
#define SPEAKER 10

const byte ledsOffset = 1;
const byte ledsCount = 8;
const unsigned short bootMelody[] = {37, 44, 48, 51, 55, 58, 58, 58};
byte starter = 1;
byte status = 0; //0:ルーレット, 1:サーブ待ち, 2:ラリー中, 3:得点, 4：決着
unsigned long ms = 0;
byte position;
byte lastPosition = 0;
unsigned long lastTime = 0;
bool startBlink = false;
bool s1Push = false;
unsigned long s1Timer = 0;
byte s1Points = 0;
bool s2Push = false;
unsigned long s2Timer = 0;
byte s2Points = 0;
float velocity = 0.5;
const byte failMelody[] = {98, 87.3, 73.4, 61.7};
const byte pointsMelody[] = {52, 56, 59, 64};
byte resultBar = 0;
const unsigned short resultMelody[] = {69, 71, 73, 75, 73, 75, 76, 0, 71, 68, 1, 71, 69, 71, 73, 75, 73, 75, 76, 1, 1, 1, 0, 64, 69, 71, 73, 75, 73, 75, 76, 0, 71, 68, 1, 83, 81, 1, 80, 78, 1, 80, 76, 1, 1, 88, 0};
bool resultSound = true;

void on(int pin){
  digitalWrite(pin, HIGH);
}

void off(int pin){
  digitalWrite(pin, LOW);
}

void setAll(bool value){
  for(byte i=1+ledsOffset;i<=ledsCount+ledsOffset;i++){
    digitalWrite(i, value);
  }
}

void setup(){
  randomSeed(analogRead(A0));
  starter = random(0, 100)<50 ? 0 : 1;
  Serial.begin(9600);
  for(byte i=1+ledsOffset;i<=ledsCount+ledsOffset;i++){
    pinMode(i, OUTPUT);
  }
  pinMode(S1, INPUT_PULLUP);
  pinMode(S2, INPUT_PULLUP);
  pinMode(SPEAKER, OUTPUT);
  for(byte i=0;i<sizeof(bootMelody)/2;i++){
    tone(SPEAKER, 27.5*pow(1.059463094, bootMelody[i]));
    on(ledsCount-i+ledsOffset);
    delay(31);
  }
  delay(425);
  for(byte i=0;i<=ledsCount;i++){
    off(ledsCount-i+ledsOffset);
    on(ledsCount-i+ledsOffset+1);
    delay(50);
  }
  delay(425);
  noTone(SPEAKER);
  delay(250);
  setAll(LOW);
  on(ledsCount+ledsOffset);
  tone(SPEAKER, 27.5*pow(1.059463094, 63));
  delay(50);
  tone(SPEAKER, 27.5*pow(1.059463094, 75));
  for(byte i=1;i<=ledsCount-1;i++){
    on(ledsCount-i+ledsOffset);
    off(ledsCount-i+ledsOffset+1);
    delay(80);
  }
  delay(500);
  noTone(SPEAKER);
  setAll(LOW);
  delay(2000);
}

void loop(){
  ms = millis();
  if(s1Push != !digitalRead(S1)){
    s1Push = !digitalRead(S1);
    if(s1Push){
      tone(SPEAKER, 784);
      s1Timer = ms;
    }
  }
  
  if(s2Push != !digitalRead(S2)){
    s2Push = !digitalRead(S2);
    if(s2Push){
      tone(SPEAKER, 784);
      s2Timer = ms;
    }
  }
  if((status==4 && resultBar > sizeof(resultMelody)/2) || status != 4){
    if(ms-s1Timer>50 && ms-s2Timer>50){
      noTone(SPEAKER);
    }else if(25<ms-s1Timer && 25<ms-s2Timer) tone(SPEAKER, 1047);
  }

  switch(status){
    case 0:
      noTone(SPEAKER);
      s1Points = 0;
      s2Points = 0;
      resultBar = 0;
      resultSound = true;
      for(byte i=50;i <= (starter ? 205 : 200);i+=5){
        tone(SPEAKER, 440, 50);
        for(byte j=1+ledsOffset;j<=floor(ledsCount/2)+ledsOffset;j++){
          i%2 ? on(j) : off(j);
        }
        for(byte j=ledsCount+ledsOffset;j>floor(ledsCount/2)+ledsOffset;j--){
          i%2 ? off(j) : on(j);
        }
        delay(50);
        noTone(SPEAKER);
        delay(i-50);
      }
      delay(1000);
      setAll(LOW);
      status = 1;
      delay(500);
      break;
    
    case 1:
      if(ms-lastTime > 500){
        if(startBlink){
          off((starter ? 1 : ledsCount)+ledsOffset);
          startBlink = false;
        }else{
          on((starter ? 1 : ledsCount)+ledsOffset);
          startBlink = true;
        }
        lastTime = ms;
      }
      if(starter ? s1Push : s2Push){
        status = 2;
        position = starter ? 1 : ledsCount;
        velocity = starter ? 1 : -1;
        lastTime = ms;
      }
      break;
    
    case 2:
      on(position + ledsOffset);
      if(lastPosition) off(lastPosition + ledsOffset);
      if((position==1 && ms-s1Timer<100 && velocity<0) || (position==ledsCount && ms-s2Timer<200 && velocity>0)){
        if(position==1 ? ms-s1Timer : ms-s2Timer <= 10 && velocity >= 1){
          velocity = velocity<0 ? 2 : -2;
        }else if(position==1 ? ms-s1Timer : ms-s2Timer <= 50 && velocity >= 1){
          velocity *= -1.05;
        }else if(position==1 ? ms-s1Timer : ms-s2Timer <= 100){
          velocity = velocity<0 ? 1 : -1;
        }else velocity = velocity<0 ? 0.7 : -0.7;
      }
      if(ms-lastTime > 1000/(ledsCount*abs(velocity))){
        lastPosition = position;
        velocity>0 ? position++ : position--;
        lastTime = ms;
      }
      if(position < 1 || position > ledsCount){
        position < 1 ? s2Points++ : s1Points++;
        setAll(LOW);
        for(byte i=0;i<sizeof(failMelody);i++){
          tone(SPEAKER, failMelody[i]);
          delay(50);
          noTone(SPEAKER);
        }
        status = 3;
      }
      break;
    
    case 3:
      delay(500);
      for(byte i=0;i<floor(ledsCount/2);i++){
        if(!constrain(s1Points-i, 0, floor(ledsCount/2)) && !constrain(s2Points-i, 0, floor(ledsCount/2))) break;
        if(constrain(s1Points-i, 0, floor(ledsCount/2))){
          on(1+i+ledsOffset);
        }
        if(constrain(s2Points-i, 0, floor(ledsCount/2))){
          on(ledsCount-i+ledsOffset);
        }
        tone(SPEAKER, 27.5*pow(1.059463094, pointsMelody[i]-1));
        delay(50);
        noTone(SPEAKER);
      }
      delay(3000);
      for(int i=1;i<=ledsCount;i++){
        off(i+ledsOffset);
      }
      starter = starter ? 0 : 1;
      status = s1Points>=floor(ledsCount/2) || s2Points>=floor(ledsCount/2) ? 4 : 1;
      break;
    
    case 4:
      if(ms-lastTime > 125){
        if(resultSound){
          if(resultMelody[resultBar] > 1){
            if(resultMelody[resultBar]==resultMelody[resultBar-1]){
              noTone(SPEAKER);
              delay(5);
            }
            tone(SPEAKER, 27.5*pow(1.059463094, resultMelody[resultBar]-13));
          }else if(resultMelody[resultBar] == 0){
            noTone(SPEAKER);
          }
          if(resultBar>=sizeof(resultMelody)/2) resultSound = false;
        }
        if(s1Points>=floor(ledsCount/2)){
          if(resultBar % 6 >= 3){
            on(L1);
            off(L2);
            on(L3);
            off(L4);
          }else{
            off(L1);
            on(L2);
            off(L3);
            on(L4);
          }
        }
        if(s2Points>=floor(ledsCount/2)){
          if(resultBar % 6 >= 3){
            on(L5);
            off(L6);
            on(L7);
            off(L8);
          }else{
            off(L5);
            on(L6);
            off(L7);
            on(L8);
          }
        }
        resultBar++;
        lastTime = ms;
      }
      if(s1Push || s2Push) status = 0;
      break;
  }
}

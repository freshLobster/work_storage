//Pins on metro
#define audio 5
#define reset 4
#define pin1 8
#define pin2 7
#define pin3 6
#define eye0 "big_blue"
#define eye1 "doom_spiral"
#define eye2 "hypno_red"
#define eye3 "toonstripe"

char val = '0';

//clears the pin states back to default
void clearState(){
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, LOW);
  digitalWrite(pin3, LOW);
}

void setState(int state){
  String selection = "";
  switch (state){

    case 0:
      clearState();
      selection = eye0;
      digitalWrite(reset, LOW);
      break;
    case 1:
      clearState();
      selection = eye1;
      digitalWrite(pin1, HIGH);
      digitalWrite(reset, LOW);
      break;
    case 2:
      clearState();
      selection = eye2;    
      digitalWrite(pin2, HIGH);
      digitalWrite(reset, LOW);
      break;
    case 3:
      clearState();
      selection = eye3;    
      digitalWrite(pin3, HIGH);
      digitalWrite(reset, LOW);
      break;
    case 4:
      selection = "sound fx";
      digitalWrite(audio, HIGH);
      delay(30);
      digitalWrite(audio, LOW);
    default:
      break;

  }
  Serial.println(selection);
  digitalWrite(reset, HIGH);
  
}

void setup()
{
  Serial.begin(9600);//Opening serial port
  //set pin modes
  pinMode(audio, OUTPUT);
  pinMode(reset, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  digitalWrite(reset, HIGH);

}
void loop()
{
  if (Serial.available())//If connected
  {
    val = Serial.read();
    //convert char to int
    int state = val - '0';
    
    setState(state);
    
    
  }
  //digitalWrite(reset, HIGH);
}

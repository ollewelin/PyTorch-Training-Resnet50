#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int val;    // variable to read the value from the analog pin

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);
}
#define CLOSE_TIME 60 //Example 30 = 60 seconds close time
void loop() {
int blink_timer =0 ;
int i=0;
  while(1)
  {
    if(digitalRead(12)==HIGH){
      //Close cat flap servo 10 minutes
      myservo.write(130);
      digitalWrite(13, HIGH);
      for(i=0;i<CLOSE_TIME;i++) {
        digitalWrite(13, LOW);
        delay(1000);
        digitalWrite(13, HIGH);
        delay(1000);
      }
    }
    else
    {
      myservo.write(30);
      blink_timer++;
      if(blink_timer > 3000){
        blink_timer =0;
      }
      else
      {
        if(blink_timer < 1500){
        digitalWrite(13, LOW);
        }
        else
        {
          digitalWrite(13, HIGH);
        }
      }
    }
  }
}
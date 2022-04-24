#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int val;    // variable to read the value from the analog pin

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);
}
#define CLOSE_TIME 90 //Example 30 = 60 seconds close time


void loop() {
int blink_timer =0 ;
int state_close = 1;
int i=0;
  while(1)
  {
    if(digitalRead(12)==HIGH){
      //Close cat flap servo 10 minutes
      //Move servo back and forth a little bit to reduce hum sound if mechanic are sticky  
      myservo.write(135);
      delay(2500);
      myservo.write(130);
 //     delay(1000);
 //     myservo.write(130);
      state_close = 1;
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
      if(state_close == 1)
      {
        //Move servo back and forth a little bit to reduce hum sound if mechanic are sticky      
        myservo.write(20);
        delay(3000);
        myservo.write(40);
        delay(1000);
        myservo.write(30);
        delay(1000);
        myservo.write(35);
        delay(1000);
        

     //   myservo.write(33);
     //   delay(1000);
      }
      
      myservo.write(33);
      state_close = 0;
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

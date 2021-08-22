/*
All credit for this code belongs to grafalex (https://www.thingiverse.com/thing:1258082). 

Thank you very much for sharing this.

*/

#include <Servo.h> 
 
Servo myservo;
 
int min_pos = 10;
int max_pos = 180;

unsigned long last_click_timestamp = 0;
int fast_clicks = 0;
bool tease_mode = false;
 
void setup() 
{ 
  myservo.attach(2);
  myservo.write(min_pos);
  
  pinMode(4, INPUT_PULLUP);

  randomSeed(analogRead(0));

  Serial.begin(115200);
  Serial.println("Setup done");
  
  last_click_timestamp = millis();
} 

bool move_servo(int end_pos, int step_delay, bool look_for_switch)
{
  int cur_pos = myservo.read();
  
//  Serial.print("Moving to ");
//  Serial.print(end_pos);
//  Serial.print(" delay = ");
//  Serial.println(step_delay);

  // Check if we are already there
  if(cur_pos == end_pos)
  {
//    Serial.println("Already there");
    return true;
  }

  // Move from current position to the end position  
  int servo_step = cur_pos < end_pos ? 1 : -1;
  bool switching_state = (cur_pos >= end_pos);
  for(; cur_pos != end_pos; cur_pos += servo_step)
  {                                
    myservo.write(cur_pos);
    delay(step_delay);

    // If servo is moving forward - expect for switch off
    // if moving backward - switch on will break the loop
    if(look_for_switch && (digitalRead(4) == switching_state))
    {
//      Serial.println("Switch change detected");
      return false;
    }
  } 
  
//  Serial.println("Move done");
  return true;
}

void tease()
{
  const int tease_min_pos = 90;
  const int tease_max_pos = 120;

  Serial.println("Teasing!");
  
  move_servo(tease_min_pos, random(4, 6), false);
  delay(200 + random(2000));
  
  do
  {
    Serial.println("Tease loop!");

    if(digitalRead(4)) // User again clicked the switch
      return;
    
    move_servo(tease_max_pos, random(2, 6), false);
    delay(random(1000)); 
    move_servo(tease_min_pos, random(4, 6), false);
    delay(200 + random(2000));  
  }
  while(random(5) > 2);

  if(digitalRead(4)) // User again clicked the switch
    return;
  
  Serial.println("Exiting tease loop!");
  move_servo(min_pos, random(4, 6), false);
  
  // No longer tease
  if(random(5) > 2)
  {
    Serial.println("No longer tease!");
    tease_mode = false;
  }
}

void check_for_bothering()
{
  const int fast_click_threshold = 2000; //2 seconds
  const int clicks_for_tease_mode = 4;

  unsigned long cur_timestamp = millis();
  
  // Check whether previous click was not too far ago
  if(cur_timestamp - last_click_timestamp > fast_click_threshold)
  {
    // If last click was to far back (normal user) - reset the clicks counter
    Serial.println("Slow Click!");
    fast_clicks = 0;
  }
  else
  {
    // Our user is nerd of clicking
    Serial.println("Fast Click!");
    fast_clicks++;
  }
  
  last_click_timestamp = cur_timestamp;
  
  // If user bother us with clicking - enter tease mode
  if(fast_clicks > clicks_for_tease_mode)
  {
    Serial.println("Tease mode");
    tease_mode = true;
  }
}

bool should_tease()
{
  const int tease_threshold = 4000; // 5 seconds after the last click

  unsigned long cur_timestamp = millis();

  // Check whether there was no action during last few seconds so we may tease
  if(tease_mode && (cur_timestamp - last_click_timestamp > tease_threshold))
    return true;
    
  return false;
}

void loop() 
{ 
  int speed1 = random(2, 5);
  int speed2 = random(2, 8);

  if(should_tease())
    tease();

  //TODO Sleep while not turned on
  // Wait for click
  if(!digitalRead(4))
    return;
    
  //Serial.println("Turned on!");
  
  check_for_bothering();

  // Move forward until switch is off  
  move_servo(max_pos, speed1, true);

  //Serial.println("Turned off");
  
  // Now move backward until min pos or user clicked switch again
  move_servo(min_pos, speed2, true);
  
  //Serial.println("End of lopp");
} 

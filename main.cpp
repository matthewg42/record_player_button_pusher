#include <Arduino.h>
#include <Servo.h> 
 
Servo sg90;

#define PIR_PIN				A1
#define LED_PIN				13
#define SERVO_PIN			11	
#define REST_POS			40
#define PRESS_POS			63
#define PRESS_DELAY_MS		100
#define MOVE_DELAY_MS		15
#define REST_DELAY_SEC		(8*60)	
#define HEARTBEAT_DELAY 	2000
 
int pos;
unsigned long last_heartbeat = 0;

// Move arm to position where switch is pressed
void depress()
{
	digitalWrite(LED_PIN, HIGH);
	sg90.attach(SERVO_PIN);
    sg90.write(REST_POS);
    for (int i=REST_POS; i<=PRESS_POS; i++) {
        delay(MOVE_DELAY_MS);
        sg90.write(i);
    }   
}

// Move arm back to rest position
void unpress()
{
	sg90.attach(SERVO_PIN);
    for (int i=PRESS_POS; i>=REST_POS; i--) {
        delay(MOVE_DELAY_MS);
        sg90.write(i);
    }   
	sg90.detach();
	digitalWrite(LED_PIN, LOW);
}

// Press the button on the record player
void press()
{
	depress();
	delay(PRESS_DELAY_MS);
	unpress();
}

// on and off for times in ms
void blink(int on=5, int off=95)
{
	digitalWrite(LED_PIN, HIGH);
	delay(on);
	digitalWrite(LED_PIN, LOW);
	delay(off);
}

// Do the heartbeat display (potentially with a double flash)
void heartbeat_flash(bool double_flash)
{
	if (double_flash) {
		blink(2, 100);
		blink(2, 0);
	}
	else {
		blink(2, 0);
	}
}

// Give a quick flash of the indicator LED if it hasn't been 
// flashed for a while to show that the code is still running.
void heartbeat(bool double_flash=false)
{
	unsigned long now = millis();
	if (last_heartbeat + HEARTBEAT_DELAY < now) {
		heartbeat_flash(double_flash);	
		last_heartbeat = now;
	}
	else if (now < last_heartbeat) { // handle millis wrap 
		heartbeat_flash(double_flash);	
		last_heartbeat = now;
	} 
}

void hb_delay(int seconds)
{
	for(int i=0; i<seconds*10; i++) {
		delay(100);
		heartbeat(false);
	}
}

bool pir_activated()
{
	return analogRead(PIR_PIN) > 100;
}

// Run when Arduino is reset / powered on
void setup() 
{ 
	// Setup LED and turn it off
	pinMode(LED_PIN, OUTPUT);
	pinMode(SERVO_PIN, OUTPUT);
	pinMode(PIR_PIN, INPUT);

	digitalWrite(LED_PIN, LOW);

	// reset in case we're not in the rest position
	sg90.attach(SERVO_PIN);
	sg90.write(REST_POS);
	sg90.detach();
	
	// Let the PIR sensor settle down - taking some readings seems
	// to help this - perhaps clearing the analog state of the pin...
	for (int i=0; i<3; i++) {
		digitalWrite(LED_PIN, HIGH);
		pir_activated();
		digitalWrite(LED_PIN, LOW);
		delay(200);
	}
} 

// Runs repeatedly after setup() until power is disconnected or
// reset button pressed
void loop() 
{ 
	// Wait for motion detection from the PIR sensor
	while(!pir_activated()) {
		// ...but flash the indicator LED every so often so we know
		// the code is still active
		heartbeat(true);
	}

	// Push the button, and then rest for some time
	press();
	hb_delay(REST_DELAY_SEC);
} 



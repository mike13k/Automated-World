/*
 * Automated-World.c
 *
 * Created: 12/12/2018 1:54:37 PM
 * Author : Kiss My Keyboard Team
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdlib.h>		/* Include standard library file */
#include <stdio.h>		/* Include standard I/O library file */
#include "uart.h"

#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define BIT(x) (0x01 << (x))

#define corner_1_x 0
#define corner_1_y 1
#define corner_2_x 2
#define corner_2_y 3
#define corner_3_x 4
#define corner_3_y 5
#define corner_4_x 6
#define corner_4_y 7

// BITS MACROS
#define A_DIR_BIT 2
#define B_DIR_BIT 4

#define DUTY_CYCLE_MED 102
#define MOTOR_SPEED_HIGH 180 

enum MOTOR {A, B};
enum DIRECTION {BACKWARD, FORWARD};

void input_key_logic(char input);
void init_uart();
void init_pwm_config();
void move(enum MOTOR, enum DIRECTION, unsigned char speed);

int main(void)
{
	init_pwm_config();
	move(A, BACKWARD, 0);
	move(B, BACKWARD, 0);
	
	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;

	char input;
	while(1) {
		input_key_logic(input);
	}
	
	return 0;
}

void input_key_logic(char input) {
	_delay_ms(250);
	input = getchar();
	if(input == 'w') {
		move(A, FORWARD, 256 - MOTOR_SPEED_HIGH);
		move(B, FORWARD, 256 - MOTOR_SPEED_HIGH);
		} else if (input == 's') {
		move(A, BACKWARD, MOTOR_SPEED_HIGH);
		move(B, BACKWARD, MOTOR_SPEED_HIGH);
		} else if(input == 'd') {
		move(A, FORWARD, MOTOR_SPEED_HIGH);
		move(B, BACKWARD, 256-MOTOR_SPEED_HIGH);
		} else if(input == 'a') {
		move(A, BACKWARD, 256- MOTOR_SPEED_HIGH);
		move(B, FORWARD, MOTOR_SPEED_HIGH);
		} else if(input == 'p') {
		move(A, BACKWARD, 0);
		move(B, BACKWARD, 0);
		} else {
		printf("Undefined Key!\n");
	}
}

void move(enum MOTOR motor, enum DIRECTION direction, unsigned char speed) {
	switch(motor) {
		case A:	if(direction == BACKWARD) {
					bit_clear(PORTD, BIT(A_DIR_BIT));
				} else {
					bit_set(PORTD, BIT(A_DIR_BIT));
				}
				OCR0B = speed;
				break;
		
		case B:	if(direction == BACKWARD) {
					bit_clear(PORTD, BIT(B_DIR_BIT));
				} else {
					bit_set(PORTD, BIT(B_DIR_BIT));
				}
				OCR0A = speed;
				break;
		}
}

void init_pwm_config() {
	// #PIN 2: A-1B #PIN 6: A-1A
	// #PIN 4: B-1B #PIN 5: B-1A
	
	DDRD |= 0b01110100;
	
	_delay_ms(150);
	OCR0A = 0;
	OCR0B = 0;
	
	OCR0A = 102;
	OCR0B = 102;
	bit_set(PORTD, BIT(A_DIR_BIT));
	bit_set(PORTD, BIT(B_DIR_BIT));
	TCCR0A = 0b10100001;
	TCCR0B = 0b00000001;
}

void main_logic_commented() {
		
	//bool calibration = true;
	//float current_x_pos = 0;
	//float current_y_pos = 0;
	//float past_x_pos = 0;
	//float past_y_pos = 0;
	//float boundries[] = {0,0,0,0,0,0,0,0};
	//
	//if(calibration) {		// Is (Calibration) Mode or (Running) Mode?
		//
		//// wait for button press, if clicked:
		//while(1 /* !button? */) {}
		//
		//// car is moving and scanning the borders, limits should be set according to Cartesian coordinates:
		//
			////* Phase 1: (Move down) till you hit a line with (Color Sensor), Set a limit (0,0)
		//while(1 /* !color sensor */) {
			//// move car down
		//}
		//
			////* Once hit a line, (Move counter Clockwise) accordingly with the line being tracked.
		//while(1 /* !threshold */) {
			//// move car right
		//}
		//
			////* Phase 2: Once hit a threshold of (y-change) while moving, Push a limit (current_X, 0).
		//boundries[corner_1_x] = current_x_pos;
		//boundries[corner_1_y] = 0;
		//while(1 /* !threshold */) {
			//// move car up
		//}
		//
			////* Phase 3: Once hit a threshold of (x-change) while moving, Push a limit (past_limit_X, current_Y)
		//boundries[corner_2_x] = past_x_pos;
		//boundries[corner_2_y] = current_y_pos;
		//while(1 /* !threshold */) {
			//// move car left
		//}
		//
			////* Phase 4: Once hit a threshold of (y-change) while moving, Push a limit (current_X, past_limit_Y)
		//boundries[corner_3_x] = current_x_pos;
		//boundries[corner_3_y] = past_y_pos;	
		//
			////* Phase 5: Push a limit (last_X, 0)
		//boundries[corner_4_x] = past_x_pos;
		//boundries[corner_4_y] = 0;
			//
			////* Phase 6: Rotate car and move anywhere inside the closed rectangle.
//
//
			//
		//calibration = 0;		// After calibration -> turn to (Running) Mode.
	//}
	//else if(!calibration) {	// else if (Running) Mode:
		//// wait for button press, if clicked:
			//// Calculate (toBeTraveledDistance) relative to the PWM to be generated
			//// Calculate (PositionToBeAt) using (CurrentPosition + toBeTraveledDistance)
			//
			//if (1) {		// Is PositionToBeAt in &bounds or Not:
				//// Calculate &bounds using (limits[4] array] -- Could be done previously
		//
				//if (1) {		// if PositionToBeAt in &bounds:
					//// Generate PWM accordingly and move car.
				//}
				//else {
					//// Stop Car and flash lights accordingly
				//}
			//}
	//}
	//
	//return 0;
}
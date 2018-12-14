/*
 * Automated-World.c
 *
 * Created: 12/12/2018 1:54:37 PM
 * Author : Kiss My Keyboard Team
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdlib.h>		/* Include standard library file */
#include <stdio.h>		/* Include standard I/O library file */
#include "MPU6050_res_define.h"							/* Include MPU6050 register define file */
#include "I2C_Master_H_file.h"							/* Include I2C Master header file */
#include "UART_H.h"										/* Include USART header file */
#include "MOTORS_Controller_H.h"						

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


#define LEFT_LIGHT_SENSOR_BIT 0
#define RIGHT_LIGHT_SENSOR_BIT 4

#define GYRO_THRESHOLD 0.3


float Acc_x,Acc_y,Acc_z,Temperature,Gyro_x,Gyro_y,Gyro_z;
float pitch = 0;
float roll = 0;
float yaw = 0;
float v_pitch;
float v_roll;
float v_yaw;
float a_pitch;
float a_roll;
float a_yaw;
float Xg=0,Yg=0,Zg=0;
char buffer[20], float_[10];
float gyro_x_calc=0, gyro_y_calc=0, gyro_z_calc=0;


bool calibration = true;
volatile float current_x_pos = 0;
float current_y_pos = 0;
volatile float past_x_pos = 0;
float past_y_pos = 0;
float boundries[] = {0,0,0,0,0,0,0,0};

volatile float toBeTraveled_X = 0;
float toBeTraveled_Y = 0;
volatile float toBePosition_X = 0;
float toBePosition_Y = 0;

volatile bool isCarMovementAllowed = true;

float unit_step = 1;

void Init_MPU6050();
void Init_UART();
void Init_PWM_Config();
void Init_LineTracker_Config();

void MPU_Start_Loc();
void MPU_Perform_Calc();
void MPU_Read_RawValue();
void serial_input_logic();
void lineTracker();
void distance_calc_Init();

float get_X(float angle);
float get_Y(float angle);

enum GAME_MODE {CALIBRATION, RUNTIME} current_mode;
	
int main(void)
{
	_delay_ms(1000);
	
	I2C_Init();											/* Initialize I2C */
	uart_init();										/* Initialize UART with 9600 baud rate */
	
	Init_MPU6050();										/* Initialize MPU6050 */
	distance_calc_Init();
	
	MOTORS_Init_Config();
	Init_LineTracker_Config();
	
	stdout = &uart_output;
	stdin  = &uart_input;
	
	current_mode = RUNTIME;
	
	sei();
	
	while(1) {		
		
		if(current_mode == CALIBRATION) {
			lineTracker();
		} else {	
			serial_input_logic();
		}
		
		// GYROSCOPE LOGIC
		MPU_Read_RawValue();
		MPU_Perform_Calc();
		
		
		dtostrf( Xg, 3, 2, float_ );
		sprintf(buffer," Gx = %s%c/s\t",float_,0xF8);
		printf(buffer);
			
		dtostrf( Yg, 3, 2, float_ );
		sprintf(buffer," Gy = %s%c/s\t",float_,0xF8);
		printf(buffer);
			
		dtostrf( Zg, 3, 2, float_ );
		sprintf(buffer," Gz = %s%c/s\r\n",float_,0xF8);
		printf(buffer);
		
		MOTORS_stop();
		_delay_ms(20);
		
	}
	
	return 0;
}


void serial_input_logic() 
{
	char input;
	input = getchar();

	if (input == 'F') {
		MOTORS_move_forward();
		//_delay_ms(100);
	} else if (input == 'B') {
		MOTORS_move_backward();
		//_delay_ms(50);
	} else if (input == 'R') {
		MOTORS_move_right();
		//_delay_ms(100);
	} else if (input == 'L') {
		MOTORS_move_left();
		//_delay_ms(100);
	} else if (input == 'S' ) {
		MOTORS_stop();
	} else if (input == 'W') {
		current_mode = RUNTIME;
	} else if (input == 'w') {
		current_mode = CALIBRATION;
	} else {
		printf("Undefined Key!\n");
	}
	

}

void MPU_Start_Loc()
{
	I2C_Start_Wait(0xD0);								/* I2C start with device write address */
	I2C_Write(ACCEL_XOUT_H);							/* Write start location address from where to read */
	I2C_Repeated_Start(0xD1);							/* I2C start with device read address */
}

void MPU_Perform_Calc()
{
		// Calculating Pitch, roll and yaw
		v_pitch=(Gyro_x/131);
		if(v_pitch==-1)
		v_pitch=0;
		v_roll=(Gyro_y/131);
		if(v_roll==1)
		v_roll=0;
		v_yaw=Gyro_z/131;
		a_pitch=(v_pitch*0.046);
		a_roll=(v_roll*0.046);
		a_yaw=(v_yaw*0.045);
		pitch= pitch + a_pitch;
		roll= roll + a_roll;
		yaw= yaw + a_yaw;
			
		gyro_x_calc = Gyro_x/131;
		gyro_y_calc = Gyro_y/131;
		gyro_z_calc = Gyro_z/131;
			
		if(((gyro_x_calc +0.078) > GYRO_THRESHOLD) || ((gyro_x_calc +0.078) < -GYRO_THRESHOLD)) {
			Xg += (gyro_x_calc/2.5) +0.078;
		}
			
		if((gyro_y_calc > GYRO_THRESHOLD) || (gyro_y_calc < -GYRO_THRESHOLD)) {
			Yg += gyro_y_calc;
		}
			
		if((gyro_z_calc > GYRO_THRESHOLD) || (gyro_z_calc < -GYRO_THRESHOLD)) {
			Zg += gyro_z_calc;
		}
}

void MPU_Read_RawValue()
{
	MPU_Start_Loc();									/* Read Gyro values */
	Acc_x = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Acc_y = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Acc_z = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Temperature = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_x = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_y = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_z = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Nack());
	I2C_Stop();
}

void Init_MPU6050()										/* Gyro initialization function */
{
	_delay_ms(150);										/* Power up time >100ms */
	I2C_Start_Wait(0xD0);								/* Start with device write address */
	I2C_Write(SMPLRT_DIV);								/* Write to sample rate register */
	I2C_Write(0x07);									/* 1KHz sample rate */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(PWR_MGMT_1);								/* Write to power management register */
	I2C_Write(0x01);									/* X axis gyroscope reference frequency */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(CONFIG);									/* Write to Configuration register */
	I2C_Write(0x00);									/* Fs = 8KHz */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(GYRO_CONFIG);								/* Write to Gyro configuration register */
	I2C_Write(0x18);									/* Full scale range +/- 2000 degree/C */
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(INT_ENABLE);								/* Write to interrupt enable register */
	I2C_Write(0x01);
	I2C_Stop();
}

ISR (TIMER1_COMPA_vect)
{
	// Calculate (toBeTraveledDistance) relative to the PWM to be generated
	toBeTraveled_X = get_X(Xg);
	toBeTraveled_Y = get_Y(Xg);
	
	// Calculate (PositionToBeAt) using (CurrentPosition + toBeTraveledDistance)
	toBePosition_X = current_x_pos + toBeTraveled_X;
	toBePosition_Y = current_y_pos + toBeTraveled_Y;
	
	//if (toBePosition_X < boundries[corner_1_x] && toBePosition_X > boundries[corner_4_x] && toBePosition_Y > 0 && toBePosition_Y < boundries[corner_2_y]) {		// Is PositionToBeAt in &bounds or Not:
	
	// if PositionToBeAt in &bounds:
	// Generate PWM accordingly and move car.
	current_x_pos = toBePosition_X;
	current_y_pos = toBePosition_Y;
	isCarMovementAllowed = true;
	//}
	//else {
	// Stop Car and flash lights accordingly
	isCarMovementAllowed = false;
	//}
}

void distance_calc_Init()
{
	OCR1A = 3125;
	TCCR1A = 0x02;
	TCCR1B = 0x05;
	TIMSK1 = (1 << OCIE1A);
}

void Init_LineTracker_Config(){
	// #PIN 14 : Left Sensor [PIN 8 in Arduino (PB0)
	// #PIN 18 : Right Sensor [PIN 12 in Arduino (PB4)
	
	DDRB &= 0b11101110;
	
}

void lineTracker(){
	if(bit_get(PINB , BIT(LEFT_LIGHT_SENSOR_BIT)) && bit_get(PINB , BIT(RIGHT_LIGHT_SENSOR_BIT))){
		//move(A , FORWARD , 255 - MOTOR_SPEED_HIGH);
		//move(B , FORWARD , 255 - MOTOR_SPEED_HIGH);
		move(A , BACKWARD , 255 - MOTOR_SPEED_ROTATION_HI);
		move(B , FORWARD , MOTOR_SPEED_ROTATION_HI);
		printf("0\n");
	}
	else if (bit_get(PINB , BIT(LEFT_LIGHT_SENSOR_BIT)) && ~(bit_get(PINB , BIT(RIGHT_LIGHT_SENSOR_BIT)))){
		
		move(A , BACKWARD , 255 - MOTOR_SPEED_ROTATION_HI);
		move(B , FORWARD ,MOTOR_SPEED_ROTATION_HI);
		printf("1\n");
	}
	else if (~(bit_get(PINB , BIT(LEFT_LIGHT_SENSOR_BIT))) && bit_get(PINB , BIT(RIGHT_LIGHT_SENSOR_BIT))){
		move(A , FORWARD , MOTOR_SPEED_ROTATION_HI);
		move(B , BACKWARD , 255 - MOTOR_SPEED_ROTATION_HI);
		
		printf("2\n");
	}
	else if (~(bit_get(PINB , BIT(LEFT_LIGHT_SENSOR_BIT))) && (~bit_get(PINB , BIT(RIGHT_LIGHT_SENSOR_BIT)))){
		move(A , FORWARD , 255 - MOTOR_SPEED_NORMAL_HI);
		move(B , FORWARD , 255 - MOTOR_SPEED_NORMAL_HI);
		printf("3\n");
	}



	//if (bit_get(PINB , BIT(LEFT_LIGHT_SENSOR_BIT))){
		//
		//move(A , FORWARD , 255 - MOTOR_SPEED_HIGH);
		//move(B , FORWARD , 255 - MOTOR_SPEED_HIGH);
		//
		//printf("2\n");
	//}
	//else {
		//move(A , FORWARD , MOTOR_SPEED_HIGH);
		//move(B , BACKWARD ,255 - MOTOR_SPEED_HIGH);
		//printf("3\n");
	//}
	_delay_ms(50);
}

float get_X(float angle)
{
	return (unit_step * cos(angle));
}

float get_Y(float angle)
{
	return (unit_step * sin(angle));

}

void main_logic_commented() {
		
	//bool calibration = true;
	//float current_x_pos = 0;
	//float current_y_pos = 0;
	//float past_x_pos = 0;
	//float past_y_pos = 0;
	//float boundries[] = {0,0,0,0,0,0,0,0};
		//
	//float toBeTraveled_X = 0;
	//float toBeTraveled_Y = 0;
	//float toBePosition_X = 0;
	//float toBePosition_Y = 0;
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
			//
			//// Calculate (toBeTraveledDistance) relative to the PWM to be generated
			//toBeTraveled_X = get_X(Xg);
			//toBeTraveled_Y = get_Y(Xg);
			//
			//// Calculate (PositionToBeAt) using (CurrentPosition + toBeTraveledDistance)
			//toBePosition_X = current_x_pos + toBeTraveled_X;
			//toBePosition_Y = current_y_pos + toBeTraveled_Y;
			//
			//if (toBePosition_X < boundries[corner_1_x] && toBePosition_X > boundries[corner_4_x] && toBePosition_Y > 0 && toBePosition_Y < boundries[corner_2_y]) {		// Is PositionToBeAt in &bounds or Not:
				//
				//// if PositionToBeAt in &bounds:
				//// Generate PWM accordingly and move car.
					//
			//}
			//else {
				//// Stop Car and flash lights accordingly
			//}
			//
	//}
	
}
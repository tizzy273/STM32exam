/*
 * elevator.c
 *
 *  Created on: 19 feb 2021
 *      Author: Tizio
*/

#include "stm32_unict_lib.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

enum
{
	ST_idle,
	ST_setup_speed,
	ST_opening_doors,
	ST_closing_doors,
	ST_running
};

int state = ST_idle;
int previous_state;

char s[5];
int selected_floor;
int current_floor = 1;
int destination_floor;
int speed;


//timing leds
int yellow_flash_counter;
int red_flash_counter;
int green_flash_counter;

//timing doors
int opening_time;
int closing_time;

//timing floors in running state
int count_floor;

//boolean for log
int booked;
int floor_reached;
int speed_set;
int doors_open;
int doors_closed;
int going_up;
int going_down;
int not_moving;

queue *booked_floors;


// initialization
void start()
{

	CONSOLE_init();

	DISPLAY_init();

	GPIO_init(GPIOB);
	GPIO_init(GPIOC);

	GPIO_config_output(GPIOB,0);	//red led
	GPIO_config_output(GPIOC,2);	//green led
	GPIO_config_output(GPIOC,3);	//yellow sled
	GPIO_config_output(GPIOB,8);

	GPIO_config_input(GPIOB,10);	//x
	GPIO_config_input(GPIOB,6);		//t

	GPIO_config_EXTI(GPIOB,EXTI10);
	EXTI_enable(EXTI10,FALLING_EDGE);

	GPIO_config_EXTI(GPIOB,EXTI6);
	EXTI_enable(EXTI6,FALLING_EDGE);

	ADC_init(ADC1,ADC_RES_8,ADC_ALIGN_RIGHT);
	ADC_channel_config(ADC1,GPIOC,1,11);	//Trimmer selection_floor
	ADC_channel_config(ADC1, GPIOC, 0, 10);	//Trimmer selection_speed
	ADC_on(ADC1);

	TIM_init(TIM2);	//Timer for led, closing and opening doors
	TIM_config_timebase(TIM2,8400,1000);
	TIM_enable_irq(TIM2, IRQ_UPDATE);
	TIM_set(TIM2,0);
	TIM_on(TIM2);

	TIM_init(TIM3); //Timer for speed elevator
	TIM_config_timebase(TIM3,8400,5000); // Initial speed = 1.0 sec/piano
	TIM_enable_irq(TIM3, IRQ_UPDATE);
	TIM_set(TIM3,0);
	TIM_on(TIM3);

	booked_floors = create_queue(); //Initialization queue
}


void floor_selection()
{
	ADC_sample_channel(ADC1,11);
	ADC_start(ADC1);
	while(!ADC_completed(ADC1)){}
	int value = ADC_read(ADC1);
	selected_floor = (value * 8) / 255 + 1;	//[1,9]
}

void speed_selection()
{
	ADC_sample_channel(ADC1,10);
	ADC_start(ADC1);
	while(!ADC_completed(ADC1)){}
	int value = ADC_read(ADC1);
	speed = (value * 6) / 255 + 4;	//[4,10]
	TIM_config_timebase(TIM3,8400,(speed * 1000)/2);	//I need half period to report half floor
}

void log_text()
{
	if(booked)
	{
		printf("Piano %d prenotato \n", selected_floor);
		booked = 0;
	}
	if(floor_reached)
	{
		printf("Piano %d raggiunto \n", destination_floor);
		floor_reached = 0;
	}
	if(speed_set)
	{
		if(speed != 10)
			printf("Velocita' ascensore di 0.%d sec/piano impostata \n",speed);
		else
			printf("Velocita' ascensore di 1 sec/piano impostata \n");
		speed_set = 0;
	}
	if(doors_closed)
	{
		printf("Chiusura porte... \n");
		doors_closed = 0;
	}
	if(doors_open)
	{
		printf("Apertura porte... \n");
		doors_open = 0;
	}
	if(going_up)
	{
		printf("L'ascensore sta salendo... \n");
		going_up = 0;
	}
	if(going_down)
	{
		printf("L'ascensore sta scendendo... \n");
		going_down = 0;
	}
	if(not_moving)
	{
		printf("Ti trovi gia' al piano %d \n", current_floor);
		not_moving = 0;
	}

}



int main()
{

	start();

	for(;;)
	{

		log_text();

		switch (state)
		{
			case ST_idle:

				floor_selection();
				sprintf(s,"%d   ",selected_floor);
				DISPLAY_puts(0,s);
				DISPLAY_dp(2,0);
				GPIO_write(GPIOB,8,0);

			break;
			case ST_setup_speed:

				speed_selection();
				sprintf(s,"SP%02d",speed);
				GPIO_write(GPIOB,8,1);
				DISPLAY_dp(2,1);
				DISPLAY_puts(0,s);

			break;
			case ST_closing_doors:

				DISPLAY_puts(0,"CLOS");
				DISPLAY_dp(2,0);
				GPIO_write(GPIOB,8,0);

			break;
			case ST_opening_doors:

				DISPLAY_puts(0,"OPEn");
				DISPLAY_dp(2,0);
				GPIO_write(GPIOB,8,0);

			break;
			case ST_running:

				DISPLAY_dp(2,0);
				GPIO_write(GPIOB,8,0);
			break;
		}
	}


}




void TIM2_IRQHandler(void) //led and doors
{
	if(state == ST_running)
	{
		yellow_flash_counter++;
		if(yellow_flash_counter >= 2)	//yellow led flashing period 200 ms
		{
			GPIO_toggle(GPIOC,3);
			yellow_flash_counter  = 0;
		}
	}
	else if (state == ST_closing_doors)
	{
		closing_time++;
		red_flash_counter++;
		if(red_flash_counter >= 2)	//red led flashing period 200 ms
		{
			GPIO_toggle(GPIOB,0);
			red_flash_counter = 0;
		}
		if(closing_time >= 15) //closing time 1.5 s
		{
			closing_time = 0;
			GPIO_write(GPIOB,0,0);
			count_floor = 0;

			//boolean values for log
			if(current_floor < destination_floor)
				going_up = 1;
			else if (current_floor > destination_floor)
				going_down = 1;
			else not_moving = 1;

			state = ST_running;

		}
	}
	else if(state == ST_opening_doors)
	{
		opening_time++;
		green_flash_counter++;
		if(green_flash_counter >= 2)	//yellow led flashing period 200 ms
		{
			GPIO_toggle(GPIOC,2);
			green_flash_counter = 0;
		}
		if(opening_time >= 15) //opening time 1.5 ss
		{
			opening_time = 0;
			GPIO_write(GPIOC,2,0);
			if(!is_empty(booked_floors))
				{
					doors_closed = 1;	//boolean value for log

					destination_floor = pop(booked_floors);
					state = ST_closing_doors;
				}
			else
				state = ST_idle;
		}
	}
	TIM_update_clear(TIM2);
}


void TIM3_IRQHandler(void) //elevator speed
{
	if(state == ST_running)
	{

		if(count_floor % 2 == 0) //floor
		{
			sprintf(s,"%d   ",current_floor);
			DISPLAY_puts(0,s);
		}
		else	//half floor
		{
			sprintf(s,"%d-  ",current_floor);
			DISPLAY_puts(0,s);
			if(current_floor < destination_floor)
				current_floor++;
			else if(current_floor > destination_floor)
				current_floor--;
			else
			{
				doors_open = 1;	//boolean value for log
				floor_reached = 1;	//boolean value for log

				GPIO_write(GPIOC,3,0);
				state = ST_opening_doors;
			}
		}

		count_floor++;
	}
	TIM_update_clear(TIM3);
}


void EXTI15_10_IRQHandler(void)
{

	if(EXTI_isset(EXTI10))	//confirm floor selection
	{

		if(state == ST_running || state == ST_closing_doors  || state == ST_opening_doors)
		{
			booked = 1;	//boolean value for log

			floor_selection();
			push(booked_floors, selected_floor);
		}
		else if(state == ST_idle)
		{
			doors_closed = 1;	//boolean value for log

			destination_floor = selected_floor;
			state = ST_closing_doors;
		}
		EXTI_clear(EXTI10);
	}
}


void EXTI9_5_IRQHandler(void)
{
	if(EXTI_isset(EXTI6))	//setup speed
	{
		if(state == ST_setup_speed)
		{
			speed_set = 1;	//boolean value for log
			state = previous_state;
		}
		else
		{
			previous_state = state;
			state = ST_setup_speed;
		}

		EXTI_clear(EXTI6);
	}
}



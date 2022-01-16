
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#define INPUT 0
#define OUTPUT 1
char* MODE[]={"INPUT","OUTPUT"};
#define LOW 0
#define HIGH 1
char* DIGITAL[]={"LOW","HIGH"};
#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 4


void pinMode(int pin, int mode){
	printf("pin %d setup to %s\n",pin,MODE[mode]);
}

void digitalWrite(int pin, int value){
	//printf("pin %d Write to %s\n",pin,DIGITAL[value]);
}

void analogWrite(int pin, int value){
	printf("pin %d Write to %d",pin,value);
}

*/int count=0;
int last[5]={1,2,3,4,5};
int analogRead(int pin){
	if(!(rand()%20))
		last[pin]=(rand()%250)*(rand()%5);
	if(last[pin]>100) count++;
	if(count>10){
		count=0;
		last[pin]=87;
	}
	return last[pin];
}
void delay(int time){
	Sleep(time);
}


/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////


#define LEDs 7
#define tables 3
#define button_groups 5
#define inputs 14

//use HANDLE_ to send the event signal
typedef int HANDLE_;

int LED_pins[]={2,3,4,5,6,7,8};
int table_pins[]={9,10,11};
int input_pins[button_groups]={A0,A1,A2,A3,A4};

#define zero	1<<0
#define one		1<<1
#define two		1<<2
#define three	1<<3
#define four	1<<4
#define five	1<<5
#define six		1<<6
#define seven	1<<7
#define eight	1<<8
#define nine	1<<9
#define equ		1<<10//	=
#define clear	1<<11//	C
#define multp	1<<12//	X
#define add		1<<13//	+
int value01=100;
int value12=350;
int value23=700;

int buffer[tables]={0,0,0};

int digital_mapping[10][7]={
{1, 1, 1, 1, 1, 1, 0},
{0, 1, 1, 0, 0, 0, 0},
{1, 1, 0, 1, 1, 0, 1},
{1, 1, 1, 1, 0, 0, 1},
{0, 1, 1, 0, 0, 1, 1},
{1, 0, 1, 1, 0, 1, 1},
{1, 0, 1, 1, 1, 1, 1},
{1, 1, 1, 0, 0, 0, 0},
{1, 1, 1, 1, 1, 1, 1},
{1, 1, 1, 1, 0, 1, 1}
};


void lightUp();
HANDLE_ check_Input();
int eventHandler(HANDLE_);

void setup(){
	for(int i=0;i<LEDs;i++){
		pinMode(LED_pins[i],OUTPUT);
	}
	for(int i=0;i<tables;i++){
		pinMode(table_pins[i],OUTPUT);
	}
	//for(int i=0;i<button_groups;i++){
	//	pinMode(input_pins[i],INPUT);
	//}
}

void loop(){	
	static HANDLE_ handleButton=0;
	
	if(handleButton=check_Input()){
		eventHandler(check_Input());
		while(check_Input()) lightUp();		
	}else lightUp();
}




int eventHandler(HANDLE_ handle){
	static int stage=0;
	static int operands[]={0,0,0};
	static int operator=0;//0 is add, 1 is multp
	
	//configure an operand to represent
	if(stage==2 || handle & clear){
		for(int i=0;i<3;i++) operands[i]=0;
		stage=0;

	}else if(handle<1024){
		for(int i = 0; i<10; i++){
			if(handle & (1<<i)){
				operands[stage] = operands[stage] * 10 % 1000 + i;
				break;
			}
		}
	}else if(handle & add){
		operator=0;
		stage=1;
	}		
	else if(handle & multp){
		operator=1;
		stage=1;
	}else if(handle & equ){
		if(stage==1){
			if(operator==0) operands[2]=operands[0]+operands[1];
			if(operator==1) operands[2]=operands[0]*operands[1];
			delay(1200);
		}
		stage=2;
	}else return -1;
	
	//upload
	buffer[0]=(operands[stage]/1)%10;
	buffer[1]=(operands[stage]/10)%10;
	buffer[2]=(operands[stage]/100)%10;
	return 0;
}


void lightUp(){
	for(int i=0;i<tables;i++){
		//control a Table to light
		for(int j=0;j<tables;j++){
			digitalWrite(table_pins[j],LOW);
		}
		digitalWrite(table_pins[i],HIGH);
		
		//light the table
		for(int j=0;j<LEDs;j++){
			digitalWrite(LED_pins[j],digital_mapping[buffer[i]][j]);
		}
	}
}

HANDLE_ check_Input(){
	static int little_map[]={
		clear,multp,add,
		seven,eight,nine,
		four,five,six,
		one,two,three,
		0,zero,equ};
		
	HANDLE_ input_signal=0;
	
	HANDLE_ read_value;

	for(int i=0;i<5;i++){
		read_value=analogRead(input_pins[i]);
		//printf("pin %d read: %d\n",i,read_value);
		if(read_value<value01)
			continue;
		if(read_value<value12)
			input_signal |= little_map[3*i+2];
		else if(read_value<value23)
			input_signal |= little_map[3*i+1];
		else
			input_signal |= little_map[3*i];		
	}
	return input_signal;
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////

int main(){
	srand(time(NULL));
	setup();
	while(1){
		loop();
	}
	return 0;
}


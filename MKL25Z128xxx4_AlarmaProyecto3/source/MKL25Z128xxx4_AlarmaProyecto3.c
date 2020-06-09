#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include "fsl_tpm.h"
#include "keypad.h"
#include "string.h"

void LCD_Data(uint8_t *high, uint8_t *low, uint8_t c);
void Write_Data(uint8_t data);
void LCD_CMD(uint8_t cmd);
void LCD_Write(uint8_t data);
void LCD_Clear();
void LCD_Return();
void LCD_Mode(uint8_t ID, uint8_t S);
void LCD_Set(uint8_t D, uint8_t C, uint8_t B);
void LCD_Cursor(uint8_t SC, uint8_t RL);
void LCD_Activate(uint8_t DL, uint8_t N, uint8_t F);
void LCD_CGRAM(uint8_t Address);
void LCD_DDRAM(uint8_t Address);
void Delay (uint32_t delay);
void DelayTPM();

#define DELAY 100000
#define buzzer 7u
#define rojo 0u
#define verde 3u
#define amarillo 4u
#define sensor1 5u
#define sensor2 6u
#define sensor3 10u
#define sensor4 11u

typedef struct{
	uint8_t Buzzer;
	uint8_t lamarillo;
	uint8_t lverde;
	uint8_t lrojo;
	uint16_t TIMER_MOD;
} state;

int main(void) {

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();
    tpm_config_t config;
    TPM_GetDefaultConfig(&config);
    config.prescale= kTPM_Prescale_Divide_128;
    TPM_Init(TPM0, &config);
    TPM_Init(TPM1, &config);
    keypad k;
    keypad_config config1;
    get_default_keypad_config(&config1);
    set_keypad_config(&k, &config1);

    char key=0, anterior;
    char clave[12]={'A','C','A','B','A','D','A','1','3','7','9',0};
    char Dato[12]={'0','0','0','0','0','0','0','0','0','0','0',0};
    char reset[12]={'0','0','0','0','0','0','0','0','0','0','0',0};

    int j=0;


    state FSM[12];

        FSM[0]=(state){.lrojo=0u, .lverde=1u, .lamarillo=0u, .Buzzer=0u};/* Alarma desactivada*/
        FSM[1]=(state){.lrojo=0u, .lverde=1u, .lamarillo=0u, .Buzzer=0u};/* Alarma desactivada, estado donde se pide clave*/
        FSM[2]=(state){.lrojo=0u, .lverde=1u, .lamarillo=0u, .Buzzer=0u};/* Alarma desactivada, se verifica la clave*/
        FSM[3]=(state){.lrojo=0u, .lverde=0u, .lamarillo=1u, .Buzzer=0u, .TIMER_MOD=5120u};/* Alarma "activada" 20 segs sin sensores activos*/
        FSM[4]=(state){.lrojo=0u, .lverde=0u, .lamarillo=1u, .Buzzer=0u};/* Alarma activada, con sensores activos*/
        FSM[5]=(state){.lrojo=1u, .lverde=0u, .lamarillo=0u, .Buzzer=1u};/* Alarma activada, sensor detectó algo*/
        FSM[6]=(state){.lrojo=1u, .lverde=0u, .lamarillo=0u, .Buzzer=1u};/* Alarma activada, pide clave*/
        FSM[7]=(state){.lrojo=1u, .lverde=0u, .lamarillo=0u, .Buzzer=1u};/* Alarma activada, verifica clave*/
        FSM[8]=(state){.lrojo=0u, .lverde=1u, .lamarillo=0u, .Buzzer=0u};/* Alarma desactivada, clave incorrecta */

    	uint8_t Sensor1;
    	uint8_t Sensor2;
    	uint8_t Sensor3;
    	uint8_t Sensor4;
        uint8_t estado=0;
        uint16_t TIMER_ON=0;
        uint32_t timerbandera;
        uint8_t texto=0;

        while(1){
             GPIO_WritePinOutput(GPIOC, rojo, FSM[estado].lrojo);
             GPIO_WritePinOutput(GPIOC, verde, FSM[estado].lverde);
             GPIO_WritePinOutput(GPIOC, amarillo, FSM[estado].lamarillo);
             GPIO_WritePinOutput(GPIOC, buzzer, FSM[estado].Buzzer);

           	Sensor1=GPIO_ReadPinInput(GPIOC, sensor1);
           	Sensor2=GPIO_ReadPinInput(GPIOC, sensor2);
           	Sensor3=GPIO_ReadPinInput(GPIOC, sensor3);
           	Sensor4=GPIO_ReadPinInput(GPIOC, sensor4);



         	timerbandera=TPM_GetStatusFlags(TPM0);

           	anterior=key;
           	key=read_keypad(&k);

           	switch (estado){

           	case 0:
           		if(texto==0){
           		    LCD_Set(1u,1u,1u);
           		    LCD_Activate(1u,1u,0u);
           		    LCD_Clear();
           		    LCD_Write('A');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write('M');
           		    LCD_Write('A');
           		    LCD_DDRAM(40u);
           		    LCD_Write('D');
           		    LCD_Write('E');
           		    LCD_Write('S');
           		    LCD_Write('A');
           		    LCD_Write('C');
           		    LCD_Write('T');
           		    LCD_Write('I');
           		    LCD_Write('V');
           		    LCD_Write('A');
           		    LCD_Write('D');
           		    LCD_Write('A');
           			texto=1;
           		}

           	    if(key==0 && anterior!=0){
           	    	key=anterior;
           	    		if(key=='*' ){

           	    			texto=0;
           	    			estado=1;
           	    }
           	    		else if(key=='#'){
           	    			texto=0;
           	    			 estado=9;
           	    		}
           	    		key=0;
           	    }

           	    		break;

           	case 1: /*Ingresar la clave*/
        		if(!texto){    /* Evita que la palabra clave aparezca varias veces */
           		    LCD_Set(1u,1u,1u);
           		    LCD_Clear();
           		    LCD_Write('C');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('V');
           		    LCD_Write('E');
        			texto=1;
        		}
        		if(key==0 && anterior!=0){
        			key=anterior;
        			if(key!='#'){

        				Dato[j]=key;
               		    LCD_Set(1u,1u,1u);
               		    LCD_Write('*');
        		    	j++;
        		    	DelayTPM();
        			}
        			else if (key=='#'){

        				printf("\n");
        				texto=0;
        				j=0;
        				estado=2;


        				}
        			key=0;

        			}



           	break;

        	case 2: /*verifica la clave*/
        		if(strcmp(clave,Dato)==0){ /*Compara lo que tiene clave con Dato (0 cuando coincide)*/
            		strcpy(Dato,reset); /*Regresa al varlor original a Dato*/
            		key=0;
            		estado=3;
        		}
        		else{
        			printf("\nClave incorrecta, intenta de nuevo\n");
           		    key=0;
            		strcpy(Dato,reset);
        			estado=8;
        		}
        	break;

        	case 3: /*alarma armada sensores inactivos, dura 20s*/
           		if(texto==0){
           		    LCD_Set(1u,1u,1u);
           		    LCD_Activate(1u,1u,0u);
           		    LCD_Clear();
           		    LCD_Write('A');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write('M');
           		    LCD_Write('A');
           		    LCD_DDRAM(40u);
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write('M');
           		    LCD_Write('A');
           		    LCD_Write('D');
           		    LCD_Write('A');
           			texto=1;
           		}

                if(TIMER_ON==0){
                	TPM_SetTimerPeriod(TPM0, FSM[3].TIMER_MOD);
                    TPM_StartTimer(TPM0, kTPM_SystemClock);
                    TIMER_ON=1;
                 }

                if(timerbandera){
                TPM_ClearStatusFlags(TPM0, 1u<8u);
                TIMER_ON=0;
                texto=0;
                TPM_StopTimer(TPM0);
                TPM0->CNT=0;
                estado=4;
                }
       	    	break;

           	case 4:
           		if(texto==0){
           		    LCD_Set(1u,1u,1u);
           		    LCD_Activate(1u,1u,0u);
           		    LCD_Clear();
           		    LCD_Write('A');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write('M');
           		    LCD_Write('A');
           		    LCD_Write(' ');
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write('M');
           		    LCD_Write('A');
           		    LCD_Write('D');
           		    LCD_Write('A');
           		    LCD_Write('-');
           		    LCD_Write('>');
           		    LCD_DDRAM(40u);
           		    LCD_Write('S');
           		    LCD_Write('E');
           		    LCD_Write('N');
           		    LCD_Write('S');
           		    LCD_Write('O');
           		    LCD_Write('R');
           		    LCD_Write('E');
           		    LCD_Write('S');
           		    LCD_Write(' ');
           		    LCD_Write('A');
           		    LCD_Write('C');
           		    LCD_Write('T');
           		    LCD_Write('I');
           		    LCD_Write('V');
           		    LCD_Write('O');
           		    LCD_Write('S');
           			texto=1;
           		}
           	 	if(Sensor1==0){
               		texto=0;
           	           		estado=5;
           	           	}
           	 	if(Sensor2==0){
               		texto=0;
           	           		estado=5;
           	           	}
           	 	if(Sensor3==0){
               		texto=0;
           	        		estado=5;
           	           	}
           	 	if(Sensor4==0){
               		texto=0;
           	           		estado=5;
           	           	}
           	break;
           	case 5:
        		if(Sensor1==0){
           		    LCD_Set(1u,1u,1u);
           		    LCD_Activate(1u,1u,0u);
           		    LCD_Clear();
           		    LCD_Write('S');
           		    LCD_Write('E');
           		    LCD_Write('N');
           		    LCD_Write('S');
           		    LCD_Write('O');
           		    LCD_Write('R');
           		    LCD_Write('1');
           		    LCD_Write(' ');
           		    LCD_Write('A');
           		    LCD_Write('C');
           		    LCD_Write('T');
           		    LCD_Write('I');
           		    LCD_Write('V');
           		    LCD_Write('O');
           		    LCD_DDRAM(40u);
           		    LCD_Write('I');
           		    LCD_Write('N');
           		    LCD_Write('G');
           		    LCD_Write('R');
           		    LCD_Write('E');
           		    LCD_Write('S');
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write(' ');
           		    LCD_Write('C');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('V');
           		    LCD_Write('E');

        	    }
        	    if(Sensor2==0){
           		    LCD_Set(1u,1u,1u);
           		    LCD_Activate(1u,1u,0u);
           		    LCD_Clear();
           		    LCD_Write('S');
           		    LCD_Write('E');
           		    LCD_Write('N');
           		    LCD_Write('S');
           		    LCD_Write('O');
           		    LCD_Write('R');
           		    LCD_Write('2');
           		    LCD_Write(' ');
           		    LCD_Write('A');
           		    LCD_Write('C');
           		    LCD_Write('T');
           		    LCD_Write('I');
           		    LCD_Write('V');
           		    LCD_Write('O');
           		    LCD_DDRAM(40u);
           		    LCD_Write('I');
           		    LCD_Write('N');
           		    LCD_Write('G');
           		    LCD_Write('R');
           		    LCD_Write('E');
           		    LCD_Write('S');
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write(' ');
           		    LCD_Write('C');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('V');
           		    LCD_Write('E');
        	    }
        	    if(Sensor3==0){
           		    LCD_Set(1u,1u,1u);
           		    LCD_Activate(1u,1u,0u);
           		    LCD_Clear();
           		    LCD_Write('S');
           		    LCD_Write('E');
           		    LCD_Write('N');
           		    LCD_Write('S');
           		    LCD_Write('O');
           		    LCD_Write('R');
           		    LCD_Write('3');
           		    LCD_Write(' ');
           		    LCD_Write('A');
           		    LCD_Write('C');
           		    LCD_Write('T');
           		    LCD_Write('I');
           		    LCD_Write('V');
           		    LCD_Write('O');
           		    LCD_DDRAM(40u);
           		    LCD_Write('I');
           		    LCD_Write('N');
           		    LCD_Write('G');
           		    LCD_Write('R');
           		    LCD_Write('E');
           		    LCD_Write('S');
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write(' ');
           		    LCD_Write('C');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('V');
           		    LCD_Write('E');
        	    }
        	    if(Sensor4==0){
           		    LCD_Set(1u,1u,1u);
           		    LCD_Activate(1u,1u,0u);
           		    LCD_Clear();
           		    LCD_Write('S');
           		    LCD_Write('E');
           		    LCD_Write('N');
           		    LCD_Write('S');
           		    LCD_Write('O');
           		    LCD_Write('R');
           		    LCD_Write('4');
           		    LCD_Write(' ');
           		    LCD_Write('A');
           		    LCD_Write('C');
           		    LCD_Write('T');
           		    LCD_Write('I');
           		    LCD_Write('V');
           		    LCD_Write('O');
           		    LCD_DDRAM(40u);
           		    LCD_Write('I');
           		    LCD_Write('N');
           		    LCD_Write('G');
           		    LCD_Write('R');
           		    LCD_Write('E');
           		    LCD_Write('S');
           		    LCD_Write('A');
           		    LCD_Write('R');
           		    LCD_Write(' ');
           		    LCD_Write('C');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('V');
           		    LCD_Write('E');
        	    }
        	    if(key==0 && anterior!=0){
        	           	    	key=anterior;
        	           	    		if(key=='#' ){
        	           	    			key=0;
        	           	    			estado=6;
        	           	    		}
        	           	    		key=0;

        	    }

           	break;

           	case 6:

           		if(texto==0){
           		    LCD_Set(1u,1u,1u);
           		    LCD_Clear();
           		    LCD_Write('C');
           		    LCD_Write('L');
           		    LCD_Write('A');
           		    LCD_Write('V');
           		    LCD_Write('E');
           		    key=0;
           			texto=1;
           		}

       	    if(key==0 && anterior!=0){
       	    	key=anterior;
       	    		if(key!='#' ){
        				Dato[j]=key;
               		    LCD_Set(1u,1u,1u);
               		    LCD_Write('*');
               		    j++;
        		    	DelayTPM();
       	    		}
       	    		else{
        				printf("\n");
        				texto=0;
        				j=0;
        				estado=7;
        				key=0;
       	    	}
       	    		key=0;
       	    }


           	break;

           	case 7:
        		if(strcmp(clave,Dato)==0){
            		printf("\nClave Correcta\n");
            		strcpy(Dato,reset);
            		Sensor1=1; /*Regresa sensores a valor original*/
            		Sensor2=1;
					Sensor3=1;
					Sensor4=1;
					key=0;
            		estado=0;
        		}
        		else{
        			printf("\nContrasena Incorrecta\n");
        			strcpy(Dato,reset);
        			key=0;
        			estado=6;
        		}
        	break;


           	case 8:
           		if(!texto){
           			LCD_Set(1u,1u,1u);
           		    LCD_Activate(1u,1u,0u);
           			LCD_Clear();
           			LCD_Write('C');
           			LCD_Write('L');
           			LCD_Write('A');
           			LCD_Write('V');
           			LCD_Write('E');
           			LCD_Write(' ');
           			LCD_Write('I');
           			LCD_Write('N');
           			LCD_Write('C');
           			LCD_Write('O');
           			LCD_Write('R');
           			LCD_Write('E');
           			LCD_Write('C');
           			LCD_Write('T');
           			LCD_Write('A');
           		    LCD_DDRAM(40u);

           			texto=1;
           		}

           		if(key==0 && anterior!=0){
           		 key=anterior;
   	    		if(key=='#' ){
   	    			texto=0;
   	    			estado=1;
   	    		}
   	    		key=0;
           		}

   	    		break;

           	}
           	}
        }





void LCD_Data(uint8_t *high, uint8_t *low, uint8_t c)
{
  *high=(c & 0xF0) >> 4;
  *low= c & 0x0F;
}
void Write_Data(uint8_t data)
{

	for (uint8_t i=0;i<4;i++) //low
	{
		if(data & (1u<<i))
		{
			GPIO_SetPinsOutput(GPIOB, 1u<<i);
		}
		else
		{
			GPIO_ClearPinsOutput(GPIOB, 1u<<i);
		}
	}


	for (uint8_t i=0;i<4;i++) //High
	{
		if(data & (1u<<(i+4)))
		{
			GPIO_SetPinsOutput(GPIOE, 1u<<(i+20));
		}
		else
		{
			GPIO_ClearPinsOutput(GPIOE, 1u<<(i+20));
		}
	}
}
void LCD_CMD(uint8_t cmd)
{
	//Pone LCD_RS en bajo
	GPIO_ClearPinsOutput(GPIOC, 1u<<BOARD_INITPINS_LCD_RS_PIN);
	//Pone LCD_RW en bajo
	GPIO_ClearPinsOutput(GPIOC, 1u<<BOARD_INITPINS_LCD_RW_PIN);
	//Pone LCD_E en alto
	GPIO_SetPinsOutput(GPIOE, 1u<<BOARD_INITPINS_LCD_E_PIN);
	Write_Data(cmd);
	//Pone LCD_E en bajo
	GPIO_ClearPinsOutput(GPIOE, 1u<<BOARD_INITPINS_LCD_E_PIN);
	Delay(DELAY);

}
void LCD_Write(uint8_t data)
{
	//Pone LCD_RS en alto
	GPIO_SetPinsOutput(GPIOC, 1u<<BOARD_INITPINS_LCD_RS_PIN);
	//Pone LCD_RW en bajo
	GPIO_ClearPinsOutput(GPIOC, 1u<<BOARD_INITPINS_LCD_RW_PIN);
	//Pone LCD_E en alto
	GPIO_SetPinsOutput(GPIOE, 1u<<BOARD_INITPINS_LCD_E_PIN);
	Write_Data(data);
	//Pone LCD_E en bajo
	GPIO_ClearPinsOutput(GPIOE, 1u<<BOARD_INITPINS_LCD_E_PIN);
	Delay(DELAY);
}
void LCD_Clear()
{
	uint8_t cmd=0x01u;
	LCD_CMD(cmd);
}
void LCD_Return()
{
	uint8_t cmd=0x02u;
	LCD_CMD(cmd);
}
void LCD_Mode(uint8_t ID, uint8_t S)
{
	uint8_t cmd=4u;
	if(ID)
		cmd |=2u;
	if(S)
		cmd |=1u;
	LCD_CMD(cmd);
}
void LCD_Set(uint8_t D, uint8_t C, uint8_t B)
{
	uint8_t cmd=8u;
	if(D)
		cmd |= 4u;
	if(C)
		cmd |= 2u;
	if(B)
		cmd |= 1u;
	LCD_CMD(cmd);
}
void LCD_Cursor(uint8_t SC, uint8_t RL)
{
	uint8_t cmd=16u;
	if(SC)
		cmd |=8u;
	if(RL)
		cmd |=4u;
	LCD_CMD(cmd);
}
void LCD_Activate(uint8_t DL, uint8_t N, uint8_t F)
{
	uint8_t cmd=32u;
	if(DL)
		cmd |=16u;
	if(N)
		cmd |=8u;
	if(F)
		cmd |=4u;
	LCD_CMD(cmd);
}
void LCD_CGRAM(uint8_t Address)
{
	uint8_t cmd = 0x40u;
	cmd |= Address & 0x3Fu;
	LCD_CMD(cmd);
}
void LCD_DDRAM(uint8_t Address)
{
	uint8_t cmd = 0x80u;
	cmd |= Address & 0x7Fu;
	LCD_CMD(cmd);
}
void Delay(uint32_t delay)
{
	for(uint32_t i=0;i<delay;i++)
		__asm("NOP");
}

void DelayTPM(){
	uint32_t Mask=1u<<8u;
	uint32_t Mask_off=Mask;

	TPM_SetTimerPeriod(TPM1, 100u);
	TPM_StartTimer(TPM1, kTPM_SystemClock);
	while(!(TPM1->STATUS & Mask)){
	}
	if(TPM1->STATUS & Mask){
		TPM1->STATUS &=Mask_off;
		TPM_StopTimer(TPM1);
		TPM1->CNT=0;

	}
}

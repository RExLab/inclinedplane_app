#ifndef APP_H
#define APP_H

// ###########################################################################################################################################
// DESENVOLVIMENTO
#define LOG_MODBUS		pdON

// ###########################################################################################################################################
// MODEL WORKTEMP

#define MODEL			"ms10"	// somente 4 chars, por causa so modbus
#define FIRMWARE_VERSION "1.0"	// usar x.y s� com uma casa decimal, pois estamos usando a impress�o de temperatura
								// usar 3 chars por causa do modbus
#define PROGAM_NAME "MSIP" MODEL " firmware v" FIRMWARE_VERSION " [compiled in " __DATE__ " " __TIME__ "]" CMD_TERMINATOR


#define NUMBER_OF_POSITIONS  7    	// Quantidade de sensores ópticos



// ###########################################################################################################################################
// INCLUDES


// ###########################################################################################################################################
// MODBUS
#define COM_PORT "/dev/ttyAMA0" 	// Consultar /dev/ttySxxx do linux
									// Opensuse com placa pci para serial = "/dev/ttyS4"
									// Raspberry "/dev/ttyAMA0"
									// Para descobrir qual o n� da porta UART fa�a o seguinte:
									// 		Conecte o RASP com um PC via cabo serial.
									//		No PC com um terminal de comnica��o serial abra uma conex�o com RASP a 115200bps 8N1
									//		No RAPS liste as portas tty: ls tty*
									// 		No RAPS na linha de comando envie uma mensagem para cada porta serial: echo ola /dev/ttyXXXX
									//			Fa�a para para todas as portas listadas at� que receba a mensagem no terminal do PC

#define MODBUS_BAUDRATE		B57600 // N�o usar acima de 57600, pois h� erro de recep��o do raspberry.
									// Deve ser algum bug de hardware do rasp porque o baudrate do rasp n�o fica indentico do ARM
									// pois a comunica��o com PC a 115200 funciona bem.
									// Ou a tolerancia de erro no rasp n�o � t�o grande como no PC onde o ARM tem um erro consider�vel
									//	TODO Quando usar o oscilador interno do ARM refazer os testes a sabe com usando oscilador interno do ARM isso se resolve

// ###########################################################################################################################################
// CONTROLE DO SISTEMA

typedef enum {
	cmdNONE = 0,
	cmdGET_INFO,        
    cmdGET_ACTION,      
    cmdGET_STATUS,      
	cmdGET_FAIL,
	cmdGET_MASS,
	cmdGET_ANGLE,
	cmdSET_ANGLE,
	cmdSET_CANCEL,
	cmdSET_FALLSTART,
	cmdGET_FALLTIME,
	cmdSET_SENSORPOS,
        cmdSET_COMPENSATION	
} tCommand;




typedef struct {
	// flash de controle de a��o para acesso ao RH. Esses flags devem ser atualizados via rede
	unsigned exit:1;		// 1 Sinaliza para cair fora do programa
	unsigned getInfo:1;		// Sinaliza para capturar as informa��es do recurso de hardware
	unsigned getAction:1;
	unsigned getStatus:1;
	unsigned GetFallTime:1;
	unsigned expStart:1;
	unsigned expCancel:1;
	unsigned SetAngle:1;
	unsigned GetFail:1;
  	unsigned GetMass:1;
	unsigned GetAngle:1;
	unsigned SetCancel:1;
	unsigned SetFallStart:1;
	unsigned SetFallTime:1;
	unsigned SetForceSensor:1;
        unsigned SetCompensation:1;

	int stsCom;			// Status de comunica��o com RH via modbus
							//		0: Sem comunica��o com o RH. O mesmo n�o est� conectado, ou est� desligado, ou n�o h� dispositivo neste endere�o.
							//		1: O RH recebeu uma fun��o que n�o foi implementada;
							//		2: Foi acessado a um endere�o de registrador inexistente;
							//		3: Foi tentado gravar um valor inv�lido no registrador do RH;
							//		4: Um irrecuper�vel erro ocorreu enquanto o RH estava tentando executar a a��o solicitada;
							//		5: Comunica��o estabelecida com sucesso
	uint rhID;			// Qual o ID do RH no barramento modbus
	string rhModel;		// Modelo do RH
	string rhFirmware;	// Firmware vers�o
	uint mass;  	// Leitura do peso em gramas da esfera
	short int angle;  // Leitura do �ngulo do plano
	uint sensor_pos; // posi��o do sensor de posi��o 1 a 6
	int action;			// ACTION: A��o do recurso de hardware.
						// Sinaliza se h� ou n�o uma a��o sendo executado no plano. Estas a��o pode ser:  
						// �	Fazendo a inclina��o do plano; 
						// �	Posicionando do sensor de peso no local de pesagem;
						// �	Fazendo as medi��es de tempos na queda da esfera. 
						// Se qualquer uma dessas a��es estiver em curso o registrar ACTION ficar� ligado e o plano n�o vai atender a nenhum outro comando at� que o registrador venha a ser desligado novamente, sinalizando o fim da a��o.

	int status;     	// Guarda o status do experimento
                    		// 0: Sinaliza que o experimento ou processo de configura��o dos term�metros foi feito com sucesso;
                    		// 1: N�o foi poss�vel se comunicar pelo menos com um term�metro;
                   	 		// 2: Pelo menos um dos term�metros deu erro de leitura de temperatura, ou a mesma ficou fora dos limites de trabalho
	int error; 			// Erros do sistema. 
							//  �	Valor 0: sinaliza que n�o erros com o sistema;
							//	�	BIT 0: Quando ligado h� erro de comunica��o com os visores;
							//  �	BIT 1: Quando ligado h� erro de leitura do �ngulo do plano. 


	uint falltime;      
						// Posi��o 0 ->  Leitura do tempo em milissegundos da chegada da esfera no primeiro ponto
						// Posi��o 5 -> Leitura do tempo em milissegundos da chegada da esfera no sexto ponto
	uint position_sensor;
	int angle_value;
        int compensation;
} tControl, *pControl;

typedef enum {readREGS, writeREG, writeREGS} tcmd;


// ###############################################################################
// PROTOTIPOS
int modbus_Init(void);
void modbus_SendCommand( tCommand c) ;
void init_control_tad();
void * modbus_Process(void * params);

#endif

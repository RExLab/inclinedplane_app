#ifndef APP_H
#define APP_H

// ###########################################################################################################################################
// DESENVOLVIMENTO
#define LOG_MODBUS		pdON

// ###########################################################################################################################################
// MODEL WORKTEMP

#define MODEL			"ms10"	// somente 4 chars, por causa so modbus
#define FIRMWARE_VERSION "1.0"	// usar x.y só com uma casa decimal, pois estamos usando a impressão de temperatura
								// usar 3 chars por causa do modbus
#define PROGAM_NAME "MSIP" MODEL " firmware v" FIRMWARE_VERSION " [compiled in " __DATE__ " " __TIME__ "]" CMD_TERMINATOR


#define NUMBER_OF_POSITIONS  6    	// Quantidade de termômetro que o RH deve gerenciar. O ID modbus do termômetro deve ser sequencial de 1 a quantidade de termômetro



// ###########################################################################################################################################
// INCLUDES


// ###########################################################################################################################################
// MODBUS
#define COM_PORT "/dev/ttyAMA0" 	// Consultar /dev/ttySxxx do linux
									// Opensuse com placa pci para serial = "/dev/ttyS4"
									// Raspberry "/dev/ttyAMA0"
									// Para descobrir qual o nó da porta UART faça o seguinte:
									// 		Conecte o RASP com um PC via cabo serial.
									//		No PC com um terminal de comnicação serial abra uma conexão com RASP a 115200bps 8N1
									//		No RAPS liste as portas tty: ls tty*
									// 		No RAPS na linha de comando envie uma mensagem para cada porta serial: echo ola /dev/ttyXXXX
									//			Faça para para todas as portas listadas até que receba a mensagem no terminal do PC

#define MODBUS_BAUDRATE		B57600 // Não usar acima de 57600, pois há erro de recepção do raspberry.
									// Deve ser algum bug de hardware do rasp porque o baudrate do rasp não fica indentico do ARM
									// pois a comunicação com PC a 115200 funciona bem.
									// Ou a tolerancia de erro no rasp não é tão grande como no PC onde o ARM tem um erro considerável
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
	
} tCommand;




typedef struct {
	// flash de controle de ação para acesso ao RH. Esses flags devem ser atualizados via rede
	unsigned exit:1;		// 1 Sinaliza para cair fora do programa
	unsigned getInfo:1;		// Sinaliza para capturar as informações do recurso de hardware
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

	int stsCom;			// Status de comunicação com RH via modbus
							//		0: Sem comunicação com o RH. O mesmo não está conectado, ou está desligado, ou não há dispositivo neste endereço.
							//		1: O RH recebeu uma função que não foi implementada;
							//		2: Foi acessado a um endereço de registrador inexistente;
							//		3: Foi tentado gravar um valor inválido no registrador do RH;
							//		4: Um irrecuperável erro ocorreu enquanto o RH estava tentando executar a ação solicitada;
							//		5: Comunicação estabelecida com sucesso
	uint rhID;			// Qual o ID do RH no barramento modbus
	string rhModel;		// Modelo do RH
	string rhFirmware;	// Firmware versão
	uint mass;  	// Leitura do peso em gramas da esfera
	short int angle;  // Leitura do ângulo do plano
	uint sensor_pos; // posição do sensor de posição 1 a 6
	int action;			// ACTION: Ação do recurso de hardware.
						// Sinaliza se há ou não uma ação sendo executado no plano. Estas ação pode ser:  
						// •	Fazendo a inclinação do plano; 
						// •	Posicionando do sensor de peso no local de pesagem;
						// •	Fazendo as medições de tempos na queda da esfera. 
						// Se qualquer uma dessas ações estiver em curso o registrar ACTION ficará ligado e o plano não vai atender a nenhum outro comando até que o registrador venha a ser desligado novamente, sinalizando o fim da ação.

	int status;     	// Guarda o status do experimento
                    		// 0: Sinaliza que o experimento ou processo de configuração dos termômetros foi feito com sucesso;
                    		// 1: Não foi possível se comunicar pelo menos com um termômetro;
                   	 		// 2: Pelo menos um dos termômetros deu erro de leitura de temperatura, ou a mesma ficou fora dos limites de trabalho
	int error; 			// Erros do sistema. 
							//  •	Valor 0: sinaliza que não erros com o sistema;
							//	•	BIT 0: Quando ligado há erro de comunicação com os visores;
							//  •	BIT 1: Quando ligado há erro de leitura do ângulo do plano. 


	uint falltime;      
						// Posição 0 ->  Leitura do tempo em milissegundos da chegada da esfera no primeiro ponto
						// Posição 5 -> Leitura do tempo em milissegundos da chegada da esfera no sexto ponto
	uint position_sensor;
	int angle_value;
} tControl, *pControl;

typedef enum {readREGS, writeREG, writeREGS} tcmd;


// ###############################################################################
// PROTOTIPOS
int modbus_Init(void);
void modbus_SendCommand( tCommand c) ;
void init_control_tad();
void * modbus_Process(void * params);

#endif

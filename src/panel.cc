#include "timer/timer.h"
#include "_config_cpu_.h"
#include "uart/uart.h"
#include "modbus/modbus_master.h"
#include <nan.h>
#include "app.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <v8.h>
#include <string.h>
#define TIMEOUT 100;

using namespace v8;
pthread_t id_thread;
static int waitResponse = pdFALSE; // Testas com inicio pdFalse
static tCommand cmd;
static u16 regs[120]; // registrador de trabalho para troca de dados com os multimetros
tControl control;

int modbus_Init(void) {
    //fprintf(flog, "Abrindo UART %s"CMD_TERMINATOR, COM_PORT);
#if (LOG_MODBUS == pdON)
    printf("Abrindo UART %s"CMD_TERMINATOR, COM_PORT);
#endif
    if (uart_Init(COM_PORT, MODBUS_BAUDRATE) == pdFAIL) {
        //fprintf(flog, "Erro ao abrir a porta UART"CMD_TERMINATOR);
        //fprintf(flog, "    Verifique se essa porta n�o esteja sendo usada por outro programa,"CMD_TERMINATOR);
        //fprintf(flog, "    ou se o usu�rio tem permiss�o para usar essa porta: chmod a+rw %s"CMD_TERMINATOR, COM_PORT);
#if (LOG_MODBUS == pdON)
        printf("Erro ao abrir a porta UART"CMD_TERMINATOR);
        printf("    Verifique se essa porta n�o esteja sendo usada por outro programa,"CMD_TERMINATOR);
        printf("    ou se o usu�rio tem permiss�o para usar essa porta: chmod a+rw %s"CMD_TERMINATOR, COM_PORT);
#endif
        return 0;
    }

    //fprintf(flog, "Port UART %s aberto com sucesso"CMD_TERMINATOR, COM_PORT);
#if (LOG_MODBUS == pdON)
    printf("Port UART %s aberto com sucesso a %d bps"CMD_TERMINATOR, COM_PORT, MODBUS_BAUDRATE);
#endif
    modbus_MasterInit(uart_SendBuffer, uart_GetChar, uart_ClearBufferRx);
    modbus_MasterAppendTime(now, 3000);

    return 1;
}

// para grava��o os valores dos registradores devem estar preenchidos
// ap�s a leitura os registradores estar�o preenchidos

// Envia um comando para o escravo, nesta mesma fun��o � feito o procedimento de erro de envio
// Uma vez enviado o comando com sucesso caprturar resposta no modbus_Process
//	BUSY: Ficar na espera da resposta
//  ERROR: Notificar  o erro e tomar procedimento cab�veis
//  OK para escrita: Nada, pois os valores dos registradores foram salvos no escravo com sucesso
//	OK para Leitura: Capturar os valores dos registradores lidos do escravo
// Parametros
// c: Tipo de comando para ser enviado ao escravo
// funcResponse: Ponteiro da fun��o para processar a resposta da comunica��o

void modbus_SendCommand(tCommand c) {
    if (waitResponse) return; // pdFAIL;

    cmd = c;
    // APONTA QUAIS REGISTRADORES A ACESSAR NO DISPOSITIVO
    // -----------------------------------------------------------------------------------------------------------------

    // Comando para ler os registradores: modelo e vers�o firmware
    tcmd typeCMD = readREGS;
    uint addrInit = 0;
    uint nRegs = 3;
    u16 value = 0;

    if (cmd == cmdGET_ACTION) {
        addrInit = 0x11;
        nRegs = 1;

    } else if (cmd == cmdGET_FAIL) {
        addrInit = 0x12;
        nRegs = 1;

    } else if (cmd == cmdGET_MASS) {
        addrInit = 0x13;
        nRegs = 1;

    } else if (cmd == cmdGET_ANGLE) {
        addrInit = 0x14;
        nRegs = 1;

    } else if (cmd == cmdSET_ANGLE) {
        typeCMD = writeREG;
        addrInit = 0x22;
        nRegs = 1;
        value = control.angle_value;

    } else if (cmd == cmdSET_CANCEL) {
        typeCMD = writeREG;
        addrInit = 0x23;
        nRegs = 1;
        value = 0;

    } else if (cmd == cmdSET_FALLSTART) {
        typeCMD = writeREG;
        addrInit = 0x21;
        nRegs = 1;
        value = 0;
    } else if (cmd == cmdGET_FALLTIME) {
        addrInit = 0x15 + control.position_sensor;

        if (control.position_sensor)
            nRegs = 2;
        else
            nRegs = 1;

    } else if (cmd == cmdSET_SENSORPOS) {
        typeCMD = writeREG;
        addrInit = 0x20;
        value = control.sensor_pos;
        nRegs = 1;
    }else if (cmd == cmdSET_COMPENSATION) {
        typeCMD = writeREG;
        addrInit = 0x24;
        value = control.compensation;
        nRegs = 1;
    }


    // ENVIA O COMANDO AO DISPOSITIVO ESCRAVO
    // -----------------------------------------------------------------------------------------------------------------
    int ret;
    if (typeCMD == writeREG) {
#if (LOG_MODBUS == pdON)
        printf("modbus WriteReg [cmd %d] [slave %d] [reg 0x%x] [value 0x%x]"CMD_TERMINATOR, cmd, control.rhID, addrInit, value);
#endif
        ret = modbus_MasterWriteRegister(control.rhID, addrInit, value);
    } else if (typeCMD == writeREGS) {
#if (LOG_MODBUS == pdON)
        printf("modbus WriteRegs [cmd %d] [slave %d] [reg 0x%x] [len %d]"CMD_TERMINATOR, cmd, control.rhID, addrInit, nRegs);
#endif
        ret = modbus_MasterWriteRegisters(control.rhID, addrInit, nRegs, regs);
    } else {
#if (LOG_MODBUS == pdON)
        printf("modbus ReadRegs [cmd %d] [slave %d] [reg 0x%x] [len %d]"CMD_TERMINATOR, cmd, control.rhID, addrInit, nRegs);
#endif
        ret = modbus_MasterReadRegisters(control.rhID, addrInit, nRegs, regs);
    }

    // se foi enviado com sucesso ficaremos na espera da resposta do recurso de hardware
    if (ret == pdPASS) {
        waitResponse = pdTRUE;
    }
#if (LOG_MODBUS == pdON)
        //else fprintf(flog, "modbus err[%d] send querie"CMD_TERMINATOR, modbus_MasterReadStatus());
    else {
        printf("modbus err[%d] SEND querie"CMD_TERMINATOR, modbus_MasterReadStatus());
        waitResponse = pdFALSE;
        init_control_tad();
    }
#endif

    return;
}



// processo do modbus.
//	Neste processo gerencia os envios de comandos para o recurso de hardware e fica no aguardo de sua resposta
//	Atualiza as variaveis do sistema de acordo com a resposta do recurso de hardware.

void * modbus_Process(void * params) {

    int first = 0;

    while (control.exit == 0) {

        if (first) {
            usleep(100);
        }
        first = 1;
        modbus_MasterProcess();

        // Gerenciador de envio de comandos
        // se n�o estamos esperando a resposta do SendCommand vamos analisar o pr�ximo comando a ser enviado
        if (!waitResponse) {
            if (control.getInfo) {
                modbus_SendCommand(cmdGET_INFO);

            } else if (control.SetCancel) {
                modbus_SendCommand(cmdSET_CANCEL);
                printf("Enviando %d\n", cmdSET_CANCEL);


            } else if (control.getAction) {
                modbus_SendCommand(cmdGET_ACTION);

            } else if (control.getStatus) {
                modbus_SendCommand(cmdGET_STATUS);

            } else if (control.SetForceSensor) {
                modbus_SendCommand(cmdSET_SENSORPOS);
                control.SetForceSensor = 0;

            } else if (control.SetAngle) {
                modbus_SendCommand(cmdSET_ANGLE);

            } else if (control.GetFail) {
                modbus_SendCommand(cmdGET_FAIL);

            } else if (control.GetMass) {
                modbus_SendCommand(cmdGET_MASS);

            } else if (control.GetAngle) {
                modbus_SendCommand(cmdGET_ANGLE);

            } else if (control.SetFallStart) {
                printf("Enviando %d\n", cmdSET_FALLSTART);
                modbus_SendCommand(cmdSET_FALLSTART);
                control.SetFallStart = 0;

            } else if (control.GetFallTime) {
                modbus_SendCommand(cmdGET_FALLTIME);
                printf("Enviando %d", cmdGET_FALLTIME);
            }else if (control.SetCompensation) {
                modbus_SendCommand(cmdSET_COMPENSATION);
                printf("Enviando %d", cmdSET_COMPENSATION);
                control.SetCompensation = 0;
            }
            

            continue;
        }

        int ret = modbus_MasterReadStatus();
        //	BUSY: Ficar na espera da resposta
        //  ERROR: Notificar  o erro e tomar procedimento cab�veis
        //  OK para escrita: Nada, pois os valores dos registradores foram salvos no escravo com sucesso
        //	OK para Leitura: Capturar os valores dos registradores lidos do escravo

        // se ainda est� ocupado n�o faz nada

        if (ret == errMODBUS_BUSY) {
            continue;
        }
        waitResponse = pdFALSE;
        if (ret < 0) {
            control.stsCom = modbus_MasterReadException();
#if (LOG_MODBUS == pdON)
            printf("modbus err[%d] WAIT response "CMD_TERMINATOR, ret);
            printf("status: %i\n", control.stsCom);
#endif

            continue;

        }

        control.stsCom = 5; // sinaliza que a conex�o foi feita com sucesso


        // ATUALIZA VARS QUANDO A COMUNICA��O FOI FEITA COM SUCESSO
        // -----------------------------------------------------------------------------------------------------------------
        // Comando para ler os registradores: modelo e vers�o firmware do RH
        if (cmd == cmdGET_FALLTIME) {

            if (control.position_sensor)
                control.falltime = (regs[1] << 0xff) | regs[0];
            else
                control.falltime = regs[0];

#if (LOG_MODBUS == pdON)
            printf("Fall time: %d s\n", control.falltime);
#endif
            control.GetFallTime = 0;

        } else if (cmd == cmdSET_CANCEL) {
            control.SetCancel = 0;

        } else if (cmd == cmdGET_INFO) {
#if (LOG_MODBUS == pdON)
            printf("model %c%c%c%c"CMD_TERMINATOR, (regs[0] & 0xff), (regs[0] >> 8), (regs[1] & 0xff), (regs[1] >> 8));
            printf("firware %c.%c"CMD_TERMINATOR, (regs[2] & 0xff), (regs[2] >> 8));
#endif
            control.rhModel[0] = (regs[0] & 0xff);
            control.rhModel[1] = (regs[0] >> 8);
            control.rhModel[2] = (regs[1] & 0xff);
            control.rhModel[3] = (regs[1] >> 8);
            control.rhModel[4] = 0;
            control.rhFirmware[0] = (regs[2] & 0xff);
            control.rhFirmware[1] = (regs[2] >> 8);
            control.rhFirmware[2] = 0;

            control.getInfo = 0; // sinalizo para n�o pegar mais informa��es

            // comando para ajuste dos reles, vamos sinalizar para n�o enviar mais comandos
        } else if (cmd == cmdGET_ACTION) {
            control.action = regs[0];
#if (LOG_MODBUS == pdON)
            //printf("Action: %d\n", control.action);
#endif
            control.getAction = 0; // sinalizo para n�o pegar mais a��o

            // Comando para o status do experimento
        } else if (cmd == cmdGET_STATUS) {
            control.status = regs[0];
            control.getStatus = 0;

        } else if (cmd == cmdSET_ANGLE) {
            control.SetAngle = 0;

        } else if (cmd == cmdGET_FAIL) {
            control.error = regs[0];
            control.GetFail = 0;

        } else if (cmd == cmdGET_MASS) {
            control.GetMass = 0;
            control.mass = regs[0];
#if (LOG_MODBUS == pdON)
            printf("Massa: %d g\n", control.mass);
#endif

        } else if (cmd = cmdGET_ANGLE) {
            control.GetAngle = 0;
            control.angle = (int) regs[0];

        } else if (cmd == cmdSET_FALLSTART) {
            control.SetFallStart = 0;

        } else if (cmd == cmdSET_SENSORPOS) {
            control.SetForceSensor = 0;

        } else if (cmd == cmdSET_COMPENSATION) {
            control.SetCompensation = 0;
        }
    }
    printf("Fechando programa"CMD_TERMINATOR);

    return NULL;
}

void init_control_tad() {
    control.rhID = 1; // Sinaliza que o endere�o do RH no modbus � 1
    control.getInfo = 0; // sinaliza para pegar as informa��es do RH
    control.expStart = 0;
    control.exit = 0;
    control.expCancel = 0;
    control.getAction = 0;
    control.getStatus = 0;
    control.SetAngle = 0;
    control.angle_value = 0;
    control.stsCom = 0;
    control.sensor_pos = 0;
    control.SetForceSensor = 0;
    control.GetFallTime = 0;
    control.SetCancel = 0;
    control.action = 0;
    control.status = 0;
    control.GetAngle = 0;
    control.SetFallStart = 0;
    control.GetFail = 0;
    control.compensation = 40;
    control.SetCompensation = 1;
    
    control.GetMass = 0;
    memset(control.rhModel, '\0', __STRINGSIZE__);
    memset(control.rhFirmware, '\0', __STRINGSIZE__);

    return;

}

NAN_METHOD(Setup) {
    NanScope();
    FILE *flog = fopen("msip.log", "w");

    if (modbus_Init() == 0) {
        fprintf(flog, "Erro ao abrir a porta UART"CMD_TERMINATOR);
        fprintf(flog, "Verifique se essa porta n�o esteja sendo usada por outro programa,"CMD_TERMINATOR);
        fprintf(flog, "ou se o usu�rio tem permiss�o para usar essa porta: chmod a+rw %s"CMD_TERMINATOR, COM_PORT);
        NanReturnValue(NanNew(0));

    }

    fclose(flog);

    NanReturnValue(NanNew(1));

}

NAN_METHOD(Run) {
    NanScope();
    char argv[15];
    std::string str_argv = *v8::String::Utf8Value(args[0]->ToString());
    strcpy(argv, str_argv.c_str());
    init_control_tad();

    int rthr = pthread_create(&id_thread, NULL, modbus_Process, (void *) 0); // fica funcionando at� que receba um comando via WEB para sair		
    if (rthr) {
        printf("Unable to create thread void * modbus_Process");

    }
    printf("Thread is gonna run\n");


    NanReturnValue(NanNew("1"));

}

NAN_METHOD(Exit) {
    NanScope();
    FILE * flog = fopen("msip.log", "w");
    init_control_tad();
    control.exit = 1;
    fclose(flog);
    uart_Close();
    NanReturnValue(NanNew("1"));
}

NAN_METHOD(Angle) {
    NanScope();
    int timeout = 50*TIMEOUT;
    if (args.Length() < 1) {
        NanThrowTypeError("Wrong number of arguments");
        NanReturnUndefined();
    }
    if (!args[0]->IsNumber()) {
        NanThrowTypeError("Wrong type of arguments, it should be integer");
        NanReturnUndefined();
    }


    do {
        control.getAction = 1;
        while (control.getAction) {
            usleep(100000);
            timeout--;
            if (timeout < 0) {
                control.getAction = 0;
                NanReturnValue(NanNew(-1));

            }
        }

    } while (control.action);
    control.action = 0;
    
    control.angle_value = (int) args[0]->NumberValue();
    control.SetAngle = 1;
    timeout = TIMEOUT;
    while (control.SetAngle) {
        usleep(100000);
        timeout--;
        if (timeout < 0) {
            control.SetAngle = 0;
            NanReturnValue(NanNew(-2));

        }

    }
    NanReturnValue(NanNew(1));
}

NAN_METHOD(Cancel) {
    NanScope();
    int timeout = TIMEOUT;
    control.SetCancel = 1;
    while (control.SetCancel) {
        usleep(100000);
        timeout--;
        if (timeout < 0) {
            control.SetCancel = 0;
            NanReturnValue(NanNew(-1));

        }
    }
    NanReturnValue(NanNew(1));
}

NAN_METHOD(WeighingPosition) {
    NanScope();
    int timeout = 50*TIMEOUT;
    if (args.Length() < 1) {
        NanThrowTypeError("Wrong number of arguments");
        NanReturnUndefined();
    }
    if (!args[0]->IsNumber()) {
        NanThrowTypeError("Wrong type of arguments, it should be integer");
        NanReturnUndefined();
    }

    do {
        control.getAction = 0;

        while (control.getAction) {
            usleep(100000);
            timeout--;
            if (timeout < 0) {
                init_control_tad();
                NanReturnValue(NanNew(-1));
                control.getAction = 0;
            }
        }

    } while (control.action);
    control.action = 0;

    if (args[0]->NumberValue() == 1) {
        control.sensor_pos = (int) args[0]->NumberValue();
    } else {
        control.sensor_pos = 0;

    }
    control.SetForceSensor = 1;
    timeout = TIMEOUT;
    while (control.SetForceSensor) {
        usleep(100000);
        timeout--;
        if (timeout < 0) {
            control.SetForceSensor = 0;
            NanReturnValue(NanNew(-1));

        }
    }

    NanReturnValue(NanNew(1));
}

NAN_METHOD(Weigh) {
    NanScope();
    int timeout = TIMEOUT;
    control.GetMass = 1;
    while (control.GetMass) {
        usleep(100000);
        timeout--;
        if (timeout < 0) {
            control.GetMass = 0;
            NanReturnValue(NanNew(-1));

        }
    }

    NanReturnValue(NanNew(control.mass));
}

NAN_METHOD(ExpStart) {
    NanScope();
    int timeout = 50*TIMEOUT;
    control.SetFallStart = 1;
    do {
        control.getAction = 1;

        while (control.getAction) {
            usleep(100000);
            timeout--;
            if (timeout < 0) {
                init_control_tad();
                NanReturnValue(NanNew(-1));
                control.getAction = 0;
            }
        }

    } while (control.action);
    control.action = 0;

    timeout = TIMEOUT;
    while (control.SetFallStart) {
        usleep(100000);
        timeout--;
        if (timeout < 0) {
            control.SetFallStart = 0;
            NanReturnValue(NanNew(-1));

        }
    }

    NanReturnValue(NanNew("1"));
}

NAN_METHOD(GetFallTime) {
    NanScope();
    char value[10];
    int timeout = 50*TIMEOUT;
    std::string buffertime = "\"time\": [";
    std::string bufferfalltime = "\"falltime\":";

    do {
        control.getAction = 1;

        while (control.getAction) {
            usleep(100000);
            timeout--;
            if (timeout < 0) {
                init_control_tad();
                NanReturnValue(NanNew(-1));
                control.getAction = 0;
            }
        }

    } while (control.action);
    control.action = 0;

    control.position_sensor = 0;
    control.GetFallTime = 1;
    timeout = TIMEOUT;
    while (control.GetFallTime) {
        usleep(10000);
        timeout--;
        if (timeout < 0) {
            control.GetFallTime = 0;
            NanReturnValue(NanNew("{\"error\": 3}"));

        }
    }
    sprintf(value, "%i", control.falltime);
    bufferfalltime = bufferfalltime + std::string(value);


    control.position_sensor = 1;
    int i;
    for (i = 0; i < NUMBER_OF_POSITIONS; i++) {
        control.GetFallTime = 1;
        timeout = TIMEOUT;
        while (control.GetFallTime) {
            usleep(10000);
            timeout--;
            if (timeout < 0) {
                control.GetFallTime = 0;
                NanReturnValue(NanNew("{\"error\": 3}"));

            }
        }
        sprintf(value, "%i", control.falltime);
        if (i < NUMBER_OF_POSITIONS - 1)
            buffertime = buffertime + std::string(value) + ",";
        else
            buffertime = buffertime + std::string(value) + "]";


        control.position_sensor = control.position_sensor + 2;

    }
    control.position_sensor = 0;


    NanReturnValue(NanNew("{" + buffertime + "," + bufferfalltime + "}"));
}

NAN_METHOD(GetAngle) {
    NanScope();
    std::string buffer = "angle:";
    int timeout = TIMEOUT;
    control.GetAngle = 1;
    while (control.GetAngle) {
        usleep(10000);
        timeout--;
        if (timeout < 0) {
            control.GetFallTime = 0;
            NanReturnValue(NanNew(-1));

        }
    }

    NanReturnValue(NanNew(control.angle));
}

void Init(Handle<Object> exports) { // cria fun��es para serem invocadas na aplica��o em nodejs
    exports->Set(NanNew("run"), NanNew<FunctionTemplate>(Run)->GetFunction());
    exports->Set(NanNew("setup"), NanNew<FunctionTemplate>(Setup)->GetFunction());
    exports->Set(NanNew("exit"), NanNew<FunctionTemplate>(Exit)->GetFunction());
    exports->Set(NanNew("setAngle"), NanNew<FunctionTemplate>(Angle)->GetFunction());
    exports->Set(NanNew("weighingPosition"), NanNew<FunctionTemplate>(WeighingPosition)->GetFunction());
    exports->Set(NanNew("cancel"), NanNew<FunctionTemplate>(Cancel)->GetFunction());
    exports->Set(NanNew("weigh"), NanNew<FunctionTemplate>(Weigh)->GetFunction());
    exports->Set(NanNew("angle"), NanNew<FunctionTemplate>(GetAngle)->GetFunction());
    exports->Set(NanNew("getfalltime"), NanNew<FunctionTemplate>(GetFallTime)->GetFunction());
    exports->Set(NanNew("start"), NanNew<FunctionTemplate>(ExpStart)->GetFunction());

}


NODE_MODULE(panel, Init)

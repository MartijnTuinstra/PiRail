#include <pthread.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "logger.h"
#include "com.h"
#include "mem.h"
#include "system.h"

#include "switchboard/unit.h"

#include "RNet_msg.h"

#include "Z21.h"

struct s_systemState * SYS;
pthread_t thread;

char * UART_Serial_Port;

int main(int argc, char *argv[]){

  if(argc > 1){
    printf("Got argument %s", argv[1]);
    UART_Serial_Port = (char *)_calloc(strlen(argv[1])+5, char);
    strcpy(UART_Serial_Port, argv[1]);
  }
  else{
    UART_Serial_Port = (char *)_calloc(30, char);
    strcpy(UART_Serial_Port, "/dev/ttyUSB1");
  }

  init_main();
  init_allocs();

  init_logger("uartlog.txt");
  set_level(DEBUG);
  set_logger_print_level(NONE);

  Units = (Unit **)_calloc(30, Unit *);
  unit_len = 30;

  Z21 = new Z21_Client();

/*
  pthread_create(&thread, NULL, UART, NULL);

  char cmd[300];
  char ** cmds = (char **)_calloc(20, char *);
  uint8_t max_cmds = 20;
  uint8_t cmds_len = 0;

  while (1){
    memset(cmds, 0, 20);
    cmds_len = 0;
    printf("> ");
    fgets(cmd,300,stdin);

    // Split command into arguments
    cmds[0] = strtok(cmd, " ");

    while( cmds[cmds_len] != NULL ) {
      cmds[++cmds_len] = strtok(NULL, " ");

      if(cmds_len + 1 > max_cmds){
        max_cmds += 20;
        loggerf(INFO, "Expand cmds to %d", max_cmds);
        cmds = (char * *)_realloc(cmds, max_cmds, char *);
      }
    }

    // Remove \n character from last command/argument
    cmds[cmds_len-1][strlen(cmds[cmds_len-1])-1] = 0;

    // If no command
    if(!cmds[0])
      continue;

    if(cmd[0] == 'q'){
      break;
    }
    else if(cmd[0] == 'h'){
      printf("Help:\n");
      printf("\ta [Item]            \tAdd an Item with ID\n");
      printf("\te [Item] [ID] {args}\tEdit an Item with ID\n");
      printf("\td [Item] [ID] {args}\tRemove an Item with ID\n");
      printf("\tp                   \tPrint configuration\n");
      printf("\tim [path]           \tImport web layout from path\n");
      printf("\tex {path}           \tExport web layout to path, defaults to Layout_export.txt\n");
      printf("\tpl                  \tPrint current web layout\n");
      printf("\ts                   \tSave configuration\n");
    }
    else if(strcmp(cmds[0], "DEVID") == 0){
      struct COM_t data;
      data.data[0] = 0xFF;
      data.data[1] = RNet_OPC_DEV_ID;
      data.length = 2;
      COM_Send(&data);
    }
    else if(strcmp(cmds[0], "SETOUT") == 0){
      if(cmds_len <= 3)
        continue;

      int M = atoi(cmds[1]);

      for(uint8_t i = 2; (i+1) < cmds_len; i+=2){
        int io = atoi(cmds[i]);
        union u_IO_event v;
        v.value = atoi(cmds[i+1]);

        COM_set_single_Output(M, io, v);
      }
    }
    else if(strcmp(cmds[0], "SETALLOUT") == 0){
      if(cmds_len == 1)
        continue;

      int M = atoi(cmds[1]);

      if(!Units[M]){
        char filename[30];
        sprintf(filename, "configs/units/%d.bin", M);
        auto configfile = new ModuleConfig(filename);
        configfile->read();

        new Unit(configfile);

        delete configfile;
      }

      Units[M]->updateIO(0);
    }
    else if(strcmp(cmds[0], "REQIN") == 0){
      if(cmds_len <= 1)
        continue;

      int M = atoi(cmds[1]);

      struct COM_t data;
      data.data[0] = M;
      data.data[1] = RNet_OPC_ReqReadInput;

      data.length = 2;

      COM_Send(&data);
    }
    else if(strcmp(cmds[0], "EMEGSTOP") == 0){
      loggerf(INFO, "Set Emergency Stop");

      struct COM_t data;
      data.data[0] = 0xFF; // Broadcast
      data.data[1] = RNet_OPC_SetEmergency;

      data.length = 2;

      COM_Send(&data);
    }
    else if(strcmp(cmds[0], "EMEGGO") == 0){
      loggerf(INFO, "Release Emergency Stop");

      struct COM_t data;
      data.data[0] = 0xFF; // Broadcast
      data.data[1] = RNet_OPC_RelEmergency;

      data.length = 2;

      COM_Send(&data);
    }
    else if(strcmp(cmds[0], "POWERON") == 0){
      loggerf(INFO, "Set Power On");

      struct COM_t data;
      data.data[0] = 0xFF; // Broadcast
      data.data[1] = RNet_OPC_PowerON;

      data.length = 2;

      COM_Send(&data);
    }
    else if(strcmp(cmds[0], "POWEROFF") == 0){
      loggerf(INFO, "Set Power Off");

      struct COM_t data;
      data.data[0] = 0xFF; // Broadcast
      data.data[1] = RNet_OPC_PowerOFF;

      data.length = 2;

      COM_Send(&data);
    }
    else if(strcmp(cmds[0], "CHANGENODE") == 0){
      if(cmds_len <= 2)
        continue;

      int M = atoi(cmds[1]);
      int newNode = atoi(cmds[2]);
      loggerf(INFO, "Reprogram node id %d -> %d", M, newNode);

      struct COM_t data;
      data.data[0] = M;
      data.data[1] = RNet_OPC_PowerOFF;

      data.length = 2;

      COM_Send(&data);

      COM_ACK = 0;
      COM_NACK = 0;

      while(COM_ACK == 0 && COM_NACK == 0){}

      loggerf(INFO, "Programming routine done");
    }
    else if(strcmp(cmds[0], "RESET") == 0){
      loggerf(INFO, "RESET UART with DTS signal");
      void COM_Reset();
    }
    // else if(strcmp(cmds[0], "ex") == 0 || strcmp(cmds[0], "Ex") == 0){
    //   export_Layout(&config, cmds[1]);
    // }
    // else if(strcmp(cmds[0], "im") == 0 || strcmp(cmds[0], "Im") == 0){
    //   import_Layout(&config, cmds[1]);
    // }
    // else if(strcmp(cmds[0],  "pL") == 0 || strcmp(cmds[0], "pl") == 0){
    //   print_Layout(&config);
    // }
    // else if(strcmp(cmds[0], "s") == 0){
    //   write_module_from_conf(&config, filename);
    // }
    // else if(strcmp(cmds[0], "p") == 0){
    //   print_module_config(&config, cmds, cmds_len);
    // }
  //   else
  //     printf("Not a command\n");
  }
*/
  SYS_set_state(&SYS->UART.state, Module_STOP);

  pthread_join(thread, NULL);

  _free(SYS);
  return 1;
}

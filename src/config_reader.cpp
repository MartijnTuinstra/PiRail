#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#include "utils/logger.h"
#include "utils/mem.h"

#include "uart/uart.h"
#include "uart/RNetRX.h"
#include "uart/RNetTX.h"

#include "RNet_msg.h"
#include "system.h"
#include "flags.h"
#include "utils/strings.h"

#include "switchboard/links.h"
#include "switchboard/manager.h"

#include "websocket/client.h"
#include "websocket/stc.h"

#include "config/LayoutStructure.h"
#include "config/RollingStructure.h"

#include "config/LayoutStructureEditor.h"
#include "config/RollingStructureEditor.h"

#include "config/ModuleConfig.h"
#include "config/RollingConfig.h"

#define TRAIN_CONF_PATH "configs/stock.bin"

namespace switchboard { Unit * Units(unsigned char a){return 0;}; Manager * SwManager; };
void WS_stc_Track_Layout(Websocket::Client*){}
void IO_Port::setInput(unsigned char){}


void configEditor_preview_string(char * buffer, const char * s){
  sprintf(buffer, "(%.10s)", s);
}

void configEditor_preview_uint8_t(char * buffer, uint8_t i){
  sprintf(buffer, "(%i)", i);
}

void configEditor_preview_uint16_t(char * buffer, uint16_t i){
  sprintf(buffer, "(%i)", i);
}

void configEditor_preview_bool(char * buffer, uint8_t b){
  sprintf(buffer, "(%c)", b ? 'Y' : 'N');
}

void configEditor_preview_string(char * buffer, char * s){
  sprintf(buffer, "(%s)", s);
}

void configEditor_scan_uint8_t(char * buffer, uint8_t * i){
  int t_i;
  if(sscanf(buffer, "%i", &t_i) > 0){
    *i = t_i;
  }
}

void configEditor_scan_uint16_t(char * buffer, uint16_t * i){
  int t_i;
  if(sscanf(buffer, "%i", &t_i) > 0){
    *i = t_i;
  }
}

void configEditor_scan_bool(char * buffer, uint8_t * b){
  char t_c;
  if(sscanf(buffer, "%c", &t_c) > 0){
    if (t_c == 'Y' || t_c == 'y')
      *b = 1;
    else if (t_c == 'N' || t_c == 'n')
      *b = 0;
  }
}

void configEditor_scan_bool(char * buffer, bool * b){
  char t_c;
  if(sscanf(buffer, "%c", &t_c) > 0){
    if (t_c == 'Y' || t_c == 'y')
      *b = 1;
    else if (t_c == 'N' || t_c == 'n')
      *b = 0;
  }
}

void configEditor_scan_string(char * buffer, char ** s, uint8_t * l){
  if(strlen(buffer) <= 1){
    return;
  }

  uint8_t bufferLen = strlen(buffer) - 1; // trailing newline
  buffer[bufferLen] = 0; // remove trailing newline

  *s = (char *)_realloc(*s, bufferLen, char);
  memcpy(*s, buffer, bufferLen+1);
  *l = bufferLen;
}

/*
void UART_ACK(uint8_t device){
  loggerf(INFO, "ACK");
  uart.ACK = true;
}
void UART_NACK(uint8_t device){
  loggerf(WARNING, "NACK");
  uart.NACK = true;
}

void COM_DevReset(){
  struct COM_t Tx;
  Tx.data[0] = 0xFF;  //Broadcast
  Tx.data[1] = RNet_OPC_DEV_ID;
  Tx.length  = 2;
  uart.send(&Tx);
}

void COM_DisconnectNotify(){
  // auto p = UART::Packet({0, 0}); // Broadcast
  // p.setOpcode(RNet_OPC_DisconnectNotify);

  // uart.send(&p);

  struct COM_t Tx;
  Tx.data[0] = 0xFF; // Broadcast
  Tx.data[1] = RNet_OPC_DisconnectNotify;
  Tx.length  = 2;
  uart.send(&Tx);
}


void (*UART_RecvCb[256])(uint8_t, uint8_t *) = {
  // 0x00 - 0x0F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x10 - 0x1F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x20 - 0x2F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x30 - 0x3F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x40 - 0x4F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x50 - 0x5F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x60 - 0x6F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x70 - 0x7D
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x7E
  (void (*)(uint8_t, uint8_t *))&UART_NACK,
  // 0x7F
  (void (*)(uint8_t, uint8_t *))&UART_ACK,

  // 0x80 - 0x8F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0x90 - 0x9F
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xA0 - 0xAF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xB0 - 0xBF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xC0 - 0xCF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xD0 - 0xDF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xE0 - 0xEF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  // 0xF0 - 0xFF
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
*/

void modify_Node(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return;

  int id;
  char mode = cmds[0][0];

  struct configStruct_Unit * const ConfigHeader = config->getHeader();

  if(cmds[0][0] == 'e'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Editing Node %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return;
    }
  }
  else if(cmds[0][0] == 'a'){
    printf("Node ID: (%i)\n", ConfigHeader->IO_Nodes);

    if(ConfigHeader->IO_Nodes == 0){
      printf("Calloc");
      config->Nodes = (struct configStruct_Node *)_calloc(1, struct configStruct_Node);
    }
    else{
      printf("Realloc");
      config->Nodes = (struct configStruct_Node *)_realloc(config->Nodes, ConfigHeader->IO_Nodes+1, struct configStruct_Node);
    }
    id = ConfigHeader->IO_Nodes++;

    memset(&config->Nodes[id], 0, sizeof(struct configStruct_Node));
    config->Nodes[id].Node = id;
  }
  else if(cmds[0][0] == 'r'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Removing Node %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return;
    }

    if(id == (ConfigHeader->IO_Nodes - 1) && id >= 0){
      memset(&config->Nodes[ConfigHeader->IO_Nodes - 1], 0, sizeof(struct configStruct_Node));
      config->Nodes = (struct configStruct_Node *)_realloc(config->Nodes, --ConfigHeader->IO_Nodes, struct configStruct_Node);
    }
    else{
      printf("Only last Node can be removed\n");
    }
  }


  if((mode == 'e' && cmd_len <= 3) || (mode == 'a' && cmd_len <= 2)){
    int tmp;
    char _cmd[20];
    printf("Node Size      (%i)         | ", config->Nodes[id].ports);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      if(config->Nodes[id].ports)
        config->Nodes[id].config = (struct configStruct_NodeIO *)_realloc(config->Nodes[id].config, tmp, struct configStruct_NodeIO);
      else
        config->Nodes[id].config = (struct configStruct_NodeIO *)_calloc(tmp, struct configStruct_NodeIO);

      config->Nodes[id].ports = tmp;
    }

    printf("New:      \t");
    print_Node(config->Nodes[id]);
  }
  else if(mode == 'e'){
    printf("Argment mode\n");
    cmds = &cmds[3];
    cmd_len -= 3;

    for(uint8_t i = 0; i < cmd_len;){
      printf("%s\t", cmds[i]);
      if(strcmp(cmds[i], "-h") == 0){
        // Help
        printf("Arguments for Node: \n");
        printf("\t-p              Number of IO\n");
        printf("\t-t [io] [type]  Set io type\n");
        return;
      }
      else if(strcmp(cmds[i], "-p") == 0){
        printf("set size");
        config->Nodes[id].ports = atoi(cmds[i+1]);
        config->Nodes[id].config = (struct configStruct_NodeIO *)_realloc(config->Nodes[id].config, config->Nodes[id].ports, struct configStruct_NodeIO);
        i+=2;
      }
      else if(strcmp(cmds[i], "-t") == 0){
        uint8_t port = atoi(cmds[i+1]);
        if(port > config->Nodes[id].ports){
          printf("Invalid io port\n");
          i += 3;
          continue;
        }
        printf("%d %i => %s\t%x %x\n", id, port, cmds[i+2], (0xF << (4 * ((port + 1)% 2))), (atoi(cmds[i+2]) & 0xF) << (4 * (port % 2)));
        config->Nodes[id].config[port].type = atoi(cmds[i+2]);
        printf("%lx\n", (unsigned long)&config->Nodes[id].config[port]);
        i += 3;
      }
      else if(strcmp(cmds[i], "-d") == 0){
        uint8_t port = atoi(cmds[i+1]);
        if(port > config->Nodes[id].ports){
          printf("Invalid io port\n");
          i += 3;
          continue;
        }

        config->Nodes[id].config[port].defaultState = atoi(cmds[i+2]);
        i += 3;
      }
      else if(strcmp(cmds[i], "-i") == 0){
        uint8_t port = atoi(cmds[i+1]);
        if(port > config->Nodes[id].ports){
          printf("Invalid io port\n");
          i += 3;
          continue;
        }

        bool inverting = (cmds[i+2][0] == '1');
        config->Nodes[id].config[port].inverted = inverting;
        i += 3;
      }
    }
    printf("\n");
  }
}

int8_t fgetScanf(const char * format, ...){
  char buffer[100];

  fgets(buffer, 100, stdin);

  va_list args;
  va_start (args, format);
  int8_t length = vsscanf(buffer, format, args);
  va_end (args);

  return length;
}

void scan_IO_Node(struct configStruct_IOport * port){
  int tmpNode, tmpPort;
  if(fgetScanf("%i%*c%i", &tmpNode, &tmpPort) > 1){
    port->Node = tmpNode;
    port->Port = tmpPort;
  }
}

void scan_RailLink(struct configStruct_RailLink * link){
  int tmp[3];
  if(fgetScanf("%i%*c%i%*c%i", &tmp[0], &tmp[1], &tmp[2]) > 2){
    link->module = tmp[0];
    link->id = tmp[1];
    link->type = tmp[2];
  }
}

template <class T>
T * modifyLayoutElement(T ** array, uint16_t * arraySize, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return 0;

  int id;
  char mode = cmds[0][0];
  T * tmpArray = *array;

  if(mode == 'e'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Editing ID %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return 0;
    }

    if(id >= *arraySize){
      printf("ID not valid!\n");
      return 0;
    }
  }
  else if(mode == 'a'){
    printf("Adding new item with ID: (%i)\n", *arraySize);

    if(*arraySize == 0){
      *array = (T *)_calloc(1, T);
    }
    else{
      *array = (T *)_realloc(*array, (*arraySize+1), T);
    }
    tmpArray = *array;
    memset(&(tmpArray[*arraySize]), 0, sizeof(T));
    tmpArray[*arraySize].id = *arraySize;
    id = *arraySize;
    *arraySize = *arraySize + 1;
  }
  else if(mode == 'r'){
    if(cmd_len > 2)
      id = atoi(cmds[2]);
    else{
      printf("No ID supplied\n");
      return 0;
    }

    if(id == (*arraySize - 1) && id >= 0){
      *arraySize = *arraySize - 1;
      memset(&tmpArray[*arraySize], 0, sizeof(T));
      *array = (T *)_realloc(*array, *arraySize, T);
    }
    else
      printf("Only last block can be removed (last is %i)\n", (*arraySize - 1));

    return 0;
  }

  return &tmpArray[id];
};

void modify_Block(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return;

  char mode = cmds[0][0];
  
  struct configStruct_Unit * const ConfigHeader = config->getHeader();
  struct configStruct_Block * B = modifyLayoutElement<struct configStruct_Block>(&config->Blocks, &ConfigHeader->Blocks, cmds, cmd_len);

  if(!B)
    return;

  if((mode == 'e' && cmd_len <= 3) || (mode == 'a' && cmd_len <= 2)){
    configEditor_Block(B);

    printf("New:      \t");
    print_Block(*B);
  }
  else if(mode == 'e'){
    printf("Argment mode\n");
    cmds = &cmds[3];
    cmd_len -= 3;

    for(uint8_t i = 0; i < cmd_len;){
      printf("%s\t", cmds[i]);
      if(strcmp(cmds[i], "-h") == 0){
        // Help
        printf("Arguments for Block: \n");
        printf("\t-t\tType\n");
        printf("\t-N\tNextLink\n");
        printf("\t-P\tPrevLink\n");
        printf("\t-s\tSpeed\n");
        printf("\t-l\tLength\n");
        printf("\t--OW\tOneWay\n");
        printf("\t-d\tDirection\n");
        printf("\t-r\tSwitch next and prev around\n");
        printf("\t-p\tPolarity type (0/1/2/3)\n");
        printf("\t--PIO [n] [IOPort:n] \tPolarity IO either 1 or 2 IO Ports\n");
        // printf("\t--IOIn\tIO Input Address\n");
        // printf("\t--IOOut\tIO Output Address\n");
        return;
      }
      else if(strcmp(cmds[i], "-t") == 0)
        configEditor_scan_uint8_t(cmds[++i], &B->type);
        
      else if(strcmp(cmds[i], "-N") == 0)
        configEditor_scan_RailLink(cmds[++i], &B->next);
        
      else if(strcmp(cmds[i], "-P") == 0)
        configEditor_scan_RailLink(cmds[++i], &B->prev);

      else if(strcmp(cmds[i], "-s") == 0)
        configEditor_scan_uint8_t(cmds[++i], &B->speed);

      else if(strcmp(cmds[i], "-l") == 0)
        configEditor_scan_uint16_t(cmds[++i], &B->length);

      else if(strcmp(cmds[i], "--OW") == 0){
        if(cmds[i+1][0] == 'Y' || cmds[i+1][0] == 'y')
          B->fl |= 0x1;
        else
          B->fl &= ~0x1;
        i++;
      }
      else if(strcmp(cmds[i], "-d") == 0){
        B->fl &= ~0b1110;
        B->fl |= (atoi(cmds[i+1]) & 0x3) << 1;
        i++;
      }
      else if(strcmp(cmds[i], "-r") == 0){
        std::swap(B->next, B->prev);
      }
      else if(strcmp(cmds[i], "-p") == 0){
        configEditor_scan_uint8_t(cmds[++i], &B->Polarity);
      }
      // else if(strcmp(cmds[i], "--PIO") == 0){
      //   uint8_t n = atoi(cmds[++i]);
      // }
      i++;
    }
    printf("\nNew:      \t");
    print_Block(*B);
  }
  else{
    printf("Mode not supported");
  }
}

void modify_PolarityGroup(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return;

  char mode = cmds[0][0];
  
  struct configStruct_Unit * const ConfigHeader = config->getHeader();
  struct configStruct_PolarityGroup * PG = modifyLayoutElement<struct configStruct_PolarityGroup>(&config->PolarityGroup, &ConfigHeader->PolarityGroup, cmds, cmd_len);

  if(!PG)
    return;

  if((mode == 'e' && cmd_len <= 3) || (mode == 'a' && cmd_len <= 2)){
    configEditor_PolarityGroup(PG);

    // printf("New:      \t");
    print_PolarityGroup(*PG);
  }
}

void modify_Switch(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  char mode = cmds[0][0];
  
  struct configStruct_Unit * const ConfigHeader = config->getHeader();
  struct configStruct_Switch * Sw = modifyLayoutElement<struct configStruct_Switch>(&config->Switches, &ConfigHeader->Switches, cmds, cmd_len);

  if(!Sw)
    return;

  if(mode == 'e' || mode == 'a'){
    configEditor_Switch(Sw);

    printf("New:      \t");
    print_Switch(*Sw);
  }
}

void modify_MSSwitch(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  // char _cmd[20];
  // int tmp, tmp1, tmp2;

  char mode = cmds[0][0];
  
  struct configStruct_Unit * const ConfigHeader = config->getHeader();
  struct configStruct_MSSwitch * Sw = modifyLayoutElement<struct configStruct_MSSwitch>(&config->MSSwitches, &ConfigHeader->MSSwitches, cmds, cmd_len);

  if(!Sw)
    return;

  if(mode == 'a'){
    // Initialize sub pointers
    Sw->nr_states = 1;
    Sw->states = (struct configStruct_MSSwitchState *)_calloc(1, struct configStruct_MSSwitchState);
    Sw->IO = 1;
    Sw->IO_Ports = (struct configStruct_IOport *)_calloc(1, struct configStruct_IOport);
  }
  // FIXME: add free when removing

  if(mode == 'e' || mode == 'a'){
    configEditor_MSSwitch(Sw);
    // printf("MSSwitch Detblock (%i)         | ", Sw->det_block);
    // fgets(_cmd, 20, stdin);
    // if(sscanf(_cmd, "%i", &tmp) > 0)
    //   Sw->det_block = tmp;

    // const char * typestring[3] = {"Crossing", "Turntable", "Traverse Table"};
    // printf("MSSwitch Type (%10s) | ", typestring[Sw->type]);
    // fgets(_cmd, 20, stdin);
    // if(sscanf(_cmd, "%i", &tmp) > 0)
    //   Sw->type = tmp;

    // printf("MSSwitch Nr States (%2i)      | ", Sw->nr_states);
    // fgets(_cmd, 20, stdin);
    // if(sscanf(_cmd, "%i", &tmp) > 0){
    //   if(tmp == 0){
    //     loggerf(ERROR, "Invalid number of states.");
    //     tmp = 1;
    //   }
    //   Sw->nr_states = tmp;
    //   Sw->states = (struct configStruct_MSSwitchState *)_realloc(Sw->states, tmp, struct configStruct_MSSwitchState);
    // }

    // printf("MSSwitch nr IO Ports  (%2i)    | ", Sw->IO);
    // fgets(_cmd, 20, stdin);
    // if(sscanf(_cmd, "%i", &tmp) > 0){
    //   if(tmp <= 16){
    //     Sw->IO = tmp;
    //     if(tmp == 0)
    //       tmp = 1;
    //     Sw->IO_Ports = (struct configStruct_IOport *)_realloc(Sw->IO_Ports, tmp, struct configStruct_IOport);
    //   }
    //   else{
    //     printf("Invalid length\n");
    //   }
    // }

    // for(int i = 0; i < Sw->nr_states; i++){
    //   printf("MSSwitch State %2i\n", i);
      
    //   printf(" - Link A (%2i:%2i:%2x)        | ",
    //             Sw->states[i].sideA.module,
    //             Sw->states[i].sideA.id,
    //             Sw->states[i].sideA.type);

    //   fgets(_cmd, 20, stdin);
    //   if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
    //     Sw->states[i].sideA.module = tmp;
    //     Sw->states[i].sideA.id = tmp1;
    //     Sw->states[i].sideA.type = tmp2;
    //   }
      
    //   printf(" - Link B (%2i:%2i:%2x)        | ",
    //             Sw->states[i].sideB.module,
    //             Sw->states[i].sideB.id,
    //             Sw->states[i].sideB.type);

    //   fgets(_cmd, 20, stdin);
    //   if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
    //     Sw->states[i].sideB.module = tmp;
    //     Sw->states[i].sideB.id = tmp1;
    //     Sw->states[i].sideB.type = tmp2;
    //   }
      
    //   printf(" - Speed (%3i)              | ",
    //             Sw->states[i].speed);

    //   fgets(_cmd, 20, stdin);
    //   if(sscanf(_cmd, "%i", &tmp) > 0){
    //     Sw->states[i].speed = tmp;
    //   }
      
    //   printf(" - State output (0x%4x)    | 0x",
    //             Sw->states[i].output_sequence);

    //   fgets(_cmd, 20, stdin);
    //   if(sscanf(_cmd, "%x", &tmp) > 0){
    //     Sw->states[i].output_sequence = tmp;
    //   }
      
    //   printf(" - Direction (%c)         | ",
    //             Sw->states[i].dir ? 'F' : 'R');

    //   fgets(_cmd, 20, stdin);
    //   if(sscanf(_cmd, "%d", &tmp) > 0){
    //     Sw->states[i].dir = tmp;
    //   }
    // }

    // for(int i = 0; i < Sw->IO; i++){
    //   printf("MSSwitch IO %2i - Adr (%2i:%2i)         | ",
    //             i,
    //             Sw->IO_Ports[i].Node,
    //             Sw->IO_Ports[i].Port);

    //   fgets(_cmd, 20, stdin);
    //   if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 1){
    //     Sw->IO_Ports[i].Node = tmp;
    //     Sw->IO_Ports[i].Port = tmp1;
    //   }
    // }
    printf("New:      \t");
    print_MSSwitch(*Sw, PRINT_ALL);
  }
}

void modify_Signal(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  // int id;
  // char _cmd[20];
  // int tmp, tmp1;//, tmp2;

  char mode = cmds[0][0];
    
  struct configStruct_Unit * const ConfigHeader = config->getHeader();
  struct configStruct_Signal * Sig = modifyLayoutElement<struct configStruct_Signal>(&config->Signals, &ConfigHeader->Signals, cmds, cmd_len);

  if(!Sig)
    return;

  if(mode == 'a'){
    // Initialize sub pointers
    // Sig->id = id;
    Sig->output_len = 1;
    Sig->output = (struct configStruct_IOport *)_calloc(1, struct configStruct_IOport);
    Sig->stating = (struct configStruct_SignalEvent *)_calloc(1, struct configStruct_SignalEvent);
  }
  // FIXME: add free when removing

  if(mode == 'e' || mode == 'a'){
    configEditor_Signal(Sig);
    // int tmp2;

    // printf("Signals Switches (%2i) | ", Sig->Switch_len);
    // fgets(_cmd, 20, stdin);
    // if(sscanf(_cmd, "%i", &tmp) > 0){
    //   Sig->Switches = (struct configStruct_SignalDependentSwitch *)_realloc(Sig->Switches, tmp, struct configStruct_SignalDependentSwitch);
    //   Sig->Switch_len = tmp;
    // }

    // for(int i = 0; i < Sig->Switch_len; i++){
    //   printf("-----------------------\n");
    //   printf(" - Switch %2i          |\n", i);
    //   printf(" - Switch type %4s   | ", Sig->Switches[i].type ? "MSSw" : "Sw");
    //   fgets(_cmd, 20, stdin);
    //   char tmpc;
    //   if(sscanf(_cmd, "%c", &tmpc) > 0){
    //     if(tmpc == 'M'){
    //       Sig->Switches[i].type = 1;
    //     }
    //     else if(tmpc == 'S'){
    //       Sig->Switches[i].type = 0;
    //     }
    //   }
    //   printf(" - Switch %2i          | ", Sig->Switches[i].Sw);
    //   fgets(_cmd, 20, stdin);
    //   if(sscanf(_cmd, "%i", &tmp) > 0){
    //     Sig->Switches[i].Sw = tmp;
    //   }
    //   printf(" - State %2i           | ", Sig->Switches[i].state);
    //   fgets(_cmd, 20, stdin);
    //   if(sscanf(_cmd, "%i", &tmp) > 0){
    //     Sig->Switches[i].state = tmp;
    //   }
    // }
    // printf("-----------------------\n");

    printf("New:      \t");
    print_Signals(*Sig);
  }
}

void modify_Station(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  char mode = cmds[0][0];
    
  struct configStruct_Unit * const ConfigHeader = config->getHeader();
  struct configStruct_Station * St = modifyLayoutElement<struct configStruct_Station>(&config->Stations, &ConfigHeader->Stations, cmds, cmd_len);

  if(!St)
    return;

  if(mode == 'a'){
    // Initialize sub pointers
    St->name_len = 1;
    St->name = (char *)_calloc(1, char);
    St->nr_blocks = 1;
    St->blocks = (uint8_t *)_calloc(1, uint8_t);
  }
  // FIXME: add free when removing

  if(mode == 'e' || mode == 'a'){
    configEditor_Station(St);

    printf("New:      \t");
    print_Stations(*St);
    // struct configStruct_Block tmp;
    // int dir, oneway, Out_en;
    // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
    //   &tmp.id, &tmp.type,
    //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
    //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
    //   &tmp.speed,
    //   &dir,
    //   &tmp.length,
    //   &oneway,
    //   &Out_en,
    //   &tmp.IO_In.Node, &tmp.IO_In.Port,
    //   &tmp.IO_Out.Node, &tmp.IO_Out.Port);

  }
}

void export_Layout(struct ModuleConfig * config, char * location){
  FILE * fp;
  /* open the file for writing*/
  if(location)
    fp = fopen (location,"w");
  else
    fp = fopen ("Layout_export.txt","w");

  if(!fp){
    printf("Failed to export layout\n");
    return;
  }

  /* write 10 lines of text into the file stream*/
  fprintf(fp, "%s", config->Layout->Layout);

  /* close the file*/  
  fclose (fp);
}

void import_Layout(struct ModuleConfig * config, char * location){
  FILE * fp;

  if(!location){
    char src[80];

    fgets(src, 2, stdin);
    printf("Location: ");
    fgets(src, 80, stdin);
    sscanf(src, "%s", src);
    /* open the file for reading*/
    fp = fopen (src,"r");
    if(!fp){
      loggerf(ERROR, "File not found '%s'", src);
      return;
    }
  }
  else{
    fp = fopen (location,"r");
    if(!fp){
      loggerf(ERROR, "File not found '%s'", location);
      return;
    }
  }


  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  config->Layout->Layout = (char *)_realloc(config->Layout->Layout, fsize, char);

  fread(config->Layout->Layout, fsize, 1, fp);
  // config->Layout[fsize] = 0;

  config->Layout->LayoutLength = fsize;

  /* close the file*/  
  fclose (fp);
}

void modify_Car(struct RollingConfig * config, char cmd){
  int id;
  char _cmd[80];
  int tmp;//, tmp1, tmp2;
  if(cmd == 'e'){
    printf("Car ID: ");
    fgets(_cmd, 80, stdin);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Car %i\n", id);
  }
  else if(cmd == 'a'){
    printf("Car ID: (%i)\n", config->header.Cars);
    fgets(_cmd, 80, stdin); // Clear stdin

    if(config->header.Cars == 0){
      printf("Calloc");
      config->Cars = (struct configStruct_Car *)_calloc(1, struct configStruct_Car);
    }
    else{
      printf("Realloc");
      config->Cars = (struct configStruct_Car *)_realloc(config->Cars, config->header.Cars+1, struct configStruct_Car);
    }
    memset(&config->Cars[config->header.Cars], 0, sizeof(struct configStruct_Car));
    id = config->header.Cars++;

    //Set child pointers
    config->Cars[id].name_len = 1;
    config->Cars[id].name = (char *)_calloc(1, char);
    config->Cars[id].icon_path_len = 1;
    config->Cars[id].icon_path = (char *)_calloc(1, char);
  }
  else if(cmd == 'r'){
    printf("Remove Car ID: ");
    fgets(_cmd, 80, stdin);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.Cars - 1) && id >= 0){
      _free(config->Cars[config->header.Cars - 1].name);
      _free(config->Cars[config->header.Cars - 1].icon_path);

      memset(&config->Cars[config->header.Cars - 1], 0, sizeof(struct configStruct_Car));
      config->Cars = (struct configStruct_Car *)_realloc(config->Cars, --config->header.Cars, struct configStruct_Car);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Reference Number (%i)         | ", config->Cars[id].nr);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Cars[id].nr = tmp;

    printf("Speedsteps  (%x)   | ", config->Cars[id].type >> 4);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      printf("(%i << 4) & (%i & 0x0f)\n", tmp & 0x0f, config->Cars[id].type);
      config->Cars[id].type = ((tmp & 0x0f) << 4) | (config->Cars[id].type & 0x0f);
    }

	printf("Type  (%x)   | ", config->Cars[id].type & 0x0f);
	fgets(_cmd, 80, stdin);
	if (sscanf(_cmd, "%i", &tmp) > 0) {
		config->Cars[id].type = (tmp & 0x0f) | (config->Cars[id].type & 0xf0);
	}

	printf("Detectable (%c)   | ", (config->Cars[id].flags & F_CAR_DETECTABLE) ? 'Y' : 'N');
	fgets(_cmd, 80, stdin);
	char tmpchar;
	if (sscanf(_cmd, "%c", &tmpchar) > 0) {
		if(tmpchar == 'Y' || tmpchar == 'y')
			config->Cars[id].flags |= F_CAR_DETECTABLE;
		else
			config->Cars[id].flags &= ~F_CAR_DETECTABLE;
	}

	printf("Length  (%2i)   | ", config->Cars[id].length);
	fgets(_cmd, 80, stdin);
	if (sscanf(_cmd, "%i", &tmp) > 0) {
		config->Cars[id].length = tmp;
	}

    printf("Maximum speed    (%i)         | ", config->Cars[id].max_speed);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Cars[id].max_speed = tmp;

    printf("Name      (%s) | ", config->Cars[id].name);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Cars[id].name = (char *)_realloc(config->Cars[id].name, tmp, char);
      memcpy(config->Cars[id].name, _cmd, tmp);
      config->Cars[id].name[tmp] = 0;
      config->Cars[id].name_len = tmp;
    }

    printf("Icon path (%s) | ", config->Cars[id].icon_path);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Cars[id].icon_path = (char *)_realloc(config->Cars[id].icon_path, tmp, char);
      memcpy(config->Cars[id].icon_path, _cmd, tmp);
      config->Cars[id].icon_path[tmp] = 0;
      config->Cars[id].icon_path_len = tmp;
    }

  //   printf("New:      \t");
  //   print_Stations(config->Stations[id]);
  //   // struct configStruct_Block tmp;
  //   // int dir, oneway, Out_en;
  //   // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
  //   //   &tmp.id, &tmp.type,
  //   //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
  //   //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
  //   //   &tmp.speed,
  //   //   &dir,
  //   //   &tmp.length,
  //   //   &oneway,
  //   //   &Out_en,
  //   //   &tmp.IO_In.Node, &tmp.IO_In.Port,
  //   //   &tmp.IO_Out.Node, &tmp.IO_Out.Port);

  }
}

void modify_Engine(struct RollingConfig * config, char ** cmds, uint8_t cmd_len){
  int id;
  int tmp, tmp1;//, tmp2;
  char mode = cmds[0][0];
  if(mode == 'e'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Editing block %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return;
    }
  }
  else if(mode == 'a'){
    printf("Engine ID: (%i)\n", config->header.Engines);

    if(config->header.Engines == 0){
      printf("Calloc");
      config->Engines = (struct configStruct_Engine *)_calloc(1, struct configStruct_Engine);
    }
    else{
      printf("Realloc");
      config->Engines = (struct configStruct_Engine *)_realloc(config->Engines, config->header.Engines+1, struct configStruct_Engine);
    }
    memset(&config->Engines[config->header.Engines], 0, sizeof(struct configStruct_Engine));
    // config->Engines[config->header.Engines].id = config->header.Engines;
    id = config->header.Engines++;

    //Set child pointers
    config->Engines[id].name_len = 1;
    config->Engines[id].name = (char *)_calloc(1, char);
    config->Engines[id].img_path_len = 1;
    config->Engines[id].img_path = (char *)_calloc(1, char);
    config->Engines[id].icon_path_len = 1;
    config->Engines[id].icon_path = (char *)_calloc(1, char);
    config->Engines[id].config_steps = 1;
    config->Engines[id].speed_steps = (struct configStruct_EngineSpeedSteps *)_calloc(1, struct configStruct_EngineSpeedSteps);
  }
  else if(mode == 'r'){
    if(cmd_len > 2){
      id = atoi(cmds[2]);
      printf("Removing block %i\n", id);
    }
    else{
      printf("No ID supplied\n");
      return;
    }

    if(id == (config->header.Engines - 1) && id >= 0){
      _free(config->Engines[config->header.Engines - 1].name);
      _free(config->Engines[config->header.Engines - 1].img_path);
      _free(config->Engines[config->header.Engines - 1].icon_path);
      _free(config->Engines[config->header.Engines - 1].speed_steps);

      memset(&config->Engines[config->header.Engines - 1], 0, sizeof(struct configStruct_Engine));
      config->Engines = (struct configStruct_Engine *)_realloc(config->Engines, --config->header.Engines, struct configStruct_Engine);
    }
    else{
      printf("Only last engine can be removed\n");
    }
  }


  if((mode == 'e' && cmd_len <= 3) || (mode == 'a' && cmd_len <= 2)){
    printf("DCC Address (%i)         | ", config->Engines[id].DCC_ID);
    char _cmd[80];
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Engines[id].DCC_ID = tmp;

    printf("Speedsteps  (%x)   | ", config->Engines[id].type >> 4);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      printf("(%i << 4) & (%i & 0x0f)\n", tmp & 0x0f, config->Engines[id].type);
      config->Engines[id].type = ((tmp & 0x0f) << 4) | (config->Engines[id].type & 0x0f);
    }

    printf("Type  (%x)   | ", config->Engines[id].type & 0x0f);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Engines[id].type = (tmp & 0x0f) | (config->Engines[id].type & 0xf0);
    }

    printf("Length  (%2i)   | ", config->Engines[id].length);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Engines[id].length = tmp;
    }

    printf("Calibrated Steps  (%2i)   | ", config->Engines[id].config_steps);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Engines[id].config_steps = tmp;
      _realloc(config->Engines[id].speed_steps, tmp, sizeof(struct configStruct_EngineSpeedSteps));
    }

    for(int i = 0; i < config->Engines[id].config_steps; i++){
      printf(" - Step %i (st%3i sp%3i)| ", i, config->Engines[id].speed_steps[i].step, config->Engines[id].speed_steps[i].speed);
      fgets(_cmd, 80, stdin);
      if(sscanf(_cmd, "%i %i", &tmp, &tmp1) > 1){
        if(tmp > 128){
          loggerf(WARNING, "Steps above 128 not allowed, step changed to 128");
          tmp = 128;
        }
        config->Engines[id].speed_steps[i].step = tmp;
        config->Engines[id].speed_steps[i].speed = tmp1;
      }
    }

    printf("Name      (%s) | ", config->Engines[id].name);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Engines[id].name = (char *)_realloc(config->Engines[id].name, tmp, char);
      memcpy(config->Engines[id].name, _cmd, tmp);
      config->Engines[id].name[tmp] = 0;
      config->Engines[id].name_len = tmp;
    }

    printf("Image path (%s) | ", config->Engines[id].img_path);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Engines[id].img_path = (char *)_realloc(config->Engines[id].img_path, tmp, char);
      memcpy(config->Engines[id].img_path, _cmd, tmp);
      config->Engines[id].img_path[tmp] = 0;
      config->Engines[id].img_path_len = tmp;
    }

    printf("Icon path (%s) | ", config->Engines[id].icon_path);
    fgets(_cmd, 80, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Engines[id].icon_path = (char *)_realloc(config->Engines[id].icon_path, tmp, char);
      memcpy(config->Engines[id].icon_path, _cmd, tmp);
      config->Engines[id].icon_path[tmp] = 0;
      config->Engines[id].icon_path_len = tmp;
    }
  }
  else if(mode == 'e'){
    printf("Argment mode\n");
    cmds = &cmds[3];
    cmd_len -= 3;

    for(uint8_t i = 0; i < cmd_len;){
      printf("%s\t", cmds[i]);
      if(strcmp(cmds[i], "-h") == 0){
        // Help
        printf("Arguments for Engine: \n");
        printf("\t-f\tFunction [Fn] [F_type] [F_Hold]\n");
        // printf("\t-N\tNextLink\n");
        // printf("\t-P\tPrevLink\n");
        // printf("\t-s\tSpeed\n");
        // printf("\t-l\tLength\n");
        // printf("\t--OW\tOneWay\n");
        // printf("\t-d\tDirection\n");
        // printf("\t--IOIn\tIO Input Address\n");
        // printf("\t--IOOut\tIO Output Address\n");
        return;
      }
      else if(strcmp(cmds[i], "-f") == 0){
        uint8_t Fn = atoi(cmds[i+1]);
        uint8_t Ftype = atoi(cmds[i+2]);
        uint8_t FHold = atoi(cmds[i+3]);
        loggerf(INFO, "Set F%i to type %i", Fn, Ftype);
        config->Engines[id].functions[Fn] = (FHold << 6) | (Ftype & 0x3F);
        i += 3;
      }
      i++;
    }
    printf("\n");
  }

  //   printf("New:      \t");
  //   print_Stations(config->Stations[id]);
  //   // struct configStruct_Block tmp;
  //   // int dir, oneway, Out_en;
  //   // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
  //   //   &tmp.id, &tmp.type,
  //   //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
  //   //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
  //   //   &tmp.speed,
  //   //   &dir,
  //   //   &tmp.length,
  //   //   &oneway,
  //   //   &Out_en,
  //   //   &tmp.IO_In.Node, &tmp.IO_In.Port,
  //   //   &tmp.IO_Out.Node, &tmp.IO_Out.Port);
}

void modify_TrainSet(struct RollingConfig * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1;//, tmp2;
  if(cmd == 'e'){
    printf("Train ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Train %i\n", id);
  }
  else if(cmd == 'a'){
    printf("Train ID: (%i)\n", config->header.TrainSets);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header.TrainSets == 0){
      printf("Calloc");
      config->TrainSets = (struct configStruct_TrainSet *)_calloc(1, struct configStruct_TrainSet);
    }
    else{
      printf("Realloc");
      config->TrainSets = (struct configStruct_TrainSet *)_realloc(config->TrainSets, config->header.TrainSets+1, struct configStruct_TrainSet);
    }
    memset(&config->TrainSets[config->header.TrainSets], 0, sizeof(struct configStruct_TrainSet));
    id = config->header.TrainSets++;

    //Set child pointers
    config->TrainSets[id].name_len = 1;
    config->TrainSets[id].name = (char *)_calloc(1, char);
    config->TrainSets[id].nr_stock = 1;
    config->TrainSets[id].composition = (struct configStruct_TrainSetLink *)_calloc(1, struct configStruct_TrainSetLink);
  }
  else if(cmd == 'r'){
    printf("Remove Train ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header.TrainSets - 1) && id >= 0){
      _free(config->TrainSets[config->header.TrainSets - 1].name);

      memset(&config->Engines[config->header.TrainSets - 1], 0, sizeof(struct configStruct_TrainSet));
      config->TrainSets = (struct configStruct_TrainSet *)_realloc(config->TrainSets, --config->header.TrainSets, struct configStruct_TrainSet);
    }
    else{
      printf("Only last engine can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Nr stock (%i)         | ", config->TrainSets[id].nr_stock);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->TrainSets[id].nr_stock = tmp;
      config->TrainSets[id].composition = (struct configStruct_TrainSetLink *)_realloc(config->TrainSets[id].composition, tmp, struct configStruct_TrainSetLink);
    }

    printf("Name      (%s) | ", config->TrainSets[id].name);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->TrainSets[id].name = (char *)_realloc(config->TrainSets[id].name, tmp, char);
      memcpy(config->TrainSets[id].name, _cmd, tmp);
      config->TrainSets[id].name[tmp] = 0;
      config->TrainSets[id].name_len = tmp;
    }

    printf("Catagory  (%i) | ", config->TrainSets[id].category);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->TrainSets[id].category = tmp;

    for(int i = 0; i < config->TrainSets[id].nr_stock; i++){
      printf(" - Step %i   (%02i, %04i) | ", i, config->TrainSets[id].composition[i].type, config->TrainSets[id].composition[i].id);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i %i", &tmp, &tmp1) > 1){
        config->TrainSets[id].composition[i].type = tmp;
        config->TrainSets[id].composition[i].id = tmp1;
      }
    }

  //   printf("New:      \t");
  //   print_Stations(config->Stations[id]);
  //   // struct configStruct_Block tmp;
  //   // int dir, oneway, Out_en;
  //   // scanf("%i\t%i\t%2i:%2i:%c\t\t%2i:%2i:%c\t\t%i\t%i\t%i\t%i\t%i\t%i:%i\t%i:%i",
  //   //   &tmp.id, &tmp.type,
  //   //   &tmp.next.module, &tmp.next.id, &tmp.next.type,
  //   //   &tmp.prev.module, &tmp.prev.id, &tmp.prev.type,
  //   //   &tmp.speed,
  //   //   &dir,
  //   //   &tmp.length,
  //   //   &oneway,
  //   //   &Out_en,
  //   //   &tmp.IO_In.Node, &tmp.IO_In.Port,
  //   //   &tmp.IO_Out.Node, &tmp.IO_Out.Port);

  }
}

void modify_Catagory(struct RollingConfig * config, char type, char cmd){
  int id;
  char _cmd[20];
  int tmp;//, tmp2;

  uint8_t * _header;
  struct configStruct_Category ** _cat;
  if(type == 'P'){
    _header = &config->header.PersonCatagories;
    _cat = &config->P_Cat;
  }
  else{
    _header = &config->header.CargoCatagories;
    _cat = &config->C_Cat;
  }



  if(cmd == 'e'){
    printf("Catagory ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Catagory %i\n", id);

    if (type == 'C' && id > 0x80)
      id -= 0x80;
  }
  else if(cmd == 'a'){
    printf("Catagory ID: (%i)\n", *_header);

    fgets(_cmd, 20, stdin); // Clear stdin

    if(*_header == 0){
      printf("Calloc");
      *_cat = (struct configStruct_Category *)_calloc(1, struct configStruct_Category);
    }
    else{
      printf("Realloc");
      *_cat = (struct configStruct_Category *)_realloc(*_cat, (*_header)+1, struct configStruct_Category);
    }
    memset(&(*_cat)[*_header], 0, sizeof(struct configStruct_Category));
    id = *_header;
    *_header += 1;

    //Set child pointers
    (*_cat)[id].name_len = 1;
    (*_cat)[id].name = (char *)_calloc(1, char);
  }
  else if(cmd == 'r'){
    printf("Remove Train ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (*_header - 1) && id >= 0){
      _free((*_cat)[*_header - 1].name);

      memset(&(*_cat)[*_header - 1], 0, sizeof(struct configStruct_Category));
      *_cat = (struct configStruct_Category *)_realloc(*_cat, --(*_header), struct configStruct_Category);
    }
    else{
      printf("Only last engine can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Name      (%s) | ", (*_cat)[id].name);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      (*_cat)[id].name = (char *)_realloc((*_cat)[id].name, tmp, char);
      memcpy((*_cat)[id].name, _cmd, tmp);
      (*_cat)[id].name_len = tmp;
    }
  }
}

int edit_module(char * filename, bool update){
  auto config = ModuleConfig(filename);
  int exit = 1;

  config.read();

  if(!config.parsed){
    loggerf(ERROR, "Failed to open file");
    loggerf(INFO,  "Creating New File");
    
    int connections, id;

    printf("ID? ");
    scanf("%i", &id);
    printf("How many sides does the module connect? ");
    scanf("%i", &connections);

    config.newModule(id, connections);
  }

  if(update){
    // uint8_t len = strlen(config.filename);
    config.write();
    return exit;
  }

  config.print(0, 0);

  char cmd[300];
  char ** cmds = (char **)_calloc(20, char *);
  uint8_t max_cmds = 20;
  uint8_t cmds_len = 0;

  while (1){
    memset(cmds, 0, 20 * sizeof(char *));
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
      printf("\ta, add    [Item]            \tAdd an Item with ID\n");
      printf("\te, edit   [Item] [ID] {args}\tEdit an Item with ID\n");
      printf("\tr, remove [Item] [ID] {args}\tRemove an Item with ID\n");
      printf("\tp                           \tPrint configuration\n");
      printf("\tim [path]                   \tImport web layout from path\n");
      printf("\tex {path}                   \tExport web layout to path, defaults to Layout_export.txt\n");
      printf("\tpl                          \tPrint current web layout\n");
      printf("\ts                           \tSave configuration\n");
      printf("\twio [dev]                   \tWrite configuration to hardware nodes\n");
    }
    else if(strcmp(cmds[0], "e")    == 0 || strcmp(cmds[0], "a")   == 0 || strcmp(cmds[0], "r")      == 0 ||
            strcmp(cmds[0], "edit") == 0 || strcmp(cmds[0], "add") == 0 || strcmp(cmds[0], "remove") == 0   ){
      if(cmds_len < 2){
        printf("Command too short\n");
        continue;
      }

      cmds[0][1] = 0; // Reduce edit/add/remove to one letter

      if(strcmp(cmds[1], "B") == 0){
        modify_Block(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "PG") == 0){
        modify_PolarityGroup(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "Sw") == 0){
        modify_Switch(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "MSSw") == 0){
        modify_MSSwitch(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "Sig") == 0){
        modify_Signal(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "St") == 0){
        modify_Station(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "N") == 0){
        modify_Node(&config, cmds, cmds_len);
      }
    }
    else if(strcmp(cmds[0], "ex") == 0 || strcmp(cmds[0], "Ex") == 0){
      export_Layout(&config, cmds[1]);
    }
    else if(strcmp(cmds[0], "im") == 0 || strcmp(cmds[0], "Im") == 0){
      import_Layout(&config, cmds[1]);
    }
    else if(strcmp(cmds[0],  "pL") == 0 || strcmp(cmds[0], "pl") == 0){
      print_Layout(config.Layout);
    }
    else if(strcmp(cmds[0], "s") == 0){
      if(config.write())
        loggerf(INFO, "Config Saved");
      else
        loggerf(ERROR, "Error while saving");
    }
    else if(strcmp(cmds[0], "wio") == 0){
      if(cmds_len < 3){
        loggerf(ERROR, "Not enough arguments\nusage: wio comport node_id\n");
        continue;
      }

      uint8_t offset = 0;

      if(cmds_len > 3){
        if(strcmp(cmds[3], "-o") == 0 && cmds_len > 4)
          offset = atoi(cmds[4]);
      }

      loggerf(DEBUG, "wio [%s] [%s]", cmds[1], cmds[2]);

      uart.setDevice((const char *)cmds[1]);

      uint8_t NodeID = atoi(cmds[2]);

      if(NodeID >= config.getHeader()->IO_Nodes){
        loggerf(ERROR, "Node ID does not exist");
        continue;
      }

      bool stop = false;
      uint8_t send = 0;
      uint8_t i = 0;
      
      struct configStruct_Node * N = &config.Nodes[NodeID];

      if(!uart.init()){
        loggerf(ERROR, "Failed to open serial device");
        continue;
      }

      loggerf(INFO, "Writing IO configuration to Node %i", NodeID);

      // Disable functions that are not needed / will crash due to uninitialized objects
      UART_RecvCb[RNet_OPC_DEV_ID] = 0;
      UART_RecvCb[RNet_OPC_ReadInput] = 0;

      usleep(1500000);

      uart.recv(); // Recv first 32

      while(!stop && i < N->ports && send < 10){
        if(uart.recv()){
          uart.parse();
        }

        if(send == 0){
          uint16_t PortConfig = (N->config[i].type << 12) | (N->config[i].defaultState << 8) | (N->config[i].inverted);
          COM_Configure_IO(config.getHeader()->Module, i + offset, PortConfig);
          send = 1;
        }
        else{
          send++;
        }

        if(uart.NACK){
          loggerf(WARNING, "Failed to write io %i, aborting ...", i);
          stop = true;
        }
        else if(uart.ACK){
          loggerf(INFO, "set IO config %i succesfull", i);
          i++;
          uart.ACK = 0;
          send = 0;
        }

        usleep(500);

        // loggerf(INFO, "!(%i) && %i < %i && %i < 10", stop, i, N->ports, send);
      }

      uart.close();
    }
    else if(strcmp(cmds[0], "p") == 0){
      config.print(cmds, cmds_len);
    }
  //   else
  //     printf("Not a command\n");
  }


  if(cmds_len > 1 && strcmp(cmds[1], "-r") == 0){
    exit = -1;
  }

  _free(cmds);

  // //free switches
  // for(int i = 0; i < config.header->Switches; i++){
  //   _free(config.Switches[i].IO_Ports);
  // }

  // //free msswitches
  // for(int i = 0; i < config.header->MSSwitches; i++){
  //   _free(config.MSSwitches[i].states);
  //   _free(config.MSSwitches[i].IO_Ports);
  // }

  // //free signals
  // for(int i = 0; i < config.header->Signals; i++){
  //   _free(config.Signals[i].output);
  //   _free(config.Signals[i].stating);
  // }

  // //free station
  // for(int i = 0; i < config.header->Stations; i++){
  //   _free(config.Stations[i].name);
  //   _free(config.Stations[i].blocks);
  // }

  // _free(config.Nodes);
  // _free(config.Blocks);
  // _free(config.Switches);
  // _free(config.MSSwitches);
  // _free(config.Signals);
  // _free(config.Stations);

  return exit;
}

int edit_rolling_stock(char * filename, bool update){
  loggerf(WARNING, "edit_rolling_stock %s", filename);
  auto config = RollingConfig(filename);

  config.read();

  if(!config.parsed){
    loggerf(ERROR, "Failed to open file");
    printf("No File");
    return -1;
  }

  if(update){
    config.write();
    return -1;
  }

  config.print(0, 0);

  char cmd[300];
  char ** cmds = (char **)_calloc(20, char *);
  uint8_t max_cmds = 20;
  uint8_t cmds_len = 0;

  while (1){
    memset(cmds, 0, 20 * sizeof(char *));
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

    if(strcmp(cmds[0], "q") == 0){
      break;
    }
    else if(strcmp(cmds[0], "e") == 0 || strcmp(cmds[0], "a") == 0 || strcmp(cmds[0], "r") == 0){
      if(cmds_len < 2){
        printf("Command too short\nPlease use [e/a/r] [C/E/T/P/C]");
        continue;
      }

      if(strcmp(cmds[1], "C") == 0){
        modify_Car(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "E") == 0){
        modify_Engine(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "T") == 0){
        modify_TrainSet(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "P_Cat") == 0){
        modify_Catagory(&config, 'P', cmds[0][0]);
      }
      else if(strcmp(cmds[1], "C_Cat") == 0){
        modify_Catagory(&config, 'C', cmds[0][0]);
      }
    }
    else if(strcmp(cmds[0], "s") == 0){
      config.write();
    }
    else if(strcmp(cmds[0], "p") == 0){
      config.print();
    }
    else
      printf("Not a command\n");
  }

  printf("Done\n");
  return 1;
}

///////////////////////////

struct s_systemState * SYS;

int main(int argc, char ** argv){
  logger.setfilename("log.txt");
  logger.setlevel(INFO);
  logger.setlevel_stdout(INFO);

  logger.setDetailLevel(3);

  int editing = 0;
  bool filename_arg = false;
  bool update = false;
  char filename[100] = {0};
  std::vector<char *> extrafiles;

  #define MODULE_EDITING 1
  #define ROLLING_EDITING 2

  for(int i = 1; i < argc; i++){
    if(!argv[i])
      break;

    // printf("cmd arg: %s\n", argv[i]);

    if(strcmp(argv[i], "--update") == 0)
      update = true;
    else if(strcmp(argv[i], "--module") == 0 && !editing)
        editing = MODULE_EDITING;
    else if(strcmp(argv[i], "--rolling") == 0 && !editing)
      editing = ROLLING_EDITING;
    else if(strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0){
      logger.setlevel_stdout(DEBUG);
    }
    else{ // filename
      // printf("file arg: %s\n", argv[i]);
      if(argv[i][0] == '-'){
        loggerf(WARNING, "Invalid argument: %s\n", argv[i]);
      }
      else if(!filename_arg){
        filename_arg = true;
        strcpy(filename, argv[i]);
      }
      else{
        char * str = (char *)_calloc(strlen(argv[i])+5, char);
        // printf("%x  %d", (int)((unsigned long)str), (int)strlen(argv[i]));
        strcpy(str, argv[i]);
        extrafiles.push_back(str);
      }
    }
  }

  loggerf(INFO, "%s%s %s %c\n", editing == MODULE_EDITING ? "Editing module config":"", editing == ROLLING_EDITING ? "Editing Rolling config":"", filename, update ? 'U':' ');

  restart:

  if(editing == MODULE_EDITING && filename_arg){
    edit_module(filename, update);
  }
  else if(editing == ROLLING_EDITING){
    if(filename_arg)
      edit_rolling_stock(filename, update);
    else
      edit_rolling_stock(TRAIN_CONF_PATH, update);
  }
  else{
    printf("Unknown command");
  }

  if(extrafiles.size() > 0){
    printf("Extra files Restart!!\n");
    strcpy(filename, extrafiles[0]);
    _free(extrafiles[0]);
    extrafiles.erase(extrafiles.begin());

    goto restart;
  }

  printf("Done");

  for(auto i: extrafiles)
    _free(i);
  
  loggerf(INFO, "STOPPED");

  print_allocs();
}

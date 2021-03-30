#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#include "utils/logger.h"
#include "utils/mem.h"

#include "uart/uart.h"
#include "RNet_msg.h"
#include "system.h"

#include "switchboard/links.h"

#include "config/LayoutStructure.h"
#include "config/RollingStructure.h"

#include "config/ModuleConfig.h"
#include "config/RollingConfig.h"

#define TRAIN_CONF_PATH "configs/stock.bin"

const char * rail_states_string[8] = {
  "BLOCKED",
  "DANGER",
  "RESTRICTED",
  "CAUTION",
  "PROCEED",
  "RESERVED",
  "RESERVED_SWITCH",
  "UNKNOWN" 
};


void UART_ACK(uint8_t device){
  loggerf(INFO, "ACK");
  uart.ACK = true;
}
void UART_NACK(uint8_t device){
  loggerf(WARNING, "NACK");
  uart.NACK = true;
}

void COM_Configure_IO(uint8_t M, uint8_t ioPort, uint16_t config){
  struct COM_t TX;
  TX.data[0] = M;
  TX.data[1] = RNet_OPC_SetIO;
  TX.data[2] = ioPort;
  TX.data[3] = (config & 0xFF);
  TX.data[4] = (config >> 8);
  TX.data[5] = UART_CHECKSUM_SEED ^ RNet_OPC_SetIO ^ ioPort ^ (config & 0xFF) ^ (config >> 8);
  TX.length = 6;
  uart.send(&TX);
}

void COM_DevReset(){
  struct COM_t Tx;
  Tx.data[0] = 0xFF;  //Broadcast
  Tx.data[1] = RNet_OPC_DEV_ID;
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


void modify_Node(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return;

  int id;
  char mode = cmds[0][0];
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
    printf("Node ID: (%i)\n", config->header->IO_Nodes);

    if(config->header->IO_Nodes == 0){
      printf("Calloc");
      config->Nodes = (struct configStruct_Node *)_calloc(1, struct configStruct_Node);
    }
    else{
      printf("Realloc");
      config->Nodes = (struct configStruct_Node *)_realloc(config->Nodes, config->header->IO_Nodes+1, struct configStruct_Node);
    }
    id = config->header->IO_Nodes++;

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

    if(id == (config->header->IO_Nodes - 1) && id >= 0){
      memset(&config->Nodes[config->header->IO_Nodes - 1], 0, sizeof(struct configStruct_Node));
      config->Nodes = (struct configStruct_Node *)_realloc(config->Nodes, --config->header->IO_Nodes, struct configStruct_Node);
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
        printf("%x\n", config->Nodes[id].config[port]);
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

void modify_Block(struct ModuleConfig * config, char ** cmds, uint8_t cmd_len){
  if(!cmds)
    return;
  
  int id;
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
    printf("Block ID: (%i)\n", config->header->Blocks);

    if(config->header->Blocks == 0){
      printf("Calloc");
      config->Blocks = (struct configStruct_Block *)_calloc(1, struct configStruct_Block);
    }
    else{
      printf("Realloc");
      config->Blocks = (struct configStruct_Block *)_realloc(config->Blocks, config->header->Blocks+1, struct configStruct_Block);
    }
    memset(&config->Blocks[config->header->Blocks], 0, sizeof(struct configStruct_Block));
    config->Blocks[config->header->Blocks].id = config->header->Blocks;
    id = config->header->Blocks++;

    config->Blocks[id].next.module = config->header->Module;
    config->Blocks[id].next.id = id + 1;

    config->Blocks[id].prev.module = config->header->Module;
    config->Blocks[id].prev.id = id - 1;
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

    if(id == (config->header->Blocks - 1) && id >= 0){
      memset(&config->Blocks[config->header->Blocks - 1], 0, sizeof(struct configStruct_Block));
      config->Blocks = (struct configStruct_Block *)_realloc(config->Blocks, --config->header->Blocks, struct configStruct_Block);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if((mode == 'e' && cmd_len <= 3) || (mode == 'a' && cmd_len <= 2)){
    char _cmd[20];
    int tmp, tmp1, tmp2;
    char tmpc;
    printf("Block Type      (%i)         | ", config->Blocks[id].type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Blocks[id].type = tmp;

    printf("Block Link Next (%2i:%2i:%2x)  | ", config->Blocks[id].next.module, config->Blocks[id].next.id, config->Blocks[id].next.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      config->Blocks[id].next.module = tmp;
      config->Blocks[id].next.id = tmp1;
      config->Blocks[id].next.type = tmp2;
    }

    printf("Block Link Prev (%2i:%2i:%2x)  | ", config->Blocks[id].prev.module, config->Blocks[id].prev.id, config->Blocks[id].prev.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      config->Blocks[id].prev.module = tmp;
      config->Blocks[id].prev.id = tmp1;
      config->Blocks[id].prev.type = tmp2;
    }

    printf("Block Speed     (%3i)       | ", config->Blocks[id].speed);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Blocks[id].speed = tmp;

    printf("Block Length    (%3i)       | ", config->Blocks[id].length);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Blocks[id].length = tmp;

    printf("Block Oneway    (%c)         | ",(config->Blocks[id].fl & 1)?'Y':'N');

    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%c", &tmpc) > 0){
      if(tmpc == 'Y' || tmpc == 'y')
        config->Blocks[id].fl |= 0x1;
      else
        config->Blocks[id].fl &= ~0x1;
    }

    printf("Block Direction (%i)         | ", (config->Blocks[id].fl & 0x6) >> 1);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Blocks[id].fl = ((tmp & 0x3) << 1) + (~0x6 & config->Blocks[id].fl);

    printf("Block IO out en (");
    if(config->Blocks[id].fl & 8)
      printf("Y)         | ");
    else
      printf("N)         | ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%c", &tmpc) > 0){
      if(tmpc == 'Y' || tmpc == 'y')
        config->Blocks[id].fl |= 0x8;
      else
        config->Blocks[id].fl &= ~0x8;
    }

    printf("Block IO In     (%2i:%2i)      | ", config->Blocks[id].IO_In.Node, config->Blocks[id].IO_In.Port);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
      config->Blocks[id].IO_In.Node = tmp;
      config->Blocks[id].IO_In.Port = tmp1;
    }

    if(config->Blocks[id].fl & 0x8){
      printf("Block IO Out    (%2i:%2i)       | ", config->Blocks[id].IO_Out.Node, config->Blocks[id].IO_Out.Port);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
        config->Blocks[id].IO_Out.Node = tmp;
        config->Blocks[id].IO_Out.Port = tmp1;
      }
    }
    printf("New:      \t");
    print_Block(config->Blocks[id]);
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
        // printf("\t--IOIn\tIO Input Address\n");
        // printf("\t--IOOut\tIO Output Address\n");
        return;
      }
      else if(strcmp(cmds[i], "-t") == 0){
        config->Blocks[id].type = atoi(cmds[i+1]);
        i++;
      }
      else if(strcmp(cmds[i], "-N") == 0 || strcmp(cmds[i], "-P") == 0){
        struct configStruct_RailLink * link;
        char _type[10];
        uint8_t _m, _id;

        if(cmds[i][1] == 'N')
          link = &config->Blocks[id].next;
        else
          link = &config->Blocks[id].prev;

        if(sscanf(cmds[i+1], "%hhu:%hhu:%s", &_m, &_id, _type) > 0){
          link->module = _m;
          link->id = _id;

          if(_type[0] == 'E')
            link->type = RAIL_LINK_E;
          else if(_type[0] == 'C')
            link->type = RAIL_LINK_C;
          else
            link->type = atoi(_type);
        }
        i++;
      }
      else if(strcmp(cmds[i], "-s") == 0){
        config->Blocks[id].speed = atoi(cmds[i+1]);
        i++;
      }
      else if(strcmp(cmds[i], "-l") == 0){
        config->Blocks[id].length = atoi(cmds[i+1]);
        i++;
      }
      else if(strcmp(cmds[i], "--OW") == 0){
        if(cmds[i+1][0] == 'Y' || cmds[i+1][0] == 'y')
          config->Blocks[id].fl |= 0x1;
        else
          config->Blocks[id].fl &= ~0x1;
        i++;
      }
      else if(strcmp(cmds[i], "-d") == 0){
        config->Blocks[id].fl &= ~0b1110;
        config->Blocks[id].fl |= (atoi(cmds[i+1]) & 0x3) << 1;
        i++;
      }
      i++;
    }
    printf("\n");
  }
  else{
    printf("Mode not supported");
  }
}

void modify_Switch(struct ModuleConfig * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1, tmp2;
  if(cmd == 'e'){
    printf("Switch ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Switch %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Switch ID: (%i)\n", config->header->Switches);

    if(config->header->Switches == 0){
      printf("Calloc");
      config->Switches = (struct configStruct_Switch *)_calloc(1, struct configStruct_Switch);
    }
    else{
      printf("Realloc");
      config->Switches = (struct configStruct_Switch *)_realloc(config->Switches, config->header->Switches+1, struct configStruct_Switch);
    }
    memset(&config->Switches[config->header->Switches], 0, sizeof(struct configStruct_Switch));
    config->Switches[config->header->Switches].id = config->header->Switches;
    id = config->header->Switches++;
  }
  else if(cmd == 'r'){
    printf("Remove Switch ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header->Switches - 1) && id >= 0){
      memset(&config->Switches[config->header->Switches - 1], 0, sizeof(struct configStruct_Switch));
      config->Switches = (struct configStruct_Switch *)_realloc(config->Switches, --config->header->Switches, struct configStruct_Switch);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    struct configStruct_Switch * Sw = &config->Switches[id];

    printf("Switch Detblock (%i)         | ", Sw->det_block);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      Sw->det_block = tmp;

    printf("Switch Link App (%2i:%2i:%2x)  | ", Sw->App.module, Sw->App.id, Sw->App.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      Sw->App.module = tmp;
      Sw->App.id = tmp1;
      Sw->App.type = tmp2;
    }

    printf("Switch Link Str (%2i:%2i:%2x)  | ", Sw->Str.module, Sw->Str.id, Sw->Str.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      Sw->Str.module = tmp;
      Sw->Str.id = tmp1;
      Sw->Str.type = tmp2;
    }

    printf("Switch Link Div (%2i:%2i:%2x)  | ", Sw->Div.module, Sw->Div.id, Sw->Div.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      Sw->Div.module = tmp;
      Sw->Div.id = tmp1;
      Sw->Div.type = tmp2;
    }

    printf("Switch Div Speed (%3i)  | ", Sw->speed_Div);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      Sw->speed_Div = tmp;
    }

    printf("Switch Str Speed (%3i)  | ", Sw->speed_Str);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      Sw->speed_Str = tmp;
    }

    printf("Switch IO_Ports  (%2i)       | ", Sw->IO_length);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      Sw->IO_length = tmp;
      Sw->IO_Ports = (struct configStruct_IOport *)_realloc(Sw->IO_Ports, Sw->IO_length, struct configStruct_IOport);
      Sw->IO_Event = (uint8_t *)_realloc(Sw->IO_Event, Sw->IO_length * 2, uint8_t);
    }

    printf("Switch IO Type   (%i)        | ", Sw->IO_type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      Sw->IO_type = tmp;

    for(int i = 0; i < Sw->IO_length; i++){
      printf("Switch IO%2i  (%2i:%2i)        | ", i, Sw->IO_Ports[i].Node, Sw->IO_Ports[i].Port);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
        Sw->IO_Ports[i].Node = tmp;
        Sw->IO_Ports[i].Port = tmp1;
      }
    }

    for(int i = 0; i < (Sw->IO_length * 2); i++){
      printf("IO event %2i  (%2i:%2i %s->%i) | ", i/2, Sw->IO_Ports[i/2].Node, Sw->IO_Ports[i/2].Port, (i % 2) ? "div" : "str", Sw->IO_Event[i]);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i", &tmp) > 0){
        Sw->IO_Event[i] = tmp;
      }
    }

    printf("Enable Feedback (%i)         | ", Sw->feedback_len);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      Sw->feedback_len = tmp;
      Sw->FB_Ports = (struct configStruct_IOport *)_realloc(Sw->FB_Ports, Sw->feedback_len, struct configStruct_IOport);
      Sw->FB_Event = (uint8_t *)_realloc(Sw->FB_Event, Sw->feedback_len * 2, uint8_t);
    }

    if(Sw->feedback_len){

      for(int i = 0; i < Sw->feedback_len; i++){
        printf("Feedback IO%2i  (%2i:%2i)      | ", i, Sw->FB_Ports[i].Node, Sw->FB_Ports[i].Port);
        fgets(_cmd, 20, stdin);
        if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
          Sw->FB_Ports[i].Node = tmp;
          Sw->FB_Ports[i].Port = tmp1;
        }
      }

      for(int i = 0; i < (Sw->feedback_len * 2); i++){
        printf("FB event %2i  (%2i:%2i %s->%i) | ", i/2, Sw->FB_Ports[i/2].Node, Sw->FB_Ports[i/2].Port, (i % 2) ? "div" : "str", Sw->FB_Event[i]);
        fgets(_cmd, 20, stdin);
        if(sscanf(_cmd, "%i", &tmp) > 0){
          Sw->FB_Event[i] = tmp;
        }
      }

    }
    printf("New:      \t");
    print_Switch(*Sw);
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

void modify_MSSwitch(struct ModuleConfig * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1, tmp2;
  if(cmd == 'e'){
    printf("MSSwitch ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1 || id >= config->header->MSSwitches || id < 0){
      printf("Invalid\n");
      return;
    }
    printf("Editing MSSwitch %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("MSSwitch ID: (%i)\n", config->header->MSSwitches);

    if(config->header->MSSwitches == 0){
      printf("Calloc");
      config->MSSwitches = (struct configStruct_MSSwitch *)_calloc(1, struct configStruct_MSSwitch);
    }
    else{
      printf("Realloc");
      config->MSSwitches = (struct configStruct_MSSwitch *)_realloc(config->MSSwitches, config->header->MSSwitches+1, struct configStruct_MSSwitch);
    }
    memset(&config->MSSwitches[config->header->MSSwitches], 0, sizeof(struct configStruct_MSSwitch));
    id = config->header->MSSwitches++;

    //Set child pointers
    config->MSSwitches[id].id = id;
    config->MSSwitches[id].nr_states = 1;
    config->MSSwitches[id].states = (struct configStruct_MSSwitchState *)_calloc(1, struct configStruct_MSSwitchState);
    config->MSSwitches[id].IO = 1;
    config->MSSwitches[id].IO_Ports = (struct configStruct_IOport *)_calloc(1, struct configStruct_IOport);
  }
  else if(cmd == 'r'){
    printf("Remove Switch ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header->MSSwitches - 1) && id >= 0){
      memset(&config->MSSwitches[config->header->MSSwitches - 1], 0, sizeof(struct configStruct_MSSwitch));
      config->MSSwitches = (struct configStruct_MSSwitch *)_realloc(config->MSSwitches, --config->header->MSSwitches, struct configStruct_MSSwitch);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("MSSwitch Detblock (%i)         | ", config->MSSwitches[id].det_block);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->MSSwitches[id].det_block = tmp;

    const char * typestring[3] = {"Crossing", "Turntable", "Traverse Table"};
    printf("MSSwitch Type (%10s) | ", typestring[config->MSSwitches[id].type]);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->MSSwitches[id].type = tmp;

    printf("MSSwitch Nr States (%2i)      | ", config->MSSwitches[id].nr_states);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      if(tmp == 0){
        loggerf(ERROR, "Invalid number of states.");
        tmp = 1;
      }
      config->MSSwitches[id].nr_states = tmp;
      config->MSSwitches[id].states = (struct configStruct_MSSwitchState *)_realloc(config->MSSwitches[id].states, tmp, struct configStruct_MSSwitchState);
    }

    printf("MSSwitch nr IO Ports  (%2i)    | ", config->MSSwitches[id].IO);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      if(tmp <= 16){
        config->MSSwitches[id].IO = tmp;
        if(tmp == 0)
          tmp = 1;
        config->MSSwitches[id].IO_Ports = (struct configStruct_IOport *)_realloc(config->MSSwitches[id].IO_Ports, tmp, struct configStruct_IOport);
      }
      else{
        printf("Invalid length\n");
      }
    }

    for(int i = 0; i < config->MSSwitches[id].nr_states; i++){
      printf("MSSwitch State %2i\n", i);
      
      printf(" - Link A (%2i:%2i:%2x)        | ",
                config->MSSwitches[id].states[i].sideA.module,
                config->MSSwitches[id].states[i].sideA.id,
                config->MSSwitches[id].states[i].sideA.type);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
        config->MSSwitches[id].states[i].sideA.module = tmp;
        config->MSSwitches[id].states[i].sideA.id = tmp1;
        config->MSSwitches[id].states[i].sideA.type = tmp2;
      }
      
      printf(" - Link B (%2i:%2i:%2x)        | ",
                config->MSSwitches[id].states[i].sideB.module,
                config->MSSwitches[id].states[i].sideB.id,
                config->MSSwitches[id].states[i].sideB.type);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
        config->MSSwitches[id].states[i].sideB.module = tmp;
        config->MSSwitches[id].states[i].sideB.id = tmp1;
        config->MSSwitches[id].states[i].sideB.type = tmp2;
      }
      
      printf(" - Speed (%3i)              | ",
                config->MSSwitches[id].states[i].speed);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i", &tmp) > 0){
        config->MSSwitches[id].states[i].speed = tmp;
      }
      
      printf(" - State output (0x%4x)    | 0x",
                config->MSSwitches[id].states[i].output_sequence);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%x", &tmp) > 0){
        config->MSSwitches[id].states[i].output_sequence = tmp;
      }
      
      printf(" - Direction (%c)         | ",
                config->MSSwitches[id].states[i].dir ? 'F' : 'R');

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%d", &tmp) > 0){
        config->MSSwitches[id].states[i].dir = tmp;
      }
    }

    for(int i = 0; i < config->MSSwitches[id].IO; i++){
      printf("MSSwitch IO %2i - Adr (%2i:%2i)         | ",
                i,
                config->MSSwitches[id].IO_Ports[i].Node,
                config->MSSwitches[id].IO_Ports[i].Port);

      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 1){
        config->MSSwitches[id].IO_Ports[i].Node = tmp;
        config->MSSwitches[id].IO_Ports[i].Port = tmp1;
      }
    }
    printf("New:      \t");
    print_MSSwitch(config->MSSwitches[id]);
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

void modify_Signal(struct ModuleConfig * config, char cmd){
  int id;
  char _cmd[20];
  int tmp, tmp1;//, tmp2;
  if(cmd == 'e'){
    printf("Signal ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Signal %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Signal ID: (%i)\n", config->header->Signals);

    if(config->header->Signals == 0){
      printf("Calloc");
      config->Signals = (struct configStruct_Signal *)_calloc(1, struct configStruct_Signal);
    }
    else{
      printf("Realloc");
      config->Signals = (struct configStruct_Signal *)_realloc(config->Signals, config->header->Signals+1, struct configStruct_Signal);
    }
    memset(&config->Signals[config->header->Signals], 0, sizeof(struct configStruct_Signal));
    id = config->header->Signals++;

    //Set child pointers
    config->Signals[id].id = id;
    config->Signals[id].output_len = 1;
    config->Signals[id].output = (struct configStruct_IOport *)_calloc(1, struct configStruct_IOport);
    config->Signals[id].stating = (struct configStruct_SignalEvent *)_calloc(1, struct configStruct_SignalEvent);
  }
  else if(cmd == 'r'){
    printf("Remove Signal ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header->Signals - 1) && id >= 0){
      _free(config->Signals[id].output);
      _free(config->Signals[id].stating);

      memset(&config->Signals[config->header->Signals - 1], 0, sizeof(struct configStruct_Signal));
      config->Signals = (struct configStruct_Signal *)_realloc(config->Signals, --config->header->Signals, struct configStruct_Signal);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    int tmp2;
    printf("Signals Block link (%2i:%2i:%2x)  | ", config->Signals[id].block.module, config->Signals[id].block.id, config->Signals[id].block.type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i%*c%i%*c%i", &tmp, &tmp1, &tmp2) > 2){
      config->Signals[id].block.module = tmp;
      config->Signals[id].block.id = tmp1;
      config->Signals[id].block.type = tmp2;
    }
    
    
    printf("Signals Block direction (%c)| ", config->Signals[id].direction ? 'F' : 'R');
    
    fgets(_cmd, 20, stdin);
    char tmp_char;
    if(sscanf(_cmd, "%c", &tmp_char) > 0){
      printf("Got %i\n", tmp_char);
      if(tmp_char == 'F')
        config->Signals[id].direction = 1; // NEXT
      else if(tmp_char == 'R')
        config->Signals[id].direction = 0; // PREV
    }
    
    printf("Signals Outputs (%2i)  | ", config->Signals[id].output_len);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Signals[id].output = (struct configStruct_IOport *)_realloc(config->Signals[id].output, tmp, struct configStruct_IOport);
      config->Signals[id].stating = (struct configStruct_SignalEvent *)_realloc(config->Signals[id].stating, tmp, struct configStruct_SignalEvent);
      config->Signals[id].output_len = tmp;
    }

    for(int i = 0; i < config->Signals[id].output_len; i++){
      printf("-----------------------\n");
      printf(" - IO-Address: (%02i:%02i) | ", config->Signals[id].output[i].Node, config->Signals[id].output[i].Port);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i%*c%i", &tmp, &tmp1) > 0){
        config->Signals[id].output[i].Node = tmp;
        config->Signals[id].output[i].Port = tmp1;
      }
      for(int j = 0; j < 8; j++){
        printf("   IO-Event %15s: (%2i)    | ", rail_states_string[j], config->Signals[id].stating[i].event[j]);
        fgets(_cmd, 20, stdin);
        if(sscanf(_cmd, "%i", &tmp) > 0){
          config->Signals[id].stating[i].event[j] = tmp;
        }
      }
    }
    printf("-----------------------\n");
    
    printf("Signals Switches (%2i) | ", config->Signals[id].Switch_len);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Signals[id].Switches = (struct configStruct_SignalDependentSwitch *)_realloc(config->Signals[id].Switches, tmp, struct configStruct_SignalDependentSwitch);
      config->Signals[id].Switch_len = tmp;
    }

    for(int i = 0; i < config->Signals[id].Switch_len; i++){
      printf("-----------------------\n");
      printf(" - Switch %2i          |\n", i);
      printf(" - Switch type %4s   | ", config->Signals[id].Switches[i].type ? "MSSw" : "Sw");
      fgets(_cmd, 20, stdin);
      char tmpc;
      if(sscanf(_cmd, "%c", &tmpc) > 0){
        if(tmpc == 'M'){
          config->Signals[id].Switches[i].type = 1;
        }
        else if(tmpc == 'S'){
          config->Signals[id].Switches[i].type = 0;
        }
      }
      printf(" - Switch %2i          | ", config->Signals[id].Switches[i].Sw);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i", &tmp) > 0){
        config->Signals[id].Switches[i].Sw = tmp;
      }
      printf(" - State %2i           | ", config->Signals[id].Switches[i].state);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i", &tmp) > 0){
        config->Signals[id].Switches[i].state = tmp;
      }
    }
    printf("-----------------------\n");

    printf("New:      \t");
    print_Signals(config->Signals[id]);
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

void modify_Station(struct ModuleConfig * config, char cmd){
  int id;
  char _cmd[20];
  int tmp;//, tmp1, tmp2;
  if(cmd == 'e'){
    printf("Station ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;
    printf("Editing Station %i\n", id);
    // print_Block(config->Blocks[id]);
  }
  else if(cmd == 'a'){
    printf("Station ID: (%i)\n", config->header->Stations);

    if(config->header->Stations == 0){
      printf("Calloc");
      config->Stations = (struct configStruct_Station *)_calloc(1, struct configStruct_Station);
    }
    else{
      printf("Realloc");
      config->Stations = (struct configStruct_Station *)_realloc(config->Stations, config->header->Stations+1, struct configStruct_Station);
    }
    memset(&config->Stations[config->header->Stations], 0, sizeof(struct configStruct_Station));
    id = config->header->Stations++;

    //Set child pointers
    config->Stations[id].name_len = 1;
    config->Stations[id].name = (char *)_calloc(1, char);
    config->Stations[id].nr_blocks = 1;
    config->Stations[id].blocks = (uint8_t *)_calloc(1, uint8_t);
  }
  else if(cmd == 'r'){
    printf("Remove Station ID: ");
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header->Stations - 1) && id >= 0){
      memset(&config->Stations[config->header->Stations - 1], 0, sizeof(struct configStruct_Station));
      config->Stations = (struct configStruct_Station *)_realloc(config->Stations, --config->header->Stations, struct configStruct_Station);
    }
    else{
      printf("Only last block can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Station Type (%i)         | ", config->Stations[id].type);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Stations[id].type = tmp;

    printf("Station Parrent (%i)        | ", config->Stations[id].parent);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%d", &tmp) > 0)
      config->Stations[id].parent = tmp;

    printf("Station Name (%s)\n\t\t\t | ", config->Stations[id].name);
    char _name[41] = "";
    fgets(_name, 40, stdin);
    if(strlen(_name) > 1){
      uint8_t _name_len = strlen(_name) - 1; // trailing newline
      config->Stations[id].name = (char *)_realloc(config->Stations[id].name, _name_len, char);
      memcpy(config->Stations[id].name, _name, _name_len);
      config->Stations[id].name[_name_len] = 0;
      config->Stations[id].name_len = _name_len;
    }

    printf("Station blocks nr (%2i)   | ", config->Stations[id].nr_blocks);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Stations[id].nr_blocks = tmp;
      config->Stations[id].blocks = (uint8_t *)_realloc(config->Stations[id].blocks, tmp, uint8_t);
    }

    for(int i = 0; i < config->Stations[id].nr_blocks; i++){
      printf("Station Block %2i (%2i)    | ", i, config->Stations[id].blocks[i]);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i", &tmp) > 0){
        config->Stations[id].blocks[i] = tmp;
      }
    }

    printf("New:      \t");
    print_Stations(config->Stations[id]);
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
    printf("Car ID: (%i)\n", config->header->Cars);
    fgets(_cmd, 80, stdin); // Clear stdin

    if(config->header->Cars == 0){
      printf("Calloc");
      config->Cars = (struct configStruct_Car *)_calloc(1, struct configStruct_Car);
    }
    else{
      printf("Realloc");
      config->Cars = (struct configStruct_Car *)_realloc(config->Cars, config->header->Cars+1, struct configStruct_Car);
    }
    memset(&config->Cars[config->header->Cars], 0, sizeof(struct configStruct_Car));
    id = config->header->Cars++;

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

    if(id == (config->header->Cars - 1) && id >= 0){
      _free(config->Cars[config->header->Cars - 1].name);
      _free(config->Cars[config->header->Cars - 1].icon_path);

      memset(&config->Cars[config->header->Cars - 1], 0, sizeof(struct configStruct_Car));
      config->Cars = (struct configStruct_Car *)_realloc(config->Cars, --config->header->Cars, struct configStruct_Car);
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
    printf("Engine ID: (%i)\n", config->header->Engines);

    if(config->header->Engines == 0){
      printf("Calloc");
      config->Engines = (struct configStruct_Engine *)_calloc(1, struct configStruct_Engine);
    }
    else{
      printf("Realloc");
      config->Engines = (struct configStruct_Engine *)_realloc(config->Engines, config->header->Engines+1, struct configStruct_Engine);
    }
    memset(&config->Engines[config->header->Engines], 0, sizeof(struct configStruct_Engine));
    // config->Engines[config->header->Engines].id = config->header->Engines;
    id = config->header->Engines++;

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

    if(id == (config->header->Engines - 1) && id >= 0){
      _free(config->Engines[config->header->Engines - 1].name);
      _free(config->Engines[config->header->Engines - 1].img_path);
      _free(config->Engines[config->header->Engines - 1].icon_path);
      _free(config->Engines[config->header->Engines - 1].speed_steps);

      memset(&config->Engines[config->header->Engines - 1], 0, sizeof(struct configStruct_Engine));
      config->Engines = (struct configStruct_Engine *)_realloc(config->Engines, --config->header->Engines, struct configStruct_Engine);
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

void modify_Train(struct RollingConfig * config, char cmd){
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
    printf("Train ID: (%i)\n", config->header->Trains);
    fgets(_cmd, 20, stdin); // Clear stdin

    if(config->header->Trains == 0){
      printf("Calloc");
      config->Trains = (struct configStruct_Train *)_calloc(1, struct configStruct_Train);
    }
    else{
      printf("Realloc");
      config->Trains = (struct configStruct_Train *)_realloc(config->Trains, config->header->Trains+1, struct configStruct_Train);
    }
    memset(&config->Trains[config->header->Trains], 0, sizeof(struct configStruct_Train));
    id = config->header->Trains++;

    //Set child pointers
    config->Trains[id].name_len = 1;
    config->Trains[id].name = (char *)_calloc(1, char);
    config->Trains[id].nr_stock = 1;
    config->Trains[id].composition = (struct configStruct_TrainComp *)_calloc(1, struct configStruct_TrainComp);
  }
  else if(cmd == 'r'){
    printf("Remove Train ID: ");
    fgets(_cmd, 20, stdin);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &id) < 1)
      return;

    if(id == (config->header->Trains - 1) && id >= 0){
      _free(config->Trains[config->header->Trains - 1].name);

      memset(&config->Engines[config->header->Trains - 1], 0, sizeof(struct configStruct_Train));
      config->Trains = (struct configStruct_Train *)_realloc(config->Trains, --config->header->Trains, struct configStruct_Train);
    }
    else{
      printf("Only last engine can be removed\n");
    }
  }


  if(cmd == 'e' || cmd == 'a'){
    printf("Nr stock (%i)         | ", config->Trains[id].nr_stock);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0){
      config->Trains[id].nr_stock = tmp;
      config->Trains[id].composition = (struct configStruct_TrainComp *)_realloc(config->Trains[id].composition, tmp, struct configStruct_TrainComp);
    }

    printf("Name      (%s) | ", config->Trains[id].name);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%s", _cmd) > 0){
      tmp = strlen(_cmd);
      config->Trains[id].name = (char *)_realloc(config->Trains[id].name, tmp, char);
      memcpy(config->Trains[id].name, _cmd, tmp);
      config->Trains[id].name[tmp] = 0;
      config->Trains[id].name_len = tmp;
    }

    printf("Catagory  (%i) | ", config->Trains[id].category);
    fgets(_cmd, 20, stdin);
    if(sscanf(_cmd, "%i", &tmp) > 0)
      config->Trains[id].category = tmp;

    for(int i = 0; i < config->Trains[id].nr_stock; i++){
      printf(" - Step %i   (%02i, %04i) | ", i, config->Trains[id].composition[i].type, config->Trains[id].composition[i].id);
      fgets(_cmd, 20, stdin);
      if(sscanf(_cmd, "%i %i", &tmp, &tmp1) > 1){
        config->Trains[id].composition[i].type = tmp;
        config->Trains[id].composition[i].id = tmp1;
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
    _header = &config->header->PersonCatagories;
    _cat = &config->P_Cat;
  }
  else{
    _header = &config->header->CargoCatagories;
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
      printf("\ta [Item]            \tAdd an Item with ID\n");
      printf("\te [Item] [ID] {args}\tEdit an Item with ID\n");
      printf("\td [Item] [ID] {args}\tRemove an Item with ID\n");
      printf("\tp                   \tPrint configuration\n");
      printf("\tim [path]           \tImport web layout from path\n");
      printf("\tex {path}           \tExport web layout to path, defaults to Layout_export.txt\n");
      printf("\tpl                  \tPrint current web layout\n");
      printf("\ts                   \tSave configuration\n");
    }
    else if(strcmp(cmds[0], "e") == 0 || strcmp(cmds[0], "a") == 0 || strcmp(cmds[0], "r") == 0){
      if(cmds_len < 2){
        printf("Command too short\n");
        continue;
      }

      if(strcmp(cmds[1], "B") == 0){
        modify_Block(&config, cmds, cmds_len);
      }
      else if(strcmp(cmds[1], "Sw") == 0){
        modify_Switch(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "MSSw") == 0){
        modify_MSSwitch(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "Sig") == 0){
        modify_Signal(&config, cmds[0][0]);
      }
      else if(strcmp(cmds[1], "St") == 0){
        modify_Station(&config, cmds[0][0]);
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
      config.write();
      printf("Config Saved\n");
    }
    else if(strcmp(cmds[0], "wio") == 0){
      printf("Write to IO");

      if(cmds_len < 3){
        printf("Not enough arguments\n wio comport node_id\n");
        continue;
      }

      uint8_t offset = 0;

      if(cmds_len > 3){
        if(strcmp(cmds[3], "-o") == 0 && cmds_len > 4)
          offset = atoi(cmds[4]);
      }

      printf("wio [%s] [%s]", cmds[1], cmds[2]);

      uart.setDevice((const char *)cmds[1]);

      uint8_t NodeID = atoi(cmds[2]);

      if(NodeID >= config.header->IO_Nodes){
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

      // Disable functions that are not needed / will crash due to uninitialized objects
      UART_RecvCb[RNet_OPC_DEV_ID] = 0;
      UART_RecvCb[RNet_OPC_ReadInput] = 0;

      usleep(10000000);

      COM_DevReset();

      usleep(1500000);

      uart.recv(); // Recv first 32

      while(!stop && i < N->ports && send < 10){
        if(uart.recv()){
          uart.parse();
        }

        if(send == 0){
          uint16_t PortConfig = (N->config[i].type << 12) | (N->config[i].defaultState << 8) | (N->config[i].inverted);
          COM_Configure_IO(config.header->Module, i + offset, PortConfig);
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

        usleep(1000);

        loggerf(INFO, "!(%i) && %i < %i && %i < 10", stop, i, N->ports, send);
      }

      uart.close();
    }
    else if(strcmp(cmds[0], "p") == 0){
      config.print(cmds, cmds_len);
    }
  //   else
  //     printf("Not a command\n");
  }

  int exit = 1;


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
        modify_Train(&config, cmds[0][0]);
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
  logger.setlevel(MEMORY);
  logger.setlevel_stdout(INFO);

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

    printf("cmd arg: %s\n", argv[i]);

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
      printf("file arg: %s\n", argv[i]);
      if(argv[i][0] == '-'){
        loggerf(WARNING, "Invalid argument: %s\n", argv[i]);
      }
      else if(!filename_arg){
        filename_arg = true;
        strcpy(filename, argv[i]);
      }
      else{
        char * str = (char *)_calloc(strlen(argv[i])+5, char);
        printf("%x  %d", (unsigned int)str, strlen(argv[i]));
        strcpy(str, argv[i]);
        extrafiles.push_back(str);
      }
    }
  }

  printf("editing %i\nfilename_arg %i %s\nupdate %i\n", editing, filename_arg, filename, update);

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

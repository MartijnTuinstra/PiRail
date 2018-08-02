#ifndef INCLUDE_RAIL_H
  #define INCLUDE_RAIL_H

  #include <stdio.h>
  #include <stdlib.h>
  #include <stdint.h>
  #include <unistd.h>
  #include <string.h>

  #define NEXT 0
  #define PREV 1
  #define SWITCH_CARE 0x80

  struct _switch;
  struct _msswitch;
  struct _signal;
  struct _station;

  typedef struct _station Station;
  typedef struct _switch Switch;
  typedef struct _msswitch MSSwitch;
  typedef struct _signal Signal;

  typedef struct s_IO_Port IO_Port;
  typedef struct s_node_adr Node_adr;

  struct rail_link {
    char type;
    void * p;
    char module;
    uint16_t id;
  };

  enum Rail_types {
    MAIN,
    STATION,
    SHUNTING,
    SIDING,
    SPECIAL
  };

  enum Rail_states {
    BLOCKED,          // 0
    DANGER,           // 1
    RESTRICTED,       // 2
    CAUTION,          // 3
    PROCEED,          // 4
    RESERVED,         // 5
    RESERVED_SWITCH,  // 6
    UNKNOWN           // 7
  };

  typedef struct rail_segment {
    int module;
    int id;

    //Input
    IO_Port * In;

    enum Rail_types type;
    char dir;
    IO_Port * dir_Out;
    int length;

    struct rail_link next;
    struct rail_link prev;

    Station * station;

    char max_speed;

    enum Rail_states state;
    _Bool blocked;

    char train; //Follow id
    _Bool changed;
    _Bool reversed;
    _Bool oneWay;

    Signal * NextSignal;
    Signal * PrevSignal;

    int switch_len;
    Switch ** Sw;

    int msswitch_len;
    MSSwitch ** MSSw;

  } Block;

  struct block_connect {
    int module;
    int id;
    enum Rail_types type;

    struct rail_link next;
    struct rail_link prev;
  };

  enum Station_types {
    STATION_PERSON,
    STATION_CARGO,
    STATION_PERSON_YARD,//Yard for Person only
    STATION_CARGO_YARD, //Yard for Cargo only
    STATION_YARD        //Yard for both Person and Cargo trains
  };

  typedef struct _station {
    int module;
    int id;
    int uid;
    char * name;

    Block ** blocks;
    int blocks_len;

    enum Station_types type;

    char switches_len;
    struct switch_link ** switch_link;
  } Station;


  extern Station ** stations;
  extern int stations_len;

  void init_rail();

  void Create_Segment(Node_adr IO_Adr, struct block_connect connect ,char max_speed, char dir,char len);
  void Create_Station(int module, int id, char * name, char name_len, enum Station_types type, int len, Block ** blocks);

  void Connect_Rail_links();

  int dircmp(Block *A, Block *B);
  int block_cmp(Block *A, Block *B);

  Block * Next_Switch_Block(Switch * S, char type, int flags, int level);
  Block * Next_MSSwitch_Block(MSSwitch * S, char type, int flags, int level);
  Block * Next_Special_Block(Block * B, int flags, int level);
  Block * Next(Block * B, int flags, int level);

  int Next_check_Switch(void * p, struct rail_link link, int flags);

  struct rail_link Next_link(Block * B, int flags);
  struct rail_link Prev_link(Block * B);

#endif

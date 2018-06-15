#ifndef INCLUDE_RAIL_H
  #define INCLUDE_RAIL_H

  #include <stdio.h>
  #include <stdlib.h>
  #include <stdint.h>
  #include <unistd.h>
  #include <string.h>

  struct _switch;
  struct _signal;
  struct _station;

  typedef struct _station Station;
  typedef struct _switch Switch;
  typedef struct _signal Signal;

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
    BLOCKED,
    DANGER,
    RESTRICTED,
    CAUTION,
    PROCEED,
    RESERVED,
    RESERVED_SWITCH,
    UNKNOWN
  };

  typedef struct rail_segment {
    int module;
    int id;
    int ioadr;

    enum Rail_types type;
    char dir;
    int length;

    struct rail_link next;
    struct rail_link prev;

    Station * station;

    char max_speed;

    enum Rail_states state;
    _Bool blocked;

    char train; //Follow id
    _Bool changed;
    _Bool oneWay;

    Signal * NextSignal;
    Signal * PrevSignal;

  } Block;

  struct segment_connect {
    int module;
    int id;
    enum Rail_types type;

    int next_module;
    int next_id;
    char next_type;
    
    int prev_module;
    int prev_id;
    char prev_type;
  };

  enum Station_types {
    PERSON,
    CARGO,
    YARD
  };

  typedef struct _station {
    int Module;
    int id;
    int uid;
    char * name;

    Block ** blocks;
    char blocks_len;

    enum Station_types type;

    char switches_len;
    struct switch_link ** switch_link;
  } Station;


  extern Station ** stations;
  extern int stations_len;

  void init_rail();

  void Create_Segment(int IO_Adr, struct segment_connect connect ,char max_speed, char dir,char len);
  void Create_Station();

  void Connect_Segments();

  int dircmp(Block *A, Block *B);
  int block_adrcmp(Block *A, Block *B);

  Block * Next(Block * B, int dir);
  Block * Prev(Block * B, int dir);

  struct rail_link Next_link(Block * B);
  struct rail_link Prev_link(Block * B);

#endif

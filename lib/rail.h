#ifndef INCLUDE_RAIL_H
  #define INCLUDE_RAIL_H

  struct _switch;
  struct _signal;
  struct _station;

  typedef struct _station Station;
  typedef struct _switch Switch;
  typedef struct _signal Signal;

  struct rail_link {
    char type;
    void * p;
    char Module;
    uint16_t Adr;
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
    CAUTION,
    PROCEED,
    RESERVED,
    RESERVED_SWITCH,
    UNKNOWN
  };

  typedef struct rail_segment {
    int Module;
    int Adr;

    enum Rail_types type;
    char dir;

    struct rail_link next;
    struct rail_link prev;

    Station * station;

    char max_speed;

    enum Rail_states state;

    char train; //Follow id
    _Bool changed;
    _Bool oneWay;

    Signal * NextSignal;
    Signal * PrevSignal;

  } Block;

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

    Block * blocks;
    char blocks_len;

    enum Station_types type;

    char switches_len;
    struct switch_link ** switch_link;
  } Station;


  extern Station * stations;
  extern int stations_len;


  void init_rail();

  void Create_Segment();
  void Create_Station();

  void Connect_Segments();

  Block Next(Block * B, int dir);
  Block Prev(Block * B, int dir);

  struct rail_link Next_link(Block * B);
  struct rail_link Prev_link(Block * B);

#endif
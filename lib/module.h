#ifndef _INCLUDE_MODULE_H
  #define _INCLUDE_MODULE_H

  #include "switch.h"
  #include "rail.h"
  #include "signals.h"

  typedef struct gpio_link {
    char type;
    void * p;
  } gpio_link;

  typedef struct _unit{
    char Module;

    char connections_len;
    struct _unit ** connection;

    uint8_t input_regs;
    uint8_t output_regs;

    uint8_t *BlinkMask;
    uint8_t *OutRegs;
    uint8_t  *InRegs;

    int block_len;
    Block ** B;

    int switch_len;
    Switch ** Sw;

    int msswitch_len;
    MSSwitch ** MSSw;

    int signal_len;
    Signal ** Sig;

    int station_len;
    Station ** St;
  } Unit;

  struct rail_link CAdr(int module, int id, char type);

  extern int unit_len;
  extern Unit ** Units;

  void init_modules();

  void free_modules();

  void clear_modules();

  void LoadModules(int M);

  void JoinModules();

  void setup_JSON(int arr[], int arr2[], int size, int size2);
#endif

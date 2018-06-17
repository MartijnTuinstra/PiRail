#ifndef _INCLUDE_MODULE_H
  #define _INCLUDE_MODULE_H

  #include "switch.h"
  #include "rail.h"
  #include "signals.h"

  enum gpio_types {
    gpio_NC,
    gpio_RAIL,
    gpio_SWITCH,
    gpio_MSSWITCH
  };

  typedef struct gpio_link {
    enum gpio_types type;
    void * p;
  } gpio_link;

  typedef struct _unit{
    char module;

    char connections_len;
    struct _unit ** connection;

    uint8_t input_regs;
    uint8_t output_regs;

    uint8_t *BlinkMask;
    uint8_t *OutRegs;
    uint8_t  *InRegs;

    gpio_link * input_link;

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

  void Unit_expand_IO(_Bool type, Unit * U);

  void LoadModules(int M);

  void JoinModules();

  void setup_JSON(int arr[], int arr2[], int size, int size2);
#endif

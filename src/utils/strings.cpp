

const char * S_RailStates[8] = {
  "BLOCKED",
  "DANGER",
  "RESTRICTED",
  "CAUTION",
  "PROCEED",
  "RESERVED",
  "RESERVED_SWITCH",
  "UNKNOWN" 
};


const char * S_PolarityTypes[5] = {
  "DISABLED",
  "NO IO",
  "SINGLE IO",
  "DOUBLE IO",
  "B LINKED"
};

const char * S_RailLinkTypes[256] = {
  " R ",
  " S ",
  " s ",
  "MA",
  "MB",
  "MiA",
  "MiB",
  "","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","",
  "","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","",
  "","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","",
  "","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","",
  "","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","",""," C "," E ",
};

const char * S_RailTypes[4] = {
  "MAIN",
  "STATION",
  "NOSTOP",
  "YARD"
};

// IO


const char * IO_enum_type_string[9] = {
  "IO_Undefined",
  "IO_Output",
  "IO_Output_Blink",
  "IO_Output_Servo",
  "IO_Output_PWM",
  "IO_Input",
  "IO_Input_Block",
  "IO_Input_Switch",
  "IO_Input_MSSwitch"
};

const char * IO_undefined_string[4] = {
  "IO_event_undefined",
  "IO_event_undefined",
  "IO_event_undefined",
  "IO_event_undefined",
};
const char * IO_output_string[4] = {
  "IO_event_Low",
  "IO_event_High",
  "IO_event_Pulse",
  "IO_event_Toggle"
};
const char * IO_blink_string[4] = {
  "IO_event_B_Low",
  "IO_event_B_High",
  "IO_event_Blink1",
  "IO_event_Blink2"
};
const char * IO_servo_string[4] = {
  "IO_event_Servo1",
  "IO_event_Servo2",
  "IO_event_Servo3",
  "IO_event_Servo4"
};
const char * IO_pwm_string[4] = {
  "IO_event_PWM1",
  "IO_event_PWM2",
  "IO_event_PWM3",
  "IO_event_PWM4"
};
const char ** IO_event_string[9] = {
  IO_undefined_string,
  IO_output_string,
  IO_blink_string,
  IO_servo_string,
  IO_pwm_string,
  &IO_enum_type_string[5],
  &IO_enum_type_string[6],
  &IO_enum_type_string[7],
  &IO_enum_type_string[8]
};

const char * S_TrainStates[20] = {
    "IDLE", "RESUMING", "STOPPING", "STOPPING_REVERSE", "STOPPING_WAIT", 
    "WAITING", "WAITING_DESTINATION",
    "DRIVING", "CHANGING", "UPDATE", "INITIALIZING"
};

const char * S_TrainSpeedReasons[5] = {
  "NONE", "SIGNAL", "MAXSPEED", "ROUTE"
};
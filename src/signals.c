void set_signal(struct Seg * B,int leftshift,int state){
  state = state & 0x7;

  if(!(leftshift == 0 || leftshift == 4)){
    printf("Wrong leftshift: %i\n",leftshift);
    return;
  }
  if(((B->signals >> leftshift) & 0b1000) == 0){
    printf("No Signal");
    return;
  }
  if(((B->signals >> leftshift) & 0b0111) == state){
    return;
  }else{
    if(leftshift == 4){
      printf("%i:%i:%i\tLeft Signal\t%i\n",B->Adr.M,B->Adr.B,B->Adr.S,state);
      B->signals = (B->signals & 0x8F) + (state << 4);
    }else{
      printf("%i:%i:%i\tRight Signal\t%i\n",B->Adr.M,B->Adr.B,B->Adr.S,state);
      B->signals = (B->signals & 0xF8) + state;
    }
    COM_change_signal(B);
  }
}

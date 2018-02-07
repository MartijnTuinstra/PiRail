/**//*

0 = EMPTY
1 = EMERGENY STOP
2 =               release
3 = Electrical Short
4 =                  release
5 = Train has split
6 =                 release
2-10 =  or HIGH PRIO commands
11 = NEW TRAIN [Couple id]
12 =                       release

/**/

#define ACTIVATE 0
#define RELEASE  1

void Web_Emergency_Stop(int i){
  char data[5];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 1;
  }else if(i == RELEASE){
    data[1] = 2;
  }else{
    return;
  }
  printf("Emergency_Stop (%i):[%i][%i]",i,data[0],data[1]);

  send_all(data,2,1);
}

void Web_Electrical_Stop(int i){
  char data[5];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 3;
  }else if(i == RELEASE){
    data[1] = 4;
  }else{
    return;
  }
  send_all(data,2,1);
}

void Web_Train_Split(int i,char tID,char B[]){
  printf("\n\nWeb_Train_Split(%i,%i,{%i,%i});\n\n",i,tID,B[0],B[1]);
  char data[8];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 5;
    data[2] = tID;
    data[3] = B[0];
    data[4] = B[1];
    send_all(data,5,1);
  }else if(i == RELEASE){
    data[1] = 6;
    data[2] = tID;
    send_all(data,3,1);
  }else{
    return;
  }
}

void Web_Link_Train(int i,char nr,char B[]){
  char data[8];
  data[0] = 1;
  if(i == ACTIVATE){
    data[1] = 11;
    data[2] = nr;
    data[3] = B[0];
    data[4] = B[1];
    send_all(data,5,1);
  }else if(i == RELEASE){
    data[1] = 12;
    data[2] = nr;
    data[3] = B[0];
    data[4] = B[1];
    data[5] = B[2];
    send_all(data,6,1);
  }else{
    return;
  }
}

void Web_Train_Data(char data[14]){
  printf("\n\nWeb_Train_Data;\n\n");
  char s_data[20];
  s_data[0] = 7;

  for(int i = 0;i<7;i++){
    s_data[i+1] = data[i];
  }

  send_all(s_data,8,1);
}

int status_w = 1;

struct status_struct{
  unsigned int type;
  int id;
  char data[40];
};

/**//*

0 = EMPTY
1 = EMERGENY STOP
2-10 =  or HIGH PRIO commands
11 = NEW TRAIN [Couple id]

/**/
struct status_struct * status_st[20] = {NULL};

void status_add(int type, char data[40]){
  for(int i = 0;i<40;i++){
    if(status_st[i] == NULL){
      struct status_struct *Z = (struct status_struct *)malloc(sizeof(struct status_struct));
      Z->type = type;
      Z->id = i;
      strcpy(Z->data, data);
      status_st[i] = Z;
      status_w = 1;
      printf("Write %i\n",i);
      break;
    }
  }
}
void status_rem(int id, int type){
  if(status_st[id] != NULL && status_st[id]->type == type){
    free(status_st[id]);
    status_st[id] = NULL;
    status_w = 1;
  }else{
    printf("Remove failed\n");
  }
}
/*
void * status_write(){
  while(!stop){
    while(status_w == 0){

    }
    printf("Going to write");
    status_w = 0;
    FILE *fs;

    fs = fopen("status.json","w");

    fprintf(fs,"{\"0\":[");

    int p = 0;

    for(int i = 0;i<20;i++){
      if(status_st[i] != NULL){
        if(p == 1){
          fprintf(fs,",");
        }else{
          p = 1;
        }
        //printf("Write available at i:%i\n",i);

        printf("Write [%i,[%s]]",status_st[i]->type,status_st[i]->data);
        fprintf(fs,"[%i,%i,%s]",status_st[i]->type,status_st[i]->id,status_st[i]->data);

      }
    }

    fprintf(fs,"]}");

    fclose(fs);
  }
}
*/

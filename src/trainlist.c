struct train{
	char ID;
	char speed;
	char control;
	struct adr Route[MAX_ROUTE];
};

struct train *trains[MAX_TRAINS] = {};
int iTrain = 0;
int bTrain = 0;

void add_train(char ID){
	struct train *Z = (struct train*)malloc(sizeof(struct train));

	struct adr Route[20] = {{0,0,0,0}};

	Z->ID = ID;
	Z->speed = 0;
	Z->control = 0; //0 = User, 1 = Computer
	for(int i = 0;i<MAX_ROUTE;i++){
		Z->Route[i] = Route[i];
	}
	//return Z;
	trains[iTrain++] = Z;
}

void req_train(char ID, struct adr Adr){
	char data[40] = "";
	sprintf(data, "[%i,\"%i:%i:%i\"]", ID,Adr.M,Adr.B,Adr.S);
	status_add(11,data);
	printf("\n\new train Requested %i\n\n\n",ID);
}

void setup_JSON(int arr[], int arr2[], int size, int size2){
	FILE *fr;
	printf("JSON1\n");
	fr = fopen("setup.json","w");

	fprintf(fr, "[[");

	for(int i = 0;i<size-1;i++){
		fprintf(fr,"%i,",arr[i]);
	}

	fprintf(fr,"%i],[",arr[size-1]);

	int s = 0;
	for(int i = 0;i<size2;i++){
		if(s == 0){
			s = 1;
		}else{
			fprintf(fr,",");
		}
		fprintf(fr,"%i",arr2[i]);
	}
	fprintf(fr,"]]");

	fclose(fr);

	printf("JSON1\n");
}

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

void setup_JSON(int arr[], int size){
	FILE *fr;
	printf("JSON1\n");
	fr = fopen("setup.json","w");

	fprintf(fr, "[");

	for(int i = 0;i<size-1;i++){
		fprintf(fr,"%i,",arr[i]);
	}

	fprintf(fr,"%i]",arr[size-1]);

	fclose(fr);

	printf("JSON1\n");
}

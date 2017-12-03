#ifndef t_Unit
	#include "./modules.h"
#endif

void Create_Unit(int Module){
	struct Unit *Z = (struct Unit*)malloc(sizeof(struct Unit));

	Units[Module] = Z;
}

void Create_Unit2(int Module,int OUT,int IN){
	struct Unit *Z = (struct Unit*)malloc(sizeof(struct Unit));

	Units[Module] = Z;

	struct Rail_link * A = (struct Rail_link*)calloc( IN,sizeof(struct Rail_link));
	struct Rail_link * B = (struct Rail_link*)calloc(OUT,sizeof(struct Rail_link));

	Z->In_length = IN;
	Z->In = A;
	Z->Out_length = OUT;
	Z->Out = B;
}

void join(struct SegC Adr, struct SegC link){
	printf("LINK %c%i:%i => %c%i:%i\t",Adr.type,Adr.Module,Adr.Adr,link.type,link.Module,link.Adr);
	if(Adr.type == 'R' && blocks2[Adr.Module][Adr.Adr]){
		blocks2[Adr.Module][Adr.Adr]->PrevC = link;
		printf("D\n");
	}else if(Adr.type == 'S'){
		Switch2[Adr.Module][Adr.Adr]->AppC = link;
	}else if(Adr.type == 's'){
		if(Adr_Comp2(Switch2[Adr.Module][Adr.Adr]->DivC, EMPTY_BL)){
			Switch2[Adr.Module][Adr.Adr]->DivC = link;
		}else{
			Switch2[Adr.Module][Adr.Adr]->StrC = link;
		}
	}
}

struct link Modules(int m, struct link IN){
	//Loop Left
	struct link link;
	link.Adr3 = EMPTY_BL;
	link.Adr4 = EMPTY_BL;

	int Seg_i = 0;
	int Swi_i = 0;
	int Sig_i = 0;

	if(m == 1){
		Create_Unit2(m,8,8);

		/*			,-1--.
					 |     `--0--
		1 0    \-.--2---3--
		        ||

		Switch
		0		0--1
		1   2--3
		*/
		link.Adr1.Module = m;link.Adr1.Adr = 0;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 3;link.Adr2.type = 'R';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);
		join(IN.Adr1,CAdr(m,1,'s'));
		join(IN.Adr2,CAdr(m,0,'s'));

		Create_Segment(0,CAdr(m,0,'R'),CAdr(m,1,'R'),EMPTY_BL,     speed_A,0,0,100);
		Create_Segment(1,CAdr(m,1,'R'),CAdr(m,1,'S'),CAdr(m,0,'R'),speed_A,0,2,100);
 		Create_Segment(2,CAdr(m,2,'R'),CAdr(m,0,'S'),CAdr(m,3,'R'),speed_A,0,1,100);
 		Create_Segment(3,CAdr(m,3,'R'),CAdr(m,2,'R'),EMPTY_BL,     speed_A,0,1,100);

		//LINK
		Create_Switch(CAdr(m,1,2),CAdr(m,1,'R'),CAdr(m,0,'s'),IN.Adr1,(int [2]){2,3},1);
		Create_Switch(CAdr(m,0,2),CAdr(m,2,'R'),CAdr(m,1,'s'),IN.Adr2,(int [2]){0,1},0);

		Units[m]->S[0]->Detection_Block = Units[m]->B[2];
		Units[m]->S[1]->Detection_Block = Units[m]->B[1];
	}
	else if(m == 2){
		Create_Unit2(m,8,8);

		/*			,--1-.
			--0--'     |
			--3---2-.-/					0 1
							||
		*/
		link.Adr1.Module = m;link.Adr1.Adr = 1;link.Adr1.type = 's';
		link.Adr2.Module = m;link.Adr2.Adr = 0;link.Adr2.type = 's';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,3,'R'));

		Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,1,'R'),speed_A,0,0,100);
		Create_Segment(1,CAdr(m,1,'R'),CAdr(m,0,'R'),CAdr(m,1,'S'),speed_A,0,2,100);
 		Create_Segment(2,CAdr(m,2,'R'),CAdr(m,3,'R'),CAdr(m,0,'S'),speed_A,0,1,100);
 		Create_Segment(3,CAdr(m,3,'R'),IN.Adr2,CAdr(m,2,'R'),speed_A,0,1,100);

		Create_Switch(CAdr(m,0,2),CAdr(m,2,'R'),CAdr(m,1,'s'),EMPTY_BL,(int [2]){0,1},1);
		Create_Switch(CAdr(m,1,2),CAdr(m,1,'R'),CAdr(m,0,'s'),EMPTY_BL,(int [2]){2,3},1);

		Units[m]->S[0]->Detection_Block = Units[m]->B[2];
		Units[m]->S[1]->Detection_Block = Units[m]->B[1];
	}
	else if(m == 3){//Station 4 bakken
		Create_Unit2(m,8,8);
		/*
		Blocks address (Octal)
		  				 /-- -10-11- --\
		--0-- -\1-'--- -12-13- ---`-30-/- -31-
		--2-- -3\-,--- -14-15- ---,-32-'- -33-
							 \-- -16-17- --/
							  \- -20-21- -/

		Block numbers (Decimal)
		  				 /-- --2--3- --\
		--0-- -\1-'--- --4--5- ---`--6-/- --7-
		--8-- -9\-,--- -10-11- ---,-16-'- -17-
							 \-- -12-13- --/
							  \- -14-15- -/

		Switches numbers (Decimal)
				0 1										5 6
				 2 3								 7 8
					  4							  9
 		Switches addresses (octal)
		0	 0--1			5 20-21
		1  2--3			6 22-23
		2  4--5			7 24-25
		3  6--7			8 26-27
		4 10-11			9 30-31
		*/
		link.Adr1.Module = m;link.Adr1.Adr =  7;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 17;link.Adr2.type = 'R';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,8,'R'));

		Create_Switch(CAdr(m,0,2),CAdr(m,0,'R'),CAdr(m,2,'s') ,CAdr(m,1,'S') ,(int [2]){000,001},0); //Switch 0
		Create_Switch(CAdr(m,1,2),CAdr(m,0,'s'),CAdr(m,2,'R') ,CAdr(m,4,'R') ,(int [2]){002,003},1);			//Switch 1
		Create_Switch(CAdr(m,2,2),CAdr(m,3,'S'),CAdr(m,0,'s') ,CAdr(m,8,'R') ,(int [2]){004,005},0);	//Switch 2
		Create_Switch(CAdr(m,3,2),CAdr(m,2,'S'),CAdr(m,4,'S') ,CAdr(m,10,'R'),(int [2]){006,007},1); //Switch 3
		Create_Switch(CAdr(m,4,2),CAdr(m,3,'s'),CAdr(m,12,'R'),CAdr(m,14,'R'),(int [2]){010,011},1);		//Switch 4
		Create_Switch(CAdr(m,5,2),CAdr(m,6,'s'),CAdr(m,3,'R') ,CAdr(m,5,'R') ,(int [2]){020,021},1);				//Switch 010
		Create_Switch(CAdr(m,6,2),CAdr(m,7,'R'),CAdr(m,8,'s') ,CAdr(m,5,'S') ,(int [2]){022,023},0); //Switch 011
		Create_Switch(CAdr(m,7,2),CAdr(m,8,'S'),CAdr(m,9,'S') ,CAdr(m,11,'R'),(int [2]){024,025},1); //Switch 013
		Create_Switch(CAdr(m,8,2),CAdr(m,7,'S'),CAdr(m,6,'s') ,CAdr(m,17,'R'),(int [2]){026,027},0); //Switch 014
		Create_Switch(CAdr(m,9,2),CAdr(m,7,'s'),CAdr(m,13,'R'),CAdr(m,15,'R'),(int [2]){030,031},1);     //Switch 012

		Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,0,'S'),speed_B,0,0,100);
		Create_Segment(1,CAdr(m,1,'T'),EMPTY_BL,EMPTY_BL,speed_B,0,0,100);

		Create_Segment(010,CAdr(m,2,'R'),CAdr(m,1,'s'),CAdr(m,3,'R')  ,speed_C,0,0,50);
		Create_Segment(011,CAdr(m,3,'R'),CAdr(m,2,'R'),CAdr(m,5,'s'),speed_C,0,0,50);
		Create_Segment(012,CAdr(m,4,'R'),CAdr(m,1,'s'),CAdr(m,5,'R'),  speed_C,0,0,50);
		Create_Segment(013,CAdr(m,5,'R'),CAdr(m,4,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

		Create_Segment(030,CAdr(m,6,'T'),EMPTY_BL,EMPTY_BL,speed_B,0,0,50);
		Create_Segment(031,CAdr(m,7,'R'),CAdr(m,6,'S'),EMPTY_BL,speed_B,0,0,100);


		Create_Segment(2,CAdr(m,8,'R'),EMPTY_BL,CAdr(m,2,'s'),speed_B,0,1,50);
		Create_Segment(3,CAdr(m,9,'T'),EMPTY_BL,EMPTY_BL,speed_B,0,1,50);

		Create_Segment(014,CAdr(m,10,'R'),CAdr(m,3,'s') ,CAdr(m,11,'R'),speed_C,0,0,50);
		Create_Segment(015,CAdr(m,11,'R'),CAdr(m,10,'R'),CAdr(m,7,'s') ,speed_C,0,0,50);
		Create_Segment(016,CAdr(m,12,'R'),CAdr(m,4,'s') ,CAdr(m,13,'R'),speed_C,0,0,50);
		Create_Segment(017,CAdr(m,13,'R'),CAdr(m,12,'R'),CAdr(m,9,'s') ,speed_C,0,0,50);
		Create_Segment(020,CAdr(m,14,'R'),CAdr(m,4,'s') ,CAdr(m,15,'R'),speed_C,0,0,50);
		Create_Segment(021,CAdr(m,15,'R'),CAdr(m,14,'R'),CAdr(m,9,'s') ,speed_C,0,0,50);

		Create_Segment(032,CAdr(m,16,'T'),EMPTY_BL,EMPTY_BL,speed_B,0,0,50);
		Create_Segment(033,CAdr(m,17,'R'),CAdr(m,8,'s'),EMPTY_BL,speed_B,0,1,50);
	}
	else if(m == 4){//Station 4 bakken
		Create_Unit2(m,8,32);
		/*
		Blocks addresses (Octal)
		  				 /-- -10-11- -30-31- --\
		--0-- -\1-'--- -12-13- -32-33- ---`-50-/- -51-
		--2-- -3\-,--- -14-15- -34-35- ---,-52-'- -53-
							 \-- -16-17- -36-37- --/
							  \- -20-21- -40-41- -/

		Blocks addresses (Decimal)
		  				  /-- --2--3- --4--5- --\
		--0-- --\1-'--- --6--7- --8--9- ---`-10-/- -11-
		-12-- -13\-,--- -14-15- -16-17- ---,-26'-- -27-
							  \-- -18-19- -20-21- --/
							   \- -22-23- -24-25- -/


		Switches numbers (Decimal)
				0 1										5 6
				 2 3								 7 8
					  4							  9
 		Switches addresses (octal)
		0	 0--1			5 20-21
		1  2--3			6 22-23
		2  4--5			7 24-25
		3  6--7			8 26-27
		4 10-11			9 30-31
		*/
		link.Adr1.Module = m;link.Adr1.Adr = 11;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 27;link.Adr2.type = 'R';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,12,'R'));

		Create_Switch(CAdr(m,0,2),CAdr(m,0,'R'),CAdr(m,2,'s') ,CAdr(m,1,'S') ,(int [2]){000,001},0); //Switch 0
		Create_Switch(CAdr(m,1,2),CAdr(m,0,'s'),CAdr(m,2,'R') ,CAdr(m,6,'R') ,(int [2]){002,003},1);			//Switch 1
		Create_Switch(CAdr(m,2,2),CAdr(m,3,'S'),CAdr(m,0,'s') ,CAdr(m,12,'R'),(int [2]){004,005},0);	//Switch 2
		Create_Switch(CAdr(m,3,2),CAdr(m,2,'S'),CAdr(m,4,'S') ,CAdr(m,14,'R'),(int [2]){006,007},1); //Switch 3
		Create_Switch(CAdr(m,4,2),CAdr(m,3,'s'),CAdr(m,18,'R'),CAdr(m,22,'R'),(int [2]){010,011},1);		//Switch 4
		Create_Switch(CAdr(m,5,2),CAdr(m,6,'s'),CAdr(m,5,'R') ,CAdr(m,9,'R') ,(int [2]){020,021},1);				//Switch 010
		Create_Switch(CAdr(m,6,2),CAdr(m,11,'R'),CAdr(m,8,'s'),CAdr(m,5,'S') ,(int [2]){022,023},0); //Switch 011
		Create_Switch(CAdr(m,7,2),CAdr(m,8,'S'),CAdr(m,9,'S') ,CAdr(m,17,'R'),(int [2]){024,025},1); //Switch 013
		Create_Switch(CAdr(m,8,2),CAdr(m,7,'S'),CAdr(m,6,'s') ,CAdr(m,27,'R'),(int [2]){026,027},0); //Switch 014
		Create_Switch(CAdr(m,9,2),CAdr(m,7,'s'),CAdr(m,21,'R'),CAdr(m,25,'R'),(int [2]){030,031},1);     //Switch 012

		Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,CAdr(m,0,'S'),speed_B,0,0,100);
		Create_Segment(1,CAdr(m,1,'T'),EMPTY_BL,EMPTY_BL,speed_B,0,0,100);

		Create_Segment(010,CAdr(m,2,'S'),CAdr(m,1,'s'),CAdr(m,3,'R'),speed_C,0,0,50);
		Create_Segment(011,CAdr(m,3,'S'),CAdr(m,2,'R'),CAdr(m,4,'R'),speed_C,0,0,50);
		Create_Segment(030,CAdr(m,4,'S'),CAdr(m,3,'R'),CAdr(m,5,'R'),speed_C,0,0,50);
		Create_Segment(031,CAdr(m,5,'S'),CAdr(m,4,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

		Create_Segment(012,CAdr(m,6,'S'),CAdr(m,1,'s'),CAdr(m,7,'R'),speed_C,0,0,50);
		Create_Segment(013,CAdr(m,7,'S'),CAdr(m,6,'R'),CAdr(m,8,'R'),speed_C,0,0,50);
		Create_Segment(032,CAdr(m,8,'S'),CAdr(m,7,'R'),CAdr(m,9,'R'),speed_C,0,0,50);
		Create_Segment(033,CAdr(m,9,'S'),CAdr(m,8,'R'),CAdr(m,5,'s'),speed_C,0,0,50);

		Create_Segment(050,CAdr(m,10,'T'),EMPTY_BL,EMPTY_BL,speed_B,0,0,50);
		Create_Segment(051,CAdr(m,11,'R'),CAdr(m,6,'S'),EMPTY_BL,speed_B,0,0,100);


		Create_Segment(2,CAdr(m,12,'R'),IN.Adr2,CAdr(m,2,'s'),speed_B,0,1,50);
		Create_Segment(3,CAdr(m,13,'T'),EMPTY_BL,EMPTY_BL,speed_B,0,1,50);

		Create_Segment(014,CAdr(m,14,'S'),CAdr(m,3,'s') ,CAdr(m,15,'R'),speed_C,0,1,50);
		Create_Segment(015,CAdr(m,15,'S'),CAdr(m,14,'R'),CAdr(m,16,'R'),speed_C,0,1,50);
		Create_Segment(034,CAdr(m,16,'S'),CAdr(m,15,'R'),CAdr(m,17,'R'),speed_C,0,1,50);
		Create_Segment(035,CAdr(m,17,'S'),CAdr(m,16,'R'),CAdr(m,7 ,'s'),speed_C,0,1,50);

		Create_Segment(016,CAdr(m,18,'S'),CAdr(m,4,'s') ,CAdr(m,19,'R'),speed_C,0,1,50);
		Create_Segment(017,CAdr(m,19,'S'),CAdr(m,18,'R'),CAdr(m,20,'R'),speed_C,0,1,50);
		Create_Segment(036,CAdr(m,20,'S'),CAdr(m,19,'R'),CAdr(m,21,'R'),speed_C,0,1,50);
		Create_Segment(037,CAdr(m,21,'S'),CAdr(m,20,'R'),CAdr(m,9,'s') ,speed_C,0,1,50);

		Create_Segment(020,CAdr(m,22,'S'),CAdr(m,4,'s'),CAdr(m,23,'R'),speed_C,0,1,50);
		Create_Segment(021,CAdr(m,23,'S'),CAdr(m,22,'R'),CAdr(m,24,'R'),speed_C,0,1,50);
		Create_Segment(040,CAdr(m,24,'S'),CAdr(m,23,'R'),CAdr(m,25,'R'),speed_C,0,1,50);
		Create_Segment(041,CAdr(m,25,'S'),CAdr(m,24,'R'),CAdr(m,9,'s'),speed_C,0,1,50);

		Create_Segment(052,CAdr(m,26,'T'),EMPTY_BL,EMPTY_BL,speed_B,0,1,50);
		Create_Segment(053,CAdr(m,27,'R'),CAdr(m,8,'s'),EMPTY_BL,speed_B,0,1,50);

		Units[m]->S[0]->Detection_Block = Units[m]->B[1];
		Units[m]->S[1]->Detection_Block = Units[m]->B[1];
		Units[m]->S[2]->Detection_Block = Units[m]->B[13];
		Units[m]->S[3]->Detection_Block = Units[m]->B[13];
		Units[m]->S[4]->Detection_Block = Units[m]->B[13];

		Units[m]->S[5]->Detection_Block = Units[m]->B[10];
		Units[m]->S[6]->Detection_Block = Units[m]->B[10];
		Units[m]->S[7]->Detection_Block = Units[m]->B[26];
		Units[m]->S[8]->Detection_Block = Units[m]->B[26];
		Units[m]->S[9]->Detection_Block = Units[m]->B[26];

		/*Linking Switches*/
		 struct L_Swi_t * B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));
		 struct SegC ADR = CAdr(m,0,'S');
		 B_Swi->Adr = ADR;
		 B_Swi->states[0] = 0;
		 B_Swi->states[1] = 1;
		 Switch2[m][2]->L_Swi[0] = B_Swi;
		 Switch2[m][6]->L_Swi[0] = B_Swi;
		 Switch2[m][8]->L_Swi[0] = B_Swi;

		 B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

		 ADR.Adr = 2;
		 B_Swi->Adr = ADR;
		 B_Swi->states[0] = 0;
		 B_Swi->states[1] = 1;
		 Switch2[m][0]->L_Swi[0] = B_Swi;
		 Switch2[m][6]->L_Swi[1] = B_Swi;
		 Switch2[m][8]->L_Swi[1] = B_Swi;

		 B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

	 	 ADR.Adr = 6;
	 	 B_Swi->Adr = ADR;
		 B_Swi->states[0] = 0;
		 B_Swi->states[1] = 1;
		 Switch2[m][0]->L_Swi[1] = B_Swi;
		 Switch2[m][2]->L_Swi[1] = B_Swi;
		 Switch2[m][8]->L_Swi[2] = B_Swi;

		 B_Swi = (struct L_Swi_t*)malloc(sizeof(struct L_Swi_t));

	 	 ADR.Adr = 8;
	 	 B_Swi->Adr = ADR;
		 B_Swi->states[0] = 0;
		 B_Swi->states[1] = 1;
		 Switch2[m][0]->L_Swi[2] = B_Swi;
		 Switch2[m][2]->L_Swi[2] = B_Swi;
		 Switch2[m][6]->L_Swi[2] = B_Swi;
		/**/
		//
		/*Setting Switch preferences*/
			struct P_Swi_t * P = (struct P_Swi_t *)malloc(sizeof(struct P_Swi_t));
			P->type = 0; 		//Always
			P->state = 0;		//Straigth when approaching switch
			Switch2[m][8]->pref[0] = P;
			P = (struct P_Swi_t *)malloc(sizeof(struct P_Swi_t));
			P->type = 0; 		//Always
			P->state = 1;		//Diverging when approaching switch
			Switch2[m][2]->pref[0] = P;
		//
		/*Stations*/
			Create_Station(m,"Spoor 1",1,4,(int [4]){2,3,4,5});
			Create_Station(m,"Spoor 2",1,4,(int [4]){6,7,8,9});
			Create_Station(m,"Spoor 3",1,4,(int [4]){14,15,16,17});
			Create_Station(m,"Spoor 4",1,4,(int [4]){18,19,20,21});
			Create_Station(m,"Spoor 5",1,4,(int [4]){22,23,24,25});
		//
		/*Signals*/
			//All type 2 signals
			//NSignals

			short adr[3] = {0,1,2};
			char b[BLOCK_STATES] = {1,2,4}; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			char c[BLOCK_STATES] = {1,0,0};

			create_signal2(blocks2[m][2],3,adr,b,c,0);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 3;adr[1] = 4;adr[2] = 5;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 1;c[1] = 0;c[2] = 0;

			create_signal2(blocks2[m][6],3,adr,b,c,0);


			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 16;adr[1] = 17;adr[2] = 18;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 1;c[1] = 0;c[2] = 0;

			create_signal2(blocks2[m][17],3,adr,b,c,0);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 19;adr[1] = 20;adr[2] = 21;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 1;c[1] = 0;c[2] = 0;

			create_signal2(blocks2[m][21],3,adr,b,c,0);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 22;adr[1] = 23;adr[2] = 24;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 1;c[1] = 0;c[2] = 0;

			create_signal2(blocks2[m][25],3,adr,b,c,0);

			//PSignals

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 7;adr[1] = 8;adr[2] = 9;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 0;c[1] = 0;c[2] = 0;

			create_signal2(blocks2[m][14],3,adr,b,c,1);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 10;adr[1] = 11;adr[2] = 12;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 0;c[1] = 0;c[2] = 0;

			create_signal2(blocks2[m][18],3,adr,b,c,1);

			memset(adr,0,3);memset(b,0,BLOCK_STATES);memset(c,0,BLOCK_STATES);
			adr[0] = 13;adr[1] = 14;adr[2] = 15;
			b[0] = 1;b[1] = 2;b[2] = 4; /*GREEN,AMBER,RED,BLOCKED,PARKED,RESERVED,UNKNOWN 6*/
			c[0] = 0;c[1] = 0;c[2] = 0;

			create_signal2(blocks2[m][22],3,adr,b,c,1);
		//
		/*One Way*/
			Units[m]->B[0]->oneWay = TRUE;
			Units[m]->B[11]->oneWay = TRUE;
			Units[m]->B[12]->oneWay = TRUE;
			Units[m]->B[27]->oneWay = TRUE;
		//

		Units[m]->B[1]->dir  = 0;
		Units[m]->B[10]->dir = 0;
		Units[m]->B[13]->dir  = 1;
		Units[m]->B[26]->dir = 1;

	}/*
	else if(m == 5){//T piece

		Create_Unit(m);

		join(IN.Adr1,C_Adr(m,1,1));
		Create_Segment(Seg_i++,C_Adr(m,1,1),IN.Adr1,END_BL,speed_A,0,0,100);

		C_Seg(Seg_i++,C_Adr(m,2,0),0);
		C_Seg(Seg_i++,C_Adr(m,5,0),0);

		join(IN.Adr2,C_AdrT(m,2,1,'S'));
		Create_Switch(Swi_i++,C_Adr(m,2,1),IN.Adr2,C_Adr(m,3,1),C_AdrT(m,5,1,'s'),1);
		Create_Switch(Swi_i++,C_Adr(m,5,1),END_BL,C_Adr(m,4,1),C_AdrT(m,2,1,'s'),1);

		Create_Segment(Seg_i++,C_Adr(m,3,1),C_AdrT(m,2,1,'s'),END_BL,speed_A,0,1,50);
		Create_Segment(Seg_i++,C_Adr(m,4,1),C_AdrT(m,5,1,'s'),END_BL,speed_A,0,2,50);

		blocks[m][2][0]->dir  = 1;
		blocks[m][5][0]->dir = 1;

		link.Adr1 = C_Adr(m,1,1);
		link.Adr2 = C_AdrT(m,5,1,'S');
		link.Adr3 = C_AdrT(m,4,1,'R');
		link.Adr4 = C_AdrT(m,3,1,'R');
	}
	else if(m == 6){//Rangeer Brug
		Create_Unit(m);
		join(IN.Adr1,C_Adr(m,1,1));
		Create_Segment(Seg_i++,C_Adr(m,1,1),IN.Adr1,C_AdrT(m,3,1,'s'),speed_A,0,0,90);

		join(IN.Adr2,C_Adr(m,2,1));
		Create_Segment(Seg_i++,C_Adr(m,2,1),IN.Adr2,C_AdrT(m,3,1,'s'),speed_A,0,1,90);

		C_Seg(Seg_i++,C_Adr(m,3,0),0);

		Create_Switch(Swi_i++,C_Adr(m,3,1),END_BL,C_Adr(m,2,1),C_Adr(m,1,1),1);

		blocks[m][1][1]->oneWay = TRUE;
		blocks[m][2][1]->oneWay = TRUE;

		link.Adr1 = C_AdrT(m,3,1,'S');
		link.Adr2 = END_BL;
	}
	else if(m == 7){//Rangeer
		Create_Unit(m);

		C_Seg(Seg_i++,C_Adr(m,1,0),0);

		Create_Switch(Swi_i++,C_Adr(m,1,1),C_AdrT(m,1,2,'s'),C_Adr(m,3,1),C_Adr(m,2,1),1);
		Create_Switch(Swi_i++,C_Adr(m,1,2),C_AdrT(m,1,3,'m'),C_Adr(m,4,1),C_AdrT(m,1,1,'S'),1);

		join(IN.Adr1,C_AdrT(m,1,3,'M'));
		struct adr A[10] = {{m,1,2,'S'},{m,5,1,'R'},{m,1,4,'S'},END_BL};
		struct adr B[10] = {link.Adr1,link.Adr1,link.Adr1,END_BL};
		Create_Moduls(Swi_i++,C_Adr(m,1,3),A,B,3);
		//Create_Switch(Swi_i++,C_Adr(m,1,3),link.Adr1,C_AdrT(m,1,2,'S'),C_AdrT(m,1,4,'S'),1);
		//Create_Switch(Swi_i++,C_Adr(m,1,4),C_AdrT(m,1,3,'s'),C_AdrT(m,1,5,'S'),C_Adr(m,5,1),1);

		Create_Switch(Swi_i++,C_Adr(m,1,4),C_AdrT(m,1,3,'m'),C_Adr(m,6,1),C_AdrT(m,1,5,'S'),1);
		Create_Switch(Swi_i++,C_Adr(m,1,5),C_AdrT(m,1,4,'s'),C_Adr(m,7,1),C_Adr(m,8,1),1);

		Create_Segment(Seg_i++,C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,2),C_Adr(m,2,1),C_Adr(m,2,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,3),C_Adr(m,2,2),C_Adr(m,2,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,4),C_Adr(m,2,3),C_Adr(m,2,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,5),C_Adr(m,2,4),C_Adr(m,2,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,2,6),C_Adr(m,2,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,2),C_Adr(m,3,1),C_Adr(m,3,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,3),C_Adr(m,3,2),C_Adr(m,3,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,4),C_Adr(m,3,3),C_Adr(m,3,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,5),C_Adr(m,3,4),C_Adr(m,3,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,3,6),C_Adr(m,3,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,4,1),C_AdrT(m,1,2,'s'),C_Adr(m,4,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,2),C_Adr(m,4,1),C_Adr(m,4,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,3),C_Adr(m,4,2),C_Adr(m,4,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,4),C_Adr(m,4,3),C_Adr(m,4,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,5),C_Adr(m,4,4),C_Adr(m,4,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,4,6),C_Adr(m,4,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,5,1),C_AdrT(m,1,3,'m'),C_Adr(m,5,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,2),C_Adr(m,5,1),C_Adr(m,5,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,3),C_Adr(m,5,2),C_Adr(m,5,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,4),C_Adr(m,5,3),C_Adr(m,5,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,5),C_Adr(m,5,4),C_Adr(m,5,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,5,6),C_Adr(m,5,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,6,1),C_AdrT(m,1,4,'s'),C_Adr(m,6,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,2),C_Adr(m,6,1),C_Adr(m,6,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,3),C_Adr(m,6,2),C_Adr(m,6,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,4),C_Adr(m,6,3),C_Adr(m,6,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,5),C_Adr(m,6,4),C_Adr(m,6,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,6,6),C_Adr(m,6,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,7,1),C_AdrT(m,1,5,'s'),C_Adr(m,7,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,2),C_Adr(m,7,1),C_Adr(m,7,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,3),C_Adr(m,7,2),C_Adr(m,7,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,4),C_Adr(m,7,3),C_Adr(m,7,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,5),C_Adr(m,7,4),C_Adr(m,7,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,7,6),C_Adr(m,7,5),END_BL,speed_A,0,0,50);

		Create_Segment(Seg_i++,C_Adr(m,8,1),C_AdrT(m,1,5,'s'),C_Adr(m,8,2),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,2),C_Adr(m,8,1),C_Adr(m,8,3),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,3),C_Adr(m,8,2),C_Adr(m,8,4),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,4),C_Adr(m,8,3),C_Adr(m,8,5),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,5),C_Adr(m,8,4),C_Adr(m,8,6),speed_A,0,0,50);
		Create_Segment(Seg_i++,C_Adr(m,8,6),C_Adr(m,8,5),END_BL,speed_A,0,0,50);

		link.Adr1 = END_BL;
		link.Adr2 = END_BL;
	}*/
	else if(m == 8 || m == 9 || m == 10 || m == 12 || m == 13 || m == 14){//Stad
		Create_Unit2(m,8,8);
		/*
		Block addresses (Octal)
		--00--02--010--012--
		--01--03--011--013--

		Block numbers (Decimal)
		--0--1--2--3--
		--4--5--6--7--
		*/
		link.Adr1.Module = m;link.Adr1.Adr = 3;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 7;link.Adr2.type = 'R';
		//void Create_Segment(int Unit_Adr, struct adr Adr,struct adr NAdr,struct adr PAdr  ,char max_speed,char state,char dir,char len);
		//void Create_Segment(struct SegC Adr,char type, struct SegC Next, struct SegC Prev,char max_speed,char state,char dir,char len);

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,4,'R'));

		Create_Segment(0  ,CAdr(m,0,'R'),IN.Adr1      ,CAdr(m,1,'R'),speed_A,0,0,100);
		Create_Segment(02 ,CAdr(m,1,'R'),CAdr(m,0,'R'),CAdr(m,2,'R'),speed_A,0,0,100);
		Create_Segment(010,CAdr(m,2,'R'),CAdr(m,1,'R'),CAdr(m,3,'R'),speed_A,0,0,100);
		Create_Segment(012,CAdr(m,3,'R'),CAdr(m,2,'R'),EMPTY_BL     ,speed_A,0,0,100);

		Create_Segment(1  ,CAdr(m,4,'R'),IN.Adr2      ,CAdr(m,5,'R'),speed_A,0,1,100);
		Create_Segment(03 ,CAdr(m,5,'R'),CAdr(m,4,'R'),CAdr(m,6,'R'),speed_A,0,1,100);
		Create_Segment(011,CAdr(m,6,'R'),CAdr(m,5,'R'),CAdr(m,7,'R'),speed_A,0,1,100);
		Create_Segment(013,CAdr(m,7,'R'),CAdr(m,6,'R'),EMPTY_BL     ,speed_A,0,1,100);
	}
	else if(m == 11){//Korte Bocht
		Create_Unit2(m,8,8);

		link.Adr1.Module = m;link.Adr1.Adr = 0;link.Adr1.type = 'R';
		link.Adr2.Module = m;link.Adr2.Adr = 1;link.Adr2.type = 'R';

		join(IN.Adr1,CAdr(m,0,'R'));
		join(IN.Adr2,CAdr(m,1,'R'));

		Create_Segment(0,CAdr(m,0,'R'),IN.Adr1,EMPTY_BL,speed_A,0,0,100);
		Create_Segment(1,CAdr(m,1,'R'),IN.Adr2,EMPTY_BL,speed_A,0,1,100);
	}
	return link;
}

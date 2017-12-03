#define speed_A 250
#define speed_B 140
#define speed_C 90
#define speed_D 60


struct link Modules(int m, struct link IN){
	//Loop Left
	struct link link;
	link.Adr3 = C_AdrT(0,0,0,'e');
	link.Adr4 = C_AdrT(0,0,0,'e');

	printf("%i:%i:%i\t%i:%i:%i\t%i:%i:%i\t%i:%i:%i\n",link.Adr1.M,link.Adr1.B,link.Adr1.S,link.Adr2.M,link.Adr2.B,link.Adr2.S,link.Adr3.M,link.Adr3.B,link.Adr3.S,link.Adr4.M,link.Adr4.B,link.Adr4.S);

	if(m == 1){
		Create_Segment(C_Adr(m,1,1),C_AdrT(m,2,1,'S'),END_BL,speed_A,0,2,1);
		 Create_Switch(C_Adr(m,2,1),C_Adr(m,1,1),C_AdrT(m,3,1,'s'),IN.Adr1,1);

		 Create_Switch(C_Adr(m,3,1),C_Adr(m,4,1),C_AdrT(m,2,1,'s'),IN.Adr2,1);
		Create_Segment(C_Adr(m,4,1),C_AdrT(m,3,1,'S'),END_BL,speed_A,0,1,1);

		link.Adr1 = C_Adr(m,1,1);
		link.Adr2 = C_Adr(m,4,1);
	}
	else if(m == 2){
		block(IN.Adr1)->PAdr = C_Adr(m,1,1);
		Create_Segment(C_Adr(m,1,1),IN.Adr1,C_AdrT(m,2,1,'S'),speed_A,0,2,1);
		  Create_Switch(C_Adr(m,2,1),C_Adr(m,1,1),C_AdrT(m,3,1,'s'),END_BL,1);

		  Create_Switch(C_Adr(m,3,1),C_Adr(m,4,1),C_AdrT(m,2,1,'s'),END_BL,1);
		block(IN.Adr2)->PAdr = C_Adr(m,4,1);
		Create_Segment(C_Adr(m,4,1),IN.Adr2,C_AdrT(m,3,1,'S'),speed_A,0,1,1);

		link.Adr1 = C_AdrT(m,2,1,'s');
		link.Adr2 = C_AdrT(m,3,1,'s');
	}
	else if(m == 3){//Station 3 bakken
		block(IN.Adr1)->PAdr = C_Adr(m,1,1);
		Create_Segment(C_Adr(m,1,1),IN.Adr1,C_AdrT(m,1,1,'S'),speed_C,0,0,1);
		 Create_Switch(C_Adr(m,1,1),C_Adr(m,1,1),C_Adr(m,2,1),C_Adr(m,3,1),1);

		Create_Segment(C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),speed_C,0,0,1);
		Create_Segment(C_Adr(m,2,2),C_Adr(m,2,1),C_AdrT(m,4,1,'s'),speed_C,0,0,1);

		Create_Segment(C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),speed_C,0,0,1);
		Create_Segment(C_Adr(m,3,2),C_Adr(m,3,1),C_AdrT(m,4,1,'s'),speed_C,0,0,1);

		Create_Segment(C_Adr(m,4,1),C_AdrT(m,4,1,'S'),END_BL,speed_C,0,0,1);
		 Create_Switch(C_Adr(m,4,1),C_Adr(m,4,1),C_Adr(m,2,2),C_Adr(m,3,2),1);

		block(IN.Adr2)->PAdr = C_Adr(m,5,1);
		Create_Segment(C_Adr(m,5,1),IN.Adr2,C_AdrT(m,6,1,'S'),speed_C,0,1,1);
		 Create_Switch(C_Adr(m,6,1),C_Adr(m,5,1),C_AdrT(m,6,2,'S'),C_Adr(m,7,1),1);
		 Create_Switch(C_Adr(m,6,2),C_AdrT(m,6,1,'s'),C_Adr(m,8,1),C_Adr(m,9,1),1);

		Create_Segment(C_Adr(m,7,1),C_AdrT(m,6,1,'s'),C_Adr(m,7,2),speed_C,0,1,1);
		Create_Segment(C_Adr(m,7,2),C_Adr(m,7,1),C_AdrT(m,10,1,'s'),speed_C,0,1,1);

		Create_Segment(C_Adr(m,8,1),C_AdrT(m,6,2,'s'),C_Adr(m,8,2),speed_C,0,1,1);
		Create_Segment(C_Adr(m,8,2),C_Adr(m,8,1),C_AdrT(m,10,2,'s'),speed_C,0,1,1);

		Create_Segment(C_Adr(m,9,1),C_AdrT(m,6,2,'s'),C_Adr(m,9,2),speed_C,0,1,1);
		Create_Segment(C_Adr(m,9,2),C_Adr(m,9,1),C_AdrT(m,10,2,'s'),speed_C,0,1,1);

		 Create_Switch(C_Adr(m,10,1),C_Adr(m,11,1),C_AdrT(m,10,2,'S'),C_Adr(m,7,2),1);
		 Create_Switch(C_Adr(m,10,2),C_AdrT(m,10,1,'s'),C_Adr(m,8,2),C_Adr(m,9,2),1);
		Create_Segment(C_Adr(m,11,1),C_AdrT(m,10,1,'S'),END_BL,speed_C,0,1,1);

		link.Adr1 = C_Adr(m,4,1);
		link.Adr2 = C_Adr(m,11,1);
	}
	else if(m == 4){//Station 4 bakken
		block(IN.Adr1)->PAdr = C_Adr(m,1,1);
		Create_Segment(C_Adr(m,1,1),IN.Adr1,C_AdrT(m,1,1,'S'),speed_C,0,0,1);
		 Create_Switch(C_Adr(m,1,1),C_Adr(m,1,1),C_Adr(m,2,1),C_Adr(m,4,1),1);

		Create_Segment(C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),speed_C,0,0,1);
		Create_Segment(C_Adr(m,2,2),C_Adr(m,2,1),C_Adr(m,2,3),speed_C,0,0,1);
		Create_Segment(C_Adr(m,2,3),C_Adr(m,2,2),C_Adr(m,2,4),speed_C,0,0,1);
		Create_Segment(C_Adr(m,2,4),C_Adr(m,2,3),C_AdrT(m,4,1,'s'),speed_C,0,0,1);

		Create_Segment(C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),speed_C,0,0,1);
		Create_Segment(C_Adr(m,3,2),C_Adr(m,3,1),C_Adr(m,3,3),speed_C,0,0,1);
		Create_Segment(C_Adr(m,3,3),C_Adr(m,3,2),C_Adr(m,3,4),speed_C,0,0,1);
		Create_Segment(C_Adr(m,3,4),C_Adr(m,3,3),C_AdrT(m,4,1,'s'),speed_C,0,0,1);

		Create_Segment(C_Adr(m,4,1),C_AdrT(m,4,1,'S'),END_BL,speed_C,0,0,1);
		 Create_Switch(C_Adr(m,4,1),C_Adr(m,4,1),C_Adr(m,2,4),C_Adr(m,3,4),1);

		block(IN.Adr2)->PAdr = C_Adr(m,5,1);
		Create_Segment(C_Adr(m,5,1),IN.Adr2,C_AdrT(m,6,1,'S'),speed_C,0,1,1);
		 Create_Switch(C_Adr(m,6,1),C_Adr(m,5,1),C_AdrT(m,6,2,'S'),C_Adr(m,7,1),1);
		 Create_Switch(C_Adr(m,6,2),C_AdrT(m,6,1,'s'),C_Adr(m,8,1),C_Adr(m,9,1),1);

		Create_Segment(C_Adr(m,7,1),C_AdrT(m,6,1,'s'),C_Adr(m,7,2),speed_C,0,1,1);
		Create_Segment(C_Adr(m,7,2),C_Adr(m,7,1),C_Adr(m,7,3),speed_C,0,1,1);
		Create_Segment(C_Adr(m,7,3),C_Adr(m,7,2),C_Adr(m,7,4),speed_C,0,1,1);
		Create_Segment(C_Adr(m,7,4),C_Adr(m,7,3),C_AdrT(m,10,1,'s'),speed_C,0,1,1);

		Create_Segment(C_Adr(m,8,1),C_AdrT(m,6,2,'s'),C_Adr(m,8,2),speed_C,0,1,1);
		Create_Segment(C_Adr(m,8,2),C_Adr(m,8,1),C_Adr(m,8,3),speed_C,0,1,1);
		Create_Segment(C_Adr(m,8,3),C_Adr(m,8,2),C_Adr(m,8,4),speed_C,0,1,1);
		Create_Segment(C_Adr(m,8,4),C_Adr(m,8,3),C_AdrT(m,10,2,'s'),speed_C,0,1,1);

		Create_Segment(C_Adr(m,9,1),C_AdrT(m,6,2,'s'),C_Adr(m,9,2),speed_C,0,1,1);
		Create_Segment(C_Adr(m,9,2),C_Adr(m,9,1),C_Adr(m,9,3),speed_C,0,1,1);
		Create_Segment(C_Adr(m,9,3),C_Adr(m,9,2),C_Adr(m,9,4),speed_C,0,1,1);
		Create_Segment(C_Adr(m,9,4),C_Adr(m,9,3),C_AdrT(m,10,2,'s'),speed_C,0,1,1);

		 Create_Switch(C_Adr(m,10,1),C_Adr(m,11,1),C_AdrT(m,10,2,'S'),C_Adr(m,7,4),1);
		 Create_Switch(C_Adr(m,10,2),C_AdrT(m,10,1,'s'),C_Adr(m,8,4),C_Adr(m,9,4),1);
		Create_Segment(C_Adr(m,11,1),C_AdrT(m,10,1,'S'),END_BL,speed_C,0,1,1);

		link.Adr1 = C_Adr(m,4,1);
		link.Adr2 = C_Adr(m,11,1);
	}
	else if(m == 5){//T piece

		block(IN.Adr1)->PAdr = C_Adr(m,1,1);
		Create_Segment(C_Adr(m,1,1),IN.Adr1,END_BL,speed_A,0,0,1);

		block(IN.Adr2)->PAdr = C_AdrT(m,2,1,'S');
		Create_Switch(C_Adr(m,2,1),IN.Adr2,C_Adr(m,3,1),C_AdrT(m,5,1,'s'),1);
		Create_Switch(C_Adr(m,5,1),END_BL,C_Adr(m,4,1),C_AdrT(m,2,1,'s'),1);

		Create_Segment(C_Adr(m,3,1),C_AdrT(m,2,1,'s'),END_BL,speed_A,0,1,1);
		Create_Segment(C_Adr(m,4,1),C_AdrT(m,5,1,'s'),END_BL,speed_A,0,0,1);

		link.Adr1 = C_Adr(m,1,1);
		link.Adr2 = C_AdrT(m,5,1,'S');
		link.Adr3 = C_AdrT(m,3,1,'s');
		link.Adr4 = C_AdrT(m,2,1,'s');
	}
	else if(m == 6){//Rangeer Brug
		block(IN.Adr1)->PAdr = C_Adr(m,1,1);
		Create_Segment(C_Adr(m,1,1),IN.Adr1,C_AdrT(m,3,1,'s'),speed_A,0,0,1);

		block(IN.Adr2)->PAdr = C_Adr(m,2,1);
		Create_Segment(C_Adr(m,2,1),IN.Adr2,C_AdrT(m,3,1,'s'),speed_A,0,1,1);
		Create_Switch(C_Adr(m,3,1),END_BL,C_Adr(m,2,1),C_Adr(m,1,1),1);

		link.Adr1 = C_Adr(m,3,1);
		link.Adr2 = END_BL;
	}
	else if(m == 7){//Rangeer
		Create_Switch(C_Adr(m,1,1),C_AdrT(m,1,2,'s'),C_Adr(m,3,1),C_Adr(m,2,1),1);
		Create_Switch(C_Adr(m,1,2),C_AdrT(m,1,3,'s'),C_Adr(m,4,1),C_AdrT(m,1,1,'S'),1);


		block(link.Adr1)->PAdr = C_AdrT(m,1,3,'S');
		Create_Switch(C_Adr(m,1,3),link.Adr1,C_AdrT(m,1,2,'S'),C_AdrT(m,1,4,'S'),1);
		Create_Switch(C_Adr(m,1,4),C_AdrT(m,1,3,'s'),C_AdrT(m,1,5,'S'),C_Adr(m,5,1),1);

		Create_Switch(C_Adr(m,1,5),C_AdrT(m,1,4,'s'),C_Adr(m,6,1),C_AdrT(m,1,6,'S'),1);
		Create_Switch(C_Adr(m,1,6),C_AdrT(m,1,5,'s'),C_Adr(m,7,1),C_Adr(m,8,1),1);

		Create_Segment(C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),speed_A,0,0,1);
		Create_Segment(C_Adr(m,2,2),C_Adr(m,2,1),C_Adr(m,2,3),speed_A,0,0,1);
		Create_Segment(C_Adr(m,2,3),C_Adr(m,2,2),C_Adr(m,2,4),speed_A,0,0,1);
		Create_Segment(C_Adr(m,2,4),C_Adr(m,2,3),C_Adr(m,2,5),speed_A,0,0,1);
		Create_Segment(C_Adr(m,2,5),C_Adr(m,2,4),C_Adr(m,2,6),speed_A,0,0,1);
		Create_Segment(C_Adr(m,2,6),C_Adr(m,2,5),END_BL,speed_A,0,0,1);

		Create_Segment(C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),speed_A,0,0,1);
		Create_Segment(C_Adr(m,3,2),C_Adr(m,3,1),C_Adr(m,3,3),speed_A,0,0,1);
		Create_Segment(C_Adr(m,3,3),C_Adr(m,3,2),C_Adr(m,3,4),speed_A,0,0,1);
		Create_Segment(C_Adr(m,3,4),C_Adr(m,3,3),C_Adr(m,3,5),speed_A,0,0,1);
		Create_Segment(C_Adr(m,3,5),C_Adr(m,3,4),C_Adr(m,3,6),speed_A,0,0,1);
		Create_Segment(C_Adr(m,3,6),C_Adr(m,3,5),END_BL,speed_A,0,0,1);

		Create_Segment(C_Adr(m,4,1),C_AdrT(m,1,2,'s'),C_Adr(m,4,2),speed_A,0,0,1);
		Create_Segment(C_Adr(m,4,2),C_Adr(m,4,1),C_Adr(m,4,3),speed_A,0,0,1);
		Create_Segment(C_Adr(m,4,3),C_Adr(m,4,2),C_Adr(m,4,4),speed_A,0,0,1);
		Create_Segment(C_Adr(m,4,4),C_Adr(m,4,3),C_Adr(m,4,5),speed_A,0,0,1);
		Create_Segment(C_Adr(m,4,5),C_Adr(m,4,4),C_Adr(m,4,6),speed_A,0,0,1);
		Create_Segment(C_Adr(m,4,6),C_Adr(m,4,5),END_BL,speed_A,0,0,1);

		Create_Segment(C_Adr(m,5,1),C_AdrT(m,1,4,'s'),C_Adr(m,5,2),speed_A,0,0,1);
		Create_Segment(C_Adr(m,5,2),C_Adr(m,5,1),C_Adr(m,5,3),speed_A,0,0,1);
		Create_Segment(C_Adr(m,5,3),C_Adr(m,5,2),C_Adr(m,5,4),speed_A,0,0,1);
		Create_Segment(C_Adr(m,5,4),C_Adr(m,5,3),C_Adr(m,5,5),speed_A,0,0,1);
		Create_Segment(C_Adr(m,5,5),C_Adr(m,5,4),C_Adr(m,5,6),speed_A,0,0,1);
		Create_Segment(C_Adr(m,5,6),C_Adr(m,5,5),END_BL,speed_A,0,0,1);

		Create_Segment(C_Adr(m,6,1),C_AdrT(m,1,5,'s'),C_Adr(m,6,2),speed_A,0,0,1);
		Create_Segment(C_Adr(m,6,2),C_Adr(m,6,1),C_Adr(m,6,3),speed_A,0,0,1);
		Create_Segment(C_Adr(m,6,3),C_Adr(m,6,2),C_Adr(m,6,4),speed_A,0,0,1);
		Create_Segment(C_Adr(m,6,4),C_Adr(m,6,3),C_Adr(m,6,5),speed_A,0,0,1);
		Create_Segment(C_Adr(m,6,5),C_Adr(m,6,4),C_Adr(m,6,6),speed_A,0,0,1);
		Create_Segment(C_Adr(m,6,6),C_Adr(m,6,5),END_BL,speed_A,0,0,1);

		Create_Segment(C_Adr(m,7,1),C_AdrT(m,1,6,'s'),C_Adr(m,7,2),speed_A,0,0,1);
		Create_Segment(C_Adr(m,7,2),C_Adr(m,7,1),C_Adr(m,7,3),speed_A,0,0,1);
		Create_Segment(C_Adr(m,7,3),C_Adr(m,7,2),C_Adr(m,7,4),speed_A,0,0,1);
		Create_Segment(C_Adr(m,7,4),C_Adr(m,7,3),C_Adr(m,7,5),speed_A,0,0,1);
		Create_Segment(C_Adr(m,7,5),C_Adr(m,7,4),C_Adr(m,7,6),speed_A,0,0,1);
		Create_Segment(C_Adr(m,7,6),C_Adr(m,7,5),END_BL,speed_A,0,0,1);

		Create_Segment(C_Adr(m,8,1),C_AdrT(m,1,6,'s'),C_Adr(m,8,2),speed_A,0,0,1);
		Create_Segment(C_Adr(m,8,2),C_Adr(m,8,1),C_Adr(m,8,3),speed_A,0,0,1);
		Create_Segment(C_Adr(m,8,3),C_Adr(m,8,2),C_Adr(m,8,4),speed_A,0,0,1);
		Create_Segment(C_Adr(m,8,4),C_Adr(m,8,3),C_Adr(m,8,5),speed_A,0,0,1);
		Create_Segment(C_Adr(m,8,5),C_Adr(m,8,4),C_Adr(m,8,6),speed_A,0,0,1);
		Create_Segment(C_Adr(m,8,6),C_Adr(m,8,5),END_BL,speed_A,0,0,1);

		link.Adr1 = END_BL;
		link.Adr2 = END_BL;
	}
	else if(m == 8 || m == 9){//Stad
		block(IN.Adr1)->PAdr = C_Adr(m,1,1);
		Create_Segment(C_Adr(m,1,1),IN.Adr1,C_Adr(m,2,1),speed_A,0,0,1);
		Create_Segment(C_Adr(m,2,1),C_Adr(m,1,1),C_Adr(m,3,1),speed_A,0,0,1);
		Create_Segment(C_Adr(m,3,1),C_Adr(m,2,1),C_Adr(m,4,1),speed_A,0,0,1);
		Create_Segment(C_Adr(m,4,1),C_Adr(m,3,1),END_BL,speed_A,0,0,1);

		block(IN.Adr2)->PAdr = C_Adr(m,5,1);
		Create_Segment(C_Adr(m,5,1),IN.Adr2,C_Adr(m,6,1),speed_A,0,1,1);
		Create_Segment(C_Adr(m,6,1),C_Adr(m,5,1),C_Adr(m,7,1),speed_A,0,1,1);
		Create_Segment(C_Adr(m,7,1),C_Adr(m,6,1),C_Adr(m,8,1),speed_A,0,1,1);
		Create_Segment(C_Adr(m,8,1),C_Adr(m,7,1),END_BL,speed_A,0,1,1);

		link.Adr1 = C_Adr(m,4,1);
		link.Adr2 = C_Adr(m,8,1);
	}
	else if(m == 9){//Empty

	}
	else if(m == 10){//Bocht
		block(IN.Adr1)->PAdr = C_Adr(m,1,1);
		Create_Segment(C_Adr(m,1,1),IN.Adr1,C_Adr(m,4,1),speed_A,0,0,1);/*
		Create_Segment(C_Adr(m,2,1),C_Adr(m,1,1),C_Adr(m,3,1),speed_A,0,0,1);
		Create_Segment(C_Adr(m,3,1),C_Adr(m,2,1),C_Adr(m,4,1),speed_A,0,0,1);*/
		Create_Segment(C_Adr(m,4,1),C_Adr(m,1,1),END_BL,speed_A,0,0,1);

		block(IN.Adr2)->PAdr = C_Adr(m,5,1);
		Create_Segment(C_Adr(m,5,1),IN.Adr2,C_Adr(m,8,1),speed_A,0,1,1);/*
		Create_Segment(C_Adr(m,6,1),C_Adr(m,5,1),C_Adr(m,7,1),speed_A,0,1,1);
		Create_Segment(C_Adr(m,7,1),C_Adr(m,6,1),C_Adr(m,8,1),speed_A,0,1,1);*/
		Create_Segment(C_Adr(m,8,1),C_Adr(m,5,1),END_BL,speed_A,0,1,1);

		link.Adr1 = C_Adr(m,4,1);
		link.Adr2 = C_Adr(m,8,1);
	}

	return link;
}

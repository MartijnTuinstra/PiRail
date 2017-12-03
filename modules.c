
void Module_1(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	//Loop Left
	int m = 1;
	Create_Segment(C_Adr(m,1,1),C_AdrT(m,2,1,'S'),R1,0xFA,0,2,1);
	 Create_Switch(C_Adr(m,2,1),C_Adr(m,1,1),C_AdrT(m,3,1,'s'),L1,1);

	 Create_Switch(C_Adr(m,3,1),C_Adr(m,4,1),C_AdrT(m,2,1,'s'),L2,1);
	Create_Segment(C_Adr(m,4,1),C_AdrT(m,3,1,'S'),R2,0xFA,0,1,1);
}

void Module_2(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	//Loop Right
	int m = 2;
	Create_Segment(C_Adr(m,1,1),L1,C_AdrT(m,2,1,'S'),0xFA,0,2,1);
	 Create_Switch(C_Adr(m,2,1),C_Adr(m,1,1),C_AdrT(m,3,1,'s'),R1,1);

	 Create_Switch(C_Adr(m,3,1),C_Adr(m,4,1),C_AdrT(m,2,1,'s'),R2,1);
	Create_Segment(C_Adr(m,4,1),L2,C_AdrT(m,3,1,'S'),0xFA,0,1,1);
}

void Module_3(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	//Station 3 bakken
	int m = 3;
	Create_Segment(C_Adr(m,1,1),L1,C_AdrT(m,1,1,'S'),0xFA,0,0,1);
	 Create_Switch(C_Adr(m,1,1),C_Adr(m,1,1),C_Adr(m,2,1),C_Adr(m,3,1),1);

	Create_Segment(C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,2),C_Adr(m,2,1),C_AdrT(m,4,1,'s'),0xFA,0,0,1);

	Create_Segment(C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,2),C_Adr(m,3,1),C_AdrT(m,4,1,'s'),0xFA,0,0,1);

	Create_Segment(C_Adr(m,4,1),C_AdrT(m,4,1,'S'),R1,0xFA,0,0,1);
	 Create_Switch(C_Adr(m,4,1),C_Adr(m,4,1),C_Adr(m,2,2),C_Adr(m,3,2),1);


	Create_Segment(C_Adr(m,5,1),L2,C_AdrT(m,6,1,'S'),0xFA,0,1,1);
	 Create_Switch(C_Adr(m,6,1),C_Adr(m,5,1),C_AdrT(m,6,2,'S'),C_Adr(m,7,1),1);
	 Create_Switch(C_Adr(m,6,2),C_AdrT(m,6,1,'s'),C_Adr(m,8,1),C_Adr(m,9,1),1);

	Create_Segment(C_Adr(m,7,1),C_AdrT(m,6,1,'s'),C_Adr(m,7,2),0xFA,0,1,1);
	Create_Segment(C_Adr(m,7,2),C_Adr(m,7,1),C_AdrT(m,10,1,'s'),0xFA,0,1,1);

	Create_Segment(C_Adr(m,8,1),C_AdrT(m,6,2,'s'),C_Adr(m,8,2),0xFA,0,1,1);
	Create_Segment(C_Adr(m,8,2),C_Adr(m,8,1),C_AdrT(m,10,2,'s'),0xFA,0,1,1);

	Create_Segment(C_Adr(m,9,1),C_AdrT(m,6,2,'s'),C_Adr(m,9,2),0xFA,0,1,1);
	Create_Segment(C_Adr(m,9,2),C_Adr(m,9,1),C_AdrT(m,10,2,'s'),0xFA,0,1,1);

	 Create_Switch(C_Adr(m,10,1),C_Adr(m,11,1),C_AdrT(m,10,2,'S'),C_Adr(m,7,2),1);
	 Create_Switch(C_Adr(m,10,2),C_AdrT(m,10,1,'s'),C_Adr(m,8,2),C_Adr(m,9,2),1);
	Create_Segment(C_Adr(m,11,1),C_AdrT(m,10,1,'S'),R2,0xFA,0,1,1);
}

void Module_4(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	//Station 4 bakken
	int m = 4;
	Create_Segment(C_Adr(m,1,1),L1,C_AdrT(m,1,1,'S'),0xFA,0,0,1);
	 Create_Switch(C_Adr(m,1,1),C_Adr(m,1,1),C_Adr(m,2,1),C_Adr(m,4,1),1);

	Create_Segment(C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,2),C_Adr(m,2,1),C_Adr(m,2,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,3),C_Adr(m,2,2),C_Adr(m,2,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,4),C_Adr(m,2,3),C_AdrT(m,4,1,'s'),0xFA,0,0,1);

	Create_Segment(C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,2),C_Adr(m,3,1),C_Adr(m,3,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,3),C_Adr(m,3,2),C_Adr(m,3,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,4),C_Adr(m,3,3),C_AdrT(m,4,1,'s'),0xFA,0,0,1);

	Create_Segment(C_Adr(m,4,1),C_AdrT(m,4,1,'S'),R1,0xFA,0,0,1);
	 Create_Switch(C_Adr(m,4,1),C_Adr(m,4,1),C_Adr(m,2,4),C_Adr(m,3,4),1);


	Create_Segment(C_Adr(m,5,1),L2,C_AdrT(m,6,1,'S'),0xFA,0,1,1);
	 Create_Switch(C_Adr(m,6,1),C_Adr(m,5,1),C_AdrT(m,6,2,'S'),C_Adr(m,7,1),1);
	 Create_Switch(C_Adr(m,6,2),C_AdrT(m,6,1,'s'),C_Adr(m,8,1),C_Adr(m,9,1),1);

	Create_Segment(C_Adr(m,7,1),C_AdrT(m,6,1,'s'),C_Adr(m,7,2),0xFA,0,1,1);
	Create_Segment(C_Adr(m,7,2),C_Adr(m,7,1),C_Adr(m,7,3),0xFA,0,1,1);
	Create_Segment(C_Adr(m,7,3),C_Adr(m,7,2),C_Adr(m,7,4),0xFA,0,1,1);
	Create_Segment(C_Adr(m,7,4),C_Adr(m,7,3),C_AdrT(m,10,1,'s'),0xFA,0,1,1);

	Create_Segment(C_Adr(m,8,1),C_AdrT(m,6,2,'s'),C_Adr(m,8,2),0xFA,0,1,1);
	Create_Segment(C_Adr(m,8,2),C_Adr(m,8,1),C_Adr(m,8,3),0xFA,0,1,1);
	Create_Segment(C_Adr(m,8,3),C_Adr(m,8,2),C_Adr(m,8,4),0xFA,0,1,1);
	Create_Segment(C_Adr(m,8,4),C_Adr(m,8,3),C_AdrT(m,10,2,'s'),0xFA,0,1,1);

	Create_Segment(C_Adr(m,9,1),C_AdrT(m,6,2,'s'),C_Adr(m,9,2),0xFA,0,1,1);
	Create_Segment(C_Adr(m,9,2),C_Adr(m,9,1),C_Adr(m,9,3),0xFA,0,1,1);
	Create_Segment(C_Adr(m,9,3),C_Adr(m,9,2),C_Adr(m,9,4),0xFA,0,1,1);
	Create_Segment(C_Adr(m,9,4),C_Adr(m,9,3),C_AdrT(m,10,2,'s'),0xFA,0,1,1);

	 Create_Switch(C_Adr(m,10,1),C_Adr(m,11,1),C_AdrT(m,10,2,'S'),C_Adr(m,7,4),1);
	 Create_Switch(C_Adr(m,10,2),C_AdrT(m,10,1,'s'),C_Adr(m,8,4),C_Adr(m,9,4),1);
	Create_Segment(C_Adr(m,11,1),C_AdrT(m,10,1,'S'),R2,0xFA,0,1,1);
}

void Module_5(struct adr L1,struct adr L2,struct adr R1,struct adr R2,struct adr B1,struct adr B2){
	//T Piece
	int m = 5;
	Create_Segment(C_Adr(m,1,1),L1,R1,0xFA,0,0,1);

	Create_Switch(C_Adr(m,2,1),L2,C_Adr(m,3,1),C_AdrT(m,5,1,'s'),1);
	Create_Switch(C_Adr(m,5,1),R2,C_Adr(m,4,1),C_AdrT(m,2,1,'s'),1);

	Create_Segment(C_Adr(m,3,1),C_AdrT(m,2,1,'s'),B2,0xFA,0,1,1);
	Create_Segment(C_Adr(m,4,1),C_AdrT(m,5,1,'s'),B1,0xFA,0,0,1);
}

void Module_6(struct adr L1,struct adr L2,struct adr R){
	//Rangeer brug
	int m = 6;
	Create_Segment(C_Adr(m,1,1),L1,C_AdrT(m,3,1,'s'),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,1),L2,C_AdrT(m,3,1,'s'),0xFA,0,1,1);
	Create_Switch(C_Adr(m,3,1),R,C_Adr(m,2,1),C_Adr(m,1,1),1);
}

void Module_7(struct adr L){
	//Rangeer
	int m = 7;
	Create_Switch(C_Adr(m,1,1),C_AdrT(m,1,2,'s'),C_Adr(m,3,1),C_Adr(m,2,1),1);
	Create_Switch(C_Adr(m,1,2),C_AdrT(m,1,3,'m'),C_Adr(m,4,1),C_AdrT(m,1,1,'S'),1);

	struct adr * A[10] = {&L,&L,&L,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	struct adr * B[10] = {c_AdrT(m,1,2,'S'),c_Adr(m,5,1),c_AdrT(m,1,5,'S'),NULL,NULL,NULL,NULL,NULL,NULL,NULL};

	Create_Moduls(C_Adr(m,1,3),B,A,3);

	Create_Switch(C_Adr(m,1,4),C_AdrT(m,1,3,'m'),C_Adr(m,6,1),C_AdrT(m,1,5,'S'),1);
	Create_Switch(C_Adr(m,1,5),C_AdrT(m,1,4,'s'),C_Adr(m,7,1),C_Adr(m,8,1),1);

	Create_Segment(C_Adr(m,2,1),C_AdrT(m,1,1,'s'),C_Adr(m,2,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,2),C_Adr(m,2,1),C_Adr(m,2,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,3),C_Adr(m,2,2),C_Adr(m,2,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,4),C_Adr(m,2,3),C_Adr(m,2,5),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,5),C_Adr(m,2,4),C_Adr(m,2,6),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,6),C_Adr(m,2,5),END_BL,0xFA,0,0,1);

	Create_Segment(C_Adr(m,3,1),C_AdrT(m,1,1,'s'),C_Adr(m,3,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,2),C_Adr(m,3,1),C_Adr(m,3,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,3),C_Adr(m,3,2),C_Adr(m,3,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,4),C_Adr(m,3,3),C_Adr(m,3,5),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,5),C_Adr(m,3,4),C_Adr(m,3,6),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,6),C_Adr(m,3,5),END_BL,0xFA,0,0,1);

	Create_Segment(C_Adr(m,4,1),C_AdrT(m,1,2,'s'),C_Adr(m,4,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,4,2),C_Adr(m,4,1),C_Adr(m,4,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,4,3),C_Adr(m,4,2),C_Adr(m,4,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,4,4),C_Adr(m,4,3),C_Adr(m,4,5),0xFA,0,0,1);
	Create_Segment(C_Adr(m,4,5),C_Adr(m,4,4),C_Adr(m,4,6),0xFA,0,0,1);
	Create_Segment(C_Adr(m,4,6),C_Adr(m,4,5),END_BL,0xFA,0,0,1);

	Create_Segment(C_Adr(m,5,1),C_AdrT(m,1,3,'m'),C_Adr(m,5,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,5,2),C_Adr(m,5,1),C_Adr(m,5,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,5,3),C_Adr(m,5,2),C_Adr(m,5,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,5,4),C_Adr(m,5,3),C_Adr(m,5,5),0xFA,0,0,1);
	Create_Segment(C_Adr(m,5,5),C_Adr(m,5,4),C_Adr(m,5,6),0xFA,0,0,1);
	Create_Segment(C_Adr(m,5,6),C_Adr(m,5,5),END_BL,0xFA,0,0,1);

	Create_Segment(C_Adr(m,6,1),C_AdrT(m,1,4,'s'),C_Adr(m,6,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,6,2),C_Adr(m,6,1),C_Adr(m,6,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,6,3),C_Adr(m,6,2),C_Adr(m,6,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,6,4),C_Adr(m,6,3),C_Adr(m,6,5),0xFA,0,0,1);
	Create_Segment(C_Adr(m,6,5),C_Adr(m,6,4),C_Adr(m,6,6),0xFA,0,0,1);
	Create_Segment(C_Adr(m,6,6),C_Adr(m,6,5),END_BL,0xFA,0,0,1);

	Create_Segment(C_Adr(m,7,1),C_AdrT(m,1,5,'s'),C_Adr(m,7,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,7,2),C_Adr(m,7,1),C_Adr(m,7,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,7,3),C_Adr(m,7,2),C_Adr(m,7,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,7,4),C_Adr(m,7,3),C_Adr(m,7,5),0xFA,0,0,1);
	Create_Segment(C_Adr(m,7,5),C_Adr(m,7,4),C_Adr(m,7,6),0xFA,0,0,1);
	Create_Segment(C_Adr(m,7,6),C_Adr(m,7,5),END_BL,0xFA,0,0,1);

	Create_Segment(C_Adr(m,8,1),C_AdrT(m,1,5,'s'),C_Adr(m,8,2),0xFA,0,0,1);
	Create_Segment(C_Adr(m,8,2),C_Adr(m,8,1),C_Adr(m,8,3),0xFA,0,0,1);
	Create_Segment(C_Adr(m,8,3),C_Adr(m,8,2),C_Adr(m,8,4),0xFA,0,0,1);
	Create_Segment(C_Adr(m,8,4),C_Adr(m,8,3),C_Adr(m,8,5),0xFA,0,0,1);
	Create_Segment(C_Adr(m,8,5),C_Adr(m,8,4),C_Adr(m,8,6),0xFA,0,0,1);
	Create_Segment(C_Adr(m,8,6),C_Adr(m,8,5),END_BL,0xFA,0,0,1);
}

void Module_8(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	//Stad
	int m = 8;
	Create_Segment(C_Adr(m,1,1),L1,C_Adr(m,2,1),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,1),C_Adr(m,1,1),C_Adr(m,3,1),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,1),C_Adr(m,2,1),C_Adr(m,4,1),0xFA,0,0,1);
	Create_Segment(C_Adr(m,4,1),C_Adr(m,3,1),R1,0xFA,0,0,1);

	Create_Segment(C_Adr(m,5,1),L2,C_Adr(m,6,1),0xFA,0,1,1);
	Create_Segment(C_Adr(m,6,1),C_Adr(m,5,1),C_Adr(m,7,1),0xFA,0,1,1);
	Create_Segment(C_Adr(m,7,1),C_Adr(m,6,1),C_Adr(m,8,1),0xFA,0,1,1);
	Create_Segment(C_Adr(m,8,1),C_Adr(m,7,1),R2,0xFA,0,1,1);
}

void Module_10(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	//Stad
	int m = 10;
	Create_Segment(C_Adr(m,1,1),L1,C_Adr(m,4,1),0xFA,0,0,1);/*
	Create_Segment(C_Adr(m,2,1),C_Adr(m,1,1),C_Adr(m,3,1),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,1),C_Adr(m,2,1),C_Adr(m,4,1),0xFA,0,0,1);*/
	Create_Segment(C_Adr(m,4,1),C_Adr(m,1,1),R1,0xFA,0,0,1);

	Create_Segment(C_Adr(m,5,1),L2,C_Adr(m,8,1),0xFA,0,1,1);/*
	Create_Segment(C_Adr(m,6,1),C_Adr(m,5,1),C_Adr(m,7,1),0xFA,0,1,1);
	Create_Segment(C_Adr(m,7,1),C_Adr(m,6,1),C_Adr(m,8,1),0xFA,0,1,1);*/
	Create_Segment(C_Adr(m,8,1),C_Adr(m,5,1),R2,0xFA,0,1,1);
}


void Module_1(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	int m = 1;
	Create_Segment(C_Adr(m,1,1),C_AdrT(m,2,1,'S'),R1,0xFA,0,2,1);
	 Create_Switch(C_Adr(m,2,1),C_Adr(m,1,1),C_AdrT(m,3,1,'s'),L1,1);

	 Create_Switch(C_Adr(m,3,1),C_Adr(m,4,1),C_AdrT(m,2,1,'s'),L2,1);
	Create_Segment(C_Adr(m,4,1),C_AdrT(m,3,1,'S'),R2,0xFA,0,1,1);
}

void Module_2(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	int m = 2;
	Create_Segment(C_Adr(m,1,1),L1,C_AdrT(m,2,1,'S'),0xFA,0,2,1);
	 Create_Switch(C_Adr(m,2,1),C_Adr(m,1,1),C_AdrT(m,3,1,'s'),R1,1);

	 Create_Switch(C_Adr(m,3,1),C_Adr(m,4,1),C_AdrT(m,2,1,'s'),R2,1);
	Create_Segment(C_Adr(m,4,1),L2,C_AdrT(m,3,1,'S'),0xFA,0,1,1);
}

void Module_3(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
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

void Module_5(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	int m = 5;
	Create_Segment(C_Adr(m,1,1),L1,C_Adr(m,2,1),0xFA,0,0,1);
	Create_Segment(C_Adr(m,2,1),C_Adr(m,1,1),C_Adr(m,3,1),0xFA,0,0,1);
	Create_Segment(C_Adr(m,3,1),C_Adr(m,2,1),C_Adr(m,4,1),0xFA,0,0,1);
	Create_Segment(C_Adr(m,4,1),C_Adr(m,3,1),R1,0xFA,0,0,1);

	Create_Segment(C_Adr(m,5,1),L2,C_Adr(m,6,1),0xFA,0,1,1);
	Create_Segment(C_Adr(m,6,1),C_Adr(m,5,1),C_Adr(m,7,1),0xFA,0,1,1);
	Create_Segment(C_Adr(m,7,1),C_Adr(m,6,1),C_Adr(m,8,1),0xFA,0,1,1);
	Create_Segment(C_Adr(m,8,1),C_Adr(m,7,1),R2,0xFA,0,1,1);
}

void Module_7(struct adr L1,struct adr L2,struct adr R1,struct adr R2){
	int m = 7;
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

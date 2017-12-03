
void Module_1(unsigned short L1,unsigned short L2,unsigned short R1,unsigned short R2){
	int m = 1;
	Create_Segment(C_Adr(m,1,1),C_Adr(m,1,1)+0x2000,L1,0xFA,0x10);
	 Create_Switch(C_Adr(m,1,1),C_Adr(m,1,1),C_Adr(m,2,1),C_Adr(m,4,1),1);

	Create_Segment(C_Adr(m,2,1),C_Adr(m,4,1)+0x4000,C_Adr(m,1,1)+0x4000,0xFA,0x110);

	Create_Segment(C_Adr(m,3,1),C_Adr(m,4,1)+0x4000,C_Adr(m,1,1)+0x4000,0xFA,0x110);

	Create_Segment(C_Adr(m,4,1),R1,C_Adr(m,4,1)+0x2000,0xFA,0x10);
	 Create_Switch(C_Adr(m,4,1),C_Adr(m,4,1),C_Adr(m,2,1),C_Adr(m,3,1),1);


	Create_Segment(C_Adr(m,5,1),C_Adr(m,6,1)+0x2000,L1,0xFA,0x410);
	 Create_Switch(C_Adr(m,6,1),C_Adr(m,5,1),C_Adr(m,6,2)+0x2000,C_Adr(m,7,1),1);
	 Create_Switch(C_Adr(m,6,2),C_Adr(m,6,1)+0x4000,C_Adr(m,8,1),C_Adr(m,9,1),1);

	Create_Segment(C_Adr(m,7,1),C_Adr(m,10,1)+0x2000,C_Adr(m,6,1)+0x4000,0xFA,0x510);

	Create_Segment(C_Adr(m,8,1),C_Adr(m,10,2)+0x4000,C_Adr(m,6,2)+0x4000,0xFA,0x510);

	Create_Segment(C_Adr(m,9,1),C_Adr(m,10,2)+0x4000,C_Adr(m,6,2)+0x4000,0xFA,0x510);

	 Create_Switch(C_Adr(m,10,1),C_Adr(m,11,1),C_Adr(m,10,2)+0x2000,C_Adr(m,7,2),1);
	 Create_Switch(C_Adr(m,10,2),C_Adr(m,10,1)+0x4000,C_Adr(m,8,2),C_Adr(m,9,2),1);
	Create_Segment(C_Adr(m,11,1),R1,C_Adr(m,10,1)+0x2000,0xFA,0x410);
}

void Module_3(unsigned short L1,unsigned short L2,unsigned short R1,unsigned short R2){
	Create_Segment(C_Adr(3,1,1),C_Adr(3,2,1),L1,0xFA,0x10);
	Create_Segment(C_Adr(3,2,1),C_Adr(3,3,1),C_Adr(3,1,1),0xFA,0x10);
	Create_Segment(C_Adr(3,3,1),C_Adr(3,4,1),C_Adr(3,2,1),0xFA,0x10);
	Create_Segment(C_Adr(3,4,1),R1,C_Adr(3,3,1),0xFA,0x10);

	Create_Segment(C_Adr(3,5,1),C_Adr(3,6,1),L1,0xFA,0x410);
	Create_Segment(C_Adr(3,6,1),C_Adr(3,7,1),C_Adr(3,5,1),0xFA,0x410);
	Create_Segment(C_Adr(3,7,1),C_Adr(3,8,1),C_Adr(3,6,1),0xFA,0x410);
	Create_Segment(C_Adr(3,8,1),R1,C_Adr(3,7,1),0xFA,0x410);
}

void Module_5(unsigned short L1,unsigned short L2,unsigned short R1,unsigned short R2){
	int m = 5;
	Create_Segment(C_Adr(m,1,1),C_Adr(m,1,1)+0x2000,L1,0xFA,0x10);
	 Create_Switch(C_Adr(m,1,1),C_Adr(m,1,1),C_Adr(m,2,1),C_Adr(m,4,1),1);

	Create_Segment(C_Adr(m,2,1),C_Adr(m,2,2),C_Adr(m,1,1)+0x4000,0xFA,0x120);
	Create_Segment(C_Adr(m,2,2),C_Adr(m,2,1)+0x2000,C_Adr(m,2,1),0xFA,0x120);

	Create_Segment(C_Adr(m,3,1),C_Adr(m,3,2),C_Adr(m,1,1)+0x4000,0xFA,0x120);
	Create_Segment(C_Adr(m,3,2),C_Adr(m,4,1)+0x4000,C_Adr(m,4,1),0xFA,0x120);

	Create_Segment(C_Adr(m,4,1),R1,C_Adr(m,2,1)+0x2000,0xFA,0x10);
	Create_Switch(C_Adr(m,4,1),C_Adr(m,4,1),C_Adr(m,2,2),C_Adr(m,3,1),1);


	Create_Segment(C_Adr(m,5,1),C_Adr(m,6,1)+0x2000,L1,0xFA,0x410);
	 Create_Switch(C_Adr(m,6,1),C_Adr(m,5,1),C_Adr(m,6,2)+0x2000,C_Adr(m,7,1),1);
	 Create_Switch(C_Adr(m,6,2),C_Adr(m,6,1)+0x4000,C_Adr(m,8,1),C_Adr(m,9,1),1);

	Create_Segment(C_Adr(m,7,1),C_Adr(m,7,2),C_Adr(m,6,1)+0x4000,0xFA,0x520);
	Create_Segment(C_Adr(m,7,2),C_Adr(m,10,1)+0x2000,C_Adr(m,7,1),0xFA,0x520);

	Create_Segment(C_Adr(m,8,1),C_Adr(m,8,2),C_Adr(m,6,1)+0x4000,0xFA,0x520);
	Create_Segment(C_Adr(m,8,2),C_Adr(m,10,2)+0x4000,C_Adr(m,8,1),0xFA,0x520);

	Create_Segment(C_Adr(m,9,1),C_Adr(m,9,2),C_Adr(m,6,1)+0x4000,0xFA,0x520);
	Create_Segment(C_Adr(m,9,2),C_Adr(m,10,2)+0x4000,C_Adr(m,9,1),0xFA,0x520);

	 Create_Switch(C_Adr(m,10,1),C_Adr(m,11,1),C_Adr(m,10,2)+0x2000,C_Adr(m,7,2),1);
	 Create_Switch(C_Adr(m,10,2),C_Adr(m,10,1)+0x4000,C_Adr(m,8,2),C_Adr(m,9,2),1);
	Create_Segment(C_Adr(m,11,1),R1,C_Adr(m,10,1)+0x2000,0xFA,0x410);
}

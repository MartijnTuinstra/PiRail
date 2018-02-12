struct procces_block {
	_Bool blocked;
	char length;
	struct Seg * B[5];
};

void change_block_state2(struct procces_block * A,int State){
	if(!A->blocked){
		for(int i = 0;i<A->length;i++){
			if(A->B[i]->state != State){
				A->B[i]->change = 1;
				A->B[i]->state = State;
			}
		}
	}else{
		for(int i = 0;i<A->length;i++){
			if(A->B[i]->blocked){
				break;
			}
			if(A->B[i]->state != State){
				A->B[i]->change = 1;
				A->B[i]->state = State;
			}
		}
	}
}

void procces(struct Seg * B,int debug){
	if(B->type == 'T'){
		if(!B->blocked){
			B->train = 0;
		}
		if(debug){
			struct Seg *BA = B;
			if(BA->train != 0){
				printf("ID: %i\t%c%i:%i\n",BA->train,BA->Module,BA->id);
			}
			//printf("\t\t          \tA  %i:%i:%i;T%iR%i",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->train,BA->dir);
			//if(BA->blocked){
			//	printf("B");
			//}
			//printf("\n");
		}
	}
	else{
		//printf("B\n");

		struct procces_block BPPP,BPP,BP,BA,BN,BNN,BNNN;
		BA.B[0] = B;
		BA.blocked = B->blocked;
		BA.length = 1;

		struct Seg * bl[8] = {0};
		struct Seg * bpl[6] = {0};
		struct Seg * bp,*bpp,*bppp;
		struct Seg * Bl;
		bl[0] = B;
		bpl[0] = B;
		Bl = B;
		int i = 0;
		int p = 0;
		struct Seg * tB;
		//Get blocks in avalable path
		int q = 4;

		//Get the 3 next blocks
		for(i = 0;(i+1)<q;i){
			if(bl[i]->type != 'T'){ //If block has no contact points
				Bl = bl[i];
			}
			i++;
			//printf("i%i\t%i:%i:%i:%c\n",i,B.Adr.M,B.Adr.B,B.Adr.S,B.Adr.type);
			struct Rail_link A;
			if(dir_Comp(B,Bl)){
				A = NADR2(Bl);
			}else{
				A = PADR2(Bl);
			}
			if(A.type == 0){
				//printf("A.type == 0\n");
				q = i;
				break;
			}else if(A.type == 's' || A.type == 'S' || A.type == 'm' || A.type == 'M'){
				//printf("Check_switch\n");
				if(!check_Switch(Bl,0,FALSE)){
					//printf("WSw\n");
					q = i;
					break;
				}
			}
			bl[i] = Next2(B,i);
			//printf(".%i<%i\n",i,q);
			//printf("%i  %c%i:%i\t",i,bl[i]->type,bl[i]->Module,bl[i]->id);
			if(i > 1 && bl[i-1]->type == 'T' && bl[i]->type == 'T' && !Block_cmp(bl[i-1],bl[i])){
				//printf("Double T rail\n");
				q++;
			}else if(!bl[i]){
				q = i;
				break;
			}
		}
		i--;

		char r = 4;
		//Get the 3 previous blocks
		for(p = 0;p<=r;){
			if(bpl[p]->type != 'T'){ //If block has no contact points
				Bl = bpl[p];
			}
			p++;
			//printf("i%i\t%i:%i:%i:%c\n",i,B.Adr.M,B.Adr.B,B.Adr.S,B.Adr.type);
			struct Rail_link A;
			if(dir_Comp(B,Bl)){
				A = PADR2(Bl);
			}else{
				A = NADR2(Bl);
			}
			if(A.type == 0){
				//printf("A.type == 0\n");
				r = p;
				break;
			}else if(A.type == 's' || A.type == 'S' || A.type == 'm' || A.type == 'M'){
				//printf("Check_switch %i:%i\n",Bl->Module,Bl->id);
				if(!check_Switch(Bl,1,FALSE)){
					//printf("WSw\n");
					r = p;
					break;
				}
			}
			bpl[p] = Prev2(B,p);
			//printf(".%i<%i\n",p,r);
			//printf("%i  %c%i:%i\t",p,bpl[p]->type,bpl[p]->Module,bpl[p]->id);
			if(p > 1 && bpl[p-1]->type == 'T' && bpl[p]->type == 'T' && !Block_cmp(bpl[p-1],bpl[p])){
				//printf("Double T rail\n");
				r++;
			}
			if(p == r && bpl[p]->type == 'T'){
				r++;
			}
		}
		p--;

		i = 1;
		p = 1;
		int j = 0;
		int k = -1;
		int l = -1;

		//Clear pointer
		BPPP.B[0] = NULL;BPPP.B[1] = NULL;BPPP.B[2] = NULL;BPPP.B[3] = NULL;BPPP.B[4] = NULL;
		BPP.B[0] = NULL;BPP.B[1] = NULL;BPP.B[2] = NULL;BPP.B[3] = NULL;BPP.B[4] = NULL;
		BP.B[0] = NULL;BP.B[1] = NULL;BP.B[2] = NULL;BP.B[3] = NULL;BP.B[4] = NULL;
		BN.B[0] = NULL;BN.B[1] = NULL;BN.B[2] = NULL;BN.B[3] = NULL;BN.B[4] = NULL;
		BNN.B[0] = NULL;BNN.B[1] = NULL;BNN.B[2] = NULL;BNN.B[3] = NULL;BNN.B[4] = NULL;
		BNNN.B[0] = NULL;BNNN.B[1] = NULL;BNNN.B[2] = NULL;BNNN.B[3] = NULL;BNNN.B[4] = NULL;
		//Clear data
		BPPP.blocked = 0;BPP.blocked = 0;BP.blocked = 0;BN.blocked = 0;BNN.blocked = 0;BNNN.blocked = 0;
		BPPP.length = 0;BPP.length = 0;BP.length = 0;BN.length = 0;BNN.length = 0;BNNN.length = 0;

		//Assign next pointers
		//k == number of block ahead
		for(i;i<q;i++){
			if(i > 1 && bl[i-1]->type == 'T' && bl[i]->type == 'T'){
				j++;
			}else{
				k++;
				j = 0;
			}

			if(k == 0 && bl[i]){
				BN.B[j] = bl[i];
				BN.blocked |= BN.B[j]->blocked;
				BN.length = j+1;
			}else if(k == 1 && bl[i]){
				BNN.B[j] = bl[i];
				BNN.blocked |= BNN.B[j]->blocked;
				BNN.length = j+1;
			}else if(k == 2 && bl[i]){
				BNNN.B[j] = bl[i];
				BNNN.blocked |= BNNN.B[j]->blocked;
				BNNN.length = j+1;
			}
		}
		k++;
		i = k;


		//Assign prev pointers
		//p == number of block backward
		for(p;p<=r;p++){
			if(p > 1 && bpl[p-1]->type == 'T' && bpl[p]->type == 'T'){
				//printf("%c%i:%i==%c%i:%i\n",bpl[p-1]->type,bpl[p-1]->Module,bpl[p-1]->id,bpl[p]->type,bpl[p]->Module,bpl[p]->id);
				j++;
			}else{
				l++;
				j = 0;
			}

			if(l == 0 && bpl[p]){
				BP.B[j] = bpl[p];
				BP.blocked |= BP.B[j]->blocked;
				BP.length++;
			}else if(l == 1 && bpl[p]){
				BPP.B[j] = bpl[p];
				BPP.blocked |= BPP.B[j]->blocked;
				BPP.length++;
			}else if(l == 2 && bpl[p]){
				BPPP.B[j] = bpl[p];
				BPPP.blocked |= BPPP.B[j]->blocked;
				BPPP.length++;
			}
		}
		l++;
		p = l;


		//------------------------------------------------------------------------------------------ roadmap: more efficient scanning of the block. skip the blocks that are not changed/blocked and there neighbours are also not blocked
		//SPEEDUP function / if all blocks are not blocked skip!!
		/*if(k > 0 && !BA.blocked && !BN.blocked){
			if(p > 0 && !BP.blocked){
				return;
			}else if(p == 0){
				return;
			}
		}*/

		/*Train ID following*/
			if(!BA.blocked && BA.B[0]->train != 0){
				//Reset
				BA.B[0]->train = 0;
			}else if(BA.blocked && BA.B[0]->train != 0 && train_link[BA.B[0]->train] && !train_link[BA.B[0]->train]->Cur_Block){
				 train_link[BA.B[0]->train]->Cur_Block = BA.B[0];
			}
			if(k > 0 && BA.blocked && !BP.blocked && BN.blocked && BN.B[0]->train && !BA.B[0]->train){
				//REVERSED
				BA.B[0]->dir ^= 0b100;
				BN.B[0]->dir ^= 0b100;
			}
			else if(p > 0 && k > 0 && BA.blocked && BA.B[0]->train == 0 && BN.B[0]->train == 0 && BP.B[0]->train == 0){
				//NEW TRAIN
				BA.B[0]->train = ++bTrain;
				//Create a message for WebSocket
				WS_NewTrain(bTrain,BA.B[0]->Module,BA.B[0]->id);
			}
			else if(p > 0 && k > 0 && BN.blocked && BP.blocked && BN.B[0]->train == BP.B[0]->train){
				//A train has split
				WS_TrainSplit(BN.B[0]->train,BP.B[0]->Module,BP.B[0]->id,BN.B[0]->Module,BN.B[0]->id);
			}
			if(p > 0 && BP.blocked && BA.blocked && BA.B[0]->train == 0 && BP.B[0]->train != 0){
				BA.B[0]->train = BP.B[0]->train;
				if(train_link[BA.B[0]->train])
					train_link[BA.B[0]->train]->Cur_Block = BA.B[0];
			}
			if(k > 0 && BN.B[0]->type == 'T' && BN.blocked){
				if(BN.B[0]->train == 0 && BN.B[0]->blocked && BA.blocked && BA.B[0]->train != 0){
					BN.B[0]->train = BA.B[0]->train;
					if(train_link[BN.B[0]->train])
						train_link[BN.B[0]->train]->Cur_Block = BN.B[0];
				}else if(BN.length > 1){
					for(int a = 1;a<BN.length;a++){
						if(BN.B[a-1]->blocked && BN.B[a]->blocked && BN.B[a]->train == 0 && BN.B[a-1]->train != 0){
							BN.B[a]->train = BN.B[a-1]->train;
							if(train_link[BN.B[a]->train])
								train_link[BN.B[a]->train]->Cur_Block = BN.B[a];
							break;
						}
					}
				}
			}
		/**/
		/**/
		/*Check switch*/
			//
			int New_Switch = 0;
			struct Rail_link NAdr,NNAdr;

			NAdr = NADR2(BA.B[BA.length - 1]);
			if(k > 0){
				NNAdr = NADR2(BN.B[BN.length - 1]);
			}

			if((NNAdr.type == 's' || NNAdr.type == 'S' || NNAdr.type == 'm' || NNAdr.type == 'M') && BA.blocked){
				//There is a switch after the next block
				if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->Destination){
					//If train has a route
					if(check_Switch_state(NNAdr)){
						//Switch is free to use
						New_Switch = 2;
						printf("Free switch ahead (OnRoute %i:%i)\n",BA.B[0]->Module,BA.B[0]->id);
						if(!free_Route_Switch(BN.B[BN.length - 1],0,train_link[BA.B[0]->train])){
							printf("FAILED to set switch according Route\n");
							New_Switch = 0;
						}
					}
				}else{
					//No route
					if(check_Switch_state(NNAdr)){
						//Switch is free to use
						New_Switch = 2;
						printf("Free switch ahead %i:%i\n",BA.B[0]->Module,BA.B[0]->id);
						if(!check_Switch(BN.B[BN.length - 1],0,TRUE)){
							//The switch is in the wrong state / position
							printf("Check Switch\n");
							if(!free_Switch(BN.B[BN.length - 1],0)){
								New_Switch = 0;
				}}}}
			}else if((NAdr.type == 's' || NAdr.type == 'S' || NAdr.type == 'm' || NAdr.type == 'M') && BA.blocked){
				//There is a switch after the current block
				if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->Destination){
					//If train has a route
					if(check_Switch_state(NAdr)){
						//Switch is free to use
						New_Switch = 1;
						printf("Free switch ahead (OnRoute %i:%i)\n",BA.B[0]->Module,BA.B[0]->id);
						if(!free_Route_Switch(BA.B[BA.length - 1],0,train_link[BA.B[0]->train])){
							New_Switch = 0;
						}
					}
				}else{
					//No route
					if(check_Switch_state(NNAdr)){
						New_Switch = 1;
						//Switch is free to use
						printf("Free switch ahead %i:%i\n",BA.B[0]->Module,BA.B[0]->id);
						if(!check_Switch(BN.B[BN.length - 1],0,TRUE)){
							//The switch is in the wrong state / position
							printf("Check Switch\n");
							if(!free_Switch(BN.B[BN.length - 1],0)){
								printf("FAILED to set switch according Route\tSTOPPING TRAIN\n");
								New_Switch = 0;
				}}}}
			}

			//Extend if switches are thrown
			if(New_Switch > 0){
				printf("Switch thrown");
				if(k < 1){
					printf("BN  NEEDED\t");
					BN.B[0] = Next2(BA.B[0],1+BN.length);
					if(BN.B[0]){
						BN.length++;
						BN.blocked |= BN.B[0]->blocked;
						if(BN.B[0]->type == 'T'){
							BN.B[1] = Next2(BA.B[0],1+BN.length);
							if(BN.B[1]->type == 'T'){
								BN.length++;
								BN.blocked |= BN.B[1]->blocked;
							}else{
								BN.B[1] = NULL;
							}
						}
						k++;
					}
				}
				if(k < 2){
					printf("BNN  NEEDED\t");
					BNN.B[0] = Next2(BA.B[0],1+BN.length+BNN.length);
					if(BNN.B[0]){
						BNN.length++;
						BNN.blocked |= BNN.B[0]->blocked;
						if(BNN.B[0]->type == 'T'){
							BNN.B[1] = Next2(BA.B[0],1+BN.length+BNN.length);
							if(BNN.B[1]->type == 'T'){
								BNN.length++;
								BNN.blocked |= BNN.B[1]->blocked;
							}else{
								BNN.B[1] = NULL;
							}
						}
						k++;
					}
				}
				if(k < 3){
					BNNN.B[0] = Next2(BA.B[0],1+BN.length+BNN.length+BNNN.length);
					if(BNNN.B[0]){
						BNNN.length++;
						BNNN.blocked |= BNNN.B[0]->blocked;
						if(BNNN.B[0]->type == 'T'){
							BNNN.B[1] = Next2(BA.B[0],1+BN.length+BNN.length+BNNN.length);
							if(BNNN.B[1]->type == 'T'){
								BNNN.length++;
								BNNN.blocked |= BNNN.B[1]->blocked;
							}else{
								BNNN.B[1] = NULL;
							}
						}
						k++;
					}
				}
			}

			if(New_Switch == 1){
				change_block_state2(&BN,RESERVED);
			}else if(New_Switch == 2){
				change_block_state2(&BNN,RESERVED);
			}

			/*

				if((NAdr.type == 's' || NAdr.type == 'S' || NAdr.type == 'm' || NAdr.type == 'M') && BA.blocked){
					if(((NAdr.type == 's' || NAdr.type == 'S') && NAdr.Sw->Detection_Block && NAdr.Sw->Detection_Block->state != RESERVED) ||
						 ((NAdr.type == 'm' || NAdr.type == 'M') &&  NAdr.M->Detection_Block &&  NAdr.M->Detection_Block->state != RESERVED)){
						if(!check_Switch(BN.B[BN.length - 1],0,TRUE)){
							//The switch is not reserved and is in the wrong position
							printf("Check Switch\n");
							if(free_Switch2(BN.B[BN.length - 1],0)){
								printf("Freed");

								printf("BNN RESERVED\n");
								change_block_state2(&BNN,RESERVED);
							}
						}
						else{
							//If switch is in correct position but is not reserved
							change_block_state2(&BNN,RESERVED);
						}
					}
				}
			}*/
		/**/
		/**/
		/*Reverse block after one or two zero-blocks*/
			//If the next block is reversed, and not blocked
			if(i > 0 && BA.blocked && BN.B[0] && BN.B[0]->type != 'T' && !BN.blocked && !dir_Comp(BA.B[0],BN.B[0])){
				BN.B[0]->dir ^= 0b100;
			}

			//Reverse one block in advance
			if(i > 1 && BA.blocked && BNN.B[0] && BNN.B[0]->type != 'T' && !dir_Comp(BA.B[0],BNN.B[0]) &&
							(BN.B[0]->type == 'T' || !(dir_Comp(BA.B[0],BN.B[0]) == dir_Comp(BN.B[0],BNN.B[0])))){

				printf("Reverse in advance 1\n");
				if(BNN.B[0]->type == 'S'){ //Reverse whole platform if it is one
					printf("Whole platform\n");
					for(int a = 0;a<8;a++){
						if(BNN.B[0]->Station->Blocks[a]){
							if(BNN.B[0]->Station->Blocks[a]->blocked){
								break;
							}
							BNN.B[0]->Station->Blocks[a]->dir ^= 0b100;
						}
					}
				}else{
					BNN.B[0]->dir ^= 0b100;
				}
			}
		/**/
		/**/
		/*State coloring*/
			//Block behind train (blocked) becomes RED
			//Second block behind trin becomes AMBER
			//After that GREEN

			//Double 0-block counts as one block

			//If current block is blocked and previous block is free
			if(p > 0 && BA.blocked && !BP.blocked){
				change_block_state2(&BP,RED);
				if(p > 2)
					change_block_state2(&BPPP,GREEN);
				if(p > 1)
					change_block_state2(&BPP,AMBER);
			}
			else if(i > 0 && !BA.blocked && BN.blocked && BN.B[0]->type == 'T'){
				change_block_state2(&BA,RED);
				if(p > 0)
					change_block_state2(&BP,AMBER);
				if(p > 1)
					change_block_state2(&BPP,GREEN);
			}
			else if(p > 1 && k > 1 && !BA.blocked && !BN.blocked && !BP.blocked && !BNN.blocked && !BPP.blocked){
				change_block_state2(&BA,GREEN);
				if(p > 2 && !BPPP.blocked){
					change_block_state2(&BP,GREEN);
				}
				if(k > 2 && !BNNN.blocked){
					change_block_state2(&BN,GREEN);
				}
			}
		/**/
		/**/
		/*Signals*/
			//If a signal is at Next end and BN exists
			if(BA.B[0]->NSi && k > 0){
				//Wrong Switch
				//if current block is in forward and there are blocked switches
				// or if the block is in the wrong direction (reverse)
				if(((BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2) && !check_Switch(BA.B[0],0,TRUE)) || (BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6)){
					set_signal(BA.B[0]->NSi,SIG_RED);
				}

				if(!(BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6) && check_Switch(BA.B[0],0,TRUE) && i > 0){
					//Next block is RED/Blocked
					if(BN.blocked || BN.B[0]->state == RED){
						set_signal(BA.B[0]->NSi,SIG_RED);
					}else if(BN.B[0]->state == PARKED){
						set_signal(BA.B[0]->NSi,SIG_RESTRICTED); //Flashing RED
					}else if(BN.B[0]->state == AMBER){	//Next block AMBER
						set_signal(BA.B[0]->NSi,SIG_AMBER);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA.B[0]->NSi,SIG_GREEN);
					}
				}
			}
			else if(BA.B[0]->NSi && k == 0){ //If the track stops due to switches, set it to RED / DANGER
				set_signal(BA.B[0]->NSi,SIG_RED);
			}

			//If a signal is at Prev side and BP exists
			if(BA.B[0]->PSi && p > 0){
				//printf("Signal at %i:%i\n",BA.B[0]->Module,BA.B[0]->id);
				//printf("check_Switch: %i\n",check_Switch(BA.B[0],0,TRUE));
				//printf("Signal at %i:%i:%i\t0x%x\n",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->signals);
				//if current block is in reverse and there are blocked switches
				// or if the block is in the wrong direction (forward)
				if(((BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6) && !check_Switch(BA.B[0],0,TRUE)) || (BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2)){
					set_signal(BA.B[0]->PSi,SIG_RED);
					//printf("%i:%i:%i\tRed signal R2\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}

				if(!(BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2) && check_Switch(BA.B[0],0,TRUE) && i > 0){
					//Next block is RED/Blocked
					if(BN.blocked || BN.B[0]->state == RED){
						set_signal(BA.B[0]->PSi,SIG_RED);
					}else if(BN.B[0]->state == PARKED){
						set_signal(BA.B[0]->PSi,SIG_RESTRICTED); //Flashing RED
					}else if(BN.B[0]->state == AMBER){	//Next block AMBER
						set_signal(BA.B[0]->PSi,SIG_AMBER);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA.B[0]->PSi,SIG_GREEN);
					}
				}
			}
			else if(BA.B[0]->PSi && p == 0){ //If the track stops due to switches, set it to RED / DANGER
				set_signal(BA.B[0]->NSi,SIG_RED);
			}
		/**/
		/**/
		/*TRAIN control*/
			//Only if track is DCC controled and NOT DC!!
			if(BA.blocked && train_link[BA.B[0]->train]){
				if(k == 0){
					train_link[BA.B[0]->train]->halt = 1;
				}else if(k > 0 && train_link[BA.B[0]->train]->halt == 1){
					train_link[BA.B[0]->train]->halt = 0;
				}
			}
			if(digital_track == 1){

			/*SPEED*/
				//Check if current and next block are blocked, and have different trainIDs
				if(BA.blocked && BN.B[0]->blocked && BA.B[0]->train != BN.B[0]->train){
					if(train_link[BA.B[0]->train]){
						//Kill train
						printf("COLLISION PREVENTION\n\t");
						train_stop(train_link[BA.B[0]->train]);
					}else{
						//No train coupled
						printf("COLLISION PREVENTION\tEM_STOP\n\t");
						WS_EmergencyStop(); //WebSocket Emergency Stop
					}
				}
				//Check if next block is a RED block
				if(((BA.blocked && !BN.blocked && BN.B[0]->state == RED) || (k == 0 && BA.blocked))){
					if(train_link[BA.B[0]->train]){
						if(train_link[BA.B[0]->train]->timer != 1){
							//Fire stop timer
							printf("NEXT SIGNAL: RED\n\tSTOP TRAIN:");
							train_signal(BA.B[0],train_link[BA.B[0]->train],RED);
						}
					}else{
						//No train coupled
						printf("STOP TRAIN\tEM_STOP\n\t");
						WS_EmergencyStop();
					}
				}
				else if(k > 1 && BN.blocked && !BNN.blocked && BNN.B[0]->state == RED){
					if(train_link[BN.B[0]->train]){
						if(train_link[BN.B[0]->train]->timer != 1){
							//Fire stop timer
							printf("NEXT SIGNAL: RED\n\tSTOP TRAIN:");
							train_signal(BN.B[0],train_link[BN.B[0]->train],RED);
						}
					}else{
						//No train coupled
						printf("STOP TRAIN\tEM_STOP\n\t");
						WS_EmergencyStop();
					}
				}
				//Check if next block is a AMBER block
				if(((BA.blocked && !BN.blocked && BN.B[0]->state == AMBER) || (k == 1 && BA.blocked && !BN.blocked))){
					printf("Next AMBER\n");
					if(train_link[BA.B[0]->train]){
						if(train_link[BA.B[0]->train]->timer != 1){
							//Fire slowdown timer
							printf("NEXT SIGNAL: AMBER\n\tSLOWDOWN TRAIN:\t");
							train_signal(BA.B[0],train_link[BA.B[0]->train],RED);
						}
					}
				}

				//If the next 2 blocks are free, accelerate
				//If the next block has a higher speed limit than the current
				if(k > 0 && !BN.blocked && BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->timer != 2 && train_link[BA.B[0]->train]->timer != 1){
					if((BN.B[0]->state == GREEN || BN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_speed < BA.B[0]->max_speed && BN.B[0]->max_speed >= BA.B[0]->max_speed){
						printf("Next block has a higher speed limit (%i > %i)",BN.B[0]->max_speed,BA.B[0]->max_speed);
						train_speed(BA.B[0],train_link[BA.B[0]->train],BA.B[0]->max_speed);
					}
				}

				//If the next block has a lower speed limit than the current
				if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && train_link[BA.B[0]->train]->timer != 2){
					if(k > 0 && (BN.B[0]->state == GREEN || BN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_speed > BN.B[0]->max_speed && BN.B[0]->type != 'T'){
						printf("Next block has a lower speed limit");
						train_speed(BN.B[0],train_link[BA.B[0]->train],BN.B[0]->max_speed);
					}else if(k > 1 && BN.B[0]->type == 'T' && BNN.B[0]->type != 'T' && (BNN.B[0]->state == GREEN || BNN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_speed > BNN.B[0]->max_speed){
						printf("Block after Switches has a lower speed limit");
						train_speed(BNN.B[0],train_link[BA.B[0]->train],BNN.B[0]->max_speed);
					}else if(train_link[BA.B[0]->train]->cur_speed != BN.B[0]->max_speed && BN.B[0]->type != 'T'){
						printf("%i <= %i\n",train_link[BA.B[0]->train]->cur_speed,BN.B[0]->max_speed && BN.B[0]->type != 'T');
					}
				}
			//
			/*Station / Route*/
				//If next block is the destination
				if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && !Block_cmp(0,train_link[BA.B[0]->train]->Destination)){
					if(k > 0 && Block_cmp(train_link[BA.B[0]->train]->Destination,BN.B[0])){
						printf("Destination almost reached\n");
						train_signal(BA.B[0],train_link[BA.B[0]->train],AMBER);
					}else if(Block_cmp(train_link[BA.B[0]->train]->Destination,BA.B[0])){
						printf("Destination Reached\n");
						train_signal(BA.B[0],train_link[BA.B[0]->train],RED);
						train_link[BA.B[0]->train]->Destination = 0;
						train_link[BA.B[0]->train]->halt = TRUE;
					}
				}

			}
		/**/
		/**/
		/*Debug info*/
			if(BA.B[0]->train != 0){
				//printf("ID: %i\t%i:%i:%i\n",BA->train,BA->Adr.M,BA->Adr.B,BA->Adr.S);
			}
			if(debug){// || B->Module == 4){
				if(p > 2){
					printf("PPP ");
					for(int i = 1;i>=0;i--){
						if(BPPP.B[i]){
							printf("%02i:%02i",BPPP.B[i]->Module,BPPP.B[i]->id);
							if(BPPP.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}else{
					printf("                    ");
				}
				if(p > 1){
					printf("PP ");
					for(int i = 1;i>=0;i--){
						if(BPP.B[i]){
							printf("%02i:%02i",BPP.B[i]->Module,BPP.B[i]->id);
							if(BPP.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}else{
					printf("                   ");
				}
				if(p > 0){
					printf("P ");
					for(int i = 1;i>=0;i--){
						if(BP.B[i]){
							printf("%02i:%02i",BP.B[i]->Module,BP.B[i]->id);
							if(BP.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}else{
					printf("                  ");
				}
				printf("A%i %c%02i:%02i;T%iD%iS%i",BA.length,BA.B[0]->type,BA.B[0]->Module,BA.B[0]->id,BA.B[0]->train,BA.B[0]->dir,BA.B[0]->state);
				if(BA.B[0]->blocked){
					printf("B");
				}
				printf("\t");
				if(i > 0){
					printf("N ");
					for(int i = 0;i<2;i++){
						if(BN.B[i]){
							printf("%02i:%02i",BN.B[i]->Module,BN.B[i]->id);
							if(BN.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}if(i > 1){
					printf("NN ");
					for(int i = 0;i<2;i++){
						if(BNN.B[i]){
							printf("%02i:%02i",BNN.B[i]->Module,BNN.B[i]->id);
							if(BNN.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}
				if(i > 2){
					printf("NNN ");
					for(int i = 0;i<2;i++){
						if(BNNN.B[i]){
							printf("%02i:%02i",BNNN.B[i]->Module,BNNN.B[i]->id);
							if(BNNN.B[i]->blocked){
								printf("B  ");
							}else{
								printf("   ");
							}
						}else{
							printf("        ");
						}
					}
				}
				//if(i == 0){
				printf("\n");
				//}
				if(BA.B[0]->PSi || BA.B[0]->NSi){
					if(BA.B[0]->PSi){
						printf("\t\t\t\t\t\t%i  ",BA.B[0]->PSi->id);
						uint8_t state = BA.B[0]->PSi->state & 0x7F;
						if(state == SIG_RED  )printf("RED");
						if(state == SIG_AMBER)printf("AMBER");
						if(state == SIG_GREEN)printf("GREEN");
						if(state == SIG_RESTRICTED)printf("RESTR");
						printf("\t\t");
					}else{
						printf("\t\t\t\t\t\t\t\t\t");
					}
					if(BA.B[0]->NSi){
						printf("\t%i  ",BA.B[0]->NSi->id);
						uint8_t state = BA.B[0]->NSi->state & 0x7F;
						if(state == SIG_RED  )printf("RED");
						if(state == SIG_AMBER)printf("AMBER");
						if(state == SIG_GREEN)printf("GREEN");
						if(state == SIG_RESTRICTED)printf("RESTR");
					}
					printf("\n\n");
				}
			}
		/**/
	}
}

void procces_accessoire(){
	for(int i = 0;i<MAX_Modules;i++){
		if(Units[i] && Units[i]->Sig_change){
			printf("Signals of module %i changed\n",i);
			for(int j = 0;j<(MAX_Blocks*MAX_Segments);j++){
				if(Units[i]->Signals[j]){
					printf("Signal id: %i\n",Units[i]->Signals[j]->id);
				}
			}
			Units[i]->Sig_change = FALSE;
		}
	}
}

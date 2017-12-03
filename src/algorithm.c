/*void procces(struct adr adr,int debug){
	if(adr.S == 0){
		if(blocks[adr.M][adr.B][adr.S]->blocked == 0){
			blocks[adr.M][adr.B][adr.S]->train = 0;
		}
		//if(debug){
			struct Seg *BA = blocks[adr.M][adr.B][adr.S];
			if(BA->train != 0){
				//printf("ID: %i\t%i:%i:%i\n",BA->train,BA->Adr.M,BA->Adr.B,BA->Adr.S);
			}
			//printf("\t\t          \tA  %i:%i:%i;T%iR%i",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->train,BA->dir);
			//if(BA->blocked){
			//	printf("B");
			//}
			//printf("\n");
		//}
	}
	else{
		//printf("B\n");

		struct adr bl[4] = {0};
		struct adr bp,bp2;
		bl[0] = adr;
		int i = 0;
		int p = 0;
		struct Seg B;
		//Get blocks in avalable path
		for(i = 0;i<4;i){
			B = *blocks[bl[i].M][bl[i].B][bl[i].S];
			i++;
			//printf("i%i\t%i:%i:%i:%c\t%i:%i:%i:%c\n",i,B.Adr.M,B.Adr.B,B.Adr.S,B.Adr.type,NADR(B.Adr).M,NADR(B.Adr).B,NADR(B.Adr).S,NADR(B.Adr).type);
			if(NADR(B.Adr).type == 'e' && B.Adr.S != 0){
				break;
			}else if(NADR(B.Adr).type == 's' || NADR(B.Adr).type == 'S' || NADR(B.Adr).type == 'm' || NADR(B.Adr).type == 'M'){
				//printf("Check_switch\n");
				if(!check_Switch(B.Adr,0)){
					//printf("Switch checked\n");
					break;
				}
			}
			bl[i] = Next(adr,i)->Adr;
		}
		i--;

		//Setup previous address
		if(check_Switch(adr,1)){
			bp = Prev(adr,1)->Adr;
			p  = 1;
		}
		if(bp.S != 0 && p == 1 && check_Switch(bp,1) == 1){
			bp2 = Prev(adr,2)->Adr;
			p = 2;
		}

		struct Seg *BA = blocks[bl[0].M][bl[0].B][bl[0].S];
		struct Seg *BPP;
		struct Seg *BP;
		struct Seg *BN;
		struct Seg *BNN;
		struct Seg *BNNN;

		if(i > 0){
			BN = blocks[bl[1].M][bl[1].B][bl[1].S];
		}
		//printf("|");
		if(i > 1){
			BNN = blocks[bl[2].M][bl[2].B][bl[2].S];
		}
		//printf("|");
		if(i > 2){
			BNNN = blocks[bl[3].M][bl[3].B][bl[3].S];
		}
		//printf("|");
		if(p > 0){
			BP = blocks[bp.M][bp.B][bp.S];
		}
		//printf("|");
		if(p > 1){
			BPP = blocks[bp2.M][bp2.B][bp2.S];
		}
		//printf("|\n");

		//SPEEDUP/ if all blocks are not blocked skip!!
		/*
		if(i > 2 && !BA->blocked && !BN->blocked && !BNN->blocked && !BNNN->blocked){
			if(p > 0 && !BP->blocked){
				return;
			}else if(p == 0){
				return;
			}
		}*/

		/*Train ID following
			if(!BA->blocked && BA->train != 0){
				//Reset
				//printf("Reset");
				BA->train = 0;
			}
			if(i > 0 && BA->blocked && !BP->blocked && BN->train && !BA->train){
				//printf("Reverse\n");
				BA->dir ^= 0b100;
				//REVERSED
			}
			else if(p > 0 && i > 0 && BA->blocked && BA->train == 0 && BN->train == 0 && BP->train == 0){
					//NEW TRAIN
					BA->train = ++bTrain;
					//req_train(bTrain,BA->Adr);
					Web_Link_Train(ACTIVATE,bTrain,BA->Adr);
			}
			else if(p > 0 && i > 0 && BN->blocked && BP->blocked && BN->train == BP->train){
				//SPLIT
				Web_Train_Split(ACTIVATE,BN->train,BN->Adr);
			}
			if(p > 0 && BP->blocked && BA->blocked && BA->train == 0){
				BA->train = BP->train;
			}
			if(i > 0 && BN->train == 0 && BN->blocked && BA->blocked){
				BN->train = BA->train;
			}
			if(i > 1 && BNN->train == 0 && BNN->blocked && BN->blocked){
				BNN->train = BN->train;
			}
		/**/
		/**/
		/*Check switch
			//
			if(i > 0 && (NADR(BN->Adr).type == 's' || NADR(BN->Adr).type == 'S' || NADR(BN->Adr).type == 'm' || NADR(BN->Adr).type == 'M') && BA->blocked){
				if(!check_Switch(BN->Adr,0)){
					if(free_Switch(BN->Adr,0)){
						if(i < 2){
							BNN = Next(BN->Adr,1);
							BNNN = Next(BN->Adr,2);
							i = 3;
						}
						if(BNN->state != RESERVED){
							if(Adr_Comp(NADR(BN->Adr),BN->Adr)){
								change_block_state(BN,RESERVED);
								if(i > 1 && BNN->Adr.S == 0){
									change_block_state(BNN,RESERVED);
								}
							}else{
								change_block_state(BNN,RESERVED);
								if(i > 2 && BNNN->Adr.S == 0){
									change_block_state(BNNN,RESERVED);
								}
							}
						}
					}
				}else{
					if(BNN->state != RESERVED){
						if(Adr_Comp(NADR(BN->Adr),BN->Adr)){
							//printf("asdfjkkkkkkkk\n");
							change_block_state(BN,RESERVED);
							if(i > 1 && BNN->Adr.S == 0){
								change_block_state(BNN,RESERVED);
							}
						}else{
							change_block_state(BNN,RESERVED);
							if(i > 2 && BNNN->Adr.S == 0){
								change_block_state(BNNN,RESERVED);
							}
						}
					}
				}
			}
			else if(i > 0 && p > 0 && (NADR(BA->Adr).type == 's' || NADR(BA->Adr).type == 'S' || NADR(BA->Adr).type == 'm' || NADR(BA->Adr).type == 'M') && BP->blocked){
				if(!check_Switch(BA->Adr,0)){
					if(free_Switch(BA->Adr,0)){
						if(i < 2){
							BN = Next(BA->Adr,1);
							BNN = Next(BA->Adr,2);
							i = 2;
						}
						if(BN->state != RESERVED){
							if(Adr_Comp(NADR(BA->Adr),BA->Adr)){
								change_block_state(BA,RESERVED);
								if(i > 1 && BN->Adr.S == 0){
									change_block_state(BN,RESERVED);
								}
							}else{
								change_block_state(BN,RESERVED);
								if(i > 2 && BNN->Adr.S == 0){
									change_block_state(BNN,RESERVED);
								}
							}
						}
					}
				}else{
					if(BN->state != RESERVED){
						if(Adr_Comp(NADR(BA->Adr),BA->Adr)){
							//printf("asdfjkkkkkkkk\n");
							change_block_state(BA,RESERVED);
							if(i > 1 && BN->Adr.S == 0){
								change_block_state(BN,RESERVED);
							}
						}else{
							change_block_state(BN,RESERVED);
							if(i > 2 && BNN->Adr.S == 0){
								change_block_state(BNN,RESERVED);
							}
						}
					}
				}
			}
		/**/
		/**/
		/*Reverse block after one or two zero-blocks
			//If Next block is a Switch-block and the block after that is a normal block and that block is in reversed direction
			if(i > 1 && BA->blocked && !dir_Comp(BA,BNN) && BN->Adr.S == 0 && BNN->Adr.S != 0){
				printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->dir);
				printf("Reverse in advance 1\n");
				if(BNN->Adr.S == 1){
					for(int a = 1;a<MAX_Segments;a++){
						if(blocks[BNN->Adr.M][BNN->Adr.B][a] != NULL){
							if(blocks[BNN->Adr.M][BNN->Adr.B][a]->blocked){
								break;
							}
							blocks[BNN->Adr.M][BNN->Adr.B][a]->dir ^= 0b100;
						}
					}
				}else{
					//Reverse whole block
					for(int a = MAX_Segments;0<a;a--){
						if(blocks[BNN->Adr.M][BNN->Adr.B][a] != NULL){
							//Break if there is a Train in the Block
							if(blocks[BNN->Adr.M][BNN->Adr.B][a]->blocked){
								break;
							}
							blocks[BNN->Adr.M][BNN->Adr.B][a]->dir ^= 0b100;
						}
					}
				}
			}
			//If the next block and the block after it are both a Switch-block and the block after that is a normal block and that block is in reversed direction
			if(i > 2 && BA->blocked && !dir_Comp(BA,BNNN) && BN->Adr.S == 0 && BNN->Adr.S == 0 && BNNN->Adr.S != 0){
				printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BNNN->Adr.M,BNNN->Adr.B,BNNN->Adr.S,BNNN->dir);
				printf("Reverse in advance 2\n");
				if(BNNN->Adr.S == 1){
					for(int a = 1;a<MAX_Segments;a++){
						if(blocks[BNNN->Adr.M][BNNN->Adr.B][a] != NULL){
							if(blocks[BNNN->Adr.M][BNNN->Adr.B][a]->blocked){
								break;
							}
							blocks[BNNN->Adr.M][BNNN->Adr.B][a]->dir ^= 0b100;
						}
					}
				}else{
					//Reverse whole block
					for(int a = MAX_Segments;0<a;a--){
						if(blocks[BNNN->Adr.M][BNNN->Adr.B][a] != NULL){
							//Break if there is a Train in the Block
							if(blocks[BNNN->Adr.M][BNNN->Adr.B][a]->blocked){
								break;
							}
							blocks[BNNN->Adr.M][BNNN->Adr.B][a]->dir ^= 0b100;
						}
					}
				}
			}
			//If the next block is reversed, and not blocked
			if(i > 0 && BA->blocked && !dir_Comp(BA,BN) && BN->Adr.S != 0 && !BN->blocked){
				printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->dir);
				printf("Reverse next\n");
				BN->dir ^= 0b100;
			}
		/**/
		/**/
		/*State coloring
			//Block behind train (blocked) becomes RED
			//Second block behind trin becomes AMBER
			//After that GREEN

			//Double 0-block counts as one block

			//Self
			if(i > 2 && BN->Adr.S == 0 && BNN->Adr.S == 0 && !BN->blocked && (BNN->blocked || BNNN->blocked)){
				change_block_state(BA,AMBER);
			}
			else if(i > 1 && p > 1 && !dir_Comp(BA,BN) && BNN->blocked && BPP->blocked){
				change_block_state(BA,RED);
				change_block_state(BN,AMBER);
				change_block_state(BP,AMBER);
			}
			else if(i > 1 && p > 1 && !dir_Comp(BA,BN) && !BNN->blocked && !BPP->blocked){
				change_block_state(BA,GREEN);
			}
			else if(i > 1 && !dir_Comp(BA,BNN) && BN->Adr.S == 0 && !BNN->blocked && BNN->state == GREEN){
				change_block_state(BA,GREEN);
			}
			else if(i > 0 && BN->blocked){
				change_block_state(BA,RED);
			}
			else if(i > 1 && dir_Comp(BA,BNN) && BNN->blocked && !BN->blocked){
				change_block_state(BA,AMBER);
			}
			else if(i > 2 && dir_Comp(BA,BNN) && !BNN->blocked && !BN->blocked){
				if(BA->state != RESERVED){
					change_block_state(BA,GREEN);
				}
			}
			else if(i == 0){
				change_block_state(BA,AMBER);
			}
			else if(i == 1){
				change_block_state(BA,GREEN);
			}

			//Next
			if(i > 2 && BN->Adr.S == 0 && BNN->Adr.S == 0 && !BN->blocked && (BNN->blocked || BNNN->blocked)){
				change_block_state(BN,RED);
				change_block_state(BNN,RED);
			}else if(i > 2 && dir_Comp(BA,BNNN) && BNN->blocked && !BN->blocked){
				change_block_state(BN,RED);
			}
			else if(i > 2 && dir_Comp(BA,BNNN) && BNNN->blocked && !BNN->blocked){
				change_block_state(BN,AMBER);
			}
			else if(i > 2 && dir_Comp(BA,BNNN) && !BN->blocked && !BNN->blocked && !BNNN->blocked && !(BNN->Adr.S == 0 && BNNN->Adr.S == 0)){
				if(BN->state != RESERVED){
					change_block_state(BN,GREEN);
				}
			}

			//Next Next
			if(i > 2 && dir_Comp(BA,BNNN) && BNNN->blocked && !BNN->blocked){
				change_block_state(BNN,RED);
			}

			//Prev
			if(i > 2 && p > 0 && BP->blocked && BNNN->blocked && !dir_Comp(BP,BNN)){
				change_block_state(BNN,AMBER);
				change_block_state(BN,RED);
				change_block_state(BA,AMBER);
				debug = 1;
				//printf("SPECIAL\n");
			}
			else if(p > 0 && i > 0 && dir_Comp(BN,BP) && !BA->blocked && BN->blocked){
				change_block_state(BP,AMBER);
			}
			else if(p > 0 && i > 0 && dir_Comp(BN,BP) && !BA->blocked && !BN->blocked){
				if(BP->state != RESERVED){
					change_block_state(BP,GREEN);
				}
			}
		/**/
		/**/
		/*Signals
			if(BA->NSi != NULL){
				//Wrong Switch
				//printf("Signal at %i:%i:%i\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				if((BA->dir == 0 || BA->dir == 1 || BA->dir == 2) && !check_Switch(BA->Adr,0) || (BA->dir == 4 || BA->dir == 5 || BA->dir == 6)){
					set_signal(BA->NSi,RED_S);
					//printf("%i:%i:%i\tRed signal L1\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}

				if(!(BA->dir == 4 || BA->dir == 5 || BA->dir == 6) && check_Switch(BA->Adr,0) && i > 0){
					//Next block is RED/Blocked
					if(BN->blocked || BN->state == RED){
						set_signal(BA->NSi,RED_S);
					}else if(BN->state == PARKED){
						set_signal(BA->NSi,AMBER_F_S); //Flashing RED
					}else if(BN->state == AMBER){	//Next block AMBER
						set_signal(BA->NSi,AMBER_S);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA->NSi,GREEN_S);
					}
				}
			}

			if(BA->PSi != NULL){
				//printf("Signal at %i:%i:%i\t0x%x\n",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->signals);
				if((BA->dir == 4 || BA->dir == 5 || BA->dir == 6) && !check_Switch(BA->Adr,0) || (BA->dir == 0 || BA->dir == 1 || BA->dir == 2)){
					set_signal(BA->PSi,RED_S);
					//printf("%i:%i:%i\tRed signal R2\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}

				if(!(BA->dir == 0 || BA->dir == 1 || BA->dir == 2) && check_Switch(BA->Adr,0) && i > 0){
					//Next block is RED/Blocked
					if(BN->blocked || BN->state == RED){
						set_signal(BA->PSi,RED_S);
					}else if(BN->state == PARKED){
						set_signal(BA->PSi,AMBER_F_S); //Flashing RED
					}else if(BN->state == AMBER){	//Next block AMBER
						set_signal(BA->PSi,AMBER_S);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA->PSi,GREEN_S);
					}
				}
			}
		/**/
		/**/
		/*TRAIN control
			//Only if track is DCC controled and NOT DC!!
			if(digital_track == 1){

			/*SPEED
				//Check if current and next block are blocked, and have different trainIDs
				if(BA->blocked && BN->blocked && BA->train != BN->train){
					//Kill train
					printf("COLLISION PREVENTION\n\t");
					train_stop(train_link[BA->train]);
				}
				//Check if next block is a RED block
				if(((BA->blocked && !BN->blocked && BN->state == RED) || (i == 0 && BA->blocked)) && train_link[BA->train]->timer != 1){
					//Fire stop timer
					printf("NEXT SIGNAL: RED\n\tSTOP TRAIN:");
					train_signal(BA,train_link[BA->train],RED);
				}
				//Check if next block is a AMBER block
				if(((BA->blocked && !BN->blocked && BN->state == AMBER) || (i == 1 && BA->blocked && !BN->blocked)) && train_link[BA->train]->timer != 1){
					//Fire slowdown timer
					printf("NEXT SIGNAL: AMBER\n\tSLOWDOWN TRAIN:\t");
					train_signal(BA,train_link[BA->train],AMBER);
				}

				//If the next 2 blocks are free, accelerate
				//If the next block has a higher speed limit than the current
				if(i > 0 && !BN->blocked && BA->train != 0 && train_link[BA->train] != NULL && train_link[BA->train]->timer != 2 && train_link[BA->train]->timer != 1){
					if((BN->state == GREEN || BN->state == RESERVED) && train_link[BA->train]->cur_speed < BA->max_speed && BN->max_speed >= BA->max_speed){
						printf("Next block has a higher speed limit (%i > %i)",BN->max_speed,BA->max_speed);
						train_speed(BA,train_link[BA->train],BA->max_speed);
					}
				}

				//If the next block has a lower speed limit than the current
				if(BA->train != 0 && train_link[BA->train] != NULL && train_link[BA->train]->timer != 2){
					if((BN->state == GREEN || BN->state == RESERVED) && train_link[BA->train]->cur_speed > BN->max_speed && BN->Adr.S != 0){
						printf("Next block has a lower speed limit");
						train_speed(BN,train_link[BA->train],BN->max_speed);
					}else if(i > 1 && BN->Adr.S == 0 && BNN->Adr.S != 0 && (BNN->state == GREEN || BNN->state == RESERVED) && train_link[BA->train]->cur_speed > BNN->max_speed){
						printf("Block after Switches has a lower speed limit");
						train_speed(BNN,train_link[BA->train],BNN->max_speed);
					}else if(train_link[BA->train]->cur_speed != BN->max_speed && BN->Adr.S != 0){
						printf("%i <= %i\n",train_link[BA->train]->cur_speed,BN->max_speed && BN->Adr.S != 0);
					}
				}
			//
			/*Station / Route
			//If next block is the destination
			if(BA->train != 0 && train_link[BA->train] && !Adr_Comp(END_BL,train_link[BA->train]->Destination)){
				if(i > 0 && Adr_Comp(train_link[BA->train]->Destination,BN->Adr)){
					printf("Destination almost reached\n");
					train_signal(BNN,train_link[BA->train],AMBER);
				}else if(Adr_Comp(train_link[BA->train]->Destination,BA->Adr)){
					printf("Destination Reached\n");
					train_signal(BNN,train_link[BA->train],RED);
				}
			}

			}
		/**/
		/**/
		/*Debug info
			if(BA->train != 0){
				//printf("ID: %i\t%i:%i:%i\n",BA->train,BA->Adr.M,BA->Adr.B,BA->Adr.S);
			}
			if(debug){
				if(p > 0){
					printf("\t\tP %i:%i:%i;B:%i\t",BP->Adr.M,BP->Adr.B,BP->Adr.S,BP->blocked);
				}else{
					printf("\t\t          \t");
				}
				printf("A%i %i:%i:%i;T%iD%iS%i",i,adr.M,adr.B,adr.S,BA->train,BA->dir,BA->state);
				if(BA->blocked){
					printf("B");
				}
				printf("\t");
				if(i > 0){
					printf("N %i:%i:%i;B%iDC%i\t",BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->blocked,dir_Comp(BA,BN));
				}
				if(i > 1){
					printf("NN %i:%i:%i;B%iDC%i\t",BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->blocked,dir_Comp(BA,BNN));
				}
				if(i > 2){
					printf("NNN %i:%i:%i;B%iDC%i\t",BNNN->Adr.M,BNNN->Adr.B,BNNN->Adr.S,BNNN->blocked,dir_Comp(BA,BNNN));
				}
				//if(i == 0){
					printf("\n");
				//}
			}
	}
}*/

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

void procces2(struct adr adr,int debug){
	if(adr.S == 0){
		if(!blocks[adr.M][adr.B][adr.S]->blocked){
			blocks[adr.M][adr.B][adr.S]->train = 0;
		}
		//if(debug){
			struct Seg *BA = blocks[adr.M][adr.B][adr.S];
			if(BA->train != 0){
				printf("ID: %i\t%i:%i:%i\n",BA->train,BA->Adr.M,BA->Adr.B,BA->Adr.S);
			}
			//printf("\t\t          \tA  %i:%i:%i;T%iR%i",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->train,BA->dir);
			//if(BA->blocked){
			//	printf("B");
			//}
			//printf("\n");
		//}
	}
	else{
		//printf("B\n");

		struct adr bl[4] = {0};
		struct adr bp,bpp;
		bl[0] = adr;
		int i = 0;
		int p = 0;
		struct Seg B;
		//Get blocks in avalable path
		int q = 4;

		struct adr breakAdr = C_Adr(4,10,4);
		if(Adr_Comp(adr,breakAdr)){
			adr = adr;
		}
		for(i = 0;i<q;i){
			B = *blocks[bl[i].M][bl[i].B][bl[i].S];
			i++;
			//printf("i%i\t%i:%i:%i:%c\n",i,B.Adr.M,B.Adr.B,B.Adr.S,B.Adr.type);
			if(NADR(B.Adr).type == 'e' && B.Adr.S != 0){
				q = i;
				break;
			}else if(NADR(B.Adr).type == 's' || NADR(B.Adr).type == 'S' || NADR(B.Adr).type == 'm' || NADR(B.Adr).type == 'M'){
				//printf("Check_switch\n");
				if(!check_Switch(B.Adr,0,FALSE)){
					//printf("Switch checked\n");
					q = i;
					break;
				}
			}
			bl[i] = Next(adr,i)->Adr;
			if(i > 1 && bl[i-1].S == 0 && bl[i].S == 0 && !Adr_Comp(bl[i],END_BL)){
				q++;
			}
		}
		i--;

		//Setup previous address
		if(check_Switch(adr,1,FALSE)){
			bp = Prev(adr,1)->Adr;
			p  = 1;
		}
		if(bp.S != 0 && p == 1 && check_Switch(bp,1,FALSE) == 1){
			bpp = Prev(adr,2)->Adr;
			p = 2;
		}

		i = 1;
		int j = 0;
		int k = -1;

		struct procces_block BA;
		BA.B[0] = blocks[bl[0].M][bl[0].B][bl[0].S];
		BA.blocked = BA.B[0]->blocked;
		BA.length = 1;
		struct procces_block BPP,BP,BN,BNN,BNNN;

		//Clear pointer
		BPP.B[0] = NULL;BPP.B[1] = NULL;BPP.B[2] = NULL;BPP.B[3] = NULL;BPP.B[4] = NULL;
		BP.B[0] = NULL;BP.B[1] = NULL;BP.B[2] = NULL;BP.B[3] = NULL;BP.B[4] = NULL;
		BN.B[0] = NULL;BN.B[1] = NULL;BN.B[2] = NULL;BN.B[3] = NULL;BN.B[4] = NULL;
		BNN.B[0] = NULL;BNN.B[1] = NULL;BNN.B[2] = NULL;BNN.B[3] = NULL;BNN.B[4] = NULL;
		BNNN.B[0] = NULL;BNNN.B[1] = NULL;BNNN.B[2] = NULL;BNNN.B[3] = NULL;BNNN.B[4] = NULL;
		//Clear data
		BPP.blocked = 0;BP.blocked = 0;BN.blocked = 0;BNN.blocked = 0;BNNN.blocked = 0;
		BPP.length = 0;BP.length = 0;BN.length = 0;BNN.length = 0;BNNN.length = 0;

		for(i;i<q;i++){
			if(i > 1 && bl[i-1].S == 0 && bl[i].S == 0){
				j++;
			}else{
				k++;
				j = 0;
			}

			if(k == 0){
				BN.B[j] = blocks[bl[i].M][bl[i].B][bl[i].S];
				BN.blocked |= BN.B[j]->blocked;
				BN.length = j+1;
			}else if(k == 1){
				BNN.B[j] = blocks[bl[i].M][bl[i].B][bl[i].S];
				BNN.blocked |= BNN.B[j]->blocked;
				BNN.length = j+1;
			}else if(k == 2){
				BNNN.B[j] = blocks[bl[i].M][bl[i].B][bl[i].S];
				BNNN.blocked |= BNNN.B[j]->blocked;
				BNNN.length = j+1;
			}
		}
		k++;

		if(p > 0){
			BP.B[0] = blocks[bp.M][bp.B][bp.S];
			BP.blocked |= BP.B[0]->blocked;
			BP.length = 1;
		}

		if(p > 1){
			BPP.B[0] = blocks[bpp.M][bpp.B][bpp.S];
			BPP.blocked |= BPP.B[0]->blocked;
			BPP.length = 1;
		}

		//SPEEDUP function / if all blocks are not blocked skip!!
		/*
		if(k > 2 && !BA.blocked && !BN.blocked && !BNN.blocked && !BNNN.blocked){
			if(p > 0 && !BP.blocked){
				return;
			}else if(p == 0){
				return;
			}
		}*/

		/*Train ID following*/
			if(!BA.blocked && BA.B[0]->train != 0){
				//Reset
				//printf("Reset");
				BA.B[0]->train = 0;
			}
			if(k > 0 && BA.blocked && !BP.blocked && BN.B[0]->train && !BA.B[0]->train){
				//printf("Reverse\n");
				BA.B[0]->dir ^= 0b100;
				//REVERSED
			}
			else if(p > 0 && k > 0 && BA.blocked && BA.B[0]->train == 0 && BN.B[0]->train == 0 && BP.B[0]->train == 0){
					//NEW TRAIN
					BA.B[0]->train = ++bTrain;
					//req_train(bTrain,BA->Adr);
					Web_Link_Train(ACTIVATE,bTrain,BA.B[0]->Adr);
			}
			else if(p > 0 && k > 0 && BN.blocked && BP.blocked && BN.B[0]->train == BP.B[0]->train){
				//SPLIT
				Web_Train_Split(ACTIVATE,BN.B[0]->train,BN.B[0]->Adr);
			}
			if(p > 0 && BP.blocked && BA.blocked && BA.B[0]->train == 0 && BP.B[0]->train != 0){
				BA.B[0]->train = BP.B[0]->train;
			}
			if(k > 0 && BN.blocked){
				if(BA.blocked && BN.B[0]->train == 0){
					BN.B[0]->train = BA.B[0]->train;
				}else if(BN.length > 1){
					for(int a = 1;a<BN.length;a++){
						if(BN.B[a-1]->blocked && BN.B[a]->blocked && BN.B[a]->train == 0 && BN.B[a-1]->train != 0){
							BN.B[a]->train = BN.B[a-1]->train;
							break;
						}
					}
				}
			}
			if(k > 1 && BNN.B[0]->train == 0 && BNN.B[0]->blocked && BN.blocked && BN.length > 1){
				BNN.B[0]->train = BN.B[BN.length-1]->train;
			}
		/**/
		/**/
		/*Check switch*/
			//
			//struct adr AdrBNN = BNN.B[BNN.length - 1]->Adr;
			//struct adr AdrBNNN = BNNN.B[BNNN.length - 1]->Adr;
			if(k > 0){
				//Address of next block
				struct adr AdrBA = BN.B[BN.length - 1]->Adr;
				//Address of object after next block
				struct adr NAdr = NADR(AdrBA);

				if((NAdr.type == 's' || NAdr.type == 'S' || NAdr.type == 'm' || NAdr.type == 'M') && BA.blocked){
					printf("Check Switch\n");
					if(!check_Switch(AdrBA,0,TRUE)){
						if(free_Switch(AdrBA,0)){
							printf("Freed");
							if(k < 2){
								printf("BNN  NEEDED\t");
								BNN.B[0] = Next(BA.B[0]->Adr,BA.length+BN.length+BNN.length);
								BNN.length++;
								BNN.blocked |= BNN.B[0]->blocked;
								if(BNN.B[0]->Adr.S == 0){
									BNN.B[1] = Next(BA.B[0]->Adr,BA.length+BN.length+BNN.length);
									if(BNN.B[1]->Adr.S == 0){
										BNN.length++;
										BNN.blocked |= BNN.B[1]->blocked;
									}else{
										BNN.B[1] = NULL;
									}
								}
								k++;
							}
							if(k < 3){
								BNNN.B[0] = Next(BA.B[0]->Adr,BA.length+BN.length+BNN.length+BNNN.length);
								BNNN.length++;
								BNNN.blocked |= BNNN.B[0]->blocked;
								if(BNNN.B[0]->Adr.S == 0){
									BNNN.B[1] = Next(BA.B[0]->Adr,BA.length+BN.length+BNN.length+BNNN.length);
									if(BNNN.B[1]->Adr.S == 0){
										BNNN.length++;
										BNNN.blocked |= BNNN.B[1]->blocked;
									}else{
										BNNN.B[1] = NULL;
									}
								}
								k++;
							}
							printf("BNN RESERVED\n");
							change_block_state2(&BNN,RESERVED);
						}
					}else{
						change_block_state2(&BNN,RESERVED);
					}
				}
			}

			if(0){
				printf("BPP%i: ",BPP.length);
				for(int a = 0;a<BPP.length;a++){
					printf("%i:%i:%i %i",BPP.B[a]->Adr.M,BPP.B[a]->Adr.B,BPP.B[a]->Adr.S,BPP.B[a]->dir);
					if(BPP.B[a]->blocked){
						printf("B");
					}else{
						printf(" ");
					}
					printf("%i   ",BPP.B[0]->train);
				}
				for(int a = BPP.length;a<2;a++){printf("          ");}
				printf("\t");

				printf("BP%i : ",BP.length);
				for(int a = 0;a<BP.length;a++){
					printf("%i:%i:%i %i",BP.B[a]->Adr.M,BP.B[a]->Adr.B,BP.B[a]->Adr.S,BP.B[a]->dir);
					if(BP.B[a]->blocked){
						printf("B");
					}else{
						printf(" ");
					}
					printf("%i   ",BP.B[0]->train);
				}
				for(int a = BP.length;a<2;a++){printf("          ");}
				printf("\t");

				printf("BA%i: ",BA.length);
				for(int a = 0;a<BA.length;a++){
					printf("%i:%i:%i %i",BA.B[a]->Adr.M,BA.B[a]->Adr.B,BA.B[a]->Adr.S,BA.B[a]->dir);
					if(BA.B[a]->blocked){
						printf("B");
					}else{
						printf(" ");
					}
					printf("%i   ",BA.B[0]->train);
				}
				for(int a = BA.length;a<2;a++){printf("          ");}
				printf("\t");

				printf("\tBN%i:",BN.length);
				for(int a = 0;a<BN.length;a++){
					printf("%i:%i:%i %i",BN.B[a]->Adr.M,BN.B[a]->Adr.B,BN.B[a]->Adr.S,BN.B[a]->dir);
					if(BN.B[a]->blocked){
						printf("B");
					}else{
						printf(" ");
					}
					printf("%i   ",BN.B[0]->train);
				}
				for(int a = BN.length;a<2;a++){printf("          ");}
				printf("\t");

				printf("\tBNN%i:",BNN.length);
				for(int a = 0;a<BNN.length;a++){
					printf("%i:%i:%i %i",BNN.B[a]->Adr.M,BNN.B[a]->Adr.B,BNN.B[a]->Adr.S,BNN.B[a]->dir);
					if(BNN.B[a]->blocked){
						printf("B");
					}else{
						printf(" ");
					}
					printf("%i   ",BNN.B[0]->train);
				}
				for(int a = BNN.length;a<2;a++){printf("          ");}
				printf("\t");

				printf("\tBNNN%i:",BNNN.length);
				for(int a = 0;a<BNNN.length;a++){
					printf("%i:%i:%i %i",BNNN.B[a]->Adr.M,BNNN.B[a]->Adr.B,BNNN.B[a]->Adr.S,BNNN.B[a]->dir);
					if(BNNN.B[a]->blocked){
						printf("B");
					}else{
						printf(" ");
					}
					printf("%i  ",BNNN.B[0]->train);
				}
				printf("\n");
			}
		/**/
		/**/
		/*Reverse block after one or two zero-blocks*/
			//If Next block is a Switch-block and the block after that is a normal block and that block is in reversed direction
			if(i > 1 && BA.blocked && !dir_Comp(BA.B[0],BNN.B[0]) && BN.length > 1 && BNN.length == 1){
				printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA.B[0]->Adr.M,BA.B[0]->Adr.B,BA.B[0]->Adr.S,BA.B[0]->dir,BNN.B[0]->Adr.M,BNN.B[0]->Adr.B,BNN.B[0]->Adr.S,BNN.B[0]->dir);
				printf("Reverse in advance 1\n");
				if(BNN.B[0]->Adr.S == 1){
					for(int a = 1;a<MAX_Segments;a++){
						if(blocks[BNN.B[0]->Adr.M][BNN.B[0]->Adr.B][a] != NULL){
							if(blocks[BNN.B[0]->Adr.M][BNN.B[0]->Adr.B][a]->blocked){
								break;
							}
							blocks[BNN.B[0]->Adr.M][BNN.B[0]->Adr.B][a]->dir ^= 0b100;
						}
					}
				}else{
					//Reverse whole block
					for(int a = MAX_Segments;0<a;a--){
						if(blocks[BNN.B[0]->Adr.M][BNN.B[0]->Adr.B][a] != NULL){
							//Break if there is a Train in the Block
							if(blocks[BNN.B[0]->Adr.M][BNN.B[0]->Adr.B][a]->blocked){
								break;
							}
							blocks[BNN.B[0]->Adr.M][BNN.B[0]->Adr.B][a]->dir ^= 0b100;
						}
					}
				}
			}
			//If the next block is reversed, and not blocked
			if(i > 0 && BA.blocked && !dir_Comp(BA.B[0],BN.B[0]) && BN.B[0]->Adr.S != 0 && !BN.blocked){
				//printf("%i:%i:%i:%i <-> %i:%i:%i:%i\t",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->dir,BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->dir);
				printf("Reverse next\n");
				BN.B[0]->dir ^= 0b100;
			}
		/**/
		/**/
		/*State coloring*/
			//Block behind train (blocked) becomes RED
			//Second block behind trin becomes AMBER
			//After that GREEN

			//Double 0-block counts as one block

			//Self
			if(k > 0 && BN.blocked && BN.length > 1){
				change_block_state2(&BN,RED);
			}

			if(k > 0 && BN.blocked){
				change_block_state2(&BA,RED);
			}else if(k == 0 || (k > 0 && BN.B[0]->Adr.S == 0 && BNN.B[0]->oneWay && !dir_Comp(BA.B[0],BNN.B[0]) ||
								(k > 1 && BNN.blocked))){
				change_block_state2(&BA,AMBER);
			}else{
				change_block_state2(&BA,GREEN);
			}

			if(k > 1 && !BN.blocked && BNN.blocked){
				change_block_state2(&BN,RED);
			}else if(k > 2 && BNNN.blocked){
				if(dir_Comp(BA.B[0],BNN.B[0])){
					change_block_state2(&BN,AMBER);
				}
			}else if(k > 2 && !BN.blocked && !BNN.blocked && !BNNN.blocked && BN.B[0]->state != RESERVED){
				change_block_state2(&BN,GREEN);
			}
			/*
			if(i > 2 && BN->Adr.S == 0 && BNN->Adr.S == 0 && !BN->blocked && (BNN->blocked || BNNN->blocked)){
				change_block_state(BA,AMBER);
			}
			else if(i > 1 && p > 1 && !dir_Comp(BA,BN) && BNN->blocked && BPP->blocked){
				change_block_state(BA,RED);
				change_block_state(BN,AMBER);
				change_block_state(BP,AMBER);
			}
			else if(i > 1 && p > 1 && !dir_Comp(BA,BN) && !BNN->blocked && !BPP->blocked){
				change_block_state(BA,GREEN);
			}
			else if(i > 1 && !dir_Comp(BA,BNN) && BN->Adr.S == 0 && !BNN->blocked && BNN->state == GREEN){
				change_block_state(BA,GREEN);
			}
			else if(i > 0 && BN->blocked){
				change_block_state(BA,RED);
			}
			else if(i > 1 && dir_Comp(BA,BNN) && BNN->blocked && !BN->blocked){
				change_block_state(BA,AMBER);
			}
			else if(i > 2 && dir_Comp(BA,BNN) && !BNN->blocked && !BN->blocked){
				if(BA->state != RESERVED){
					change_block_state(BA,GREEN);
				}
			}
			else if(i == 0){
				change_block_state(BA,AMBER);
			}
			else if(i == 1){
				change_block_state(BA,GREEN);
			}

			//Next
			if(i > 2 && BN->Adr.S == 0 && BNN->Adr.S == 0 && !BN->blocked && (BNN->blocked || BNNN->blocked)){
				change_block_state(BN,RED);
				change_block_state(BNN,RED);
			}else if(i > 2 && dir_Comp(BA,BNNN) && BNN->blocked && !BN->blocked){
				change_block_state(BN,RED);
			}
			else if(i > 2 && dir_Comp(BA,BNNN) && BNNN->blocked && !BNN->blocked){
				change_block_state(BN,AMBER);
			}
			else if(i > 2 && dir_Comp(BA,BNNN) && !BN->blocked && !BNN->blocked && !BNNN->blocked && !(BNN->Adr.S == 0 && BNNN->Adr.S == 0)){
				if(BN->state != RESERVED){
					change_block_state(BN,GREEN);
				}
			}

			//Next Next
			if(i > 2 && dir_Comp(BA,BNNN) && BNNN->blocked && !BNN->blocked){
				change_block_state(BNN,RED);
			}

			//Prev
			if(i > 2 && p > 0 && BP->blocked && BNNN->blocked && !dir_Comp(BP,BNN)){
				change_block_state(BNN,AMBER);
				change_block_state(BN,RED);
				change_block_state(BA,AMBER);
				debug = 1;
				//printf("SPECIAL\n");
			}
			else if(p > 0 && i > 0 && dir_Comp(BN,BP) && !BA->blocked && BN->blocked){
				change_block_state(BP,AMBER);
			}
			else if(p > 0 && i > 0 && dir_Comp(BN,BP) && !BA->blocked && !BN->blocked){
				if(BP->state != RESERVED){
					change_block_state(BP,GREEN);
				}
			}
		/**/
		/**/
		/*Signals*/
			if(BA.B[0]->NSi != NULL && k > 0){
				//Wrong Switch
				//printf("Signal at %i:%i:%i\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				if((BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2) && !check_Switch(BA.B[0]->Adr,0,TRUE) || (BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6)){
					set_signal(BA.B[0]->NSi,RED_S);
					//printf("%i:%i:%i\tRed signal L1\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}

				if(!(BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6) && check_Switch(BA.B[0]->Adr,0,TRUE) && i > 0){
					//Next block is RED/Blocked
					if(BN.blocked || BN.B[0]->state == RED){
						set_signal(BA.B[0]->NSi,RED_S);
					}else if(BN.B[0]->state == PARKED){
						set_signal(BA.B[0]->NSi,AMBER_F_S); //Flashing RED
					}else if(BN.B[0]->state == AMBER){	//Next block AMBER
						set_signal(BA.B[0]->NSi,AMBER_S);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA.B[0]->NSi,GREEN_S);
					}
				}
			}

			if(BA.B[0]->PSi != NULL && p > 0){
				//printf("Signal at %i:%i:%i\t0x%x\n",BA->Adr.M,BA->Adr.B,BA->Adr.S,BA->signals);
				if((BA.B[0]->dir == 4 || BA.B[0]->dir == 5 || BA.B[0]->dir == 6) && !check_Switch(BA.B[0]->Adr,0,TRUE) || (BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2)){
					set_signal(BA.B[0]->PSi,RED_S);
					//printf("%i:%i:%i\tRed signal R2\n",BA->Adr.M,BA->Adr.B,BA->Adr.S);
				}

				if(!(BA.B[0]->dir == 0 || BA.B[0]->dir == 1 || BA.B[0]->dir == 2) && check_Switch(BA.B[0]->Adr,0,TRUE) && i > 0){
					//Next block is RED/Blocked
					if(BN.blocked || BN.B[0]->state == RED){
						set_signal(BA.B[0]->PSi,RED_S);
					}else if(BN.B[0]->state == PARKED){
						set_signal(BA.B[0]->PSi,AMBER_F_S); //Flashing RED
					}else if(BN.B[0]->state == AMBER){	//Next block AMBER
						set_signal(BA.B[0]->PSi,AMBER_S);
					}else{ // //Next block AMBER	if(BN->state == GREEN)
						set_signal(BA.B[0]->PSi,GREEN_S);
					}
				}
			}
		/**/
		/**/
		/*TRAIN control*/
			//Only if track is DCC controled and NOT DC!!
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
						Web_Emergency_Stop(ACTIVATE);
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
						Web_Emergency_Stop(ACTIVATE);
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
						Web_Emergency_Stop(ACTIVATE);
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
					if(k > 0 && (BN.B[0]->state == GREEN || BN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_speed > BN.B[0]->max_speed && BN.B[0]->Adr.S != 0){
						printf("Next block has a lower speed limit");
						train_speed(BN.B[0],train_link[BA.B[0]->train],BN.B[0]->max_speed);
					}else if(k > 1 && BN.B[0]->Adr.S == 0 && BNN.B[0]->Adr.S != 0 && (BNN.B[0]->state == GREEN || BNN.B[0]->state == RESERVED) && train_link[BA.B[0]->train]->cur_speed > BNN.B[0]->max_speed){
						printf("Block after Switches has a lower speed limit");
						train_speed(BNN.B[0],train_link[BA.B[0]->train],BNN.B[0]->max_speed);
					}else if(train_link[BA.B[0]->train]->cur_speed != BN.B[0]->max_speed && BN.B[0]->Adr.S != 0){
						printf("%i <= %i\n",train_link[BA.B[0]->train]->cur_speed,BN.B[0]->max_speed && BN.B[0]->Adr.S != 0);
					}
				}
			//
			/*Station / Route*/
			//If next block is the destination
			if(BA.B[0]->train != 0 && train_link[BA.B[0]->train] && !Adr_Comp(END_BL,train_link[BA.B[0]->train]->Destination)){
				if(k > 0 && Adr_Comp(train_link[BA.B[0]->train]->Destination,BN.B[0]->Adr)){
					printf("Destination almost reached\n");
					train_signal(BA.B[0],train_link[BA.B[0]->train],AMBER);
				}else if(Adr_Comp(train_link[BA.B[0]->train]->Destination,BA.B[0]->Adr)){
					printf("Destination Reached\n");
					train_signal(BA.B[0],train_link[BA.B[0]->train],RED);
					train_link[BA.B[0]->train]->Destination = END_BL;
					train_link[BA.B[0]->train]->halt = TRUE;
				}
			}

			}
		/**/
		/**/
		/*Debug info
			if(BA->train != 0){
				//printf("ID: %i\t%i:%i:%i\n",BA->train,BA->Adr.M,BA->Adr.B,BA->Adr.S);
			}
			if(debug){
				if(p > 0){
					printf("\t\tP %i:%i:%i;B:%i\t",BP->Adr.M,BP->Adr.B,BP->Adr.S,BP->blocked);
				}else{
					printf("\t\t          \t");
				}
				printf("A%i %i:%i:%i;T%iD%iS%i",i,adr.M,adr.B,adr.S,BA->train,BA->dir,BA->state);
				if(BA->blocked){
					printf("B");
				}
				printf("\t");
				if(i > 0){
					printf("N %i:%i:%i;B%iDC%i\t",BN->Adr.M,BN->Adr.B,BN->Adr.S,BN->blocked,dir_Comp(BA,BN));
				}
				if(i > 1){
					printf("NN %i:%i:%i;B%iDC%i\t",BNN->Adr.M,BNN->Adr.B,BNN->Adr.S,BNN->blocked,dir_Comp(BA,BNN));
				}
				if(i > 2){
					printf("NNN %i:%i:%i;B%iDC%i\t",BNNN->Adr.M,BNNN->Adr.B,BNNN->Adr.S,BNNN->blocked,dir_Comp(BA,BNNN));
				}
				//if(i == 0){
					printf("\n");
				//}
			}*/
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

#pragma once
#if 0
int NOP = 0, ADD = 1, SUB = 2, MUL = 3, DIV = 4, MOD = 5,
AND = 6, OR = 7, XOR = 8, EQZ = 9, EQ = 10, NE = 11,
LT = 12, GT = 13, LE = 14, GE = 15,
DROP = 16, LD32 = 17, SD32 = 18, LD8 = 19, SD8 = 20,
CONST32 = 21, CONST8 = 22, CALL = 23, RET = 24, BR = 25, BRIF = 26,
LAND = 27, LOR = 28, LNOT = 29,
PRINTF = 10, MEMCMP = 31, MALLOC = 32, MEMCHR = 33, FREE = 34,
FOPEN = 35, FREAD = 36, FWRITE = 37, FCLOSE = 38;
int run_asm(int *Asm, int Len, int *Stack, int Top, int Frame){
	if(Len == 0)return *(Stack + Top - 1);
	if(*Asm == NOP){
	} else if(*Asm == ADD){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) + *(Stack + Top);
	} else if(*Asm == SUB){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) - *(Stack + Top);
	} else if(*Asm == MUL){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) * *(Stack + Top);
	} else if(*Asm == DIV){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) / *(Stack + Top);
	} else if(*Asm == MOD){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) % *(Stack + Top);
	} else if(*Asm == AND){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) & *(Stack + Top);
	} else if(*Asm == OR){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) | *(Stack + Top);
	} else if(*Asm == XOR){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) ^ *(Stack + Top);
	} else if(*Asm == EQ){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) == *(Stack + Top);
	} else if(*Asm == NE){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) != *(Stack + Top);
	} else if(*Asm == LT){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) < *(Stack + Top);
	} else if(*Asm == GT){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) > *(Stack + Top);
	} else if(*Asm == LE){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) <= *(Stack + Top);
	} else if(*Asm == GE){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) >= *(Stack + Top);
	} else if(*Asm == LAND){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) && *(Stack + Top);
	} else if(*Asm == LOR){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) || *(Stack + Top);
	} else if(*Asm == EQZ){
		*(Stack + Top - 1) = *(Stack + Top - 1) == 0;
	} else if(*Asm == DROP){
		Top = Top - 1;
	} else if(*Asm == LNOT){
		*(Stack + Top - 1) = !(*(Stack + Top - 1));
	} else if(*Asm == CONST32 || *Asm == CONST8){
		Asm = Asm + 1;
		Top = Top + 1;
		Len = Len - 1;
		*(Stack + Top - 1) = *Asm;
	} else if(*Asm == BR){
		Top = Top - 1;
		Asm = Asm + *(Stack + Top);
	} else if(*Asm == BRIF){
		Top = Top - 2;
		if(*(Stack + Top + 1)){
			Asm = Asm + *(Stack + Top);
		}
	} else if(*Asm == CALL){
		Top = Top - 1;
		Asm = Asm + *(Stack + Top);
	} else if(*Asm == RET){
		Top = Top - 1;
		Asm = Asm + *(Stack + Top);
	} else if(*Asm == LD32){
		*(Stack + Top - 1) = *(int*)(*(Stack + Top - 1));
	} else if(*Asm == SD32){
		Top = Top - 2;
		*(int*)(*(Stack + Top)) = *(Stack + Top + 1);
	} else if(*Asm == LD8){
		*(Stack + Top - 1) = *(char*)(*(Stack + Top - 1));
	} else if(*Asm == SD8){
		Top = Top - 2;
		*(char*)(*(Stack + Top)) = (char)*(Stack + Top + 1);
	} else if(*Asm == PRINTF){
		Top = Top - 1;
		*(Stack + Top - 1) = printf((char*)*(Stack + Top - 1), *(Stack + Top));
	} else if(*Asm == MEMCMP){
		Top = Top - 2;
		*(Stack + Top - 1) = memcmp((char*)*(Stack + Top - 1), (char*)*(Stack + Top), *(Stack + Top + 1));
	} else if(*Asm == MEMCHR){
		Top = Top - 2;
		*(Stack + Top - 1) = (int)memchr((char*)*(Stack + Top - 1), *(Stack + Top), *(Stack + Top + 1));
	} else if(*Asm == MALLOC){
		*(Stack + Top - 1) = (int)malloc(*(Stack + Top - 1));
	} else if(*Asm == FREE){
		Top = Top - 1;
		free((char*)*(Stack + Top));
	} else if(*Asm == FOPEN){
		Top = Top - 1;
		*(Stack + Top - 1) = (int)fopen((char*)*(Stack + Top - 1), (char*)*(Stack + Top));
	} else if(*Asm == FREAD){
		Top = Top - 3;
		*(Stack + Top - 1) = fread((char*)*(Stack + Top - 1), *(Stack + Top), *(Stack + Top + 1), (FILE*)*(Stack + Top + 2));
	} else{

	}
	return run_asm(Asm + 1, Len - 1, Stack, Top, Frame);
}
int expr_asm(int *OpStack, int *OpResTypeStack, int *OpPrioStack, int TopOp, int *Asm, int *AsmLen, int Op, int OpPrio){
	if(TopOp == 0 || *(OpStack + TopOp - 1) == '('){
		return TopOp - 1;
	} else if(*(OpPrioStack + TopOp - 1) > OpPrio){
		if(*(OpResTypeStack + TopOp - 1) == Ptr32Exp && (*(OpPrioStack + TopOp - 1) == AddExp)){
			*(Asm + *AsmLen + 0) = CONST32;
			*(Asm + *AsmLen + 1) = 4;
			*(Asm + *AsmLen + 2) = MUL;
			*(Asm + *AsmLen + 3) = *(OpStack + TopOp - 1);
			*AsmLen = *AsmLen + 4;
		} else{
			*(Asm + *AsmLen + 0) = *(OpStack + TopOp - 1);
			*AsmLen = *AsmLen + 1;
		}
		return expr_asm(OpStack, OpResTypeStack, OpPrioStack, TopOp - 1, Asm, AsmLen, Op, OpPrio);
	} else{
		*(OpStack + TopOp) = Op;
		*(OpPrioStack + TopOp) = OpPrio;
		TopOp = TopOp + 1;
		return TopOp;
	}
}
int assign_expr(char **Tokens, char **TTypes, int *TVals, int TOffset,
	int *OpStack, int *OpResTypeStack, int *OpPrioStack, int TopOp, int *Asm, int *AsmLen){

	if(**Tokens == ';'){
		return TOffset;
	} else if(*(TTypes + TOffset) == LEXdigit || **(TTypes + TOffset) == '\''){
		*(Asm + *AsmLen + 0) = CONST32;
		*(Asm + *AsmLen + 1) = *TVals;
		*AsmLen = *AsmLen + 2;
		*(OpResTypeStack + TopOp) = IntExp;
	} else if(*(TTypes + TOffset) == Symbol32){
		*(Asm + *AsmLen + 0) = CONST32;
		*(Asm + *AsmLen + 1) = *TVals;
		*AsmLen = *AsmLen + 2;
		*(Asm + *AsmLen + 0) = LD32;
		*AsmLen = *AsmLen + 1;
		*(OpResTypeStack + TopOp) = IntExp;
	} else if(*(TTypes + TOffset) == Symbal8){
		*(Asm + *AsmLen + 0) = CONST32;
		*(Asm + *AsmLen + 1) = *TVals;
		*AsmLen = *AsmLen + 2;
		*(Asm + *AsmLen + 0) = LD8;
		*AsmLen = *AsmLen + 1;
		*(OpResTypeStack + TopOp) = IntExp;
	} else if(*(TTypes + TOffset) == Ptr32){
		*(Asm + *AsmLen + 0) = CONST32;
		*(Asm + *AsmLen + 1) = *TVals;
		*AsmLen = *AsmLen + 2;
		*(Asm + *AsmLen + 0) = LD32;
		*AsmLen = *AsmLen + 1;
		*(OpResTypeStack + TopOp) = Ptr32Exp;
	} else if(*(TTypes + TOffset) == Ptr8){
		*(Asm + *AsmLen + 0) = CONST32;
		*(Asm + *AsmLen + 1) = *TVals;
		*AsmLen = *AsmLen + 2;
		*(Asm + *AsmLen + 0) = LD32;
		*AsmLen = *AsmLen + 1;
		*(OpResTypeStack + TopOp) = Ptr8Exp;
	} else if(**TTypes == '"'){
		*(Asm + *AsmLen + 0) = CONST32;
		*(Asm + *AsmLen + 1) = (int)*Tokens;
		*AsmLen = *AsmLen + 2;
		*(OpResTypeStack + TopOp) = Ptr8Exp;
	} else if(**Tokens == '('){
		*(OpStack + TopOp) = '(';
		*(OpPrioStack + TopOp) = CommaExp;
	} else if(**Tokens == ')'){
		*(OpStack + TopOp) = ')';
		*(OpPrioStack + TopOp) = -999;
	} else if(**TTypes == ','){

	}
	TopOp = expr_asm(OpStack, OpResTypeStack, OpPrioStack, TopOp, Asm, AsmLen, *(OpStack + TopOp), *(OpPrioStack + TopOp));
}
#endif
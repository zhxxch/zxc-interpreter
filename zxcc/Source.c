#include<stdio.h>
#include<stdlib.h>
#include<string.h>
char* LEXvoid = "void",
*LEXint = "int",
*LEXchar = "char",
*LEXif = "if",
*LEXelse = "else",
*LEXreturn = "return",
*LEXprintf = "printf",
*LEXmemcmp = "memcmp",
*LEXmalloc = "malloc",
*LEXmemchr = "memchr",
*LEXfree = "free",
*LEXfopen = "fopen",
*LEXfread = "fread",
*LEXfile = "FILE",
*LEXdigit = "0123456789",
*LEXid1st = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
*LEXid = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
*LEXop = "!&%*+-/<=>^|~",
*LEXop2 = "=&|",
*LEXpunct = "(){},;",
*Symbol32 = "Symbol32",
*Symbol8 = "Symbol8";
*Ptr32 = "Ptr32",
*Ptr8 = "Ptr8";
int NOP = 0, ADD = 1, SUB = 2, MUL = 3, DIV = 4, MOD = 5,
AND = 6, OR = 7, XOR = 8, EQZ = 9, EQ = 10, NE = 11,
LT = 12, GT = 13, LE = 14, GE = 15,
DROP = 16, LD32 = 17, SD32 = 18, LD8 = 19, SD8 = 20,
CONST32 = 21, CONST8 = 22, CALL = 23, RET = 24, BR = 25, BRIF = 26,
LAND = 27, LOR = 28, LNOT = 29,
PRINTF = 10, MEMCMP = 31, MALLOC = 32, MEMCHR = 33, FREE = 34, FOPEN = 35, FREAD = 36, FWRITE = 37, FCLOSE = 38;
char* skip_line(char *Src){
	if(*Src == '\n')return Src + 1;
	return skip_line(Src + 1);
}
char* atoint(char *src, int *val){
	if(memchr(LEXdigit, *src, 10)){
		*val = (*val) * 10 + *src - '0';
		return atoint(src + 1, val);
	} else return src;
}
char* char_literal(char *src, int *val){
	if(*(src + 1) == '\\'){
		if(*(src + 2) == 'n')*val = '\n';
		if(*(src + 2) == '\\')*val = '\\';
		if(*(src + 2) == '\'')*val = '\'';
		return src + 4;
	} else{
		*val = *(src + 1);
		return src + 3;
	}
}
char* string_literal(char *src){
	if(*(src + 1) == '"' && *(src + 1) != '\\'){
		*(src + 1) = 0;
		return src + 2;
	} else{
		return string_literal(src + 1);
	}
}
char* identifier(char *src, int *len){
	if(memchr(LEXid, *src, 64)){
		*len = *len + 1;
		return(identifier(src + 1, len));
	}
	return src;
}

int token(char *Src, char** Tokens, char **TTypes, int *TVals, int Num){
	*Tokens = Src;
	*TTypes = 0;
	*TVals = 0;
	if(!(*Src))return Num;
	if(*Src == '#')return token(skip_line(Src), Tokens, TTypes, TVals, Num);
	else if(*Src == '/' && *(Src + 1) == '/'){
		return token(skip_line(Src), Tokens, TTypes, TVals, Num);
	} else if(memchr(LEXdigit, *Src, 10)){
		*TTypes = LEXdigit;
		return token(atoint(Src, TVals), Tokens + 1, TTypes + 1, TVals + 1, Num + 1);
	} else if(*Src == '\''){
		*TTypes = Src;
		return token(char_literal(Src, TVals), Tokens + 1, TTypes + 1, TVals + 1, Num + 1);
	} else if(*Src == '"'){
		*Tokens = Src + 1;
		*TTypes = Src;
		*TVals = Src + 1;
		return token(string_literal(Src), Tokens + 1, TTypes + 1, TVals + 1, Num + 1);
	} else if(memchr(LEXid1st, *Src, 54)){
		*TTypes = LEXid;
		return token(identifier(Src, TVals), Tokens + 1, TTypes + 1, TVals + 1, Num + 1);
	} else if(memchr(LEXop, *Src, 14)){
		if(memchr(LEXop2, *(Src + 1), 4)){
			*TTypes = LEXop2;
			*TVals = 2;
			return token(Src + 2, Tokens + 1, TTypes + 1, TVals + 1, Num + 1);
		}
		*TTypes = LEXop;
		*TVals = 1;
		return token(Src + 1, Tokens + 1, TTypes + 1, TVals + 1, Num + 1);
	} else if(memchr(LEXpunct, *Src, 9)){
		*TTypes = LEXpunct;
		*TVals = 1;
		return token(Src + 1, Tokens + 1, TTypes + 1, TVals + 1, Num + 1);
	} else{
		return token(Src + 1, Tokens, TTypes, TVals, Num);
	}
}
char* keywords_type(char* Tokens, int Len){
	if(!memcmp(LEXvoid, Tokens, Len) && Len == 4){
		return LEXvoid;
	} else if(!memcmp(LEXint, Tokens, Len) && Len == 3){
		return LEXint;
	} else if(!memcmp(LEXchar, Tokens, Len) && Len == 4){
		return LEXchar;
	} else if(!memcmp(LEXif, Tokens, Len) && Len == 2){
		return LEXif;
	} else if(!memcmp(LEXelse, Tokens, Len) && Len == 4){
		return LEXelse;
	} else if(!memcmp(LEXreturn, Tokens, Len) && Len == 6){
		return LEXreturn;
	} else if(!memcmp(LEXmemcmp, Tokens, Len) && Len == 6){
		return LEXmemcmp;
	} else if(!memcmp(LEXmalloc, Tokens, Len) && Len == 6){
		return LEXmalloc;
	} else if(!memcmp(LEXmemchr, Tokens, Len) && Len == 6){
		return LEXmemchr;
	} else if(!memcmp(LEXfree, Tokens, Len) && Len == 4){
		return LEXfree;
	} else if(!memcmp(LEXfopen, Tokens, Len) && Len == 5){
		return LEXfopen;
	} else if(!memcmp(LEXfread, Tokens, Len) && Len == 5){
		return LEXfread;
	} else if(!memcmp(LEXfile, Tokens, Len) && Len == 4){
		return LEXfile;
	} else return LEXid;
}
int keywords(char **Tokens, char **TTypes, int *TVals, int Num){
	if(Num == 0)return 0;
	if(*TTypes == LEXid){
		*TTypes = keywords_type(*Tokens, *TVals);
	}
	return keywords(Tokens + 1, TTypes + 1, TVals + 1, Num - 1);
}
int prim_expr(char **Tokens, char **TTypes, int *TVals, int *Asm, int AsmLen){
	*(Asm + 0) = CONST32;
	*(Asm + 1) = *TVals;

	if(*TTypes == Symbol32){
		*(Asm + 2) = LD32;
		AsmLen = AsmLen + 1;
	} else if(*TTypes == Symbol8){
		*(Asm + 2) = LD8;
		AsmLen = AsmLen + 1;
	} else if(*TTypes == LEXdigit){
	} else if(**TTypes == '"'){

	}
}
int CommaExp = 0, AssnExp = 1, LorExp = 2, LandExp = 3,
BorExp = 4, BxorExp = 5, BandExp = 6, EqExp = 7, RelExp = 8,
AddExp = 9, MulExp = 10, CastExp = 11, UnaryExp = 12, CallExp = 13, PrimExp = 14;
int Ptr8Exp = 15,Ptr32Exp =16, IntExp = 17;
int expr(char **Tokens, char **TTypes, int *TVals, int TOffset,
	int *OpStack, int *OpResTypeStack, int TopOp, int *Asm, int *AsmLen, int ExprType){
	if(ExprType = IntExp){
	}
	if(ExprType == PrimExp){
		*(Asm + *AsmLen + 0) = CONST32;
		*(Asm + *AsmLen + 1) = *TVals;
		*AsmLen = *AsmLen + 2;
		if(*(TTypes + TOffset) == LEXdigit || **(TTypes + TOffset) == '\''){
			*(OpResTypeStack + TopOp) = IntExp;
		} else if(*(TTypes + TOffset) == Symbol32){
			*(Asm + *AsmLen + 0) = LD32;
			*AsmLen = *AsmLen + 1;
			*(OpResTypeStack + TopOp) = IntExp;
		} else if(*(TTypes + TOffset) == Symbol8){
			*(Asm + *AsmLen + 0) = LD8;
			*AsmLen = *AsmLen + 1;
			*(OpResTypeStack + TopOp) = IntExp;
		} else if(*(TTypes + TOffset) == Ptr32){
			*(Asm + *AsmLen + 0) = LD32;
			*AsmLen = *AsmLen + 1;
			*(OpResTypeStack + TopOp) = Ptr32Exp;
		} else if(*(TTypes + TOffset) == Ptr8){
			*(Asm + *AsmLen + 0) = LD32;
			*AsmLen = *AsmLen + 1;
			*(OpResTypeStack + TopOp) = Ptr8Exp;
		} else if(**TTypes == '"'){

		}
	}
	if(**Tokens == ';'){
	}
	if(*TTypes == LEXop || *TTypes == LEXop2){

	}
}
int int_decls(char **Tokens, char **TTypes, int *TVals, int **RefVals, int TNum, int *Asm, int AsmLen){
	if(*TTypes == LEXint){

	}
}
int statement(char **Tokens, char **TTypes, int *TVals, int **RefVals, int TNum, int *Asm, int AsmLen){
	if(TNum == 0);
	if(*TTypes == LEXint){

	}
}
int print_tokens(char **Tokens, char **TTypes, int *TVals, int Num){
	if(Num == 0)return 0;
	printf("%c\t%c\t%i\n", **Tokens, **TTypes, *TVals);
	return print_tokens(Tokens + 1, TTypes + 1, TVals + 1, Num - 1);
}
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
		*(Stack + Top - 1) = *(Stack + Top - 1) + *(Stack + Top);
	} else if(*Asm == NE){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) - *(Stack + Top);
	} else if(*Asm == LT){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) * *(Stack + Top);
	} else if(*Asm == GT){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) / *(Stack + Top);
	} else if(*Asm == LE){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) + *(Stack + Top);
	} else if(*Asm == GE){
		Top = Top - 1;
		*(Stack + Top - 1) = *(Stack + Top - 1) - *(Stack + Top);
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
int main(int argc, char** argv){
	int memsz = 10000 * 10;
	if(argc < 2)return(9);
	FILE *fp = fopen(*(argv + 1), "r");
	char *Src = malloc(memsz);
	int SrcLen = fread(Src, 1, memsz, fp);
	*(Src + SrcLen) = 0;
	char** Tokens = malloc(memsz);
	char** TTypes = malloc(memsz);
	int* TVals = malloc(memsz);
	int NumTokens = token(Src, Tokens, TTypes, TVals, 0);
	keywords(Tokens, TTypes, TVals, NumTokens);
	int *Stack = malloc(memsz);
	int Top = 0, Frame = 0;
	*(Stack + Top) = argc - 2;
	Top = Top + 1;
	*(Stack + Top) = (int)(argv + 2);
	Top = Top + 1;

	return print_tokens(Tokens, TTypes, TVals, NumTokens);
}
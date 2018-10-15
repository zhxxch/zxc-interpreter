#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int MemSize;
FILE *SrcFp;
char *Src;
int SrcLen;
char** Tokens;
char** TTypes;
int* TVals;
int* SymbolAddrs;
int NumTokens;
int *Stack;
int Top = 0, Frame = 0;
char
*TTYint = "int",
*TTYchar = "char",
*TTYif = "if",
*TTYelse = "else",
*TTYreturn = "return",
*TTYprintf = "printf",
*TTYmemcmp = "memcmp",
*TTYmalloc = "malloc",
*TTYmemchr = "memchr",
*TTYfree = "free",
*TTYfopen = "fopen",
*TTYfread = "fread",
*TTYfexit = "exit",
*TTYfile = "FILE",
*TTYdigit = "0123456789",
*TTYid1st = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
*TTYid = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
*TTYop = "!&%*+-/<=>^|~",
*TTYop2 = "=&|",
*TTYpunct = "(){},;",
*Symbol32 = "_Var32",
*Symbol8 = "_Var8",
*Ptr32 = "_Ptr32",
*Ptr8 = "_Ptr8",
*SymbolFunc = "_Function";
int CommaExp = 0, AssnExp = 1, LorExp = 2, LandExp = 3,
BorExp = 4, BxorExp = 5, BandExp = 6, EqExp = 7, RelExp = 8,
AddExp = 9, MulExp = 10, CastExp = 11, UnaryExp = 12, CallExp = 13, PrimExp = 14;
int Ptr8Exp = 15, Ptr32Exp = 16, IntExp = 17, Int32Exp = 18, CharExp = 19;

char* skip_line(char *src_skip_line){
	if(*src_skip_line == '\n')return src_skip_line + 1;
	return skip_line(src_skip_line + 1);
}
char* atoint(char *src_a, int *val_int){
	if(memchr(TTYdigit, *src_a, 10)){
		*val_int = (*val_int) * 10 + *src_a - '0';
		return atoint(src_a + 1, val_int);
	} else return src_a;
}
char* char_literal(char *src_ch, int *val_ch){
	if(*(src_ch + 1) == '\\'){
		if(*(src_ch + 2) == 'n')*val_ch = '\n';
		if(*(src_ch + 2) == '\\')*val_ch = '\\';
		if(*(src_ch + 2) == '\'')*val_ch = '\'';
		return src_ch + 4;
	} else{
		*val_ch = *(src_ch + 1);
		return src_ch + 3;
	}
}
char* string_literal(char *src_in, char *src_out){
	if(*src_in == '\\'){
		if(*(src_in + 1) == 'n'){
			*src_out = '\n';
		} else{
			*src_out = *(src_in + 1);
		}
		return string_literal(src_in + 2, src_out + 1);
	} else if(*src_in == '"'){
		return src_in + 1;
	} else{
		*src_in = *src_out;
		return string_literal(src_in + 1, src_out + 1);
	}
}
char* identifier(char *src_id, int *id_len){
	if(memchr(TTYid, *src_id, 64)){
		*id_len = *id_len + 1;
		return(identifier(src_id + 1, id_len));
	}
	return src_id;
}

int token_scan(char *src_tk, int tke_num){
	*(Tokens + tke_num) = src_tk;
	*(TTypes + tke_num) = 0;
	*(TVals + tke_num) = 0;
	if(!(*src_tk))return tke_num;
	if(*src_tk == '#')return token_scan(skip_line(src_tk), tke_num);
	else if(*src_tk == '/' && *(src_tk + 1) == '/'){
		return token_scan(skip_line(src_tk), tke_num);
	} else if(memchr(TTYdigit, *src_tk, 10)){
		*(TTypes + tke_num) = TTYdigit;
		return token_scan(atoint(src_tk, (TVals + tke_num)), tke_num + 1);
	} else if(*src_tk == '\''){
		*(TTypes + tke_num) = src_tk;
		return token_scan(char_literal(src_tk, (TVals + tke_num)), tke_num + 1);
	} else if(*src_tk == '"'){
		*(TTypes + tke_num) = src_tk;
		*(TVals + tke_num) = (int)(src_tk + 1);
		return token_scan(string_literal(src_tk + 1, src_tk + 1), tke_num + 1);
	} else if(memchr(TTYid1st, *src_tk, 54)){
		*(TTypes + tke_num) = TTYid;
		*(TVals + tke_num) = 0;
		return token_scan(identifier(src_tk, (TVals + tke_num)), tke_num + 1);
	} else if(memchr(TTYop, *src_tk, 14)){
		if(memchr(TTYop2, *(src_tk + 1), 4)){
			*(TTypes + tke_num) = TTYop2;
			*(TVals + tke_num) = 2;
			return token_scan(src_tk + 2, tke_num + 1);
		}
		*(TTypes + tke_num) = TTYop;
		*(TVals + tke_num) = 1;
		return token_scan(src_tk + 1, tke_num + 1);
	} else if(memchr(TTYpunct, *src_tk, 9)){
		*(TTypes + tke_num) = TTYpunct;
		*(TVals + tke_num) = 1;
		return token_scan(src_tk + 1, tke_num + 1);
	} else{
		return token_scan(src_tk + 1, tke_num);
	}
}
char* keywords_type(char* keyword_tk, int kw_len){
	if(!memcmp(TTYint, keyword_tk, kw_len) && kw_len == 3){
		return TTYint;
	} else if(!memcmp(TTYchar, keyword_tk, kw_len) && kw_len == 4){
		return TTYchar;
	} else if(!memcmp(TTYif, keyword_tk, kw_len) && kw_len == 2){
		return TTYif;
	} else if(!memcmp(TTYelse, keyword_tk, kw_len) && kw_len == 4){
		return TTYelse;
	} else if(!memcmp(TTYreturn, keyword_tk, kw_len) && kw_len == 6){
		return TTYreturn;
	} else if(!memcmp(TTYmemcmp, keyword_tk, kw_len) && kw_len == 6){
		return TTYmemcmp;
	} else if(!memcmp(TTYmalloc, keyword_tk, kw_len) && kw_len == 6){
		return TTYmalloc;
	} else if(!memcmp(TTYmemchr, keyword_tk, kw_len) && kw_len == 6){
		return TTYmemchr;
	} else if(!memcmp(TTYfree, keyword_tk, kw_len) && kw_len == 4){
		return TTYfree;
	} else if(!memcmp(TTYfopen, keyword_tk, kw_len) && kw_len == 5){
		return TTYfopen;
	} else if(!memcmp(TTYfread, keyword_tk, kw_len) && kw_len == 5){
		return TTYfread;
	} else if(!memcmp(TTYfile, keyword_tk, kw_len) && kw_len == 4){
		return TTYfile;
	} else return TTYid;
}
int keywords_scan(int kw_scan_i){
	if(kw_scan_i < 0)return 0;
	if(*(TTypes + kw_scan_i) == TTYid){
		*(TTypes + kw_scan_i) = keywords_type(*(Tokens + kw_scan_i), *(TVals + kw_scan_i));
	}
	return keywords_scan(kw_scan_i - 1);
}

int print_tokens(int p_token_i, int num_token_print){
	if(p_token_i == num_token_print)return 0;
	printf("%c\t%c%c\t%i\t%i\t%i\n", **(Tokens + p_token_i), **(TTypes + p_token_i), *(*(TTypes + p_token_i) + 1),
		*(TVals + p_token_i), *(SymbolAddrs + p_token_i), *(Stack + *(SymbolAddrs + p_token_i)));
	return print_tokens(p_token_i + 1, num_token_print);
}
//Forward link identifiers
int defs_link(int defs_token_idx, int find_def_tkidx){
	if(find_def_tkidx == 0){
		return 0;
	} else if(**(TTypes + find_def_tkidx) == '_'
		&& *(TVals + defs_token_idx) == *(TVals + find_def_tkidx)
		&& !memcmp(*(Tokens + defs_token_idx),
			*(Tokens + find_def_tkidx), *(TVals + defs_token_idx))){
		*(TTypes + defs_token_idx) = *(TTypes + find_def_tkidx);
		*(SymbolAddrs + defs_token_idx) = *(SymbolAddrs + find_def_tkidx);
		return find_def_tkidx;
	}
	return defs_link(defs_token_idx, find_def_tkidx - 1);
}
//Backward link function identifiers
int defs_link_prototypes(int func_tk_idx, int func_identf_idx, int py_tk_idx){
	if(py_tk_idx == 0){
		return 0;
	} else if(func_tk_idx<0 && *(SymbolAddrs+py_tk_idx)==func_identf_idx){
		func_tk_idx = py_tk_idx;
	} else if(*(TTypes + py_tk_idx) == SymbolFunc
		&& *(TVals + func_tk_idx) == *(TVals + py_tk_idx)
		&& !memcmp(*(Tokens + func_tk_idx),
			*(Tokens + py_tk_idx), *(TVals + func_tk_idx))){
		*(SymbolAddrs + py_tk_idx) = func_identf_idx;
	}
	return defs_link_prototypes(func_tk_idx, func_identf_idx, py_tk_idx - 1);
}
int extern_defs_link(int ex_tk_idx, int ex_tk_num, int NumCparn, int NumParn,
	char* CurrentDefType, int ex_identf_idx, int FparamIdx){
	*(SymbolAddrs + ex_tk_idx) = 0;
	if(ex_tk_num == 0){
		return 0;
	} else if(**(Tokens + ex_tk_idx) == '{'){
		if(NumCparn == 0 && **(Tokens + ex_tk_idx - 1) == ')'){
			*(Stack + ex_identf_idx) = ex_tk_idx;
			defs_link_prototypes(-1, ex_identf_idx, ex_tk_idx - 1);
		}
		return extern_defs_link(ex_tk_idx + 1, ex_tk_num - 1,
			NumCparn + 1, NumParn, CurrentDefType, ex_identf_idx, FparamIdx);
	} else if(**(Tokens + ex_tk_idx) == '}'){
		//XXX: NumCparn = NumCparn - 1;
		return extern_defs_link(ex_tk_idx + 1, ex_tk_num - 1,
			NumCparn - 1, NumParn, CurrentDefType, ex_identf_idx, FparamIdx);
	} else if(**(Tokens + ex_tk_idx) == '('){
		return extern_defs_link(ex_tk_idx + 1, ex_tk_num - 1,
			NumCparn, NumParn + 1, CurrentDefType, ex_identf_idx, FparamIdx);
	} else if(**(Tokens + ex_tk_idx) == ')'){
		return extern_defs_link(ex_tk_idx + 1, ex_tk_num - 1,
			NumCparn, NumParn - 1, CurrentDefType, ex_identf_idx, FparamIdx);
	} else if(*(TTypes + ex_tk_idx) == TTYint){
		CurrentDefType = Symbol32;
		return extern_defs_link(ex_tk_idx + 1, ex_tk_num - 1,
			NumCparn, NumParn, CurrentDefType, ex_identf_idx, FparamIdx);
	} else if(*(TTypes + ex_tk_idx) == TTYchar){
		CurrentDefType = Symbol8;
		return extern_defs_link(ex_tk_idx + 1, ex_tk_num - 1,
			NumCparn, NumParn, CurrentDefType, ex_identf_idx, FparamIdx);
	} else if(**(Tokens + ex_tk_idx) == '*'){
		if(CurrentDefType == Symbol8){
			CurrentDefType = Ptr8;
		} else{
			CurrentDefType = Ptr32;
		}
		return extern_defs_link(ex_tk_idx + 1, ex_tk_num - 1, NumCparn, NumParn, CurrentDefType, ex_identf_idx, FparamIdx);
	} else if(*(TTypes + ex_tk_idx) == TTYid){
		if(NumParn == 0 && NumCparn == 0){
			FparamIdx = 0;
			ex_identf_idx = ex_identf_idx + 1;
			*(SymbolAddrs + ex_tk_idx) = ex_identf_idx;
			*(TTypes + ex_tk_idx) = CurrentDefType;
			if(**(Tokens + ex_tk_idx + 1) == '='){
				*(Stack + ex_identf_idx) = *(TVals + ex_tk_idx + 2);
			} else if(**(Tokens + ex_tk_idx + 1) == '('){
				*(TTypes + ex_tk_idx) = SymbolFunc;
				*(Stack + ex_identf_idx) = -1;
			}
		} else if(NumParn == 1 && NumCparn == 0){
			FparamIdx = FparamIdx - 1;
			*(SymbolAddrs + ex_tk_idx) = FparamIdx;
			*(TTypes + ex_tk_idx) = CurrentDefType;
		} else{
			defs_link(ex_tk_idx, ex_tk_idx - 1);
		}
	} else{
	}
	return extern_defs_link(ex_tk_idx + 1, ex_tk_num - 1, NumCparn, NumParn, CurrentDefType, ex_identf_idx, FparamIdx);
}

int main(int argc, char** argv){
	if(argc < 2)return(9);
	MemSize = 10000 * 10;
	SrcFp = fopen(*(argv + 1), "r");
	Src = malloc(MemSize);
	SrcLen = fread(Src, 1, MemSize, SrcFp);
	*(Src + SrcLen) = 0;
	//All Tokens
	Tokens = malloc(MemSize);
	//Token Type
	TTypes = malloc(MemSize);
	//Token values: numeric for int/char, ptr for "", length for ID
	TVals = malloc(MemSize);
	//Identifier address relative to Stack
	SymbolAddrs = malloc(MemSize);
	NumTokens = token_scan(Src, 0);
	Stack = malloc(MemSize);
	Top = 0;
	Frame = 0;
	*(Stack + Top) = argc - 2;
	Top = Top + 1;
	*(Stack + Top) = (int)(argv + 2);
	Top = Top + 1;
	// Mark keywords in TTypes
	keywords_scan(NumTokens);
	//Mark function entrance in Stack[SymbolAddrs], or variable offset in SymbolAddrs
	extern_defs_link(0, NumTokens, 0, 0, 0, Top, 0);
	return print_tokens(0, NumTokens);
}
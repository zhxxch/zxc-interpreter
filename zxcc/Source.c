#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<io.h>
#include<fcntl.h>
int StackSize;
int SymbolsMemSz;
int SrcFp;
char *Src;
int SrcLen = 0;
char** Symbols;
char** STypes;
int* SVals;
int* SymbolAddrs;
int NumSymbols = 0;
int *Stack;
int Top = 0, Frame = 0;
int PC = 0;
char *ExprType = 0;
char
*S_int = "int",
*S_char = "char",
*S_if = "if",
*S_else = "else",
*S_return = "return",
*S_printf = "printf",
*S_memcmp = "memcmp",
*S_malloc = "malloc",
*S_memchr = "memchr",
*S_memcpy = "memcpy",
*S_open = "_open",
*S_read = "_read",
*S_exit = "exit",
*S_digit = "0123456789",
*S_id1st = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
*S_id = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
*S_op = "!&%*+-/<=>^|~",
*S_op2 = "=&|",
*S_punct = "(){},;",
*Symbol32 = "_V_FV32",
*Symbol8 = "_v_Fv8",
*SymbolPtr32 = "_P_FP32",
*SymbolPtr8 = "_p_Fp8",
*Lval8addr = "*Ref8",
*Lval32addr = "*Ref32";
int SymFunc = 2;
int CommaExp = 0, AssnExp = 1, LorExp = 2, LandExp = 3,
BorExp = 4, BxorExp = 5, BandExp = 6, EqExp = 7, RelExp = 8,
AddExp = 9, MulExp = 10, CastExp = 11, UnaryExp = 12, CallExp = 13, PrimExp = 14;
int Ptr8Exp = 15, Ptr32Exp = 16, IntExp = 17, Int32Exp = 18, CharExp = 19;

char* skip_line(char *src){
	if(*src == '\n')return src + 1;
	return skip_line(src + 1);
}
char* atoint(char *src, int *val){
	if(memchr(S_digit, *src, 10)){
		*val = (*val) * 10 + *src - '0';
		return atoint(src + 1, val);
	} else return src;
}
char* char_literal_scan(char *src, int *val){
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
char* str_literal_scan(char *src_in, char *src_out){
	if(*src_in == '\\'){
		if(*(src_in + 1) == 'n'){
			*src_out = '\n';
		} else{
			*src_out = *(src_in + 1);
		}
		return str_literal_scan(src_in + 2, src_out + 1);
	} else if(*src_in == '"'){
		return src_in + 1;
	} else{
		*src_in = *src_out;
		return str_literal_scan(src_in + 1, src_out + 1);
	}
}
char* identifier_scan(char *src, int *len){
	if(memchr(S_id, *src, 64)){
		*len = *len + 1;
		return(identifier_scan(src + 1, len));
	}
	return src;
}

int SymbolsScan(char *src, int SymCtr){
	*(Symbols + SymCtr) = src;
	*(STypes + SymCtr) = 0;
	*(SVals + SymCtr) = 0;
	if(!(*src))return SymCtr;
	if(*src == '#')return SymbolsScan(skip_line(src), SymCtr);
	else if(*src == '/' && *(src + 1) == '/'){
		return SymbolsScan(skip_line(src), SymCtr);
	} else if(memchr(S_digit, *src, 10)){
		*(STypes + SymCtr) = S_digit;
		return SymbolsScan(atoint(src, (SVals + SymCtr)), SymCtr + 1);
	} else if(*src == '\''){
		*(STypes + SymCtr) = src;
		return SymbolsScan(char_literal_scan(src, (SVals + SymCtr)), SymCtr + 1);
	} else if(*src == '"'){
		*(STypes + SymCtr) = src;
		*(SVals + SymCtr) = (int)(src + 1);
		return SymbolsScan(str_literal_scan(src + 1, src + 1), SymCtr + 1);
	} else if(memchr(S_id1st, *src, 54)){
		*(STypes + SymCtr) = S_id;
		*(SVals + SymCtr) = 0;
		return SymbolsScan(identifier_scan(src, (SVals + SymCtr)), SymCtr + 1);
	} else if(memchr(S_op, *src, 14)){
		if(memchr(S_op2, *(src + 1), 4)){
			*(STypes + SymCtr) = S_op2;
			*(SVals + SymCtr) = 2;
			return SymbolsScan(src + 2, SymCtr + 1);
		}
		*(STypes + SymCtr) = S_op;
		*(SVals + SymCtr) = 1;
		return SymbolsScan(src + 1, SymCtr + 1);
	} else if(memchr(S_punct, *src, 9)){
		*(STypes + SymCtr) = S_punct;
		*(SVals + SymCtr) = 1;
		return SymbolsScan(src + 1, SymCtr + 1);
	} else{
		return SymbolsScan(src + 1, SymCtr);
	}
}
char* keywords_type(char* kw_ptr, int kw_len){
	if(!memcmp(S_int, kw_ptr, kw_len) && kw_len == 3){
		return S_int;
	} else if(!memcmp(S_char, kw_ptr, kw_len) && kw_len == 4){
		return S_char;
	} else if(!memcmp(S_if, kw_ptr, kw_len) && kw_len == 2){
		return S_if;
	} else if(!memcmp(S_else, kw_ptr, kw_len) && kw_len == 4){
		return S_else;
	} else if(!memcmp(S_return, kw_ptr, kw_len) && kw_len == 6){
		return S_return;
	} else if(!memcmp(S_printf, kw_ptr, kw_len) && kw_len == 6){
		return S_memcmp;
	} else if(!memcmp(S_memcmp, kw_ptr, kw_len) && kw_len == 6){
		return S_memcmp;
	} else if(!memcmp(S_malloc, kw_ptr, kw_len) && kw_len == 6){
		return S_malloc;
	} else if(!memcmp(S_memchr, kw_ptr, kw_len) && kw_len == 6){
		return S_memchr;
	} else if(!memcmp(S_memcpy, kw_ptr, kw_len) && kw_len == 6){
		return S_memcpy;
	} else if(!memcmp(S_open, kw_ptr, kw_len) && kw_len == 5){
		return S_open;
	} else if(!memcmp(S_read, kw_ptr, kw_len) && kw_len == 5){
		return S_read;
	} else return S_id;
}
int keywords_scan(int kw_scan_i){
	if(kw_scan_i < 0)return 0;
	if(*(STypes + kw_scan_i) == S_id){
		*(STypes + kw_scan_i) = keywords_type(*(Symbols + kw_scan_i), *(SVals + kw_scan_i));
	}
	return keywords_scan(kw_scan_i - 1);
}

int print_symbols(int symbol_id_iter, int num_symbols){
	if(symbol_id_iter == num_symbols)return 0;
	printf("%c\t%c%c%c\t%i\t%i\t%i\n", **(Symbols + symbol_id_iter), **(STypes + symbol_id_iter),
		*(*(STypes + symbol_id_iter) + 1), *(*(STypes + symbol_id_iter) + 2),
		*(SVals + symbol_id_iter), *(SymbolAddrs + symbol_id_iter),
		*(Stack + *(SymbolAddrs + symbol_id_iter)));
	return print_symbols(symbol_id_iter + 1, num_symbols);
}
//Forward link identifiers
int object_link(int sym_meet_idx, int sym_pre_iter){
	if(sym_pre_iter == 0){
		return 0;
	} else if(*(*(STypes + sym_pre_iter)) == '_'
		&& *(SVals + sym_meet_idx) == *(SVals + sym_pre_iter)
		&& !memcmp(*(Symbols + sym_meet_idx),
			*(Symbols + sym_pre_iter), *(SVals + sym_meet_idx))){
		//Link to same identifier preceding 
		*(STypes + sym_meet_idx) = *(STypes + sym_pre_iter);
		*(SymbolAddrs + sym_meet_idx) = *(SymbolAddrs + sym_pre_iter);
		return sym_pre_iter;
	}//Backward search symbol the same identifier
	return object_link(sym_meet_idx, sym_pre_iter - 1);
}

int func_link(int fsym_idx, int fentrance_idx, int sym_pre_iter){
	if(sym_pre_iter == 0){
		return 0;
	} else if(fsym_idx < 0 && *(SymbolAddrs + sym_pre_iter) == fentrance_idx){
		fsym_idx = sym_pre_iter;
	} else if(*(*(STypes + sym_pre_iter) + SymFunc + 1) == 'F'
		&& *(SVals + fsym_idx) == *(SVals + sym_pre_iter)
		&& !memcmp(*(Symbols + fsym_idx),
			*(Symbols + sym_pre_iter), *(SVals + fsym_idx))){
		//Link same identifier preceding with current
		*(SymbolAddrs + sym_pre_iter) = fentrance_idx;
	}//Backward link function identifiers
	return func_link(fsym_idx, fentrance_idx, sym_pre_iter - 1);
}
int extern_defs_link(int sym_post_iter, int sym_counter, int CurlyCtr,
	int ParenCtr, char* MetObjType, int ObjCtr, int FuncArgCtr){
	*(SymbolAddrs + sym_post_iter) = 0;
	if(sym_counter == 0){
		return 0;
	} else if(**(Symbols + sym_post_iter) == '{'){
		if(CurlyCtr == 0 && **(Symbols + sym_post_iter - 1) == ')'){
			*(Stack + ObjCtr) = sym_post_iter;
			func_link(-1, ObjCtr, sym_post_iter - 1);
		}
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr + 1, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	} else if(**(Symbols + sym_post_iter) == '}'){
		//XXX: NumCparn = NumCparn - 1;
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr - 1, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	} else if(**(Symbols + sym_post_iter) == '('){
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr + 1, MetObjType, ObjCtr, FuncArgCtr);
	} else if(**(Symbols + sym_post_iter) == ')'){
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr - 1, MetObjType, ObjCtr, FuncArgCtr);
	} else if(*(STypes + sym_post_iter) == S_int){
		MetObjType = Symbol32;
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	} else if(*(STypes + sym_post_iter) == S_char){
		MetObjType = Symbol8;
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	} else if(**(Symbols + sym_post_iter) == '*'){
		if(MetObjType == Symbol8){
			MetObjType = SymbolPtr8;
		} else{
			MetObjType = SymbolPtr32;
		}
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	} else if(*(STypes + sym_post_iter) == S_id){
		if(ParenCtr == 0 && CurlyCtr == 0){
			FuncArgCtr = 0;
			ObjCtr = ObjCtr + 1;
			*(SymbolAddrs + sym_post_iter) = ObjCtr;
			*(STypes + sym_post_iter) = MetObjType;
			if(**(Symbols + sym_post_iter + 1) == '='){
				*(Stack + ObjCtr) = *(SVals + sym_post_iter + 2);
			} else if(**(Symbols + sym_post_iter + 1) == '('){
				*(STypes + sym_post_iter) = MetObjType + SymFunc;
				*(Stack + ObjCtr) = -1;
			}
		} else if(ParenCtr == 1 && CurlyCtr == 0){
			FuncArgCtr = FuncArgCtr - 1;
			*(SymbolAddrs + sym_post_iter) = FuncArgCtr;
			*(STypes + sym_post_iter) = MetObjType;
		} else{
			object_link(sym_post_iter, sym_post_iter - 1);
		}
	} else{
	}
	return extern_defs_link(sym_post_iter + 1, sym_counter - 1, CurlyCtr, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
}
int main_entrance(int sym_pre_iter){
	if(sym_pre_iter == 0){
		return 0;
	} else if(*(*(STypes + sym_pre_iter) + SymFunc + 1) == 'F'
		&& *(SVals + sym_pre_iter) == 4
		&& !memcmp(*(Symbols + sym_pre_iter), "main", 4)){
		return sym_pre_iter;
	}
	return main_entrance(sym_pre_iter - 1);
}
int assign_expr(int A);
int prim_expr(int A){
	//FIXME: is_assign
	if(*(STypes + PC) == S_char || *(STypes + PC) == S_digit){
		//constant
		A = *(SVals + PC);
		ExprType = Symbol32;
		PC = PC + 1;
	} else if(*(*(STypes + PC)) == '_'){
		//identifier
		if(*(SymbolAddrs + PC) < 0){
			//Function args
			A = *(Stack + Top + *(SymbolAddrs + PC));
		} else{
			//Globals
			A = *(Stack + *(SymbolAddrs + PC));
		}
		ExprType = *(STypes + PC);
		PC = PC + 1;
	} else if(*(*(STypes + PC)) == '"'){
		//string-literal
		A = (int)*(Symbols + PC);
		ExprType = SymbolPtr8;
		PC = PC + 1;
	} else if(*(*(Symbols + PC)) == '('){
		//(expression)
		PC = PC + 1;
		A = assign_expr(A);
		if(*(*(Symbols + PC)) == ')'){
			PC = PC + 1;
		} else{
			//rparen expected
			ExprType = 0;
		}
	} else{
		//function-like-keyword
		//or error
		ExprType = 0;
	}

	return A;
}
int argument_expr_list(int A, int num_args){
	A = assign_expr(A);
	if(ExprType == 0){
		//()-expr
		return num_args;
	}
	*(Stack + Top) = A;
	Top = Top + 1;
	if(*(*(Symbols + PC)) == ','){
		PC = PC + 1;
		return argument_expr_list(A, num_args + 1);
	} else return num_args;
}
int call_func(int A_func_addr, int num_args, int ret_addr, char *ret_type){
	//TODO: tail call; intrinsic function calls
	PC = A_func_addr + 1;
	A_func_addr = compound_statement(ret_addr);
	PC = ret_addr;
	ExprType = ret_type;
	Top = Top - num_args;
	return A_func_addr;
}
int postfix_expression(int A, int num_args, char *func_ret_type){
	//FIXME: incorrect, see 6.5.2
	//TODO: is_assign
	//function calls
	A = prim_expr(A);
	if(ExprType == 0){
		//()-expr
		return A;
	} else if(*(*(Symbols + PC)) == '('){
		PC = PC + 1;
		func_ret_type = ExprType;
		num_args = argument_expr_list(A, 0);
		if(*(*(Symbols + PC)) == ')'){
			return call_func(A, num_args, PC + 1, func_ret_type);
		} else{
			return 0;
			//error
		}
	} else{
		return A;
		//No func call
	}
}
char *get_cast_type(){
	if(**(Symbols + PC) == ')'){
		return ExprType;
	} else if(**(Symbols + PC) == '*'){
		if(ExprType == Symbol8){
			ExprType = SymbolPtr8;
		} else{
			ExprType = SymbolPtr32;
		}
	}
	PC = PC + 1;
	return get_cast_type();
}
int unary_cast_expr(char *cast_to_type, int A, int is_assign){
	//TODO: complete u-c-expr
	if(**(Symbols + PC) == '('){
		if(*(STypes + PC + 1) == S_char){
			PC = PC + 1;
			cast_to_type = get_cast_type(S_char);
			if(**(Symbols + PC) == ')'){
				PC = PC + 1;
				A = unary_cast_expr(0, 0, 0);
				ExprType = cast_to_type;
				return A;
			} else{
				//error
			}
		} else if(*(STypes + PC + 1) == S_int){
			PC = PC + 1;
			cast_to_type = get_cast_type(S_int);
			if(**(Symbols + PC) == ')'){
				PC = PC + 1;
				A = unary_cast_expr(0, 0, 0);
				ExprType = cast_to_type;
				return A;
			} else{
				//error
			}
		} else{
			//not cast-expr
		}
	} else if(**(Symbols + PC) == '!'){
		PC = PC + 1;
		ExprType = Symbol32;
		return !unary_cast_expr(0, 0, 0);
	} else if(**(Symbols + PC) == '&'){
		//TODO
		PC = PC + 1;
		if(**(STypes + PC) == '_'){
		}
	} else if(**(Symbols + PC) == '*'){
		PC = PC + 1;
		A = unary_cast_expr(0, 0, 0);
		if(ExprType == SymbolPtr8){
			if(is_assign){
				ExprType = Lval8addr;
			} else{
				A = *(char*)A;
				ExprType = Symbol8;
			}
		} else if(ExprType == SymbolPtr32){
			if(is_assign){
				ExprType = Lval32addr;
			} else{
				A = *(int*)A;
				ExprType = Symbol32;
			}
		} else if(ExprType != 0){
			if(is_assign){
				ExprType = Lval8addr;
			} else{
				ExprType = Symbol8;
				A = *(int*)A;
			}
		} else{
			//error
		}
		return A;
	} else {//postfix-expr

	}
	return postfix_expression(A, 0, 0);
}
int assign_expr(int A){
	return A;
}
int statement(int A);
int compound_statement(int A){
	//TODO: not tested, consult 6.8
	A = statement(A);
	if(**(Symbols + PC) == '}'){
		PC = PC + 1;
		return 0;
	} else{
		return compound_statement(A);
	}
}
int expr_statement(int A){
	A = assign_expr(A);
	if(**(Symbols + PC) == ';'){
		PC = PC + 1;
	} else{
		//error
	}
}
int statement(int A){
	//TODO: expr-statement, selection-statement
	if(*(STypes + PC) == S_return){
		//jump-statement
		PC = PC + 1;
		return expr_statement(A);
	} else if(**(Symbols + PC) == '{'){
		PC = PC + 1;
		return compound_statement(A);
	} else if(*(STypes + PC) == S_if){
		PC = PC + 1;
		if(**(Symbols + PC) == '('){
			PC = PC + 1;
			A = assign_expr(0);
		} else{
			//error
		}
		if(**(Symbols + PC) == ')'){
			PC = PC + 1;
		} else{
			//error
		}
		//TODO: for if
		if(A){
			return statement(A);
		} else if(*(STypes + PC) == S_else){
			PC = PC + 1;
			return statement(A);
		}
	} else {
		return expr_statement(A);
	}
}
int main(int argc, char** argv){
	if(argc < 2)return(9);
	StackSize = 1024 * 1024 * 1;
	SymbolsMemSz = 1024 * 1024;
	//_O_RDWR|_O_TEXT
	SrcFp = _open(*(argv + 1), 16386);
	Src = malloc(StackSize);
	SrcLen = _read(SrcFp, Src, StackSize);
	*(Src + SrcLen) = 0;
	//All Tokens
	Symbols = malloc(StackSize);
	//Token Type
	STypes = malloc(StackSize);
	//Token values: numeric for int/char, ptr for "", length for ID
	SVals = malloc(StackSize);
	//Identifier address relative to Stack
	SymbolAddrs = malloc(StackSize);
	NumSymbols = SymbolsScan(Src, 0);
	Stack = malloc(StackSize);
	Top = 0;
	Frame = 0;
	*(Stack + Top) = argc - 2;
	Top = Top + 1;
	*(Stack + Top) = (int)(argv + 2);
	Top = Top + 1;
	// Mark keywords in TTypes
	keywords_scan(NumSymbols);
	//Mark function entrance in Stack[SymbolAddrs], or variable offset in SymbolAddrs
	extern_defs_link(0, NumSymbols, 0, 0, 0, Top, 0);
	return print_symbols(0, NumSymbols);
}
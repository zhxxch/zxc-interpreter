#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<io.h>
int StackSize, SymbolsMemSz, SrcFp,SrcLen = 0,NumSymbols = 0, SymFunc = 2;
char *Src;
char** Symbols, **STypes;
int* SVals, *SymbolAddrs;
int *Stack, *Top, *Frame;
int PC = 0, Returning = 0;
char *ExprType = 0;
char
*S_int = "int....KEYWORD",
*S_char = "char...KEYWORD",
*S_if = "if.....KEYWORD",
*S_else = "else...KEYWORD",
*S_return = "return.BUILTIN",
*S_printf = "printf.BUILTIN",
*S_memcmp = "memcmp.BUILTIN",
*S_malloc = "malloc.BUILTIN",
*S_memchr = "memchr.BUILTIN",
*S_memmove = "memmove.BUILTIN",
*S_open = "_open..BUILTIN",
*S_read = "_read..BUILTIN",
*S_exit = "exit...BUILTIN",
*S_digit = "0123456789",
*S_id1st = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
*S_id = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
*S_op = "<=>|&+-*/!",
*S_op2 = "=&|",
*S_punct = "(){},;",
*S_TailCall = "))))",
*Symbol32 = "&V&FV32",
*Symbol8 = "&v&Fv8",
*SymbolPtr32 = "&P&FP32",
*SymbolPtr8 = "&p&Fp8",
*Lval8addr = "*Ref8",
*Lval32addr = "*Ref32";

char* skip_line(char *src){
	if(*src == '\n'){ return src + 1; }
	return skip_line(src + 1);
}
int skip_interparen(int sym_iter, int paren_ctr){
	if(**(Symbols + sym_iter) == '('){
		return skip_interparen(sym_iter + 1, paren_ctr + 1);
	}
	if(**(Symbols + sym_iter) == ')'){
		//Note: initial parenth NOT SEEN.
		if(paren_ctr == 0){ return sym_iter + 1; }
		return skip_interparen(sym_iter + 1, paren_ctr - 1);
	}
	return skip_interparen(sym_iter + 1, paren_ctr);
}
int skip_intercurly(int sym_iter, int curly_ctr){
	if(**(Symbols + sym_iter) == '{'){
		return skip_intercurly(sym_iter + 1, curly_ctr + 1);
	}
	if(**(Symbols + sym_iter) == '}'){
		//Note: initial curly NOT SEEN.
		if(curly_ctr == 0){ return sym_iter + 1; }
		return skip_intercurly(sym_iter + 1, curly_ctr - 1);
	}
	return skip_intercurly(sym_iter + 1, curly_ctr);
}
char* atoint(char *src, int *val){
	if(memchr(S_digit, *src, 10)){
		*val = (*val) * 10 + *src - '0';
		return atoint(src + 1, val);
	}
	return src;
}
char* char_literal_scan(char *src, int *val){
	if(*(src + 1) == '\\'){
		if(*(src + 2) == 'n'){ *val = '\n'; }
		if(*(src + 2) == '\\'){ *val = '\\'; }
		if(*(src + 2) == '\''){ *val = '\''; }
		return src + 4;
	}
	*val = *(src + 1);
	return src + 3;
}
char* str_literal_scan(char *src_in, char *src_out){
	if(*src_in == '\\'){
		if(*(src_in + 1) == 'n'){ *src_out = '\n'; } else{ *src_out = *(src_in + 1); }
		return str_literal_scan(src_in + 2, src_out + 1);
	}
	if(*src_in == '"'){ return src_in + 1; }
	*src_in = *src_out;
	return str_literal_scan(src_in + 1, src_out + 1);
}
char* identifier_scan(char *src, int *len){
	if(memchr(S_id, *src, 64)){
		*len = *len + 1;
		return identifier_scan(src + 1, len);
	}
	return src;
}
int SymbolsScan(char *src, int SymCtr){
	*(Symbols + SymCtr) = src;
	*(STypes + SymCtr) = 0;
	*(SVals + SymCtr) = 0;
	if(!(*src)){ return SymCtr; }
	if(*src == '#'){ return SymbolsScan(skip_line(src), SymCtr); }
	if(*src == '/' && *(src + 1) == '/'){
		return SymbolsScan(skip_line(src), SymCtr);
	}
	if(memchr(S_digit, *src, 10)){
		*(STypes + SymCtr) = S_digit;
		return SymbolsScan(atoint(src, (SVals + SymCtr)), SymCtr + 1);
	}
	if(*src == '\''){
		*(STypes + SymCtr) = src;
		return SymbolsScan(char_literal_scan(src, (SVals + SymCtr)), SymCtr + 1);
	}
	if(*src == '"'){
		*(STypes + SymCtr) = src;
		*(SVals + SymCtr) = (int)(src + 1);
		return SymbolsScan(str_literal_scan(src + 1, src + 1), SymCtr + 1);
	}
	if(memchr(S_id1st, *src, 54)){
		*(STypes + SymCtr) = S_id;
		*(SVals + SymCtr) = 0;
		return SymbolsScan(identifier_scan(src, (SVals + SymCtr)), SymCtr + 1);
	}
	if(memchr(S_op, *src, 14)){
		if(memchr(S_op2, *(src + 1), 4)){
			*(STypes + SymCtr) = S_op2;
			*(SVals + SymCtr) = 2;
			return SymbolsScan(src + 2, SymCtr + 1);
		}
		*(STypes + SymCtr) = S_op;
		*(SVals + SymCtr) = 1;
		return SymbolsScan(src + 1, SymCtr + 1);
	}
	if(memchr(S_punct, *src, 9)){
		*(STypes + SymCtr) = S_punct;
		*(SVals + SymCtr) = 1;
		return SymbolsScan(src + 1, SymCtr + 1);
	} else{ return SymbolsScan(src + 1, SymCtr); }
}
char* keywords_type(char* kw_ptr, int kw_len){
	if(!memcmp(S_int, kw_ptr, kw_len)
		&& kw_len == 3){ return S_int; }
	if(!memcmp(S_char, kw_ptr, kw_len)
		&& kw_len == 4){ return S_char; }
	if(!memcmp(S_if, kw_ptr, kw_len)
		&& kw_len == 2){ return S_if; }
	if(!memcmp(S_else, kw_ptr, kw_len)
		&& kw_len == 4){ return S_else; }
	if(!memcmp(S_return, kw_ptr, kw_len)
		&& kw_len == 6){ return S_return; }
	if(!memcmp(S_printf, kw_ptr, kw_len)
		&& kw_len == 6){ return S_memcmp; }
	if(!memcmp(S_memcmp, kw_ptr, kw_len)
		&& kw_len == 6){ return S_memcmp; }
	if(!memcmp(S_malloc, kw_ptr, kw_len)
		&& kw_len == 6){ return S_malloc; }
	if(!memcmp(S_memchr, kw_ptr, kw_len)
		&& kw_len == 6){ return S_memchr; }
	if(!memcmp(S_memmove, kw_ptr, kw_len)
		&& kw_len == 7){ return S_memmove; }
	if(!memcmp(S_open, kw_ptr, kw_len)
		&& kw_len == 5){ return S_open; }
	if(!memcmp(S_read, kw_ptr, kw_len)
		&& kw_len == 5){ return S_read; }
	if(!memcmp(S_exit, kw_ptr, kw_len)
		&& kw_len == 4){ return S_exit; }
	return S_id;
}
int keywords_scan(int kw_scan_i){
	if(kw_scan_i < 0){ return 0; }
	if(*(STypes + kw_scan_i) == S_id){
		*(STypes + kw_scan_i) = keywords_type(*(Symbols + kw_scan_i), *(SVals + kw_scan_i));
	}
	return keywords_scan(kw_scan_i - 1);
}
//NOT FOR COMPILE
int print_symbols(int symbol_id_iter, int num_symbols){
	if(symbol_id_iter == num_symbols){ return 0; }
	printf("%c\t%c%c%c\t%i\t%i\t%i\n", **(Symbols + symbol_id_iter), **(STypes + symbol_id_iter),
		*(*(STypes + symbol_id_iter) + 1), *(*(STypes + symbol_id_iter) + 2),
		*(SVals + symbol_id_iter), *(SymbolAddrs + symbol_id_iter),
		*(Stack + *(SymbolAddrs + symbol_id_iter)));
	return print_symbols(symbol_id_iter + 1, num_symbols);
}
//Forward link identifiers
int object_link(int sym_meet_idx, int sym_pre_iter){
	if(sym_pre_iter == 0){ return 0; }
	if(*(*(STypes + sym_pre_iter)) == '&'
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
	if(sym_pre_iter == 0){ return 0; }
	if(fsym_idx < 0 && *(SymbolAddrs + sym_pre_iter) == fentrance_idx){
		fsym_idx = sym_pre_iter;
		return func_link(fsym_idx, fentrance_idx, sym_pre_iter - 1);
	}
	if(*(*(STypes + sym_pre_iter) + SymFunc + 1) == 'F'
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
	if(sym_counter == 0){ return 0; }
	if(**(Symbols + sym_post_iter) == '{'){
		if(CurlyCtr == 0 && **(Symbols + sym_post_iter - 1) == ')'){
			*(Stack + ObjCtr) = sym_post_iter;
			func_link(-1, ObjCtr, sym_post_iter - 1);
		}
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr + 1, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	}
	if(**(Symbols + sym_post_iter) == '}'){
		//XXX: NumCparn = NumCparn - 1;
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr - 1, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	}
	if(**(Symbols + sym_post_iter) == '('){
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr + 1, MetObjType, ObjCtr, FuncArgCtr);
	}
	if(**(Symbols + sym_post_iter) == ')'){
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr - 1, MetObjType, ObjCtr, FuncArgCtr);
	}
	if(*(STypes + sym_post_iter) == S_int){
		MetObjType = Symbol32;
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	}
	if(*(STypes + sym_post_iter) == S_char){
		MetObjType = Symbol8;
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	}
	if(**(Symbols + sym_post_iter) == '*'){
		if(MetObjType == Symbol8){ MetObjType = SymbolPtr8; } else{ MetObjType = SymbolPtr32; }
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	}
	if(*(STypes + sym_post_iter) == S_id){
		if(ParenCtr == 0 && CurlyCtr == 0){
			FuncArgCtr = 0;
			ObjCtr = ObjCtr + 1;
			*(SymbolAddrs + sym_post_iter) = ObjCtr;
			*(STypes + sym_post_iter) = MetObjType;
			if(**(Symbols + sym_post_iter + 1) == '='){
				*(Stack + ObjCtr) = *(SVals + sym_post_iter + 2);
			} else{
				if(**(Symbols + sym_post_iter + 1) == '('){
					*(STypes + sym_post_iter) = MetObjType + SymFunc;
					*(Stack + ObjCtr) = -1;
				}
			}
		} else{
			if(ParenCtr == 1 && CurlyCtr == 0){
				FuncArgCtr = FuncArgCtr - 1;
				*(SymbolAddrs + sym_post_iter) = FuncArgCtr;
				*(STypes + sym_post_iter) = MetObjType;
			} else{
				object_link(sym_post_iter, sym_post_iter - 1);
			}
		}
	} else{}
	return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
		CurlyCtr, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
}
int main_entrance(int sym_pre_iter){
	if(sym_pre_iter == 0){ return 0; }
	if(*(*(STypes + sym_pre_iter) + SymFunc + 1) == 'F'
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
		return A;
	}
	if(*(*(STypes + PC)) == '&'){
		//identifier, return a reference
		if(*(SymbolAddrs + PC) < 0){
			//Function args
			A = *(Top + *(SymbolAddrs + PC));
		} else{
			//Globals
			A = *(Stack + *(SymbolAddrs + PC));
		}
		ExprType = *(STypes + PC);
		PC = PC + 1;
		return A;
	}
	if(*(*(STypes + PC)) == '"'){
		//string-literal
		A = (int)*(Symbols + PC);
		ExprType = SymbolPtr8;
		PC = PC + 1;
		return A;
	}
	if(*(*(Symbols + PC)) == '('){
		//(expression)
		PC = PC + 1;
		A = assign_expr(A);
		if(*(*(Symbols + PC)) == ')'){ PC = PC + 1; } else{ ExprType = 0; }
		return A;
	} else{
		//function-like-keyword
		//or error
		ExprType = *(STypes + PC);
		return A;
	}
}
int argument_expr_list(int A, int num_args){
	A = assign_expr(A);
	if(ExprType == 0){ return num_args; }//()-expr
	*Top = A;
	Top = Top + 1;
	if(*(*(Symbols + PC)) == ','){
		PC = PC + 1;
		return argument_expr_list(A, num_args + 1);
	}
	return num_args;
}
int builtins_call(char* func_type){
	if(func_type == S_malloc){
		return (int)malloc(*Top);
	}
	if(func_type == S_memchr){
		return (int)memchr((char*)*Top,
			*(Top + 1), *(Top + 2));
	}
	if(func_type == S_memcmp){
		return memcmp((char*)*Top,
			(char*)*(Top + 1), *(Top + 2));
	}
	if(func_type == S_memmove){
		return (int)memmove((char*)*Top,
			(char*)*(Top + 1), *(Top + 2));
	}
	if(func_type == S_printf){
		return printf((char*)*Top, *(Top + 1));
	}
	if(func_type == S_open){
		return _open((char*)*Top, *(Top + 1));
	}
	if(func_type == S_read){
		return _read(*Top, (char*)*(Top + 1), *(Top + 2));
	}
	if(func_type == S_exit){
		exit(*Top);
	}
}
int compound_statement(int A);
int tail_call(int A_func_addr){
	PC = A_func_addr + 1;
	return compound_statement(0);
}
int call_func(int A_func_addr, int *frame, int ret_addr, char *ret_type){
	PC = A_func_addr + 1;
	A_func_addr = compound_statement(0);
	PC = ret_addr;
	ExprType = ret_type;
	Top = frame;
	return A_func_addr;
}
int postfix_expression(int A, int num_args, char *func_type, char *symbol_return){
	//FIXME: incorrect, see 6.5.2
	//TODO: is_assign
	//function calls
	symbol_return = *(STypes + PC - 1);
	A = prim_expr(A);
	if(ExprType == 0){ return A; }//()-expr
	if(*(*(Symbols + PC)) == '('){
		PC = PC + 1;
		func_type = ExprType;
		num_args = argument_expr_list(A, 0);
		if(*(*(Symbols + PC)) == ')'){
			PC = PC + 1;
			if(*(*(Symbols + PC)) == ';' && symbol_return == S_return){
				//tail-call
				memmove(Frame, Top - num_args, num_args * 4);
				Top = Frame + num_args;
				return tail_call(A);
			}
			if(*func_type != '&'){
				Top = Top - num_args;
				return builtins_call(func_type);
			}
			Frame = Top - num_args;
			return call_func(A, Frame, PC, func_type);
		} else{ return 0; }	//error
	} else{ return A; }	//No func call
}
char *get_cast_type(){
	if(**(Symbols + PC) == ')'){ return ExprType; } else{
		if(**(Symbols + PC) == '*'){
			if(ExprType == Symbol8){
				ExprType = SymbolPtr8;
			} else{ ExprType = SymbolPtr32; }
		}
	}
	PC = PC + 1;
	return get_cast_type();
}
int get_object_ref(int A){
	if(*(*(STypes + PC)) == '&'){
		//identifier, return a reference
		if(*(SymbolAddrs + PC) < 0){
			//Function args
			A = (int)(Top + *(SymbolAddrs + PC));
		} else{
			//Globals
			A = (int)(Stack + *(SymbolAddrs + PC));
		}
		ExprType = *(STypes + PC);
		if(ExprType == Symbol8){
			ExprType = SymbolPtr8;
		} else{ ExprType = SymbolPtr32; }
		PC = PC + 1;
	}
	return A;
}
int unary_cast_expr(char *cast_to_type, int A, int req_ref){
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
			} else{}//error
		}
		if(*(STypes + PC + 1) == S_int){
			PC = PC + 1;
			cast_to_type = get_cast_type(S_int);
			if(**(Symbols + PC) == ')'){
				PC = PC + 1;
				A = unary_cast_expr(0, 0, 0);
				ExprType = cast_to_type;
				return A;
			} else{}//error
		} else{}//not cast-expr
	}
	if(**(Symbols + PC) == '!'){
		PC = PC + 1;
		ExprType = Symbol32;
		return !unary_cast_expr(0, 0, 0);
	}
	if(**(Symbols + PC) == '&'){
		PC = PC + 1;
		return get_object_ref(A);
	}
	if(**(Symbols + PC) == '*'){
		PC = PC + 1;
		A = unary_cast_expr(0, 0, 0);
		if(req_ref){ return A; }
		if(ExprType == SymbolPtr8){
			ExprType = Symbol8;
			return *(char*)A;
		}
		if(ExprType == SymbolPtr32){
			ExprType = Symbol32;
			return *(int*)A;
		}
		if(ExprType != 0){
			ExprType = Symbol8;
			return *(char*)A;
		} else{}//error
	}
	if(req_ref){
		A = get_object_ref(A);
	}
	return postfix_expression(A, 0, 0, 0);
}
int mul_expr(int A_cast){
	if(**(Symbols + PC) == '*'){
		PC = PC + 1;
		return mul_expr(A_cast * unary_cast_expr(0, A_cast, 0));
	}
	if(**(Symbols + PC) == '/'){
		PC = PC + 1;
		return mul_expr(A_cast / unary_cast_expr(0, A_cast, 0));
	}
	return A_cast;
}
int add_expr(int A_add, int A_mul){
	//FIXME: Pointers
	if(**(Symbols + PC) == '+'){
		PC = PC + 1;
		if(ExprType == SymbolPtr32){
			//((int*)ptr+1)
			A_mul = mul_expr(unary_cast_expr(0, 0, 0));
			ExprType = SymbolPtr32;
			return add_expr(A_add+4*A_mul, 0);
		}
				if(ExprType == SymbolPtr8){
			//((char*)ptr+1)
			ExprType = SymbolPtr8;
			return add_expr(A_add+A_mul, 0);
		}
		A_mul = mul_expr(unary_cast_expr(0, 0, 0));
		if(ExprType == SymbolPtr32){
			//(1+(int*)ptr)
			ExprType = SymbolPtr32;
			return add_expr(4*A_add+A_mul, 0);
		}
		if(ExprType == SymbolPtr8){
			//(1+(char*)ptr)
			ExprType = SymbolPtr8;
			return add_expr(A_add+A_mul, 0);
		}

		return add_expr(A_add+A_mul, 0);
	}
	if(**(Symbols + PC) == '-'){
		PC = PC + 1;
				PC = PC + 1;
		if(ExprType == SymbolPtr32){
			//(ptr-1)
			A_mul = mul_expr(unary_cast_expr(0, 0, 0));
			ExprType = SymbolPtr32;
			return add_expr(A_add-4*A_mul, 0);
		}
		if(ExprType == SymbolPtr8){
			//((char*)ptr-1)
			ExprType = SymbolPtr8;
			return add_expr(A_add-A_mul, 0);
		}
		A_mul = mul_expr(unary_cast_expr(0, 0, 0));
		if(ExprType == SymbolPtr32){
			//(1-ptr):error
		}
		return add_expr(A_add-A_mul, 0);
	}
	return A_add;
}
int rel_expr(int A_add){
	if(**(Symbols + PC) == '<'){
		PC = PC + 1;
		if(*(STypes + PC) == S_op2){
			return rel_expr(A_add <= add_expr(mul_expr(unary_cast_expr(0, 0, 0)),0));
		} else{
			return rel_expr(A_add < add_expr(mul_expr(unary_cast_expr(0, 0, 0)),0));
		}
	}
	if(**(Symbols + PC) == '>'){
		PC = PC + 1;
		if(*(STypes + PC) == S_op2){
			return rel_expr(A_add >= add_expr(mul_expr(unary_cast_expr(0, 0, 0)),0));
		} else{
			return rel_expr(A_add > add_expr(mul_expr(unary_cast_expr(0, 0, 0)),0));
		}
	}
	if(*(STypes + PC) == S_op2){
		PC = PC + 1;
		if(**(Symbols + PC) == '='){
			return rel_expr(A_add == add_expr(mul_expr(unary_cast_expr(0, 0, 0)),0));
		} if(**(Symbols + PC) == '!'){
			return rel_expr(A_add != add_expr(mul_expr(unary_cast_expr(0, 0, 0)),0));
		}
	}
	return A_add;
}
int land_expr(int A_rel){
	if(**(Symbols + PC) == '&' && *(STypes + PC) == S_op2){
		PC = PC + 1;
		return land_expr(A_rel && rel_expr(add_expr(mul_expr(unary_cast_expr(0, 0, 0)),0)));
	}
	return A_rel;
}
int lor_expr(int A_land){
	if(**(Symbols + PC) == '|' && *(STypes + PC) == S_op2){
		PC = PC + 1;
		return lor(A_land && land_expr(rel_expr(add_expr(mul_expr(unary_cast_expr(0, 0, 0)),0))));
	}
	return A_land;
}
int assign_expr(int A){
	A = unary_cast_expr(0, 0, 1);
	if(**(Symbols + PC) == '='){
		//assign
	} else{
		//no assignment
	}
	return A;
}
int statement(int A);
int compound_statement(int A){
	//TODO: not tested, consult 6.8
	Returning = 0;
	A = statement(A);
	if(Returning == 1){ return A; }
	if(**(Symbols + PC) == '}'){
		PC = PC + 1;
		return A;
	}
	return compound_statement(A);
}
int expr_statement(int A){
	A = assign_expr(A);
	if(**(Symbols + PC) == ';'){ PC = PC + 1; } else{}//error
	return A;
}
int statement(int A){
	//TODO: expr-statement, selection-statement
	if(*(STypes + PC) == S_return){
		//FIXME: incorrect
		PC = PC + 1;
		Returning = 1;
		return expr_statement(A);
	}
	if(**(Symbols + PC) == '{'){
		PC = PC + 1;
		return compound_statement(A);
	}
	if(*(STypes + PC) == S_if){
		PC = PC + 1;
		if(**(Symbols + PC) == '('){
			PC = PC + 1;
			A = assign_expr(0);
		} else{}//error
		if(**(Symbols + PC) == ')'){ PC = PC + 1; } else{}//error
		if(**(Symbols + PC) == '{'){ PC = PC + 1; } else{}//error
		//TODO: for if
		if(A){ return compound_statement(A); } else{ PC = skip_intercurly(PC, 0); }
		if(*(STypes + PC) == S_else){
			PC = PC + 2;//no check {
			return compound_statement(A);
		}
	}
	if(*(STypes + PC) == S_else){
		PC = skip_intercurly(PC + 2, 0);
		return A;
	}
	return expr_statement(A);
}
int main(int argc, char** argv){
	if(argc < 2){ return(9); }
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
	Top = Stack;
	Frame = Stack;
	// Mark keywords in TTypes
	keywords_scan(NumSymbols);
	//Mark function entrance in Stack[SymbolAddrs], or variable offset in SymbolAddrs
	Top = Top + extern_defs_link(0, NumSymbols, 0, 0, 0, 1, 0);
	Frame = Top;
	*Top = argc - 1;
	Top = Top + 1;
	*Top = (int)(argv + 1);
	Top = Top + 1;
	return print_symbols(0, NumSymbols);
	PC = main_entrance(NumSymbols - 1);
	return compound_statement(0);
}

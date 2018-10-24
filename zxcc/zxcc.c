#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int StackSize, SymbolsMemSz, SrcLen = 0, NumSymbols = 0,
SymFunc = 2, PC = 0, TailCalling = 0;
char *Src;
FILE *SrcFp;
char** Symbols;
char** STypes;
int* SVals;
int *SymbolAddrs;
int *Stack;
int *Top;
int *Frame;
int *ObjRef = 0;
char *ExprType = 0;
char *S_int = "int....KEYWORD";
char *S_char = "char...KEYWORD";
char *S_if = "if.....KEYWORD";
char *S_else = "else...KEYWORD";
char *S_file = "FILE...KEYWORD";
char *S_return = "return.BUILTIN";
char *S_printf = "printf.BUILTIN";
char *S_memcmp = "memcmp.BUILTIN";
char *S_malloc = "malloc.BUILTIN";
char *S_memchr = "memchr.BUILTIN";
char *S_memmove = "memmove.BUILTIN";
char *S_fopen = "fopen..BUILTIN";
char *S_fread = "fread..BUILTIN";
char *S_exit = "exit...BUILTIN";
char *S_digit = "0123456789";
char *S_id1st = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
char *S_id = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
char *S_op = "*/+-<=>|&!";
char *S_op2 = "=&|";
char *S_punct = "(){},;";
char*Symbol32 = "$V$FV32";
char*Symbol8 = "$v$Fv8";
char*SymbolPtr32 = "$P$FP32";
char*SymbolPtr8 = "$p$Fp8";

char* skip_line(char *src){
	if(*src == '\n'){ return src + 1; }
	return skip_line(src + 1);
}
int skip_interparen(int sym_iter, int paren_ctr){
	if(**(Symbols + sym_iter) == '('){
		return skip_interparen(sym_iter + 1, paren_ctr + 1);
	}
	if(**(Symbols + sym_iter) == ')'){//Note: open ( not seen
		if(paren_ctr == 0){ return sym_iter + 1; }
		return skip_interparen(sym_iter + 1, paren_ctr - 1);
	}//return when meet expr end
	if(**(Symbols + sym_iter) == ';'){ return sym_iter; }
	return skip_interparen(sym_iter + 1, paren_ctr);
}
int skip_intercurly(int sym_iter, int curly_ctr){
	if(**(Symbols + sym_iter) == '{'){
		return skip_intercurly(sym_iter + 1, curly_ctr + 1);
	}
	if(**(Symbols + sym_iter) == '}'){//Note: open{ not seen
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
		if(*(src_in + 1) == 'n'){
			*src_out = '\n';
		} else{ *src_out = *(src_in + 1); }
		return str_literal_scan(src_in + 2, src_out + 1);
	}
	if(*src_in == '"'){ *src_out = 0; return src_in + 1; }
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
		return SymbolsScan(
			atoint(src, (SVals + SymCtr)), SymCtr + 1);
	}
	if(*src == '\''){
		*(STypes + SymCtr) = S_digit;
		return SymbolsScan(
			char_literal_scan(src, (SVals + SymCtr)), SymCtr + 1);
	}
	if(*src == '"'){
		*(STypes + SymCtr) = src;
		*(SVals + SymCtr) = (int)(src + 1);
		return SymbolsScan(
			str_literal_scan(src + 1, src + 1), SymCtr + 1);
	}
	if(memchr(S_id1st, *src, 54)){
		*(STypes + SymCtr) = S_id;
		*(SVals + SymCtr) = 0;
		return SymbolsScan(
			identifier_scan(src, (SVals + SymCtr)), SymCtr + 1);
	}
	if(memchr(S_op, *src, 10)){
		if(memchr(S_op2, *(src + 1), 3)){
			*(STypes + SymCtr) = S_op2;
			*(SVals + SymCtr) = 2;
			return SymbolsScan(src + 2, SymCtr + 1);
		}
		*(STypes + SymCtr) = S_op;
		*(SVals + SymCtr) = 1;
		return SymbolsScan(src + 1, SymCtr + 1);
	}
	if(memchr(S_punct, *src, 6)){
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
	//Note: FILE as char
	if(!memcmp(S_file, kw_ptr, kw_len)
		&& kw_len == 4){ return S_char; }
	if(!memcmp(S_return, kw_ptr, kw_len)
		&& kw_len == 6){ return S_return; }
	if(!memcmp(S_printf, kw_ptr, kw_len)
		&& kw_len == 6){ return S_printf; }
	if(!memcmp(S_memcmp, kw_ptr, kw_len)
		&& kw_len == 6){ return S_memcmp; }
	if(!memcmp(S_malloc, kw_ptr, kw_len)
		&& kw_len == 6){ return S_malloc; }
	if(!memcmp(S_memchr, kw_ptr, kw_len)
		&& kw_len == 6){ return S_memchr; }
	if(!memcmp(S_memmove, kw_ptr, kw_len)
		&& kw_len == 7){ return S_memmove; }
	if(!memcmp(S_fopen, kw_ptr, kw_len)
		&& kw_len == 5){ return S_fopen; }
	if(!memcmp(S_fread, kw_ptr, kw_len)
		&& kw_len == 5){ return S_fread; }
	if(!memcmp(S_exit, kw_ptr, kw_len)
		&& kw_len == 4){ return S_exit; }
	return S_id;
}
int keywords_scan(int kw_scan_i){
	if(kw_scan_i < 0){ return 0; }
	if(*(STypes + kw_scan_i) == S_id){
		*(STypes + kw_scan_i) = keywords_type(
			*(Symbols + kw_scan_i), *(SVals + kw_scan_i));
	}
	return keywords_scan(kw_scan_i - 1);
}
int object_link(int sym_meet_idx, int sym_pre_iter){
	if(sym_pre_iter == 0){ return 0; }
	if(*(*(STypes + sym_pre_iter)) == '$'
		&& *(SVals + sym_meet_idx) == *(SVals + sym_pre_iter)
		&& !memcmp(*(Symbols + sym_meet_idx),
			*(Symbols + sym_pre_iter), *(SVals + sym_meet_idx))){
		//Link symbols
		*(STypes + sym_meet_idx) = *(STypes + sym_pre_iter);
		*(SymbolAddrs + sym_meet_idx)
			= *(SymbolAddrs + sym_pre_iter);
		return sym_pre_iter;
	}
	return object_link(sym_meet_idx, sym_pre_iter - 1);
}

int func_link(int fsym_idx, int fentrance_idx, int sym_pre_iter){
	if(sym_pre_iter == 0){ return 0; }
	if(fsym_idx < 0 && *(SymbolAddrs + sym_pre_iter)
		== fentrance_idx){
		fsym_idx = sym_pre_iter;
		return func_link(fsym_idx, fentrance_idx, sym_pre_iter - 1);
	}
	if(**(STypes + sym_pre_iter) == '$'
		&& *(SVals + fsym_idx) == *(SVals + sym_pre_iter)
		&& !memcmp(*(Symbols + fsym_idx),
			*(Symbols + sym_pre_iter), *(SVals + fsym_idx))){
		//link function identifier
		*(SymbolAddrs + sym_pre_iter) = fentrance_idx;
	}
	return func_link(fsym_idx, fentrance_idx, sym_pre_iter - 1);
}
int extern_defs_link(int sym_post_iter, int sym_counter,
	int CurlyCtr, int ParenCtr, char* MetObjType,
	int ObjCtr, int FuncArgCtr){
	*(SymbolAddrs + sym_post_iter) = 0;
	if(sym_counter == 0){ return ObjCtr; }
	if(**(Symbols + sym_post_iter) == '{'){
		if(CurlyCtr == 0 && **(Symbols + sym_post_iter - 1) == ')'){
			//function entrance after open {
			*(Stack + ObjCtr) = sym_post_iter + 1;
			func_link(-1, ObjCtr, sym_post_iter - 1);
		}
		return extern_defs_link(sym_post_iter + 1, sym_counter - 1,
			CurlyCtr + 1, ParenCtr, MetObjType, ObjCtr, FuncArgCtr);
	}
	if(**(Symbols + sym_post_iter) == '}'){
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
		if(MetObjType == Symbol8){
			MetObjType = SymbolPtr8;
		} else{ MetObjType = SymbolPtr32; }
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
					*(STypes + sym_post_iter)
						= MetObjType + SymFunc;
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
	if(**(STypes + sym_pre_iter) == '$'
		&& *(SVals + sym_pre_iter) == 4
		&& !memcmp(*(Symbols + sym_pre_iter), "main", 4)){
		return *(Stack + *(SymbolAddrs + sym_pre_iter));
	}
	return main_entrance(sym_pre_iter - 1);
}
int assign_expr(int A, int* obj_ref);
int prim_expr(int A){
	if(*(STypes + PC) == S_digit){
		A = *(SVals + PC);
		ExprType = Symbol32;
		PC = PC + 1;
		return A;
	}
	if(*(*(STypes + PC)) == '$'){
		if(*(SymbolAddrs + PC) < 0){
			A = *(Top + *(SymbolAddrs + PC));
			ObjRef = (Top + *(SymbolAddrs + PC));
		} else{
			A = *(Stack + *(SymbolAddrs + PC));
			ObjRef = (Stack + *(SymbolAddrs + PC));
		}
		ExprType = *(STypes + PC);
		PC = PC + 1;
		return A;
	}
	if(*(*(STypes + PC)) == '"'){
		A = (int)*(SVals + PC);
		ExprType = SymbolPtr8;
		PC = PC + 1;
		return A;
	}
	if(*(*(Symbols + PC)) == '('){//(...)
		PC = PC + 1;
		A = assign_expr(A, 0);
		if(*(*(Symbols + PC)) == ')'){
			PC = PC + 1;
		} else{ ExprType = 0; }
		return A;
	}
	if(*(*(Symbols + PC)) == ')'){//()
		ExprType = 0;
		return A;
	} else{
		ExprType = *(STypes + PC);
		PC = PC + 1;
		return A;
	}
}
int argument_expr_list(int A, int args_ctr){
	A = assign_expr(A, 0);
	if(ExprType == 0){ return args_ctr; }//()-expr
	if(*(*(Symbols + PC)) == ','){ PC = PC + 1; }
	args_ctr = argument_expr_list(A, args_ctr + 1);
	*Top = A;
	Top = Top + 1;
	return args_ctr;
}
int builtins_call(char* func_type){
	if(func_type == S_malloc){
		ExprType = SymbolPtr8;
		return (int)malloc(*Top);
	}
	if(func_type == S_memchr){
		ExprType = SymbolPtr8;
		return (int)memchr((char*)*(Top + 2),
			*(Top + 1), *(Top));
	}
	if(func_type == S_memcmp){
		ExprType = Symbol32;
		return memcmp((char*)*(Top + 2),
			(char*)*(Top + 1), *(Top));
	}
	if(func_type == S_memmove){
		ExprType = SymbolPtr8;
		return (int)memmove((char*)*(Top + 2),
			(char*)*(Top + 1), *(Top));
	}
	if(func_type == S_printf){
		ExprType = Symbol32;
		return printf((char*)*(Top + 1), *Top);
	}
	if(func_type == S_fopen){
		ExprType = SymbolPtr8;
		return (int)fopen((char*)*(Top + 1), (char*)*(Top));
	}
	if(func_type == S_fread){
		ExprType = Symbol32;
		return fread((char*)*(Top + 3),
			*(Top + 2), *(Top + 1), (FILE*)*(Top));
	}
	if(func_type == S_exit){ exit(*Top); }
	return 0;
}
int compound_statement(int A, int curly_ctr);
int tail_call(int A_func_addr){
	TailCalling = 1;
	PC = A_func_addr;
	return 0;
}
int call_func(int A_func_addr, int *frame,
	int ret_addr, char *ret_type){
	PC = A_func_addr;
	TailCalling = 0;
	A_func_addr = compound_statement(0, 0);
	PC = ret_addr;
	ExprType = ret_type;
	Top = frame;
	return A_func_addr;
}
int postfix_expression(int A, int num_args,
	char *func_type, char *symbol_return){
	symbol_return = *(STypes + PC - 1);
	A = prim_expr(A);
	if(ExprType == 0){ return A; }
	if(**(Symbols + PC) == '('){
		PC = PC + 1;
		func_type = ExprType;
		num_args = argument_expr_list(A, 0);
		if(*(*(Symbols + PC)) == ')'){
			PC = PC + 1;
			if(*func_type != '$'){
				Top = Top - num_args;
				A = builtins_call(func_type);
				return A;
			}
			if(symbol_return == S_return
				&& *(*(Symbols + PC)) == ';'){
				//return identifier(...); - tail call
				Top = Top - num_args;
				memmove(Frame, Top, num_args * 4);
				Top = Frame + num_args;
				return tail_call(A);
			}
			Frame = Top - num_args;
			if(*(func_type + 1) == 'F'){
				func_type = func_type - SymFunc;
			}
			return call_func(A, Frame, PC, func_type);
		} else{ return 0; }	//syntax error
	}
	return A;
}
char *get_cast_type(char *obj_type){
	if(**(Symbols + PC) == ')'){ return obj_type; } else{
		if(**(Symbols + PC) == '*'){
			if(obj_type == Symbol8){
				obj_type = SymbolPtr8;
			} else{ obj_type = SymbolPtr32; }
		}
	}
	PC = PC + 1;
	return get_cast_type(obj_type);
}
int get_object_ref(int A){
	if(**(STypes + PC) == '$'){
		if(*(SymbolAddrs + PC) < 0){
			A = (int)(Top + *(SymbolAddrs + PC));
		} else{ A = (int)(Stack + *(SymbolAddrs + PC)); }
		ExprType = *(STypes + PC);
		PC = PC + 1;
	}
	return A;
}
int unary_cast_expr(char *cast_to_type, int A){
	if(**(Symbols + PC) == '(' && *(STypes + PC + 1) == S_char){
		PC = PC + 2;
		cast_to_type = get_cast_type(Symbol8);
		if(**(Symbols + PC) == ')'){
			PC = PC + 1;
			A = unary_cast_expr(0, 0);
			ExprType = cast_to_type;
			return A;
		} else{}//syntax error
	}
	if(**(Symbols + PC) == '(' && *(STypes + PC + 1) == S_int){
		PC = PC + 2;
		cast_to_type = get_cast_type(Symbol32);
		if(**(Symbols + PC) == ')'){
			PC = PC + 1;
			A = unary_cast_expr(0, 0);
			ExprType = cast_to_type;
			return A;
		} else{}//error
	}
	if(**(Symbols + PC) == '!'){
		PC = PC + 1;
		A = !unary_cast_expr(0, 0);
		ExprType = Symbol32;
		return A;
	}
	if(**(Symbols + PC) == '-'){
		PC = PC + 1;
		A = -unary_cast_expr(0, 0);
		ExprType = Symbol32;
		return A;
	}
	if(**(Symbols + PC) == '&'){
		PC = PC + 1;
		A = unary_cast_expr(0, 0);
		A = (int)ObjRef;
		if(ExprType == Symbol8){
			ExprType = SymbolPtr8;
		} else{ ExprType = SymbolPtr32; }
		return A;
	}
	if(**(Symbols + PC) == '*'){
		PC = PC + 1;
		A = unary_cast_expr(0, 0);
		ObjRef = (int*)A;
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
	return postfix_expression(A, 0, 0, 0);
}
int mul_expr(int A_cast){
	if(**(Symbols + PC) == '*'){
		PC = PC + 1;
		return mul_expr(A_cast * unary_cast_expr(0, A_cast));
	}
	if(**(Symbols + PC) == '/'){
		PC = PC + 1;
		return mul_expr(A_cast / unary_cast_expr(0, A_cast));
	}
	return A_cast;
}
int add_expr(int A_add, int A_mul){
	if(**(Symbols + PC) == '+'){
		PC = PC + 1;
		if(ExprType == SymbolPtr32){//((int*)ptr+1)
			A_mul = mul_expr(unary_cast_expr(0, 0));
			ExprType = SymbolPtr32;
			return add_expr(A_add + 4 * A_mul, 0);
		}
		if(ExprType == SymbolPtr8){//((char*)ptr+1)
			A_mul = mul_expr(unary_cast_expr(0, 0));
			ExprType = SymbolPtr8;
			return add_expr(A_add + A_mul, 0);
		}
		A_mul = mul_expr(unary_cast_expr(0, 0));
		if(ExprType == SymbolPtr32){//(1+(int*)ptr)
			ExprType = SymbolPtr32;
			return add_expr(4 * A_add + A_mul, 0);
		}
		if(ExprType == SymbolPtr8){//(1+(char*)ptr)
			ExprType = SymbolPtr8;
			return add_expr(A_add + A_mul, 0);
		}
		return add_expr(A_add + A_mul, 0);
	}
	if(**(Symbols + PC) == '-'){
		PC = PC + 1;
		if(ExprType == SymbolPtr32){//(ptr-1)
			A_mul = mul_expr(unary_cast_expr(0, 0));
			ExprType = SymbolPtr32;
			return add_expr(A_add - 4 * A_mul, 0);
		}
		if(ExprType == SymbolPtr8){//((char*)ptr-1)
			A_mul = mul_expr(unary_cast_expr(0, 0));
			ExprType = SymbolPtr8;
			return add_expr(A_add - A_mul, 0);
		}
		A_mul = mul_expr(unary_cast_expr(0, 0));
		if(ExprType == SymbolPtr32 || ExprType == SymbolPtr8){}//(1-ptr)
		return add_expr(A_add - A_mul, 0);
	}
	return A_add;
}
int rel_expr(int A_add){//NON-STANDARD(6.5.9): <=, == same priority
	if(**(Symbols + PC) == '<'){
		if(*(STypes + PC) == S_op2){
			PC = PC + 1;
			return rel_expr(A_add <= add_expr(mul_expr(unary_cast_expr(0, 0)), 0));
		} else{
			PC = PC + 1;
			return rel_expr(A_add < add_expr(mul_expr(unary_cast_expr(0, 0)), 0));
		}
	}
	if(**(Symbols + PC) == '>'){
		if(*(STypes + PC) == S_op2){
			PC = PC + 1;
			return rel_expr(A_add >= add_expr(mul_expr(unary_cast_expr(0, 0)), 0));
		} else{
			PC = PC + 1;
			return rel_expr(A_add > add_expr(mul_expr(unary_cast_expr(0, 0)), 0));
		}
	}
	if(*(STypes + PC) == S_op2){
		if(**(Symbols + PC) == '='){
			PC = PC + 1;
			return rel_expr(A_add == add_expr(mul_expr(unary_cast_expr(0, 0)), 0));
		} if(**(Symbols + PC) == '!'){
			PC = PC + 1;
			return rel_expr(A_add != add_expr(mul_expr(unary_cast_expr(0, 0)), 0));
		}
	}
	return A_add;
}
int landor_expr(int A_rel){//NON-STANDARD(6.5.14): &&, || same priority
	if(**(Symbols + PC) == '&' && *(STypes + PC) == S_op2){
		PC = PC + 1;
		if(A_rel){
			return landor_expr(A_rel && rel_expr(add_expr(mul_expr(unary_cast_expr(0, 0)), 0)));
		}//short-circuit
		PC = skip_interparen(PC, 0);
		return 0;
	}
	if(**(Symbols + PC) == '|' && *(STypes + PC) == S_op2){
		PC = PC + 1;
		if(A_rel){ PC = skip_interparen(PC, 0); }//short-circuit
		return landor_expr(A_rel || rel_expr(add_expr(mul_expr(unary_cast_expr(0, 0)), 0)));
	}
	return A_rel;
}
int assign_expr(int A, int *obj_ref){//only plain assignment (6.5.16.1)
	A = unary_cast_expr(0, 0);
	if(TailCalling){ return 0; }
	obj_ref = ObjRef;
	if(*(STypes + PC) == S_op && **(Symbols + PC) == '='){
		PC = PC + 1;
		if(ExprType == Symbol8){
			*(char*)obj_ref = (char)assign_expr(0, obj_ref);
		} else{ *obj_ref = assign_expr(0, obj_ref); }
	}
	return landor_expr(rel_expr(add_expr(mul_expr(A), 0)));//no assignment
}
int expr_statement(int A){
	A = assign_expr(A, 0);
	if(**(Symbols + PC) == ';'){ PC = PC + 1; }
	return A;
}
int compound_statement(int A, int curly_ctr){
	TailCalling = 0;
	if(*(STypes + PC) == S_return){
		PC = PC + 1;
		A = expr_statement(A);
		if(TailCalling){ return compound_statement(A, curly_ctr); }
		return A;
	}
	if(*(STypes + PC) == S_if){
		PC = PC + 1;
		if(**(Symbols + PC) == '('){
			PC = PC + 1;
			A = assign_expr(0, 0);
		} else{}//error
		if(**(Symbols + PC) == ')'){ PC = PC + 1; } else{}//error
		if(**(Symbols + PC) == '{'){ PC = PC + 1; } else{}//error
		if(A){
			return compound_statement(A, curly_ctr + 1);
		} else{ PC = skip_intercurly(PC, 0); }
		if(*(STypes + PC) == S_else){
			PC = PC + 1;
			if(**(Symbols + PC) == '{'){ PC = PC + 1; } else{}
			return compound_statement(A, curly_ctr + 1);
		} else{ return compound_statement(A, curly_ctr); }
	}
	if(*(STypes + PC) == S_else){
		PC = skip_intercurly(PC + 2, 0);
		return compound_statement(A, curly_ctr);
	}
	if(**(Symbols + PC) == '{'){
		PC = PC + 1;
		return compound_statement(A, curly_ctr + 1);
	}
	if(**(Symbols + PC) == '}'){
		PC = PC + 1;
		return compound_statement(A, curly_ctr - 1);
	}
	return compound_statement(expr_statement(A), curly_ctr);
}
int main(int argc, char** argv){
	if(argc < 2){ return(9); }
	StackSize = 1024 * 1024 * 10;
	SymbolsMemSz = 1024 * 128;
	SrcFp = fopen(*(argv + 1), "r");
	Src = malloc(StackSize);
	SrcLen = fread(Src, 1, StackSize, SrcFp);
	*(Src + SrcLen) = 0;
	Symbols = malloc(SymbolsMemSz);
	STypes = malloc(SymbolsMemSz);
	SVals = malloc(SymbolsMemSz);
	SymbolAddrs = malloc(SymbolsMemSz);
	NumSymbols = SymbolsScan(Src, 0);
	Stack = malloc(StackSize);
	Top = Stack + 1;
	keywords_scan(NumSymbols);
	Top = Top + extern_defs_link(0, NumSymbols, 0, 0, 0, 0, 0);
	Frame = Top;
	*Top = (int)(argv + 1);
	Top = Top + 1;
	*Top = argc - 1;
	Top = Top + 1;
	PC = main_entrance(NumSymbols - 1);
	return compound_statement(0, 0);
}
#pragma once
//此函数不参与自举
int syntax_error(int sym_idx, char *message){
	return printf("!!!%i : %s\n", sym_idx, message);
}
//此函数不参与自举
int print_symbols(int symbol_id_iter, int num_symbols){
	if(symbol_id_iter == num_symbols){ return 0; }
	printf("[%i]%c\t%c%c%c\t%i\t%i\t%i\n", symbol_id_iter, **(Symbols + symbol_id_iter), **(STypes + symbol_id_iter),
		*(*(STypes + symbol_id_iter) + 1), *(*(STypes + symbol_id_iter) + 2),
		*(SVals + symbol_id_iter), *(SymbolAddrs + symbol_id_iter),
		*(Stack + *(SymbolAddrs + symbol_id_iter)));
	return print_symbols(symbol_id_iter + 1, num_symbols);
}
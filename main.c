#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pipeline.h"

void menu();

int main() {
	int op;

	do {
		menu();
		printf(" Sua escolha: ");
		scanf("%d", &op);
		printf("\n");
	switch (op) {
	case 1:
      
		break;
	case 2:
      
		break;
    	case 3:

		break;
   	case 4:
    
		break;
    	case 5:
      
		break;
    	case 6:
      
		break;
    	case 7:
      
		break;
    	case 8:

		break;
    	default:
      		printf("Opcao invalida!");
    	}
  	} while(op != 9);
  	return 0;
}

void menu() {
	printf("\n ******* MENU *******\n");
	printf("\n 1 - Carregar memoria\n");
	printf(" 2 - Imprimir memoria\n");
	printf(" 3 - Imprimir banco de registradores\n");
	printf(" 4 - Imprimir todo o simulador\n");
	printf(" 5 - Executar step\n");
	printf(" 6 - Executar run\n");
	printf(" 7 - Salvar .asm\n");
	printf(" 8 - Volta um step\n");
	printf(" 9 - Sair\n");
}

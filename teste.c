#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct memorias {
	char meminst[256][17];
	int memdat[256];
} Mem;

typedef struct registradores {
	int pc,
	    br[8],
	    id_ex[16],
	    ex_mem[8],
	    mem_wb[6],
	    if_id[18];
} Reg;

//STRUCTS e ENUMS
typedef enum tipo {
	Tipo_R=0,
	Tipo_I=1,
	Tipo_J=2,
	Tipo_OUTROS=3
} Type;

typedef struct instrucao {
	char opcode[5],
	     rs[4],
	     rt[4],
	     rd[4],
	     funct[4],
	     imm[7],
	     addr[8];
} Inst;

typedef struct decodificador {
	int opcode,
	    rs,
	    rt,
	    rd,
	    funct,
	    imm,
	    addr;
	Type tipo;
} Decod;

typedef struct Nodo {
	int ra[8],
	    mda[256],
	    pca;
	struct Nodo *prox;
} Nodo;

typedef struct pilha {
	Nodo *topo;
} Stack;

//ASSINATURA DAS FUNCOESvoid menu();
int carregaMemInst(char mem[256][17]);
void carregarMemoriaDados(int mem[256]);
void printMemory(char mem[256][17], Instrucao *in, Decodificador *d);
void printmemory(int *memdado);
void printReg(int *reg);
void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d);
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho);
int binarioParaDecimal(const char *bin, int sinal);
void printInstrucao(Decodificador *d);
void controle(Decodificador *in, int *reg, int *memdado, int *pc);
int ULA(int op1, int op2, int opULA, int *overflow, int *flag);
void salvarAssembly(char mem[256][17]);
void executaP(char meminst[256][17], Instrucao *in, Decodificador *d, int *pc, int *registrador, int *memdados,Pilha *p);
int executaI(char meminst[256][17], Instrucao *in, Decodificador *d, int *pc, int *registrador, int *memdados,Pilha *p);
void salvarMemDados(int *memdados);
void inicia_pilha(Pilha *p);
int step_back(Pilha *p, int *r, int *md, int *pc);
void empilha(Pilha *p, int *r, int *md, int *pc);
int somador(int op1, int op2);
int limite_back(Pilha *p)

// PROGRAMA PRINCIPAL
int main() {
	Inst inst;
	Decod decod;
	Stack stack;
	Mem mem;
	Reg reg;

	inicia_pilha(&stack);

	int op, nl, resul, pc = 0;

	do {
		menu();
		printf("Sua escolha: ");
		scanf("%d", &op);
		printf("\n");
		switch (op) {
		case 1:
			carregaMemInst(meminst);
			break;
		case 2:
			carregarMemoriaDados(memdados);
			break;
		case 3:
			printMemory(meminst, &in, &d);
			printmemory(memdados);
			break;
		case 4:
			printReg(registrador);
			break;
		case 5:
			printMemory(meminst, &in, &d);
			printmemory(memdados);
			printReg(registrador);
			printf("\n\nPC: %d", pc);
			break;
		case 6:
			salvarAssembly(meminst);
			break;
		case 7:
			salvarMemDados(memdados);
			break;
		case 8:
			executaP(meminst, &in, &d, &pc,registrador,memdados, &p);
			break;
		case 9:
			executaI(meminst, &in, &d, &pc,registrador, memdados,&p);
			break;
		case 10:
			step_back(&p, registrador, memdados, &pc);
			break;
		case 11:
			printf("Voce saiu!!!");
			break;
		}
	} while(op != 11);
	return 0;
}

//FUNCOES IMPLEMENTADAS

//MENU
void menu() {
	printf("\n\n *** MENU *** \n");
	printf("1 - Carregar memoria de instrucoes\n");
	printf("2 - Carregar memoria de dados\n");
	printf("3 - Imprimir memorias\n");
	printf("4 - Imprimir banco de registradores\n");
	printf("5 - Imprimir todo o simulador\n");
	printf("6 - Salvar .asm\n");
	printf("7 - Salvar .dat\n");
	printf("8 - Executar programa\n");
	printf("9 - Executar instrucao\n");
	printf("10 - Volta uma instrucao\n");
	printf("11 - Sair\n\n");
}

// carrega memoria de instrucoes a partir de um "arquivo.mem"
int carregaMemInst(char mem[256][17]) {
	char arquivo[20];
	// abre o arquivo em modo leitura
	printf("Nome do arquivo: ");
	scanf("%s", arquivo);
	FILE *arq = fopen (arquivo, "r");
	if (!arq)
	{
		perror ("Erro ao abrir arquivo") ;
		exit (1) ;
	}
	int i = 0;
	char linha[20]; // Buffer para leitura
	while (i < 256 && fgets(linha, sizeof(linha), arq)) {
		// Remover quebras de linha e caracteres extras
		linha[strcspn(linha, "\r\n")] = '\0';

		// Ignorar linhas vazias
		if (strlen(linha) == 0) {
			continue;
		}

		strncpy(mem[i], linha, 16); // Copia atC) 16 caracteres
		mem[i][16] = '\0'; // Garante terminaC'C#o de string
		i++; // AvanC'a corretamente para a prC3xima posiC'C#o
	}
	fclose(arq);
}

// carrega memoria de dados a partir de um "arquivo.dat"
void carregarMemoriaDados(int mem[256]) {
	char arquivo[20];
	printf("Nome do arquivo: ");
	scanf("%s", arquivo);
	FILE *arq = fopen(arquivo, "r");
	if (!arq) {
		perror ("Erro ao abrir arquivo") ;
		exit (1) ;
	}
	int i = 0;
	while (fscanf(arq, "%d", &mem[i]) != EOF) {
		i++;
	}
}

// imprime memoria de instrucoes
void printMemory(char mem[256][17], Instrucao *in, Decodificador *d) {
	printf("\n############## MEMORIA DE INSTRUCOES ##############\n");
	for (int i = 0; i < 256; i++)
	{
		printf("\nInstrucao: %s\n", mem[i]);
		printf("[%d].  ", i);
		decodificarInstrucao(mem[i], in, d);
		printInstrucao(d);
		printf("\n");
	}
}

// imprime memoria de dados
void printmemory(int *memdado) {
	printf("\n############## MEMORIA DE DADOS ##############\n\n");
	for(int i=0; i<256; i++) {
		printf("[%d]. %d   ", i, memdado[i]);
		if (i % 8 == 7)
		{
			printf("\n");
		}
	}
}

// imprime banco de registradores
void printReg(int *reg) {
	for(int i=0; i<8; i++) {
		printf("\nREGISTRADOR [%d] - %d", i, reg[i]);
	}
}

// decodifica a instrucao e armazena os valores na struct do tipo Deco ja no formato int
void decodificarInstrucao(const char *bin, Instrucao *in, Decodificador *d) {
	copiarBits(bin, in->opcode, 0, 4);    // Copia os 4 bits do opcode (4 bits)
	d->opcode = binarioParaDecimal(in->opcode, 0);
	copiarBits(bin, in->rs, 4, 3);        // Copia os 3 bits do rs
	d->rs = binarioParaDecimal(in->rs, 0);
	copiarBits(bin, in->rt, 7, 3);        // Copia os 3 bits do rt
	d->rt = binarioParaDecimal(in->rt, 0);
	copiarBits(bin, in->rd, 10, 3);       // Copia os 3 bits do rd
	d->rd = binarioParaDecimal(in->rd, 0);
	copiarBits(bin, in->funct, 13, 3);    // Copia os 3 bits do funct
	d->funct = binarioParaDecimal(in->funct, 0);
	copiarBits(bin, in->imm, 10, 6);      // Copia os 6 bits do imm
	d->imm = binarioParaDecimal(in->imm, 1);
	copiarBits(bin, in->addr, 9, 7);     // Copia os 7 bits do addr
	d->addr = binarioParaDecimal(in->addr, 0);
}

// copia os bits da instrucao para cada campo da struct instrucao
void copiarBits(const char *instrucao, char *destino, int inicio, int tamanho)
{
	strncpy(destino, instrucao + inicio, tamanho); // Copia os bits desejados
	destino[tamanho] = '\0';  // Adiciona o terminador de string
}

// converte de binario para decimal
int binarioParaDecimal(const char *bin, int sinal) {
	int valor = (int)strtol(bin, NULL, 2);
	int bits = strlen(bin);

	if (sinal && bits > 0 && (valor & (1 << (bits - 1)))) {
		valor = valor - (1 << bits);
	}
	return valor;
}

// funcao para imprimir a instrucao
void printInstrucao(Decodificador *d) {
	switch (d->opcode) {
	case 0: // Tipo R (add, sub, and, or)
		switch (d->funct) {
		case 0:
			printf("add $%d, $%d, $%d", d->rd, d->rs, d->rt);
			break;
		case 2:
			printf("sub $%d, $%d, $%d", d->rd, d->rs, d->rt);
			break;
		case 4:
			printf("and $%d, $%d, $%d", d->rd, d->rs, d->rt);
			break;
		case 5:
			printf("or $%d, $%d, $%d", d->rd, d->rs, d->rt);
			break;
		}
		break;
	case 4: // addi
		printf("addi $%d, $%d, %d", d->rt, d->rs, d->imm);
		break;
	case 11: // lw
		printf("lw $%d, %d($%d)", d->rt, d->imm, d->rs);
		break;
	case 15: // sw
		printf("sw $%d, %d($%d)", d->rt, d->imm, d->rs);
		break;
	case 8: // beq
		printf("beq $%d, $%d, %d", d->rs, d->rt, d->imm);
		break;
	case 2: // j
		printf("j %d", d->addr);
		break;
	}
}

// funcao para implementacao da unidade de controle
void controle(Decodificador *d, int *reg, int *memdado, int *pc) {
	int flag=0;
	int overflow=0;

	if (d->opcode == 11) {
		reg[d->rt] = ULA(reg[d->rs], memdado[d->imm], 0, &overflow,&flag);
	}
	else if (d->opcode == 15) {
		memdado[ULA(d->rs, d->imm, 0, &overflow,&flag)] = reg[d->rt];
	}
	else if (d->opcode == 4) {
		reg[d->rt] = ULA(reg[d->rs], d->imm, 0, &overflow,&flag);
	}
	else if (d->opcode == 0) {
		if (d->funct == 0) {
			reg[d->rd] = ULA(reg[d->rs], reg[d->rt], 0, &overflow,&flag);
		}
		else if (d->funct == 2) {
			reg[d->rd] = ULA(reg[d->rs], reg[d->rt], 2, &overflow,&flag);
		}
		else if (d->funct == 4) {
			reg[d->rd] = ULA(reg[d->rs], reg[d->rt], 4, &overflow,&flag);
		}
		else if (d->funct == 5) {
			reg[d->rd] = ULA(reg[d->rs], reg[d->rt], 5, &overflow,&flag);
		}
	}
	else if (d->opcode == 8) {
		ULA(reg[d->rs], reg[d->rt], 2, &overflow, &flag);
		if(flag==1) {
			*pc = somador(*pc,d->imm);
		}
	}
	else if (d->opcode == 2) {
		*pc = d->addr;
	}
}

// funcao para implementacao da ula
int ULA(int op1, int op2, int opULA, int *overflow, int *flag) {
	int resultado;
	if(opULA == 0) {
		resultado = op1 + op2;

		if ((op1 > 0 && op2 > 0 && resultado < 0) || (op1 < 0 && op2 < 0 && resultado > 0)) {
			*overflow = 1;
			printf("OVERFLOW - ADD: %d + %d = %d (fora do intervalo de 8 bits!)\n", op1, op2, resultado);
		}
	}
	else if(opULA == 2) {
		resultado = op1 - op2;
		if(resultado == 0) {
			*flag = 1;
		}
		if ((op1 > 0 && op2 < 0 && resultado < 0) || (op1 < 0 && op2 > 0 && resultado > 0)) {
			*overflow = 1;
			printf("OVERFLOW - SUB: %d - %d = %d (fora do intervalo de 8 bits!)\n", op1, op2, resultado);
		}
	}
	else if(opULA == 4) {
		resultado = op1 & op2;
	}
	else if(opULA == 5) {
		resultado = op1 | op2;
	}
	return resultado;
}

// funcao para conversao das instrucoes para assembly e salvar "arquivo.asm"
void salvarAssembly(char mem[256][17]) {
	char arquivo[20];

	printf("Nome do arquivo .asm: ");
	scanf("%s", arquivo);

	FILE *arq = fopen(arquivo, "w");
	if (!arq) {
		perror("Erro ao criar arquivo");
		return;
	}

	for (int i = 0; i < 256; i++) {
		if (mem[i][0] == '\0') continue; // Ignora posiC'C5es vazias

		struct instrucao inst;
		Decodificador d;
		decodificarInstrucao(mem[i], &inst, &d);

		// Converte para assembly e escreve no arquivo
		switch (d.opcode) {
		case 0: // Tipo R (add, sub, and, or)
			switch (d.funct) {
			case 0:
				fprintf(arq, "add $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
				break;
			case 2:
				fprintf(arq, "sub $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
				break;
			case 4:
				fprintf(arq, "and $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
				break;
			case 5:
				fprintf(arq, "or $%d, $%d, $%d\n", d.rd, d.rs, d.rt);
				break;
			}
			break;
		case 4: // addi
			fprintf(arq, "addi $%d, $%d, %d\n", d.rt, d.rs, d.imm);
			break;
		case 11: // lw
			fprintf(arq, "lw $%d, %d($%d)\n", d.rt, d.imm, d.rs);
			break;
		case 15: // sw
			fprintf(arq, "sw $%d, %d($%d)\n", d.rt, d.imm, d.rs);
			break;
		case 8: // beq
			fprintf(arq, "beq $%d, $%d, %d\n", d.rs, d.rt, d.imm);
			break;
		case 2: // j
			fprintf(arq, "j %d\n", d.addr);
			break;
		}
	}
	fclose(arq);
	printf("Arquivo %s salvo com sucesso!\n", arquivo);
}

// executa uma instrucao
int executaI(char meminst[256][17], Instrucao *in, Decodificador *d, int *pc, int *registrador, int *memdados,Pilha *p) {
	if (strcmp(meminst[*pc], "0000000000000000") == 0 || *pc > 255) {
		printf("\nFim do programa!");
		return 1;
	}
	else
	{
		empilha(p,registrador,memdados, pc);
		decodificarInstrucao(meminst[*pc], in, d);
		printInstrucao(d);
		printf("\n");
		*pc = somador(*pc,1);
		controle(d, registrador, memdados, pc);
	}
}

// executa todo o programa
void executaP(char meminst[256][17], Instrucao *in, Decodificador *d, int *pc, int *registrador, int *memdados,Pilha *p) {
	while(executaI(meminst, in, d, pc, registrador, memdados,p) != 1);
}

// salva memoria de dados em um "arquivo.dat"
void salvarMemDados(int *memdados) {
	FILE *arquivo;
	char nomeArquivo[20];

	printf("Salvar como: ");
	scanf("%s", nomeArquivo);

	if ((arquivo = fopen(nomeArquivo, "w")) == NULL)
	{
		printf("Erro ao gerar o arquivo!");
	}
	for (int i = 0; i < 256; i++)
	{
		fprintf(arquivo, "%d\n", memdados[i]);
	}
	fclose(arquivo);
}

// inicia pilha apontando para NULL
void inicia_pilha(Pilha *p) {
	p->topo=NULL;
}

// salva registradores, memoria de instrucoes e pc na pilha
void empilha(Pilha *p, int *r, int *md, int *pc) {
	Nodo *nNodo = (Nodo*)malloc(sizeof(Nodo));
	int i;
	for(i=0; i<8; i++) {
		nNodo->ra[i]=r[i];
	}
	for(i=0; i<8; i++) {
		nNodo->mda[i]=md[i];
	}
	nNodo->pca=*pc;
	nNodo->prox=p->topo;
	p->topo=nNodo;
}

// funcao de execucao do step back
int step_back(Pilha *p, int *r, int *md, int *pc) {
	int i;

	if(limite_back(p) == 1) {
		return 1;
	} else {
		Nodo *remover = p->topo;
		for(i = 0; i < 8; i++) {
			r[i] = remover->ra[i];
		}
		for(i = 0; i < 8; i++) {
			md[i] = remover->mda[i];
		}
		*pc = remover->pca;
		p->topo = remover->prox;
		free(remover);
		return 0;
	}
}

// somador
int somador(int op1, int op2) {
	return op1 + op2;
}

// limite do step back, termina desempilhamento na primeira instrucao executada
int limite_back(Pilha *p) {
	if(p->topo==NULL) {
		printf("\nVoce voltou ao inicio!");
		return 1;
	}
}
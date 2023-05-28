#include <stdio.h>
#include <stdbool.h>
#include "pager.h"
#include "uvm.h"
#include "mmu.h"
#include "mmuproto.h"
#include "cyc.h"
#include "log.h"

//estrutura de dados que representa um quadro na memoria fisica
typedef struct Quadro{
    pid_t pid; //id do processo que utiliza o quadro
    int numPagina;
    bool disponivel; //true se disponivel, false caso contrario
}Quadro;

//estrutura de dados que representa uma tabela de paginas de um processo
typedef struct TabelaDePaginas{
	int qtdPaginas; // Usado como tamanho dos dois vetores quadros e blocos.
	int *quadros; 
	int *blocos; 
} TabelaDePaginas;

typedef struct ListaDeTabelas{
	pid_t pid;
	TabelaDePaginas *tabela;
} ListaDeTabelas;

//vetor de quadros da memoria fisica
Quadro *quadros;
int numeroQuadros;

//vetor de blocos do disco
int *vetorDeBlocos;
int qtdBlocosLivres; // informa quantos blocos ainda estão livres.
int tamanhoVetorBlocos;

//vetor de tabelas
ListaDeTabelas *listaDeTabelas;
int tamanhoListaDeTabelas;

//gu-
//inicializacao do paginador
void pager_init(int nframes, int nblocks){
    int i;

    //inicializacao do vetor de quadros da memoria fisica
    numeroQuadros = nframes;
    quadros = (Quadro*) malloc (nframes * sizeof(Quadro));
    for(i=0; i<nframes; i++){
        quadros[i].disponivel = true;
        quadros[i].numPagina = 0;
        quadros[i].pid = -1;
    }

    //inicializacao do vetor de blocos
    tamanhoVetorBlocos = nblocks;
    qtdBlocosLivres = nblocks;
    vetorDeBlocos = (int*) malloc (nblocks * sizeof(int));
    for(i = 0; i < nblocks; i++){
		vetorDeBlocos[i] = 0; //setando todas os blocos como vazios
    }

    //inicializacao da lista de tabela de paginas
    tamanhoListaDeTabelas = 1;
    listaDeTabelas = (ListaDeTabelas*) malloc (tamanhoListaDeTabelas * sizeof(ListaDeTabelas));
}
//-gu

void pager_create(pid_t pid){
    int i, j, numeroDePaginas, aux = 0;
	// Calcula o número de páginas dos vetores frames e blocks.
	numeroDePaginas = (UVM_MAXADDR - UVM_BASEADDR + 1) / sysconf(_SC_PAGESIZE);




}
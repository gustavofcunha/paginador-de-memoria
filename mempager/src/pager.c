#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pager.h"
#include "uvm.h"
#include "mmu.h"
#include "mmuproto.h"
#include "cyc.h"
#include "log.h"

//estrutura de dados que representa um quadro na memoria fisica
typedef struct Quadro {
    pid_t pid; //id do processo que utiliza o quadro
    int numPagina;
    bool disponivel; //true se disponivel, false caso contrario
    short bitReferencia; // Usado no algoritmo de segunda chance (0 - substitui, 1 - 2ª chance)
    short none;
    short escrito;
} Quadro;

//estrutura de dados que representa uma tabela de paginas de um processo
typedef struct TabelaDePaginas {
	int qtdPaginasBlocos; // Usado como tamanho dos dois vetores quadros e blocos.
	int *quadros;
	int *blocos;
} TabelaDePaginas;

typedef struct ListaDeTabelas {
	pid_t pid;
	TabelaDePaginas *tabela;
} ListaDeTabelas;

int clockPtr;

//vetor de quadros da memoria fisica
Quadro *quadros;
int numeroQuadros;

//vetor de blocos do disco
int *blocos;
int qtdBlocosLivres; // informa quantos blocos ainda estão livres.
int numeroBlocos;

//vetor de tabelas
ListaDeTabelas *listaDeTabelas;
int tamanhoListaDeTabelas;

//gu-
//inicializacao do paginador
void pager_init(int nquadros, int nblocos){
    int i;

    //inicializacao do vetor de quadros da memoria fisica
    numeroQuadros = nquadros;
    quadros = (Quadro*) malloc(nquadros* sizeof(Quadro));
    for(i=0; i<nquadros; i++){
        quadros[i].disponivel = true;
        quadros[i].numPagina = 0;
        quadros[i].pid = -1;
    }

    //inicializacao do vetor de blocos
    numeroBlocos = nblocos;
    qtdBlocosLivres = nblocos;
    blocos = (int*) malloc(nblocos* sizeof(int));
    for(i = 0; i < nblocos; i++){
		blocos[i] = 0; //setando todas os blocos como vazios (0 como conveção)
    }

    //inicializacao da lista de tabela de paginas
    tamanhoListaDeTabelas = 1;
    listaDeTabelas = (ListaDeTabelas*) malloc(tamanhoListaDeTabelas* sizeof(ListaDeTabelas));
}
//-gu

//ga-
void pager_create(pid_t pid){
    int i, j, qtdPaginas;

    // Calcula o número de páginas dos vetores quadros e blocks.
    qtdPaginas = (UVM_MAXADDR - UVM_BASEADDR + 1) / sysconf(_SC_PAGESIZE);

    // Procurando tabela vazia na lista de tabelas.
    for(i = 0; i < tamanhoListaDeTabelas; i++){
        // Se a tabela estiver vazia.
        if (listaDeTabelas[i].tabela == NULL){
            listaDeTabelas[i].pid = pid;
            listaDeTabelas[i].tabela = malloc(sizeof(TabelaDePaginas));
            listaDeTabelas[i].tabela->qtdPaginasBlocos = qtdPaginas;
            listaDeTabelas[i].tabela->quadros = malloc(qtdPaginas* sizeof(int));
            listaDeTabelas[i].tabela->blocos = malloc(qtdPaginas* sizeof(int));

            // Seta os valores para -1 (convenção de vazio).
            for(j = 0; j < qtdPaginas; j++){
                listaDeTabelas[i].tabela->quadros[j] = -1;
                listaDeTabelas[i].tabela->blocos[j] = -1;
            }

            return;
        }
    }

    // Aumenta o tamanho da lista de tabelas.
    tamanhoListaDeTabelas += 100;
    listaDeTabelas = realloc(listaDeTabelas, tamanhoListaDeTabelas * sizeof(listaDeTabelas));

    // Aloca o novo processo na primeira posição vazia.
    listaDeTabelas[tamanhoListaDeTabelas - 100].pid = pid;
    listaDeTabelas[tamanhoListaDeTabelas - 100].tabela = malloc(sizeof(listaDeTabelas));
    listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->qtdPaginasBlocos = qtdPaginas;
    listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->quadros = malloc(qtdPaginas* sizeof(int));
    listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->blocos = malloc(qtdPaginas* sizeof(int));

	for (j = 0; j < qtdPaginas; j++){
        listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->quadros[j] = -1;
        listaDeTabelas[tamanhoListaDeTabelas - 100].tabela->blocos[j] = -1;
    }

    // Define as tabelas subsequentes como vazias.
    for (j = tamanhoListaDeTabelas - 99; j < tamanhoListaDeTabelas; j++){
        listaDeTabelas[j].tabela = NULL;
    }
}
//-ga

//gu-
//aloca um quadro de memoria a um processo
void *pager_extend(pid_t pid){
    //se nao houverem mais blocos disponiveis, retorna NULL
    if(qtdBlocosLivres == 0){
        return NULL;
    }

    int i, j, posicaoBlocoAlocado;
    TabelaDePaginas *tabelaProcesso;

    //alocacao do bloco do disco
    for(i = 0 ; i < numeroBlocos; i++){
        if(blocos[i] == 0){
            blocos[i] = 1; //seta bloco para em uso
            qtdBlocosLivres--;
            posicaoBlocoAlocado = i;

            break;
        }
    }

    //busca tabela de paginas do processo
    for(i = 0; i < tamanhoListaDeTabelas; i++){
        //tabela localizada
        if(listaDeTabelas[i].pid == pid){
            tabelaProcesso = listaDeTabelas[i].tabela;
            //busca bloco livre na tabela do processo
            for(j = 0; j<tabelaProcesso->qtdPaginasBlocos; j++){
                //encontrou bloco livre na tabela do processo
                if(tabelaProcesso->blocos[j] == -1){
                    //referencia ao bloco alocado
                    tabelaProcesso->blocos[j] = posicaoBlocoAlocado;

                    break;
                }

                // se nao ha blocos livres na tabela do processo, retorna NULL
				if(j == (tabelaProcesso->qtdPaginasBlocos) - 1){
					return NULL;
                }
            }

            break;
        }
    }

    //retorna o endereço (inicio + posicao * tamanho da pagina)
    return (void*)(UVM_BASEADDR + (intptr_t)(j* sysconf(_SC_PAGESIZE)));
}
//-gu

//ga-
void pager_fault(pid_t pid, void *vaddr){
    int i, index, index2, numPagina, quadroAtual, novoQuadro, blocoAtual, novoProcesso, moveDiscoViaPid, moveDiscoViaPnum, memNoNone;
    void *addr;

    // Procura o índice da tabela de página do processo pid na lista de tabelas.
    for(i = 0; i < tamanhoListaDeTabelas; i++){
        if(listaDeTabelas[i].pid == pid){
            // Salva o índice e sai.
            index = i;

            break;
        }
    }

    // Pega o número do quadro na tabela do processo.
    numPagina = ((((intptr_t)vaddr) - UVM_BASEADDR) / (sysconf(_SC_PAGESIZE)));

    memNoNone = 1;
    for(i = 0; i < numeroQuadros; i++){
        if(quadros[i].none == 1){
            memNoNone = 0;

            break;
        }
    }

    // Se esse quadro está carregado.
    if(listaDeTabelas[index].tabela->quadros[numPagina] != -1 && listaDeTabelas[index].tabela->quadros[numPagina] != -2){
        // Salva o índice do vetor de quadros (memória).
        quadroAtual = listaDeTabelas[index].tabela->quadros[numPagina];
        // Dá permissão de escrita para o processo pid.
        mmu_chprot(pid, vaddr, PROT_READ | PROT_WRITE);
        // Marca o bit de referência no vetor de quadros (memória).
        quadros[quadroAtual].bitReferencia = 1;
        // Marca que houve escrita.
        quadros[quadroAtual].escrito = 1;
    } else { // Se não está carregado:
        if(memNoNone){
            for(i = 0; i < numeroQuadros; i++){
                addr = (void*)(UVM_BASEADDR + (intptr_t)(quadros[i].numPagina* sysconf(_SC_PAGESIZE)));
                mmu_chprot(quadros[i].pid, addr, PROT_NONE);
                quadros[i].none = 1;
            }
        }

        novoQuadro = -1;
        while(novoQuadro == -1){
            novoQuadro = -1;

            // Se o bit de referência é zero.
            if(quadros[clockPtr].bitReferencia == 0){
                novoQuadro = clockPtr;

                // Se o frame está em uso.
                if(quadros[clockPtr].disponivel == 1){
                    // Remove o frame e Salva o frame no disco se tiver permissão de escrita.
                    moveDiscoViaPid = quadros[clockPtr].pid;
                    moveDiscoViaPnum = quadros[clockPtr].numPagina;

                    for(i = 0; i < tamanhoListaDeTabelas; i++){
                        if(listaDeTabelas[i].pid == moveDiscoViaPid){
                            index2 = i;
                        }
                    }

                    blocoAtual = listaDeTabelas[index2].tabela->blocos[moveDiscoViaPnum];
                    mmu_nonresident(pid, (void*)(UVM_BASEADDR + (intptr_t)(moveDiscoViaPnum* sysconf(_SC_PAGESIZE))));

                    if(quadros[clockPtr].escrito == 1){
                        mmu_disk_write(clockPtr, blocoAtual);
                        // Marca o frame como vazio (sem uso) que está no disco.
                        listaDeTabelas[index2].tabela->quadros[moveDiscoViaPnum] = -2;
                    } else {
                        // Marca o frame como vazio (sem uso).
                        listaDeTabelas[index2].tabela->quadros[moveDiscoViaPnum] = -1;
                    }
                }

                // Coloca o novo processo no vetor de quadros.
                quadros[clockPtr].pid = pid;
                quadros[clockPtr].numPagina = numPagina;
                quadros[clockPtr].disponivel = 1;
                quadros[clockPtr].bitReferencia = 1;
                quadros[clockPtr].none = 0;

                if(listaDeTabelas[index].tabela->quadros[numPagina] == -2){
                    novoProcesso = listaDeTabelas[index].tabela->blocos[numPagina];
                    mmu_disk_read(novoProcesso, novoQuadro);
                    quadros[clockPtr].escrito = 1;
                } else {
                    mmu_zero_fill(novoQuadro);
                    quadros[clockPtr].escrito = 0;
                }

                listaDeTabelas[index].tabela->quadros[numPagina] = novoQuadro;
                mmu_resident(pid, vaddr, novoQuadro, PROT_READ /*| PROT_WRITE*/);
            } else {
                quadros[clockPtr].bitReferencia = 0;
            }

            clockPtr++;
            clockPtr %= numeroQuadros;
        }
    }
}
//-ga

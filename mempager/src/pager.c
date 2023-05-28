#include <stdio.h>
#include <stdbool.h>
#include "pager.h"
#include "uvm.h"
#include "mmu.h"
#include "cyc.h"
#include "log.h"

//estrutura de dados que representa um quadro na memoria fisica
typedef struct Quadro{
    pid_t pid; //id do processo que utiliza o quadro
    int numPagina;
    bool disponivel; //true se disponivel, false caso contrario
}Quadro;

//vetor de quadros da memoria fisica
Quadro *quadros;
int numeroQuadros;


//inicializacao do paginador
void pager_init(int nframes, int nblocks){

    //inicializacao do vetor de quadros da memoria fisica
    numeroQuadros = nframes;
    quadros = (Quadro*) malloc (nframes * sizeof(Quadro));
    for(int i=0; i<nframes; i++){
        quadros[i].disponivel = true;
        quadros[i].numPagina = 0;
        quadros[i].pid = -1;
    }
}

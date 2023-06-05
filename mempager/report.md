<!-- LTeX: language=pt-BR -->

# PAGINADOR DE MEMÓRIA -- RELATÓRIO

1. Termo de compromisso

    Ao entregar este documento preenchido, os membros do grupo afirmam que todo o código desenvolvido para este trabalho é de autoria própria.  Exceto pelo material listado no item 3 deste relatório, os membros do grupo afirmam não ter copiado material da Internet nem ter obtido código de terceiros.

2. Membros do grupo e alocação de esforço

    Preencha as linhas abaixo com o nome e o email dos integrantes do grupo.  Substitua marcadores `XX` pela contribuição de cada membro do grupo no desenvolvimento do trabalho (os valores devem somar 100%).

    * Gustavo Cunha gustavocunha@dcc.ufmg.br 50%
    * Gabriel Juarez gabrimartinsjuarez@ufmg.br 50%

3. Referências bibliográficas
    FIGUEIREDO, Flavio. Slides da disciplina Sistemas Operacionais, 2023. The Open Group. Ucontext. The Single UNIX ® Specification, Version 2, 1997. Disponível em: https://pubs.opengroup.org/onlinepubs/7908799/xsh/ucontext.h.html. PetBBC. Signal, 2023. Disponível em: https://petbcc.ufscar.br/signal/.

4. Detalhes de implementação

    1. Descreva e justifique as estruturas de dados utilizadas em sua solução.
    
        * A struct Quadro é utilizada para representar cada quadro na memória física. Ela armazena informações relevantes sobre o quadro, como o processo que está utilizando, o número da página associada, o estado de disponibilidade, o bit de referência, entre outros. Essas informações são necessárias para realizar o gerenciamento da memória física e a substituição de páginas quando necessário.
        * A struct TabelaDePaginas é utilizada para representar a tabela de páginas de um processo. Ela armazena os números dos quadros e dos blocos associados a cada página do processo. Essas informações são necessárias para mapear os endereços virtuais dos processos para os endereços físicos correspondentes.
        * A struct ListaDeTabelas é utilizada para manter uma lista de todas as tabelas de páginas dos processos. Cada entrada na lista contém o identificador do processo e um ponteiro para a tabela de páginas correspondente. Essa estrutura permite o acesso rápido às tabelas de páginas dos processos e a realização de operações de criação e de busca.
        * Além das estruturas de dados mencionadas, o código também utiliza vetores para representar o vetor de quadros da memória física (quadros), o vetor de blocos do disco (blocos), e um vetor de ListaDeTabelas para armazenar as tabelas de páginas dos processos. Esses vetores são alocados dinamicamente com base no número de quadros e blocos especificados na inicialização do paginador.
        
    3. Descreva o mecanismo utilizado para controle de acesso e modificação às páginas.

        * O mecanismo utilizado para controle de acesso e modificação às páginas de memória física é baseado no uso de tabelas de páginas e bits de controle. 
        * A tabela de páginas é uma estrutura de dados que armazena informações sobre cada página de memória virtual associada a um processo. Cada entrada na tabela de páginas contém informações como o número da página, o número do quadro correspondente (quando a página está carregada na memória física), permissões de acesso (leitura e escrita), e outros bits de controle.
        * Ao acessar uma página, seja para leitura ou escrita, o código verifica se o quadro correspondente à página está carregado na memória física. Caso esteja carregado, o bit de referência é marcado, indicando que a página foi recentemente acessada. Se a página não estiver carregada na memória física, é necessário executar o algoritmo de substituição de quadros para selecionar um novo quadro. Esse algoritmo, no caso apresentado, é baseado no algoritmo de segunda chance.
        * O algoritmo de segunda chance percorre os quadros disponíveis na memória física e verifica os bits de referência associados a cada quadro. Se encontrar um quadro com bit de referência igual a 0, esse quadro é selecionado para substituição. Caso todos os bits de referência sejam 1, o algoritmo remove o primeiro quadro encontrado e atualiza seu bit de referência para 0, selecionando-o para substituição.
        * Uma vez selecionado o quadro para substituição, o código remove esse quadro da memória física, caso seja necessário, salvando-o de volta no disco. Em seguida, o novo quadro é alocado para o processo e a tabela de páginas é atualizada com as informações correspondentes à nova alocação.
        * Quando uma página é modificada, o processo de escrita de páginas é acionado. Ele segue um processo semelhante ao de leitura de páginas, verificando se o quadro correspondente à página está carregado na memória física, marcando o bit de referência e definindo as permissões de escrita. Caso a página não esteja carregada, o algoritmo de substituição é executado e o quadro é tratado da mesma forma que na leitura de páginas.



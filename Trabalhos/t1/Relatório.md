# Relatório

Todos os requisitos do trabalho foram implementados, abrangendo a criação e o gerenciamento de processos, o controle de estados, e diferentes estratégias de escalonamento. Para isso, criou-se inicialmente a estrutura `proc_t` a fim de armazenar as informações essenciais de cada processo, como registradores, estado e métricas. Em seguida, com o objetivo de isolar a lógica de seleção do próximo processo a ser executado, elaborou-se a estrutura `esc_t`, cuja responsabilidade é manter a fila de processo prontos para execução e aplicar o algoritmo de uma determinada estratégia de escalonamento. Essas estruturas interagem por meio da estrutura `so_t`, que controla a fila do escalonador com base nas mudanças de estado de cada um dos processos. Tal abordagem promoveu uma separação clara de responsabilidades, resultando em uma estrutura desacoplada e flexível, que permite múltiplos modos de escalonamento.

Foram implementadas três estratégias distintas de escalonamento, cada uma com características específicas que determinam como os processos são selecionados e gerenciados durante sua execução:

- **Simples:** Essa estratégia escolhe o primeiro processo pronto da fila e o executa até que o mesmo seja bloqueado ou finalize sua execução de forma natural. É a abordagem mais direta e não considera preempções ou prioridades. Por sua simplicidade, é útil em cenários de baixa carga de processos, em que a troca frequente de contexto não é necessária.

- **Circular:** Assim como na estratégia simples, o escalonador inicia executando o primeiro processo pronto. Entretanto, caso o tempo de execução de um processo exceda um intervalo pré-definido chamado quantum, ele é interrompido e movido para o final da fila, cedendo sua vez ao próximo processo pronto. Essa abordagem, também conhecida como Round-Robin e promove um compartilhamento equitativo de CPU entre os processos.

- **Prioritário:** Essa estratégia introduz um mecanismo dinâmico de atualização de prioridades. Quando um processo para de executar, seja porque foi bloqueado ou porque excedeu o tempo de quantum, sua prioridade é ajustada. Processos que se bloqueiam mais rapidamente recebem maior prioridade, incentivando a execução preferencial de tarefas que realizam operações rápidas. Isso torna a estratégia adequada para sistemas em que a redução do tempo de espera médio é uma prioridade, favorecendo processos curtos e interativos.


## Testes

Alguns testes foram realizados para avaliar o desempenho de cada estratégia em diferentes cenários, variando tanto o intervalo de cada interrupção de relógio quanto o número de interrupções necessário para o quantum de cada processo. As principais métricas coletadas durante os testes incluem o tempo total de execução do sistema, o número de interrupções de cada tipo e o tempo total de cada processo em cada estado. Tais informações foram medidas em termos de número de instruções da CPU. Os resultados obtidos serão apresentados a seguir, oferecendo uma visão clara das diferenças e implicações de cada abordagem.

## Simples

Os testes se iniciaram com o escalonador mais básico, o simples, utilizando a variável configuração de INTERVALO_RELOGIO = 50. O intervalo de INTERVALO_QUANTUM não faz diferença nesse escalonador. Observe que, como esperado, não ocorreu nenhuma preempção, uma vez que cada processo executou até seu bloqueio ou seu encerramento.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 27899                   | 8517                | 0                         |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 556                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 0                         | 743                   | 0                               |
| 2   | 0                         | 13747                 | 599                             |
| 3   | 0                         | 16108                 | 519                             |
| 4   | 0                         | 27500                 | 97                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 1                       | 1                   | 0                      | 1                  |
| 2   | 7                       | 7                   | 6                      | 1                  |
| 3   | 21                      | 21                  | 20                     | 1                  |
| 4   | 131                     | 131                 | 130                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 743                     | 0                   | 0                      | 27156              |
| 2   | 9156                    | 4197                | 394                    | 13769              |
| 3   | 3774                    | 10908               | 1426                   | 11400              |
| 4   | 5709                    | 12782               | 9009                   | 0                  |

Em seguida, realizou-se um teste com o INTERVALO_RELOGIO = 40. Isso porque, durante a execução dos processos, notou-se que, quando o último processo era bloqueado para escrita, a CPU ficava parada até a próxima interrupção de relógio, embora a tela já estivesse desbloqueada algumas instruções antes. Dessa forma, reduzindo o INTERVALO_RELOGIO, foi possível fazer com que a CPU reagisse mais rapidamente ao desbloqueio da tela.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 25572                   | 5929                | 0                         |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 638                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 0                         | 755                   | 0                               |
| 2   | 0                         | 13988                 | 719                             |
| 3   | 0                         | 16255                 | 532                             |
| 4   | 0                         | 25167                 | 98                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 1                       | 1                   | 0                      | 1                  |
| 2   | 6                       | 6                   | 5                      | 1                  |
| 3   | 21                      | 21                  | 20                     | 1                  |
| 4   | 131                     | 131                 | 130                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 755                     | 0                   | 0                      | 24817              |
| 2   | 9309                    | 4319                | 360                    | 11195              |
| 3   | 3822                    | 11173               | 1260                   | 8920               |
| 4   | 5757                    | 12856               | 6554                   | 0                  |

Vale destacar que, embora a CPU executasse o SO mais frequentemente, essa abordagem diminuiu o tempo total de execução em 8,34% para o conjunto de processos analisados.

## Circular

O escalonador Round-Robin foi executado com INTERVALO_RELOGIO = 50 e INTERVALO_QUANTUM = 5. Veja que o tempo de execução foi similar ao do escalonsddor simples, mas todos os processos puderam ser executados de forma simultânea, garantindo a responsividade do sistema.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 26204                   | 6792                | 46                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 522                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 0                         | 743                   | 0                               |
| 2   | 29                        | 16321                 | 172                             |
| 3   | 12                        | 14486                 | 372                             |
| 4   | 5                         | 25805                 | 95                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 1                       | 1                   | 0                      | 1                  |
| 2   | 38                      | 38                  | 8                      | 1                  |
| 3   | 26                      | 26                  | 13                     | 1                  |
| 4   | 125                     | 125                 | 119                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 743                     | 0                   | 0                      | 25461              |
| 2   | 9186                    | 6572                | 563                    | 9500               |
| 3   | 3765                    | 9697                | 1024                   | 11327              |
| 4   | 5718                    | 11988               | 8099                   | 0                  |

## Prioritário

O escalonador prioriotário apresentou os melhores resultados e, em virtude disso, será a estratégia em que mais cenários serão analisados. Primeiramente, configurou-se as variáveis INTERVALO_RELOGIO = 50 e INTERVALO_QUANTUM = 5, seguindo o padrão dos cenários anteriores.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 22797                   | 3340                | 58                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 454                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 1                         | 2270                  | 763                             |
| 2   | 39                        | 18170                 | 173                             |
| 3   | 13                        | 11451                 | 242                             |
| 4   | 5                         | 22398                 | 104                             |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 2                       | 2                   | 0                      | 1                  |
| 2   | 48                      | 48                  | 8                      | 1                  |
| 3   | 27                      | 27                  | 13                     | 1                  |
| 4   | 101                     | 101                 | 95                     | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 743                     | 1527                | 0                      | 20527              |
| 2   | 9210                    | 8339                | 621                    | 4244               |
| 3   | 3792                    | 6548                | 1111                   | 10955              |
| 4   | 5712                    | 10555               | 6131                   | 0                  |

Em seguida, realizou-se um teste, reduzindo o número de instruções a cada interrupção de relógio e aumentando proporcionalmente o número de interrupções para o tempo de quantum, objetivando a equiparação com o testes anteriores. Desse modo, defineu as variáveis INTERVALO_RELOGIO = 40 e INTERVALO_QUANTUM = 6.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 21634                   | 1868                | 65                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 539                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 2                         | 2930                  | 725                             |
| 2   | 43                        | 18702                 | 176                             |
| 3   | 14                        | 11271                 | 222                             |
| 4   | 6                         | 21229                 | 101                             |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 3                       | 3                   | 0                      | 1                  |
| 2   | 51                      | 51                  | 7                      | 1                  |
| 3   | 29                      | 29                  | 14                     | 1                  |
| 4   | 98                      | 98                  | 91                     | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 755                     | 2175                | 0                      | 18704              |
| 2   | 9390                    | 8979                | 333                    | 2543               |
| 3   | 3867                    | 6461                | 943                    | 9966               |
| 4   | 5754                    | 9963                | 5512                   | 0                  |

Por fim, reduziu-se também o número de interrupções para o tempo de quantum pela metade, mantendo o mesmo intervalo de relógio. Com isso, o teste foi realizado com INTERVALO_RELOGIO = 40 e INTERVALO_QUANTUM = 3.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 20229                   | 418                 | 124                       |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 504                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 3                         | 2930                  | 543                             |
| 2   | 79                        | 19840                 | 112                             |
| 3   | 30                        | 10591                 | 120                             |
| 4   | 12                        | 17864                 | 59                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 4                       | 4                   | 0                      | 1                  |
| 2   | 88                      | 88                  | 8                      | 1                  |
| 3   | 48                      | 48                  | 17                     | 1                  |
| 4   | 107                     | 107                 | 94                     | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 755                     | 2175                | 0                      | 17299              |
| 2   | 9441                    | 9905                | 494                    | 0                  |
| 3   | 3870                    | 5775                | 946                    | 9241               |
| 4   | 5745                    | 6400                | 5719                   | 1960               |

Essa última abordagem, embora tenha obtido o menor tempo de execução entre todos os cenários analisados, obteve também o maior o número de preempções. Isso pode degradar o desempenho em processadores modernos, visto que, uma preempção, significa limpar todos os estágios do pipeline. Portanto, vale ressaltar que cada abordagem apresenta suas vantagens e suas desvantagens e, para alcançar o melhor desempenho, o hardware e a composição dos processos também deve ser analisados minuciosamente.

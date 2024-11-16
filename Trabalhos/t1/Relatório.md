# Relatório

Todos os requisitos do trabalho foram implementados, abrangendo a criação e o gerenciamento de processos, o controle de estados, e diferentes estratégias de escalonamento. Para isso, criou-se inicialmente a estrutura `proc_t` a fim de armazenar as informações essenciais de cada processo, como registradores, estado e métricas. Em seguida, com o objetivo de isolar a lógica de seleção do próximo processo a ser executado, elaborou-se a estrutura `esc_t`, cuja responsabilidade é manter a fila de processo prontos para execução e aplicar o algoritmo de uma determinada estratégia de escalonamento. Essas estruturas interagem por meio da estrutura `so_t`, que controla a fila do escalonador com base nas mudanças de estado de cada um dos processos. Tal abordagem promoveu uma separação clara de responsabilidades, resultando em uma estrutura desacoplada e flexível, que permite múltiplos modos de escalonamento.

Foram implementadas três estratégias distintas de escalonamento, cada uma com características específicas que determinam como os processos são selecionados e gerenciados durante sua execução:

- **Simples:** Essa estratégia escolhe o primeiro processo pronto da fila e o executa até que o mesmo seja bloqueado ou finalize sua execução de forma natural. É a abordagem mais direta e não considera preempções ou prioridades. Por sua simplicidade, é útil em cenários de baixa carga de processos, em que a troca frequente de contexto não é necessária.

- **Circular:** Assim como na estratégia simples, o escalonador inicia executando o primeiro processo pronto. Entretanto, caso o tempo de execução de um processo exceda um intervalo pré-definido chamado quantum, ele é interrompido e movido para o final da fila, cedendo sua vez ao próximo processo pronto. Essa abordagem, também conhecida como Round-Robin e promove um compartilhamento equitativo de CPU entre os processos.

- **Prioritário:** Essa estratégia introduz um mecanismo dinâmico de atualização de prioridades. Quando um processo para de executar, seja porque foi bloqueado ou porque excedeu o tempo de quantum, sua prioridade é ajustada. Processos que se bloqueiam mais rapidamente recebem maior prioridade, incentivando a execução preferencial de tarefas que realizam operações rápidas. Isso torna a estratégia adequada para sistemas em que a redução do tempo de espera médio é uma prioridade, favorecendo processos curtos e interativos.


## Testes

Alguns testes foram realizados para avaliar o desempenho de cada estratégia em diferentes cenários, variando tanto o intervalo de cada interrupção de relógio quanto o número de interrupções necessário para o quantum de cada processo. As principais métricas coletadas durante os testes incluem o tempo total de execução do sistema, o número de interrupções de cada tipo e o tempo total de cada processo em cada estado. Tais informações foram medidas em termos de número de instruções da CPU. Os resultados obtidos serão apresentados a seguir, oferecendo uma visão clara das diferenças e implicações de cada abordagem.

### Simples

Os testes se iniciaram com o escalonador mais básico, o simples, utilizando a variável configuração de INTERVALO_RELOGIO = 50. O intervalo de INTERVALO_QUANTUM não faz diferença nesse escalonador. Observe que, como esperado, não ocorreu nenhuma preempção, uma vez que cada processo executou até seu bloqueio ou seu encerramento.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 27885                   | 8503                | 0                         |

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
| 1   | 0                         | 27885                 | 10                              |
| 2   | 0                         | 13414                 | 552                             |
| 3   | 0                         | 15775                 | 503                             |
| 4   | 0                         | 27167                 | 95                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 4                       | 4                   | 3                      | 1                  |
| 2   | 7                       | 7                   | 6                      | 1                  |
| 3   | 21                      | 21                  | 20                     | 1                  |
| 4   | 131                     | 131                 | 130                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 743                     | 43                  | 27099                  | 0                  |
| 2   | 9156                    | 3864                | 394                    | 14088              |
| 3   | 3774                    | 10575               | 1426                   | 11719              |
| 4   | 5709                    | 12449               | 9009                   | 319                |

Em seguida, realizou-se um teste com o INTERVALO_RELOGIO = 40. Isso porque, durante a execução dos processos, notou-se que, quando o último processo era bloqueado para escrita, a CPU ficava parada até a próxima interrupção de relógio, embora a tela já estivesse desbloqueada algumas instruções antes. Dessa forma, reduzindo o INTERVALO_RELOGIO, foi possível fazer com que a CPU reagisse mais rapidamente ao desbloqueio da tela.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 25546                   | 5906                | 0                         |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 637                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 0                         | 25546                 | 0                               |
| 2   | 0                         | 13646                 | 670                             |
| 3   | 0                         | 15901                 | 517                             |
| 4   | 0                         | 24816                 | 96                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 4                       | 4                   | 3                      | 1                  |
| 2   | 6                       | 6                   | 5                      | 1                  |
| 3   | 21                      | 21                  | 20                     | 1                  |
| 4   | 131                     | 131                 | 130                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 755                     | 0                   | 24791                  | 0                  |
| 2   | 9303                    | 4021                | 322                    | 11511              |
| 3   | 3825                    | 10858               | 1218                   | 9248               |
| 4   | 5757                    | 12579               | 6480                   | 325                |

Vale destacar que, embora a CPU executasse o SO mais frequentemente, essa abordagem diminuiu o tempo total de execução em 8,38% para o conjunto de processos analisados.

### Circular

O escalonador Round-Robin foi executado com INTERVALO_RELOGIO = 50 e INTERVALO_QUANTUM = 5. Veja que o tempo de execução foi similar ao do escalonsddor simples, mas todos os processos puderam ser executados de forma simultânea, garantindo a responsividade do sistema.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 24049                   | 4610                | 56                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 479                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 0                         | 24049                 | 0                               |
| 2   | 39                        | 17097                 | 152                             |
| 3   | 12                        | 11989                 | 284                             |
| 4   | 5                         | 23331                 | 96                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 3                       | 3                   | 2                      | 1                  |
| 2   | 48                      | 48                  | 8                      | 1                  |
| 3   | 26                      | 26                  | 13                     | 1                  |
| 4   | 110                     | 110                 | 104                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 743                     | 0                   | 23306                  | 0                  |
| 2   | 9198                    | 7307                | 592                    | 6569               |
| 3   | 3756                    | 7403                | 830                    | 11669              |
| 4   | 5742                    | 10610               | 6979                   | 319                |

### Prioritário

O escalonador prioriotário apresentou os melhores resultados e, em virtude disso, será a estratégia em que mais cenários serão analisados. Primeiramente, configurou-se as variáveis INTERVALO_RELOGIO = 50 e INTERVALO_QUANTUM = 5, seguindo o padrão dos cenários anteriores.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 22846                   | 3389                | 57                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 455                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 0                         | 22846                 | 0                               |
| 2   | 39                        | 17900                 | 169                             |
| 3   | 13                        | 11130                 | 230                             |
| 4   | 5                         | 22128                 | 100                             |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 3                       | 3                   | 2                      | 1                  |
| 2   | 48                      | 48                  | 8                      | 1                  |
| 3   | 27                      | 27                  | 13                     | 1                  |
| 4   | 102                     | 102                 | 96                     | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 743                     | 0                   | 22103                  | 0                  |
| 2   | 9204                    | 8129                | 567                    | 4563               |
| 3   | 3792                    | 6227                | 1111                   | 11325              |
| 4   | 5718                    | 10226               | 6184                   | 319                |

Em seguida, realizou-se um teste, reduzindo o número de instruções a cada interrupção de relógio e aumentando proporcionalmente o número de interrupções para o tempo de quantum, objetivando a equiparação com o testes anteriores. Desse modo, defineu as variáveis INTERVALO_RELOGIO = 40 e INTERVALO_QUANTUM = 6.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 21682                   | 1916                | 63                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 0                  |
| Chamada de sistema | 462                |
| E/S: relógio      | 540                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Resposta}}$ | $\overline{T_{\text{Retorno}}}$ | 
| --- | ------------------------- | --------------------- | ------------------------------- |
| 1   | 0                         | 21682                 | 0                               |
| 2   | 43                        | 18288                 | 166                             |
| 3   | 14                        | 10953                 | 211                             |
| 4   | 6                         | 20952                 | 98                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 3                       | 3                   | 2                      | 1                  |
| 2   | 51                      | 51                  | 7                      | 1                  |
| 3   | 29                      | 29                  | 14                     | 1                  |
| 4   | 98                      | 98                  | 91                     | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 755                     | 0                   | 20927                  | 0                  |
| 2   | 9390                    | 8474                | 424                    | 3005               |
| 3   | 3867                    | 6143                | 943                    | 10332              |
| 4   | 5754                    | 9692                | 5506                   | 325                |

Por fim, reduziu-se também o número de interrupções para o tempo de quantum pela metade, mantendo o mesmo intervalo de relógio. Com isso, o teste foi realizado com INTERVALO_RELOGIO = 40 e INTERVALO_QUANTUM = 3.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 20250                   | 442                 | 121                       |

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
| 1   | 0                         | 20250                 | 0                               |
| 2   | 80                        | 19522                 | 107                             |
| 3   | 29                        | 10092                 | 113                             |
| 4   | 12                        | 17626                 | 62                              |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 2                       | 2                   | 1                      | 1                  |
| 2   | 89                      | 89                  | 8                      | 1                  |
| 3   | 47                      | 47                  | 17                     | 1                  |
| 4   | 107                     | 107                 | 94                     | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 755                     | 0                   | 19495                  | 0                  |
| 2   | 9441                    | 9562                | 519                    | 339                |
| 3   | 3867                    | 5311                | 914                    | 9761               |
| 4   | 5745                    | 6692                | 5189                   | 2219               |

Essa última abordagem, embora tenha obtido o menor tempo de execução entre todos os cenários analisados, obteve também o maior o número de preempções. Isso pode degradar o desempenho em processadores modernos, visto que, uma preempção, significa limpar todos os estágios do pipeline. Portanto, vale ressaltar que cada abordagem apresenta suas vantagens e suas desvantagens e, para alcançar o melhor desempenho, o hardware e a composição dos processos também deve ser analisados minuciosamente.

# Relatório

## Introdução

Todos os requisitos do trabalho foram implementandos, desde o gerenciamento da memória virtual, até o controle do tempo de acesso ao disco e algoritmos de substituição de páginas. As alterações foram organizadas com base no trabalho 1, sendo criados os seguintes arquivos para encapsular a lógica de controle das memórias e das páginas:

- `agenda.[ch]`: controla a data de disponibilidade do disco.
- `alocmem.[ch]`: responsável por gerenciar o espaço livre da memória principal, permitindo a reserva e a liberação de quadros individuais.
- `alocswap.[ch]`: responsável por gerenciar o espaço livre do memória secundária (swap), tal como o `alocmem.[ch]`, mas somente permite a alocação de regiões contíguas de quadros.
- `filapag.[ch]`: responsável por manter a fila FIFO e escolher a página vítima para substituição de acordo com seu modo (FIFO ou Segunda Chance).

Tais estruturas são criadas internamente pelo SO, o qual orquestra as chamadas tanto para o acesso dos endereços virtuais, quanto para a resolução de falta de páginas. 

As seguintes estratégias para a escolha de páginas para escolha de páginas foram implementadas:

- **FIFO**: Todas as páginas na memória principal estão em um fila, na qual o primeiro item da fila é a página mais antiga, enquanto que o último é a página mais recente. Toda vez que o sistema operacional precisa escolher uma página para ser substituída, a primeira página dessa fila é escolhida.
- **Segunda Chance**: As páginas na memória principal são mantidas em uma fila circular, na qual as páginas são inseridas na mesma ordem em que a FIFO. Para encontrar a página para ser substituída, ele seleciona a primeira página da fila e examina o bit de acesso. Se ele for 0, essa página é a vítima. Se ele for 1, zera o bit e avança a fila, fazendo com que essa página vá para o fim da fila. Esse processo continua até que se encontre uma página para ser substituída. Note que isso garante que uma página irá ser encontrada, pois, quando o algoritmo percorrer toda a fila, todas as páginas estarão com o seu bit zerado.

O objetivo deste relatório é analisar o desempenho da memória virtual em cada um dos processos, avaliando não só o tempo de execução, mas também a quantidade de faltas de página resolvidas.

## Metodologia

Para avaliar o desempenho da memória virtual, os testes foram divididos em duas partes. Na primeira, analisar-se-á a influência do número de páginas na memória principal, ao passo que, na segunda, analisar-se-á a influência do número de palavras em cada página. Isso será realizado individualmente para cada tipo de algoritmo de substituição de página. O desempenho é medido a partir da coleta de métricas do sistema operacional durante a realização dos testes. Tais métricas incluem informações gerais do sistema operacional, como o tempo total de execução, e informações específicas de cada processo, como o tempo em cada estado, o número total de páginas e o número de faltas de página solucionadas. Observe que o tempo é medido em número de instruções da CPU.   

Os testes foram todos executados com o escalonador prioritário, configurando o intervalo de interrupção do relógio para 50 instruções e o intervalo de quantum para 5 interrupções do relógio. O motivo da escolha dessa escalonador foi o seu baixo tempo de resposta, assemelhando-se ao que ocorre em sistema operacionais reais, como identificado no trabalho 1. Além disso, o tempo escolhido para o acesso ao disco foi de 10 instruções.

## Resultados e Discussão

Nesta seção, serão abordados os resultados obtidos, variando tanto o número de páginas, quanto o tamanho de cada página.

### Variação de número de página

Os testes desta suíte se iniciaram com 92 páginas, deconsiderando as reservadas para o modo supervisor, pois foi o mínimo encontrado para que todas as páginas dos processos coubessem em memória principal, utilizando 10 palavras para o tamanho de página. 

#### FIFO

O primeiro teste, assim como mencionado anteriormente, iniciou-se com 92 páginas. Dessa forma, cada página é carregada da memória secundária uma única vez e não ocorre nenhuma substituição.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 24036                   | 4212                | 41                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 91                 |
| Chamada de sistema | 462                |
| E/S: relógio      | 480                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 24036                | 0                     |
| 2   | 33                        | 18339                | 116                   |
| 3   | 8                         | 11502                | 133                   |
| 4   | 0                         | 22681                | 81                    |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 20                      | 20                  | 19                     | 1                  |
| 2   | 67                      | 67                  | 33                     | 1                  |
| 3   | 46                      | 46                  | 37                     | 1                  |
| 4   | 123                     | 123                 | 122                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 798                     | 0                   | 23238                  | 0                  |
| 2   | 9310                    | 7783                | 1246                   | 4991               |
| 3   | 3901                    | 6124                | 1477                   | 11679              |
| 4   | 5815                    | 10051               | 6815                   | 447                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 17                    | 17                  |
| 2   | 25                    | 25                  |
| 3   | 25                    | 25                  |
| 4   | 25                    | 25                  |


Em seguida, foi reduzido o número de páginas para 41, ou seja, metade do número inicial. Nesse cenário, o número de faltas de páginas obteve um pequeno aumento e, consequentemente, o tempo de execução total, devido ao maior número de acessos ao disco. No entanto, note que, apesar da memória ter diminuído pela metade, o tempo de execução permanece quase o mesmo.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 25391                   | 5371                | 40                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 146                |
| Chamada de sistema | 462                |
| E/S: relógio      | 507                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 25391                | 0                     |
| 2   | 33                        | 18944                | 93                    |
| 3   | 7                         | 12937                | 103                   |
| 4   | 0                         | 23930                | 70                    |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 25                      | 25                  | 24                     | 1                  |
| 2   | 82                      | 82                  | 48                     | 1                  |
| 3   | 64                      | 64                  | 56                     | 1                  |
| 4   | 143                     | 143                 | 142                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 818                     | 0                   | 24573                  | 0                  |
| 2   | 9358                    | 7635                | 1951                   | 5741               |
| 3   | 3954                    | 6614                | 2369                   | 11599              |
| 4   | 5890                    | 10080               | 7960                   | 553                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 17                    | 22                  |
| 2   | 25                    | 40                  |
| 3   | 25                    | 42                  |
| 4   | 25                    | 43                  |

Por fim, a partir da observação da execução do sistema operacional, encontrou-se que o número de páginas que permite a execução dos programas corretamente com o algoritmo FIFO é 9. Isso corresponde a 190 palavras de memória total. Utilizando esse valor, o processador passa mais tempo ocioso, esperando pela disponibilidade dos dispositivos, do que executando os programas.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 67907                   | 40016               | 30                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 2200               |
| Chamada de sistema | 462                |
| E/S: relógio      | 1357               |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 67907                | 6                     |
| 2   | 30                        | 35095                | 10                    |
| 3   | 0                         | 40274                | 18                    |
| 4   | 0                         | 66396                | 8                     |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 30                      | 30                  | 29                     | 1                  |
| 2   | 524                     | 524                 | 493                    | 1                  |
| 3   | 617                     | 617                 | 616                    | 1                  |
| 4   | 1134                    | 1134                | 1133                   | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 834                     | 191                 | 66882                  | 0                  |
| 2   | 11113                   | 5321                | 18661                  | 32106              |
| 3   | 6139                    | 11108               | 23027                  | 26778              |
| 4   | 9805                    | 10157               | 46434                  | 603                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 17                    | 26                  |
| 2   | 25                    | 493                 |
| 3   | 25                    | 616                 |
| 4   | 25                    | 1066                |

#### Segunda Chance

Iniciamos o primeiro teste da mesma forma do que com o algoritmo FIFO, isto é, com 92 páginas para que todos as páginas possam estar presentes em memória principal. Nesse cenário, como o algoritmo de substituição não precisa ser executado, o desempenho será o mesmo.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 24036                   | 4212                | 41                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 91                 |
| Chamada de sistema | 462                |
| E/S: relógio      | 480                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 24036                | 0                     |
| 2   | 33                        | 18339                | 116                   |
| 3   | 8                         | 11502                | 133                   |
| 4   | 0                         | 22681                | 81                    |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 20                      | 20                  | 19                     | 1                  |
| 2   | 67                      | 67                  | 33                     | 1                  |
| 3   | 46                      | 46                  | 37                     | 1                  |
| 4   | 123                     | 123                 | 122                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 798                     | 0                   | 23238                  | 0                  |
| 2   | 9310                    | 7783                | 1246                   | 4991               |
| 3   | 3901                    | 6124                | 1477                   | 11679              |
| 4   | 5815                    | 10051               | 6815                   | 447                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 17                    | 17                  |
| 2   | 25                    | 25                  |
| 3   | 25                    | 25                  |
| 4   | 25                    | 25                  |

Reduzindo o número de páginas para 41, notamos que o algoritmo Segunda Chance consegue uma pequena melhora de desempenho em comparação com o algoritmo FIFO, visto que o número de falhas de página resolvidas diminuiu em 8,84% e o tempo total de execução do sistema em 0,41%.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 25285                   | 5308                | 40                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 133                |
| Chamada de sistema | 462                |
| E/S: relógio      | 505                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 25285                | 0                     |
| 2   | 33                        | 18376                | 86                    |
| 3   | 7                         | 13387                | 122                   |
| 4   | 0                         | 23824                | 73                    |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 25                      | 25                  | 24                     | 1                  |
| 2   | 84                      | 84                  | 50                     | 1                  |
| 3   | 58                      | 58                  | 50                     | 1                  |
| 4   | 138                     | 138                 | 137                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 818                     | 0                   | 24467                  | 0                  |
| 2   | 9367                    | 7230                | 1779                   | 6203               |
| 3   | 3939                    | 7098                | 2350                   | 11043              |
| 4   | 5853                    | 10164               | 7807                   | 553                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 17                    | 22                  |
| 2   | 25                    | 43                  |
| 3   | 25                    | 36                  |
| 4   | 25                    | 33                  |

Por fim, descobriu-se que 6 é o número mínimo de páginas que permite a execução correta de todos os processos com esse algoritmo, correspondendo à 160 palavras de memória total.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 103372                  | 67150               | 30                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 4306               |
| Chamada de sistema | 462                |
| E/S: relógio      | 2067               |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 103372               | 1                     |
| 2   | 30                        | 60966                | 6                     |
| 3   | 0                         | 73472                | 11                    |
| 4   | 0                         | 101714               | 6                     |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 37                      | 37                  | 36                     | 1                  |
| 2   | 1114                    | 1114                | 1083                   | 1                  |
| 3   | 1350                    | 1350                | 1349                   | 1                  |
| 4   | 1915                    | 1915                | 1914                   | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 854                     | 49                  | 102469                 | 0                  |
| 2   | 13431                   | 7029                | 40506                  | 41650              |
| 3   | 9047                    | 14952               | 49473                  | 28995              |
| 4   | 12890                   | 12799               | 76025                  | 650                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 17                    | 33                  |
| 2   | 25                    | 1083                |
| 3   | 25                    | 1349                |
| 4   | 25                    | 1844                |

### Variação do tamanho de página

A suíte de testes desta seção foi realizada com 300 palavras de memória total, o que significa 200 palavras de memória utilizável pelos programas. Desse modo, consideraram-se 40 páginas com 5 palavras cada para o primeiro experimento e 10 páginas com 20 palavras cada para o segundo experimento.

#### FIFO

O algoritmo FIFO com 40 páginas com 5 palavras cada obteve o seguinte desempenho:

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 35367                   | 13432               | 33                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 648                |
| Chamada de sistema | 462                |
| E/S: relógio      | 706                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 35367                | 0                     |
| 2   | 31                        | 18459                | 23                    |
| 3   | 2                         | 22414                | 43                    |
| 4   | 0                         | 33357                | 27                    |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 45                      | 45                  | 44                     | 1                  |
| 2   | 176                     | 176                 | 144                    | 1                  |
| 3   | 228                     | 228                 | 225                    | 1                  |
| 4   | 377                     | 377                 | 376                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 878                     | 20                  | 34469                  | 0                  |
| 2   | 9734                    | 4218                | 4507                   | 16002              |
| 3   | 4572                    | 9936                | 7906                   | 11848              |
| 4   | 6751                    | 10410               | 16196                  | 803                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 34                    | 42                  |
| 2   | 49                    | 140                 |
| 3   | 49                    | 210                 |
| 4   | 49                    | 259                 |

Aumentando o número de palavras por página para 20 e utilizando 10 páginas, notamos que o sistema resolve um número menor de faltas de páginas em comparação com o experimento anterior. Isso porque, devido ao menor número de páginas, o sistema perde menos tempo acessando o disco.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 34812                   | 12945               | 41                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 637                |
| Chamada de sistema | 462                |
| E/S: relógio      | 695                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 34812                | 5                     |
| 2   | 35                        | 21445                | 26                    |
| 3   | 3                         | 23530                | 43                    |
| 4   | 3                         | 33671                | 29                    |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 19                      | 19                  | 18                     | 1                  |
| 2   | 200                     | 200                 | 164                    | 1                  |
| 3   | 231                     | 231                 | 227                    | 1                  |
| 4   | 356                     | 356                 | 352                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 794                     | 98                  | 33920                  | 0                  |
| 2   | 9839                    | 5314                | 6292                   | 12776              |
| 3   | 4609                    | 9976                | 8945                   | 10571              |
| 4   | 6625                    | 10460               | 16586                  | 422                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 9                     | 15                  |
| 2   | 13                    | 164                 |
| 3   | 13                    | 217                 |
| 4   | 13                    | 241                 |

#### Segunda Chance

Realizando o mesmo teste com 40 páginas com 5 palavras cada, o algoritmo Segunda Chance obteve um melhor desempenho em comparação com o FIFO. Essa melhora ocorre devido ao melhor gerenciamento desse algoritmo das páginas que são utilizadas com mais frequência.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 31956                   | 10319               | 32                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 566                |
| Chamada de sistema | 462                |
| E/S: relógio      | 638                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 31956                | 4                     |
| 2   | 32                        | 19050                | 34                    |
| 3   | 0                         | 22749                | 43                    |
| 4   | 0                         | 29946                | 33                    |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 45                      | 45                  | 44                     | 1                  |
| 2   | 158                     | 158                 | 125                    | 1                  |
| 3   | 239                     | 239                 | 238                    | 1                  |
| 4   | 300                     | 300                 | 299                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 878                     | 186                 | 30892                  | 0                  |
| 2   | 9681                    | 5445                | 3924                   | 12000              |
| 3   | 4639                    | 10340               | 7770                   | 8102               |
| 4   | 6439                    | 10052               | 13455                  | 803                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 34                    | 42                  |
| 2   | 49                    | 120                 |
| 3   | 49                    | 223                 |
| 4   | 49                    | 184                 |

Essa melhora se torna menos significativa com a redução do número de páginas para 10 com 20 palavras cada, visto que o algoritmo perde a granularidade para identificar as páginas acessadas com mais frequência.

| $N_{\text{Processos}}$ | $T_{\text{Execução}}$ | $T_{\text{Ocioso}}$ | $N_{\text{Preempções}}$ | 
| ---------------------- | ----------------------- | ------------------- | ------------------------- |
| 4                      | 33670                   | 12352               | 41                        |

| IRQ                | $N_{\text{Vezes}}$ | 
| ------------------ | ------------------ |
| Reset              | 1                  |
| Erro de execução | 499                |
| Chamada de sistema | 462                |
| E/S: relógio      | 672                |
| E/S: teclado       | 0                  |
| E/S: console       | 0                  |

| PID | $N_{\text{Preempções}}$ | $T_{\text{Retorno}}$ | $T_{\text{Resposta}}$ | 
| --- | ------------------------- | -------------------- | --------------------- |
| 1   | 0                         | 33670                | 4                     |
| 2   | 35                        | 19034                | 29                    |
| 3   | 3                         | 21215                | 49                    |
| 4   | 3                         | 32529                | 35                    |

| PID | $N_{\text{Executando}}$ | $N_{\text{Pronto}}$ | $N_{\text{Bloqueado}}$ | $N_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 20                      | 20                  | 19                     | 1                  |
| 2   | 164                     | 164                 | 128                    | 1                  |
| 3   | 194                     | 194                 | 190                    | 1                  |
| 4   | 305                     | 305                 | 301                    | 1                  |

| PID | $T_{\text{Executando}}$ | $T_{\text{Pronto}}$ | $T_{\text{Bloqueado}}$ | $T_{\text{Morto}}$ | 
| --- | ----------------------- | ------------------- | ---------------------- | ------------------ |
| 1   | 798                     | 94                  | 32778                  | 0                  |
| 2   | 9681                    | 4758                | 4595                   | 14045              |
| 3   | 4413                    | 9616                | 7186                   | 11744              |
| 4   | 6426                    | 10900               | 15203                  | 422                |

| PID | $N_{\text{Páginas}}$ | $N_{\text{Falhas}}$ | 
| --- | --------------------- | ------------------- |
| 1   | 9                     | 16                  |
| 2   | 13                    | 126                 |
| 3   | 13                    | 177                 |
| 4   | 13                    | 180                 |

## Conclusão

Os experimentos demonstraram que o algoritmo Segunda Chance supera o FIFO em cenários de memória limitada, reduzindo faltas de página e melhorando o desempenho devido à sua capacidade de priorizar páginas mais acessadas. Além disso, o aumento do tamanho das páginas diminuiu as faltas de página para ambos os algoritmos, mas reduziu a granularidade do Segunda Chance, limitando sua eficácia. 
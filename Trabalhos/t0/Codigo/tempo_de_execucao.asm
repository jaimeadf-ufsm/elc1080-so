TECL DEFINE 4
TECL_OK DEFINE 5

TELA DEFINE 6
TELA_OK DEFINE 7

RELOGIO_INSTRUCOES DEFINE 16
RELOGIO_REAL DEFINE 17

    desv main

num_1 valor 1
num_10 valor 10

char_nl valor 10
char_esp valor ' '
char_0 valor '0'

msg_continuar string "Digite qualquer caractere para finalizar..."
msg_insts string "i"
msg_segs string "s"

esc_char_tmp espaco 1
esc_uint_tmp espaco 1
esc_uint_mult espaco 1

esc_str_tmp espaco 1

insts_inicio espaco 1
segs_inicio espaco 1

insts_fim espaco 1
segs_fim espaco 1

; função para ler um caractere do teclado em A
le_char espaco 1
le_char_loop_ok
    le TECL_OK
    desvz le_char_loop_ok

    le TECL

    ret le_char

; função para ler o número de instruções do relógio em A
le_insts espaco 1
    le RELOGIO_INSTRUCOES
    ret le_insts

; função para ler o número de segundos do relógio em A
le_segs espaco 1
    le RELOGIO_REAL
    ret le_segs

; função para escrever o caractere em A na tela
esc_char espaco 1
    armm esc_char_tmp

esc_char_loop_ok
    le TELA_OK
    desvz esc_char_loop_ok

    cargm esc_char_tmp
    escr TELA
    
    ret esc_char

; funçao para escrever o inteiro positivo em A na tela
esc_uint espaco 1
    armm esc_uint_tmp

    trax
    cargm num_1
    
    trax
    div num_10
    
    desv esc_uint_cond_exp

esc_uint_loop_exp
    trax
    mult num_10

    trax
    div num_10

esc_uint_cond_exp
    desvnz esc_uint_loop_exp

    trax

esc_uint_loop_esc
    armm esc_uint_mult

    cargm esc_uint_tmp

    div esc_uint_mult
    resto num_10

    soma char_0
    chama esc_char

    cargm esc_uint_mult
    div num_10

    desvnz esc_uint_loop_esc

    ret esc_uint

; função para escrever a string terminada em nulo apontada por A na tela
esc_str espaco 1
    armm esc_str_tmp

esc_str_loop
    trax

    cargx 0
    desvz esc_str_fim

    incx

    trax
    armm esc_str_tmp
    trax

    chama esc_char

    cargm esc_str_tmp
    desv esc_str_loop

esc_str_fim
    ret esc_str

; função principal do programa
main
    cargi msg_continuar
    chama esc_str

    chama le_insts
    armm insts_inicio

    chama le_segs
    armm segs_inicio

    chama le_char

    cargm char_nl
    chama esc_char

    chama le_insts
    armm insts_fim

    chama le_segs
    armm segs_fim

    cargm insts_fim
    sub insts_inicio

    chama esc_uint

    cargi msg_insts
    chama esc_str

    cargm char_esp
    chama esc_char

    cargm segs_fim
    sub segs_inicio

    chama esc_uint

    cargi msg_segs
    chama esc_str

    para



TELA DEFINE 6
TELA_OK DEFINE 7

ALEATORIO_NUMERO DEFINE 19

    desv main

num_1 valor 1
num_10 valor 10

char_esp valor ' '
char_0 valor '0'

esc_char_tmp espaco 1
esc_uint_tmp espaco 1
esc_uint_mult espaco 1

quantidade valor 10

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

esc_uint_loop_exp
    trax
    mult num_10

    trax
    div num_10

    desvnz esc_uint_loop_exp

    trax
    
    div num_10

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


gerar_num espaco 1
    le ALEATORIO_NUMERO
    ret gerar_num

main
    desv main_condicao

main_loop
    sub num_1
    armm quantidade

    chama gerar_num
    chama esc_uint

    cargm char_esp
    chama esc_char

main_condicao
    cargm quantidade
    desvp main_loop

main_fim
    para
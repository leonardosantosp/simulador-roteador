// gcc simulacao_roteador.c -o simulacao_roteador -lm
// ./simulacao_roteador
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

typedef struct {
    unsigned long int num_eventos;
    double tempo_anterior;
    double soma_areas;
} little;

double uniforme() {
    double u = rand() / ((double) RAND_MAX + 1);
    // Limitando u entre (0,1]
    u = 1.0 - u;
    return u;
}

double gera_tempo(double l) {
    return (-1.0 / l) * log(uniforme());
}

void inicia_little(little *n) {
    n->num_eventos = 0;
    n->soma_areas = 0.0;
    n->tempo_anterior = 0.0;
}

//gera tempo atendimento em segundos
double gera_tempo_atendimento(double l, double r){
    return l/r;
}

//gera tamanho dos pacotes em bytes
double gera_tamanho_pacote(){
    double value = uniforme();
    if(value < 0.5){
        return 550;
    }else if(value < 0.9){
        return 40;
        
    }else{
        return 1500;
    }
}

double calcula_capacidade_link(double taxa_chegada, double tamanho_medio_pacote, double ocupacao_desejada) {
    return (taxa_chegada * tamanho_medio_pacote) / ocupacao_desejada;
}

int get_left_child(int index){
    return 2 * index + 1;
}

int get_right_child(int index){
    return 2 * index + 2;
}

int get_parent(int index) {
    if (index == 0) {
        return -1; // Indica que o índice 0 (raiz) não tem pai
    }
    return (index - 1) / 2;
}

void swap(double heap[], int i, int j){
    double aux = heap[i];
    heap[i] = heap[j];
    heap[j] = aux;
}

double insere_heap(double heap[], int* size, double value){
    int index = *size;
    heap[index] = value;
    (*size)++;

    int current = index;
    while (current > 0 && heap[current] < heap[get_parent(current)]) {
        swap(heap, current, get_parent(current));
        current = get_parent(current); // Move para o pai
    }
}

// Remove a raiz da heap
double remove_root(double heap[], int* size) {
    double root_return;
    if (*size <= 0) {
        printf("Heap vazia! Nada para remover.\n");
        return -1;
    }else if(*size == 1){
        (*size)--;
        return heap[0];
    }

    root_return = heap[0];
    // Substitui a raiz pelo último elemento
    heap[0] = heap[*size - 1];
    (*size)--; // Diminui o tamanho da heap

    // Ajusta a heap para manter a propriedade
    int current = 0;

    while (1) {
        int left = get_left_child(current);
        int right = get_right_child(current);
        int smallest = current;

        // Encontra o menor entre o nó atual e seus filhos
        if (left < *size && heap[left] < heap[smallest]) {
            smallest = left;
        }
        if (right < *size && heap[right] < heap[smallest]) {
            smallest = right;
        }

        // Se o menor não for o nó atual, troca e continua
        if (smallest != current) {
            swap(heap, current, smallest);
            current = smallest; // Move para o próximo nível
        } else {
            break; // Heap já está ajustada
        }
    }
    return root_return;
}

int main() {
    double heap[1000];
    int size = 0;
    srand(30); // Inicializa a semente para a geração de números aleatórios
    int n = 0;
    FILE *in_file = fopen("", "w");
    double parametro_chegada = 100;
    
    
    printf("Escolha a ocupacao desejada:\n");
    printf("1 - Ocupacao = 0.60\n");
    printf("2 - Ocupacao = 0.80\n");
    printf("3 - Ocupacao = 0.95\n");
    printf("4 - Ocupacao = 0.99\n");
    scanf("%d", &n);
    
    double capacidade_link = 0.0;
    double media_tamanho_pacote = (550 * 0.5) + (40 * 0.4) + (1500 * 0.1); // em Bytes
    // Definindo as taxas de chegada e saída com base na ocupação
    switch (n) {
        case 1: // ocupacao = 0.60
            capacidade_link = calcula_capacidade_link(parametro_chegada, media_tamanho_pacote, 0.60);
            in_file = fopen("saida60.dat", "w");
            break;
        case 2: // ocupacao = 0.80
        capacidade_link = calcula_capacidade_link(parametro_chegada, media_tamanho_pacote, 0.80);
            in_file = fopen("saida80.dat", "w");
            break;
        case 3: // ocupacao = 0.95
        capacidade_link = calcula_capacidade_link(parametro_chegada, media_tamanho_pacote, 0.95);
            in_file = fopen("saida95.dat", "w");
            break;
        case 4: // ocupacao = 0.99
        capacidade_link = calcula_capacidade_link(parametro_chegada, media_tamanho_pacote, 0.99);
            in_file = fopen("saida99.dat", "w");
            break;
    }

    if (in_file == NULL) {
        printf("Error! Could not open file.\n");
        exit(-1);
    }

    double tempo_simulacao;
    printf("Informe o tempo de simulacao (s): ");
    scanf("%lf", &tempo_simulacao);

    

    double tempo_decorrido = 0.0;

    double tempo_chegada = gera_tempo(parametro_chegada);
    double tempo_saida = DBL_MAX;

    unsigned long int fila = 0;
    unsigned long int fila_max = 0;

    double soma_ocupacao = 0.0;

    little en;
    little ew_chegadas;
    little ew_saidas;
    inicia_little(&en);
    inicia_little(&ew_chegadas);
    inicia_little(&ew_saidas);

    double extrai_dados = 100.0;
    double excedente = 0.0;
    double excedente_ew = 0.0;

    double lambda_param = 0.0;
    double en_param = 0.0;
    double ew_param = 0.0;
    double erro_little = 0.0;
    double ocupacao = 0.0;

    insere_heap(heap, &size, tempo_chegada);
    insere_heap(heap, &size, extrai_dados);

    while (tempo_decorrido <= tempo_simulacao) {

        tempo_decorrido = remove_root(heap, &size);

        // Chegada de pacotes
        if (tempo_decorrido == tempo_chegada) {
            // Sistema está ocioso?
            if (!fila) {
                double tamanho_pacote = gera_tamanho_pacote();
                 // Ajuste da taxa de saída
                tempo_saida = tempo_decorrido + gera_tempo_atendimento(tamanho_pacote,capacidade_link);

                insere_heap(heap, &size, tempo_saida);

                if (tempo_saida <= extrai_dados) {
                    soma_ocupacao += tempo_saida - tempo_decorrido;
                } else {
                    soma_ocupacao += tempo_saida - tempo_decorrido;
                    excedente = tempo_saida - extrai_dados;
                }
            }

            fila++;
            fila_max = fila > fila_max ? fila : fila_max;
            tempo_chegada = tempo_decorrido + gera_tempo(parametro_chegada);

            insere_heap(heap, &size, tempo_chegada);
            // Little
            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos++;
            en.tempo_anterior = tempo_decorrido;

            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.num_eventos++;
            ew_chegadas.tempo_anterior = tempo_decorrido;

        // Saída de pacotes
        } else if (tempo_decorrido == tempo_saida) {
            fila--;
            tempo_saida = DBL_MAX;

            // Se há mais pacotes na fila
            if (fila) {
                double tamanho_pacote = gera_tamanho_pacote();
                 // Iniciar com pacote de 550 Bytes
                tempo_saida = tempo_decorrido + gera_tempo_atendimento(tamanho_pacote, capacidade_link);

                insere_heap(heap, &size, tempo_saida);

                if (tempo_saida <= extrai_dados) {
                    soma_ocupacao += tempo_saida - tempo_decorrido;
                } else {
                    soma_ocupacao += tempo_saida - tempo_decorrido;
                    excedente = tempo_saida - extrai_dados;
                }
            }

            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos--;
            en.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.num_eventos++;
            ew_saidas.tempo_anterior = tempo_decorrido;

        // Extrai dados
        } else {
            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.tempo_anterior = tempo_decorrido;

            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.tempo_anterior = tempo_decorrido;

            lambda_param = ew_chegadas.num_eventos / tempo_decorrido;
            en_param = (en.soma_areas / tempo_decorrido);
            ew_param = ((ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos);
            erro_little = en_param - lambda_param * ew_param;
            ocupacao = (soma_ocupacao - excedente) / tempo_decorrido;

            fprintf(in_file, "%lf ", tempo_decorrido);
            fprintf(in_file, "%lf ", ocupacao);
            fprintf(in_file, "%lf ", ew_param);
            fprintf(in_file, "%lf ", en_param);
            fprintf(in_file, "%lf ", erro_little);
            fprintf(in_file, "%lf \n", lambda_param);

            extrai_dados += 100.0;

            insere_heap(heap, &size, extrai_dados);
        }
    }

    ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
    ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;

    printf("\n==================================");
    printf("\nMaior tamanho de fila alcancado: %ld\n", fila_max);
    printf("Ocupacao: %lf\n", soma_ocupacao / extrai_dados);

    double en_final = en.soma_areas / tempo_decorrido;
    double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;

    double lambda = ew_chegadas.num_eventos / tempo_decorrido;

    printf("E[N]: %lf\n", en_final);
    printf("E[W]: %lf\n", ew_final);
    printf("Erro de Little: %lf\n", en_final - lambda * ew_final);

    fclose(in_file);

    return 0;
}
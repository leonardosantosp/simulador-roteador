#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

typedef struct {
    unsigned long int num_eventos;
    double tempo_anterior;
    double soma_areas;
} Estatisticas;

double gerar_uniforme() {
    return 1.0 - (rand() / ((double) RAND_MAX + 1)); // Limita u entre (0,1]
}

double gerar_tempo(double taxa_chegada) {
    return (-1.0 / taxa_chegada) * log(gerar_uniforme());
}

void inicializar_estatisticas(Estatisticas *e) {
    e->num_eventos = 0;
    e->soma_areas = 0.0;
    e->tempo_anterior = 0.0;
}

double gerar_tempo_atendimento(double tamanho_pacote, double capacidade_link) {
    return tamanho_pacote / capacidade_link;
}

double gerar_tamanho_pacote() {
    double valor = gerar_uniforme();
    if (valor < 0.5) {
        return 550;
    } else if (valor < 0.9) {
        return 40;
    } else {
        return 1500;
    }
}

double calcular_capacidade_link(double taxa_chegada, double tamanho_medio_pacote, double ocupacao_desejada) {
    return (taxa_chegada * tamanho_medio_pacote) / ocupacao_desejada;
}

// Funções para manipulação de heap (Min Heap)
int obter_filho_esquerdo(int indice) {
    return 2 * indice + 1;
}

int obter_filho_direito(int indice) {
    return 2 * indice + 2;
}

int obter_pai(int indice) {
    return (indice == 0) ? 0 : (indice - 1) / 2;
}

void trocar(double heap[], int i, int j) {
    double aux = heap[i];
    heap[i] = heap[j];
    heap[j] = aux;
}

void inserir_heap(double heap[], int *tamanho, double valor) {
    int indice = *tamanho;
    heap[indice] = valor;
    (*tamanho)++;
    
    int atual = indice;
    while (atual > 0 && heap[atual] < heap[obter_pai(atual)]) {
        trocar(heap, atual, obter_pai(atual));
        atual = obter_pai(atual);
    }
}

double remover_raiz(double heap[], int *tamanho) {
    if (*tamanho <= 0) {
        printf("Heap vazia! Nada para remover.\n");
        return -1;
    } else if (*tamanho == 1) {
        (*tamanho)--;
        return heap[0];
    }

    double raiz = heap[0];
    heap[0] = heap[*tamanho - 1];
    (*tamanho)--;

    int atual = 0;
    while (1) {
        int esquerdo = obter_filho_esquerdo(atual);
        int direito = obter_filho_direito(atual);
        int menor = atual;

        if (esquerdo < *tamanho && heap[esquerdo] < heap[menor]) {
            menor = esquerdo;
        }
        if (direito < *tamanho && heap[direito] < heap[menor]) {
            menor = direito;
        }

        if (menor != atual) {
            trocar(heap, atual, menor);
            atual = menor;
        } else {
            break;
        }
    }
    return raiz;
}

int main() {
    srand(30); // Inicializa semente para geração de números aleatórios

    double heap[1000];
    int tamanho_heap = 0;
    int opcao = 0;
    double parametro_chegada = 100;
    double capacidade_link = 0.0;
    double media_tamanho_pacote = (550 * 0.5) + (40 * 0.4) + (1500 * 0.1);

    FILE *arquivo_saida = fopen("", "w");

    printf("Escolha a ocupacao desejada:\n");
    printf("1 - Ocupacao = 0.60\n");
    printf("2 - Ocupacao = 0.80\n");
    printf("3 - Ocupacao = 0.95\n");
    printf("4 - Ocupacao = 0.99\n");
    scanf("%d", &opcao);
    
    // Calcula a capacidade do link conforme a ocupação escolhida
    switch (opcao) {
        case 1:// 60%
            capacidade_link = calcular_capacidade_link(parametro_chegada, media_tamanho_pacote, 0.60);
            arquivo_saida = fopen("saida60.dat", "w");
            break;
        case 2:// 80%
            capacidade_link = calcular_capacidade_link(parametro_chegada, media_tamanho_pacote, 0.80);
            arquivo_saida = fopen("saida80.dat", "w");
            break;
        case 3:// 95%
            capacidade_link = calcular_capacidade_link(parametro_chegada, media_tamanho_pacote, 0.95);
            arquivo_saida = fopen("saida95.dat", "w");
            break;
        case 4:// 99%
            capacidade_link = calcular_capacidade_link(parametro_chegada, media_tamanho_pacote, 0.99);
            arquivo_saida = fopen("saida99.dat", "w");
            break;
        default:
            printf("Opcao invalida!\n");
            fclose(arquivo_saida);
            return -1;
    }

    double tempo_simulacao;
    double tempo_decorrido = 0.0;
    double tempo_chegada = gerar_tempo(parametro_chegada);
    double tempo_saida = DBL_MAX;
    double soma_ocupacao = 0.0;

    unsigned long int fila = 0;
    unsigned long int fila_max = 0;

    printf("Informe o tempo de simulacao (s): ");
    scanf("%lf", &tempo_simulacao);

    Estatisticas en, ew_chegadas, ew_saidas;
    inicializar_estatisticas(&en);
    inicializar_estatisticas(&ew_chegadas);
    inicializar_estatisticas(&ew_saidas);

    double extrai_dados = 100.0;
    double excedente = 0.0;
    double excedente_ew = 0.0;
    double lambda_param = 0.0;
    double en_param = 0.0;
    double ew_param = 0.0;
    double erro_little = 0.0;
    double ocupacao = 0.0;

    // Inserir chegada e extração de dados na heap
    inserir_heap(heap, &tamanho_heap, tempo_chegada);
    inserir_heap(heap, &tamanho_heap, extrai_dados);

    while (tempo_decorrido <= tempo_simulacao) {

        tempo_decorrido = remover_raiz(heap, &tamanho_heap);

        if (tempo_decorrido == tempo_chegada) { // Chegada de pacotes
            if (!fila) { // Sistema ocioso?
                double tamanho_pacote = gerar_tamanho_pacote();
                tempo_saida = tempo_decorrido + gerar_tempo_atendimento(tamanho_pacote, capacidade_link);
                inserir_heap(heap, &tamanho_heap, tempo_saida);

                soma_ocupacao += tempo_saida - tempo_decorrido;
            }

            fila++;
            fila_max = fila > fila_max ? fila : fila_max;
            tempo_chegada = tempo_decorrido + gerar_tempo(parametro_chegada);
            inserir_heap(heap, &tamanho_heap, tempo_chegada);

            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos++;
            en.tempo_anterior = tempo_decorrido;

            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.num_eventos++;
            ew_chegadas.tempo_anterior = tempo_decorrido;

        } else if (tempo_decorrido == tempo_saida) { // Saída de pacotes
            fila--;
            tempo_saida = DBL_MAX;

            if (fila) {
                double tamanho_pacote = gerar_tamanho_pacote();
                tempo_saida = tempo_decorrido + gerar_tempo_atendimento(tamanho_pacote, capacidade_link);
                inserir_heap(heap, &tamanho_heap, tempo_saida);

                soma_ocupacao += tempo_saida - tempo_decorrido;
            }

            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos--;
            en.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.num_eventos++;
            ew_saidas.tempo_anterior = tempo_decorrido;

        } else { // Extrai dados
            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.tempo_anterior = tempo_decorrido;

            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.tempo_anterior = tempo_decorrido;

            lambda_param = ew_chegadas.num_eventos / tempo_decorrido;
            en_param = en.soma_areas / tempo_decorrido;
            ew_param = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;
            erro_little = en_param - lambda_param * ew_param;
            ocupacao = (soma_ocupacao - excedente) / tempo_decorrido;

            fprintf(arquivo_saida, "%lf %lf %lf %lf %lf %lf\n", tempo_decorrido, ocupacao, ew_param, en_param, erro_little, lambda_param);

            extrai_dados += 100.0;
            inserir_heap(heap, &tamanho_heap, extrai_dados);
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

    fclose(arquivo_saida);

    return 0;
}

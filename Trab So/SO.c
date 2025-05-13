#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_LINHA 1024
#define MAX_LINHAS 1000
#define MAX_FILHOS 10

// Função para ler todas as linhas do arquivo de entrada
int ler_linhas(const char *nome_arquivo, char linhas[MAX_LINHAS][MAX_LINHA]) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        exit(1);
    }

    int total = 0;
    while (fgets(linhas[total], MAX_LINHA, arquivo) != NULL) {
        linhas[total][strcspn(linhas[total], "\n")] = '\0'; // remove o '\n'
        total++;
    }

    fclose(arquivo);
    return total;
}

int main() {
    char linhas[MAX_LINHAS][MAX_LINHA];
    int total_linhas = ler_linhas("entrada.txt", linhas);

    int num_processos = 4;  // número de filhos
    int linhas_por_filho = (total_linhas + num_processos - 1) / num_processos;

    int pipes[num_processos][2];

    for (int i = 0; i < num_processos; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("Erro ao criar pipe");
            exit(1);
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("Erro no fork");
            exit(1);
        } else if (pid == 0) {
            // Processo filho
            close(pipes[i][0]);  // fecha leitura

            int inicio = i * linhas_por_filho;
            int fim = (i + 1) * linhas_por_filho;
            if (fim > total_linhas) fim = total_linhas;

            for (int j = inicio; j < fim; j++) {
                char resultado[MAX_LINHA];
                snprintf(resultado, sizeof(resultado), "[Filho %d] %s\n", i, linhas[j]);
                write(pipes[i][1], resultado, strlen(resultado));
            }

            close(pipes[i][1]);
            exit(0);
        } else {
            // Processo pai
            close(pipes[i][1]);  // fecha escrita
        }
    }

    // Pai lê dos pipes e escreve no arquivo de saída
    FILE *saida = fopen("saida.txt", "w");
    if (!saida) {
        perror("Erro ao abrir arquivo de saída");
        exit(1);
    }

    for (int i = 0; i < num_processos; i++) {
        char buffer[MAX_LINHA];
        int n;

        while ((n = read(pipes[i][0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[n] = '\0';
            fputs(buffer, saida);
        }

        close(pipes[i][0]);
        wait(NULL);  // espera o filho terminar
    }

    fclose(saida);

    printf("Processamento concluído. Resultado em 'saida.txt'\n");
    return 0;
}


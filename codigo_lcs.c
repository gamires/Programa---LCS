#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_FILE1 "string1.txt"
#define DEFAULT_FILE2 "string2.txt"

// Lê todo o conteúdo do arquivo e retorna uma string alocada (free() após usar).
// Usa fseek+ftell para descobrir o tamanho e alocar.
char* lerStringArquivo(const char* nomeArquivo) {
    FILE* f = fopen(nomeArquivo, "rb"); // modo binário seguro para qualquer conteúdo
    if (f == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo %s\n", nomeArquivo);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "Erro ao posicionar no fim do arquivo %s\n", nomeArquivo);
        fclose(f);
        return NULL;
    }
    long sz = ftell(f);
    if (sz < 0) {
        fprintf(stderr, "Erro ao obter tamanho do arquivo %s\n", nomeArquivo);
        fclose(f);
        return NULL;
    }
    rewind(f);

    // Aloca +1 para terminador
    char* buf = (char*)malloc((size_t)sz + 1);
    if (buf == NULL) {
        fprintf(stderr, "Erro de alocacao de memoria (%ld bytes)\n", sz + 1);
        fclose(f);
        return NULL;
    }

    size_t read = fread(buf, 1, (size_t)sz, f);
    if (read != (size_t)sz) {
        // Pode ser problema, mas ainda assim terminamos a string com o que lemos
        // Avisamos, mas não falhamos imediatamente
        // fprintf(stderr, "Aviso: nao foi possivel ler todo o arquivo %s\n", nomeArquivo);
    }
    buf[read] = '\0';

    // Se houver uma quebra de linha no final, remover (opcional)
    if (read > 0 && (buf[read-1] == '\n' || buf[read-1] == '\r')) {
        // remove todas quebras finais \r or \n
        long end = (long)read - 1;
        while (end >= 0 && (buf[end] == '\n' || buf[end] == '\r')) {
            buf[end] = '\0';
            end--;
        }
    }

    fclose(f);
    return buf;
}

// Aloca matriz int (linhas x colunas)
int** alocarMatrizInt(int linhas, int colunas) {
    int** m = (int**)malloc(linhas * sizeof(int*));
    if (!m) return NULL;
    for (int i = 0; i < linhas; ++i) {
        m[i] = (int*)calloc(colunas, sizeof(int)); // já zera
        if (!m[i]) {
            // libera os anteriores
            for (int k = 0; k < i; ++k) free(m[k]);
            free(m);
            return NULL;
        }
    }
    return m;
}

void liberarMatrizInt(int** m, int linhas) {
    if (!m) return;
    for (int i = 0; i < linhas; ++i) free(m[i]);
    free(m);
}

int calcularComprimentoLCS(const char* X, const char* Y, int m, int n, int** dp) {
    // dp já alocado com (m+1) x (n+1)
    for (int i = 0; i <= m; ++i) dp[i][0] = 0;
    for (int j = 0; j <= n; ++j) dp[0][j] = 0;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (X[i-1] == Y[j-1]) dp[i][j] = dp[i-1][j-1] + 1;
            else dp[i][j] = (dp[i-1][j] > dp[i][j-1]) ? dp[i-1][j] : dp[i][j-1];
        }
    }
    return dp[m][n];
}

char* reconstruirLCS(const char* X, const char* Y, int m, int n, int** dp) {
    int len = dp[m][n];
    char* lcs = (char*)malloc((size_t)len + 1);
    if (!lcs) return NULL;
    lcs[len] = '\0';
    int i = m, j = n;
    int idx = len - 1;
    while (i > 0 && j > 0) {
        if (X[i-1] == Y[j-1]) {
            lcs[idx--] = X[i-1];
            i--; j--;
        } else if (dp[i-1][j] > dp[i][j-1]) {
            i--;
        } else j--;
    }
    return lcs;
}

int main(int argc, char* argv[]) {
    const char* file1 = (argc >= 2) ? argv[1] : DEFAULT_FILE1;
    const char* file2 = (argc >= 3) ? argv[2] : DEFAULT_FILE2;

    char* s1 = lerStringArquivo(file1);
    char* s2 = lerStringArquivo(file2);
    if (!s1 || !s2) {
        fprintf(stderr, "Erro ao ler os arquivos de entrada.\n");
        free(s1); free(s2);
        return 1;
    }

    int m = (int)strlen(s1);
    int n = (int)strlen(s2);
    printf("String1 (%s) tamanho=%d\n", file1, m);
    printf("String2 (%s) tamanho=%d\n", file2, n);

    int** dp = alocarMatrizInt(m+1, n+1);
    if (!dp) {
        fprintf(stderr, "Erro ao alocar a matriz dp\n");
        free(s1); free(s2);
        return 1;
    }

    int len = calcularComprimentoLCS(s1, s2, m, n, dp);
    char* lcs = reconstruirLCS(s1, s2, m, n, dp);

    printf("\n=== RESULTADO ===\n");
    printf("Comprimento da LCS: %d\n", len);
    if (lcs) printf("LCS: %s\n", lcs);
    else printf("LCS: (erro ao alocar)\n");

    free(lcs);
    free(s1); free(s2);
    liberarMatrizInt(dp, m+1);

    return 0;
}

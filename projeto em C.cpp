#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<locale.h>

typedef struct {
    char nome[50];
    char tel[20];
    char email[50];
    char status;
} reg;

void configurar_locale(void);
void limpa_buffer(void);
void ler_string(char *s, int tam);
int tamanho(FILE *arq);
void cadastrar(FILE *arq);
void consultar(FILE *arq);
void gerar_arquivo_texto(FILE *arq);
void excluir(FILE *arq);

void limpa_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void ler_string(char *s, int tam) {

    fgets(s, tam, stdin);
    s[strcspn(s, "\n")] = '\0';
}

int tamanho(FILE *arq) {
    long pos = ftell(arq);
    fseek(arq, 0, SEEK_END);
    long fim = ftell(arq);
    fseek(arq, pos, SEEK_SET);
    
    return (int)(fim / sizeof(reg));
}

void cadastrar(FILE *arq) {
    reg contato;
    contato.status = ' ';
    char confirma;

    printf("\n=== CRIAR UMA CONTA ===\n");
    printf("Registro número: %d\n", tamanho(arq) + 1);

    printf("Nome: ");
    ler_string(contato.nome, sizeof(contato.nome));
    
    printf("Telefone: ");
    ler_string(contato.tel, sizeof(contato.tel));
    
    printf("E-mail: ");
    ler_string(contato.email, sizeof(contato.email));
    
    printf("Confirmar a conta (s/n)? ");
    if (scanf("%c", &confirma) != 1) { 
        printf("Entrada inválida. Cancelando a conta.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (toupper(confirma) == 'S') {
        fseek(arq, 0, SEEK_END);
        fwrite(&contato, sizeof(reg), 1, arq);
        fflush(arq);
        printf("Contato salvo com sucesso!\n");
    } else {
        printf("Conta cancelado.\n");
    }
}

void consultar(FILE *arq) {
    int nr;
    reg contato;

    printf("\nInforme o código do contato: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    int total = tamanho(arq);
    printf("[DEBUG] Total de registros no arquivo: %d\n", total);

    if (nr <= 0 || nr > total) {
        printf("Código inválido! Total = %d\n", total);
        return;
    }

    long pos = (long)(nr - 1) * sizeof(reg);

    printf("[DEBUG] Posição em bytes para fseek: %ld\n", pos);

    if (fseek(arq, pos, SEEK_SET) != 0) {
        printf("[ERRO] Falha no fseek.\n");
        return;
    }
    printf("[DEBUG] fseek OK\n");

    size_t lido = fread(&contato, sizeof(reg), 1, arq);

    if (lido != 1) {
        printf("[ERRO] Falha ao ler registro! fread retornou: %zu\n", lido);
        return;
    }

    printf("[DEBUG] status lido = '%c'\n", contato.status);

    printf("\n=== CONTATO (CÓDIGO %d) ===\n", nr);
    if (contato.status == '*') {
        printf("Status: EXCLUÍDO LOGICAMENTE\n");
    }

    printf("Nome.....: %s\n", contato.nome);
    printf("Telefone.: %s\n", contato.tel);
    printf("E-mail...: %s\n", contato.email);
}

void gerar_arquivo_texto(FILE *arq) {
    char nomearq[80];
    reg contato;
    int i;
    int total;
    char status_str[12];

    printf("\nGerar Arquivo Texto\n");
    printf("Nome do arquivo (sem extensão): ");
    ler_string(nomearq, sizeof(nomearq));
    strcat(nomearq, ".txt");

    FILE *arqtxt = fopen(nomearq, "w");

    if (!arqtxt) {
        printf("Erro ao criar arquivo texto.\n");
        return;
    }

    fprintf(arqtxt, "RELATÓRIO COMPLETO DE CONTATOS\n\n");
    fprintf(arqtxt, "COD  %-25s %-15s %-25s STATUS\n",
            "NOME", "TELEFONE", "EMAIL");
    fprintf(arqtxt, "-------------------------------------------------------------------------------------------\n");

    total = tamanho(arq);

    for (i = 0; i < total; i++) {
        fseek(arq, i * sizeof(reg), SEEK_SET);
        fread(&contato, sizeof(reg), 1, arq);
        
        if (contato.status == '*')
            strcpy(status_str, "EXCLUIDO");
        else
            strcpy(status_str, "ATIVO");

        fprintf(arqtxt, "%03d %-25s %-15s %-25s %s\n",
                i + 1,
                contato.nome,
                contato.tel,
                contato.email,
                status_str);
    }

    fclose(arqtxt);
    printf("\nArquivo '%s' gerado com sucesso!\n", nomearq);
}

void excluir(FILE *arq) {
    int nr;
    char confirma;
    reg contato;

    printf("\nInforme o código da conta para excluir: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    int total = tamanho(arq);
    if (nr <= 0 || nr > total) {
        printf("Código inválido.\n");
        return;
    }

    long pos_byte = (long)(nr - 1) * sizeof(reg);
    fseek(arq, pos_byte, SEEK_SET);
    fread(&contato, sizeof(reg), 1, arq);

    if (contato.status == '*') {
        printf("A conta já estava excluída.\n");
        return;
    }

    printf("\nConfirmar exclusão (s/n)? ");
    if (scanf("%c", &confirma) != 1) {
        printf("Entrada inválida. Cancelando exclusão.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (toupper(confirma) == 'S') {
        contato.status = '*';
        
        fseek(arq, pos_byte, SEEK_SET);
        fwrite(&contato, sizeof(reg), 1, arq);
        fflush(arq);
        
        printf("Registro excluído com sucesso!\n");
    } else {
        printf("Exclusão cancelada.\n");
    }
}

void configurar_locale(void) {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    const char *locais[] = {
        "pt_BR.UTF-8",
        "pt_BR.utf8",
        "Portuguese_Brazil.1252",
        "Portuguese",
        ""
    };
    int i; 
    for (i = 0; i < 5; i++) {
        const char *r = setlocale(LC_ALL, locais[i]);
        if (r != NULL) {
            printf("Locale ativo: %s\n", r);
            return;
        }
    }
    printf("Aviso: Locale não pôde ser configurado.\n");
}

int main(void) {
    configurar_locale();
    
    FILE *arq = fopen("dados1.dat", "r+b"); 
    
    if (!arq) {
        arq = fopen("dados1.dat", "w+b");
        if (!arq) {
            printf("Erro crítico ao abrir/criar arquivo de dados.\n");
            return 1;
        }
    }

    int op;
    do {
        printf("\n========= MENU INICIAL =========\n");
        printf("1. Criar uma conta\n");
        printf("2. Consultar\n");
        printf("3. Gerar arquivo texto\n");
        printf("4. Deletar a conta\n");
        printf("5. Sair\n");
        printf("--------------------------------------\n");
        printf("Total de registros: %d\n", tamanho(arq));
        printf("Opção: ");
        
        if (scanf("%d", &op) != 1) {
            printf("Digite um número válido.\n");
            limpa_buffer(); 
            continue;
        }
        limpa_buffer();

        switch (op) {
            case 1: cadastrar(arq); break;
            case 2: consultar(arq); break;
            case 3: gerar_arquivo_texto(arq); break;
            case 4: excluir(arq); break;
            case 5: printf("Fechando arquivo e saindo...\n"); break;
            default: printf("Opção inválida!\n");
        }
    } while (op != 5);

    fclose(arq);
    return 0;
}



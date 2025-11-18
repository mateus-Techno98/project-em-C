/*
===================================================================================
                PROJETO FINAL — AGENDA DE CONTATOS (C PURO)
-----------------------------------------------------------------------------------
VERSÃO DIDÁTICA 2025 — COMENTADA LINHA POR LINHA

Correção de compatibilidade:
--------------------------
• As variáveis de loop (como 'i') foram movidas para o início das funções,
  garantindo a compatibilidade com o padrão C89 (ANSI C).

====================================================================================
*/

#include <stdio.h>    // Funções de I/O (printf, scanf, FILE*, etc.)
#include <stdlib.h>   // Para a função system e EXIT_FAILURE
#include <string.h>   // Para as funções de manipulação de strings (strcat, strcspn, strcpy)
#include <ctype.h>    // Para a função toupper (converter para maiúscula)
#include <locale.h>   // Para setlocale (configuração de acentos e caracteres)

/* -------------------------------------------------------------------------------
   ESTRUTURA DE DADOS
   Define o formato EXATO de cada registro salvo no arquivo binário.
   O arquivo armazena múltiplos blocos do tamanho de 'reg'.
-------------------------------------------------------------------------------- */
typedef struct {
    char nome[50];
    char tel[20];
    char email[50];
    char status;    // 1 byte
    /*
    status = ' '  -> registro ativo
    status = '*'  -> registro excluído (exclusão lógica)
    */
} reg;

/* Protótipos das funções */
void configurar_locale(void);
void limpa_buffer(void);
void ler_string(char *s, int tam);
int tamanho(FILE *arq);
void cadastrar(FILE *arq);
void consultar(FILE *arq);
void gerar_arquivo_texto(FILE *arq);
void excluir(FILE *arq);

/* ====================================================================================
   limpa_buffer
   ------------------------------------------------------------------------------------
   FUNÇÃO ESSENCIAL para evitar a leitura do '\n' (Enter) deixado pelo scanf().
   Garante que a próxima função de leitura de string (fgets) funcione corretamente.
   ==================================================================================== */
void limpa_buffer(void) {
    int c;
    // Lê e descarta caracteres até encontrar o '\n' ou o fim do arquivo (EOF).
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ====================================================================================
   ler_string
   ------------------------------------------------------------------------------------
   Função auxiliar para leitura SEGURA de strings (evita Buffer Overflow).
   ==================================================================================== */
void ler_string(char *s, int tam) {
    // 1. fgets lê a string da entrada padrão (stdin), respeitando o limite 'tam'.
    fgets(s, tam, stdin);
    
    // 2. strcspn encontra a posição do primeiro '\n' na string.
    // 3. O '\n' é substituído por '\0' (terminador nulo), "limpando" a quebra de linha.
    s[strcspn(s, "\n")] = '\0';
}

/* ====================================================================================
   tamanho
   ------------------------------------------------------------------------------------
   Calcula o número total de registros no arquivo.
   ==================================================================================== */
int tamanho(FILE *arq) {
    long pos = ftell(arq);        // Salva a posição atual
    fseek(arq, 0, SEEK_END);      // Move o ponteiro para o final do arquivo
    long fim = ftell(arq);        // Obtém o tamanho total em bytes
    fseek(arq, pos, SEEK_SET);    // Restaura o ponteiro para a posição original
    
    // Retorna o número de registros (tamanho total / tamanho do struct)
    return (int)(fim / sizeof(reg));
}

/* ====================================================================================
   cadastrar
   ------------------------------------------------------------------------------------
   Adiciona um novo registro ao final do arquivo, utilizando a função ler_string segura.
   ==================================================================================== */
void cadastrar(FILE *arq) {
    reg contato;
    contato.status = ' ';  // Novo registro é sempre ativo
    char confirma;

    printf("\n=== CADASTRAR CONTATO ===\n");
    printf("Registro número: %d\n", tamanho(arq) + 1);

    // Leitura das strings usando a função segura ler_string()
    printf("Nome: ");
    ler_string(contato.nome, sizeof(contato.nome)); // Usa sizeof() para garantir limite
    
    printf("Telefone: ");
    ler_string(contato.tel, sizeof(contato.tel));
    
    printf("E-mail: ");
    ler_string(contato.email, sizeof(contato.email));
    
    printf("Confirmar cadastro (s/n)? ");
    // O scanf com %c é usado para ler apenas o caractere de confirmação
    if (scanf("%c", &confirma) != 1) { 
        printf("Entrada inválida. Cancelando cadastro.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer(); // Limpa o buffer após o scanf

    if (toupper(confirma) == 'S') {
        fseek(arq, 0, SEEK_END);                       // Posiciona no fim (para APPEND)
        fwrite(&contato, sizeof(reg), 1, arq);         // Grava 1 bloco de 'reg'
        fflush(arq);                                   // Força a gravação imediata
        printf("Contato salvo com sucesso!\n");
    } else {
        printf("Cadastro cancelado.\n");
    }
}

/* ====================================================================================
   consultar (versão com DEBUG didático)
   ------------------------------------------------------------------------------------
   Exibe um registro pelo seu código, demonstrando o Acesso Direto (fseek).
   ==================================================================================== */
void consultar(FILE *arq) {
    int nr;
    reg contato;

    printf("\nInforme o código do contato: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }
    limpa_buffer(); // Limpeza após scanf

    int total = tamanho(arq);
    printf("[DEBUG] Total de registros no arquivo: %d\n", total);

    if (nr <= 0 || nr > total) {
        printf("Código inválido! Total = %d\n", total);
        return;
    }

    // Calcula a posição em bytes: (código - 1) * tamanho_do_registro
    long pos = (long)(nr - 1) * sizeof(reg);

    printf("[DEBUG] Posição em bytes para fseek: %ld\n", pos);

    // Move o ponteiro para a posição exata (SEEK_SET = Início do arquivo)
    if (fseek(arq, pos, SEEK_SET) != 0) {
        printf("[ERRO] Falha no fseek.\n");
        return;
    }
    printf("[DEBUG] fseek OK\n");

    // Lê 1 bloco de 'reg' para a variável 'contato'
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

/* ====================================================================================
   gerar_arquivo_texto (CORRIGIDO PARA C89)
   ------------------------------------------------------------------------------------
   Gera um relatório (.txt) formatado, lendo sequencialmente os registros binários.
   ==================================================================================== */
void gerar_arquivo_texto(FILE *arq) {
    char nomearq[80];
    reg contato;
    // CORREÇÃO C89: Variáveis declaradas no início da função
    int i;
    int total;
    char status_str[12]; // Buffer para armazenar "ATIVO" ou "EXCLUIDO"

    printf("\nGerar Arquivo Texto\n");
    printf("Nome do arquivo (sem extensão): ");
    ler_string(nomearq, sizeof(nomearq));
    strcat(nomearq, ".txt"); // Adiciona a extensão ".txt"

    FILE *arqtxt = fopen(nomearq, "w"); // Abre no modo escrita de texto

    if (!arqtxt) {
        printf("Erro ao criar arquivo texto.\n");
        return;
    }

    // Impressão do cabeçalho no arquivo .txt
    fprintf(arqtxt, "RELATÓRIO COMPLETO DE CONTATOS\n\n");
    fprintf(arqtxt, "COD  %-25s %-15s %-25s STATUS\n",
            "NOME", "TELEFONE", "EMAIL");
    fprintf(arqtxt, "-------------------------------------------------------------------------------------------\n");

    total = tamanho(arq);

    // Loop de iteração sequencial (COMPATÍVEL C89)
    for (i = 0; i < total; i++) {
        fseek(arq, i * sizeof(reg), SEEK_SET);
        fread(&contato, sizeof(reg), 1, arq);

        // Lógica de status
        if (contato.status == '*')
            strcpy(status_str, "EXCLUIDO");
        else
            strcpy(status_str, "ATIVO");

        // Escreve o registro no arquivo texto
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

/* ====================================================================================
   excluir
   ------------------------------------------------------------------------------------
   Implementa a Exclusão Lógica: atualiza o campo 'status' do registro no arquivo.
   ==================================================================================== */
void excluir(FILE *arq) {
    int nr;
    char confirma;
    reg contato;

    printf("\nInforme o código do registro para excluir: ");
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

    // 1. LÊ o registro
    long pos_byte = (long)(nr - 1) * sizeof(reg);
    fseek(arq, pos_byte, SEEK_SET);
    fread(&contato, sizeof(reg), 1, arq);

    if (contato.status == '*') {
        printf("Registro já estava excluído.\n");
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
        // 2. MODIFICA o status
        contato.status = '*';
        
        // 3. RETORNA à posição e REESCREVE o registro modificado (UPDATE no arquivo)
        fseek(arq, pos_byte, SEEK_SET);
        fwrite(&contato, sizeof(reg), 1, arq);
        fflush(arq);
        
        printf("Registro excluído com sucesso!\n");
    } else {
        printf("Exclusão cancelada.\n");
    }
}

/* ====================================================================================
   configurar_locale (CORRIGIDO PARA C89)
   -----------------
   Tenta configurar o ambiente para exibir acentos e caracteres especiais (UTF-8).
   ==================================================================================== */
void configurar_locale(void) {
    // Comando para forçar UTF-8 no terminal Windows
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
    // CORREÇÃO C89: Variável de loop 'i' declarada aqui
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

/* ====================================================================================
   main — FUNÇÃO PRINCIPAL
   ------------------------------------------------------------------------------------
   Gerencia a abertura/criação do arquivo ("dados1.dat") e o loop do menu.
   ==================================================================================== */
int main(void) {
    configurar_locale();
    
    // 1. Tenta abrir para Leitura/Escrita Binária (r+b, se já existir)
    FILE *arq = fopen("dados1.dat", "r+b"); 
    
    if (!arq) {
        // 2. Se não existe, tenta Criar para Leitura/Escrita Binária (w+b)
        arq = fopen("dados1.dat", "w+b");
        if (!arq) {
            printf("Erro crítico ao abrir/criar arquivo de dados.\n");
            return 1;
        }
    }

    int op;
    do {
        printf("\n========= AGENDA DE CONTATOS =========\n");
        printf("1. Cadastrar\n");
        printf("2. Consultar\n");
        printf("3. Gerar arquivo texto\n");
        printf("4. Excluir registro\n");
        printf("5. Sair\n");
        printf("--------------------------------------\n");
        printf("Total de registros: %d\n", tamanho(arq));
        printf("Opção: ");
        
        if (scanf("%d", &op) != 1) {
            printf("Digite um número válido.\n");
            limpa_buffer(); 
            continue;
        }
        limpa_buffer(); // Limpa o '\n'

        switch (op) {
            case 1: cadastrar(arq); break;
            case 2: consultar(arq); break;
            case 3: gerar_arquivo_texto(arq); break;
            case 4: excluir(arq); break;
            case 5: printf("Fechando arquivo e saindo...\n"); break;
            default: printf("Opção inválida!\n");
        }
    } while (op != 5);

    fclose(arq); // Fecha o arquivo
    return 0;
}

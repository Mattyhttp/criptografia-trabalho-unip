/*
==============================================================================

                           PROJETO CRIPTO LAB

 =============================================================================

  Sistema Educacional de Criptografia

  Objetivo:
Ensinar conceitos de criptografia e descriptografia por meio de algoritmos
simples implementados em linguagem C.


  Algoritmos:

  1- Cifra de Cesar
  2- Cifra de Substituicao
  3- Cifra de Transposicao


  Funcionalidades:
  - Criptografar e descriptografar mensagens
  - Consultar historico de operacoes
  - Registrar estatisticas de uso
  - Exportar historico para arquivo TXT
  - Material educativo e ajuda ao usuario

  Finalidade: Projeto academico - UNIP
  Versao: 1.1


  ==============================================================================
  */




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define MAX_HISTORICO 1000
#define TAM_CAMINHO 1024

char historico [MAX_HISTORICO][500];
char algoritmos[MAX_HISTORICO][30];
char operacoes[MAX_HISTORICO][20];
char mensagensOriginais[MAX_HISTORICO][500];
char deslocamentos[MAX_HISTORICO][10];
char datasOperacoes[MAX_HISTORICO][32];

int totalHistorico = 0;
int totalCriptografias = 0;
int totalDescriptografias = 0;

int usoCesar = 0;
int usoSubstituicao = 0;
int usoTransposicao = 0;

/* Localiza historico.dat na mesma pasta do executavel. */
static int caminhoHistorico(char caminho[TAM_CAMINHO])
{
    DWORD tamanho = GetModuleFileNameA(NULL, caminho, TAM_CAMINHO);
    char *separador;

    if (tamanho == 0 || tamanho >= TAM_CAMINHO)
        return 0;

    separador = strrchr(caminho, '\\');
    if (separador == NULL)
        separador = strrchr(caminho, '/');
    if (separador == NULL)
        return 0;

    separador[1] = '\0';
    if (strlen(caminho) + strlen("historico.dat") >= TAM_CAMINHO)
        return 0;
    strcat(caminho, "historico.dat");
    return 1;
}

static void codificarHex(const char *entrada, char *saida)
{
    static const char digitos[] = "0123456789ABCDEF";
    while (*entrada)
    {
        unsigned char c = (unsigned char)*entrada++;
        *saida++ = digitos[c >> 4];
        *saida++ = digitos[c & 15];
    }
    *saida = '\0';
}

static int valorHex(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int decodificarHex(const char *entrada, char *saida, size_t capacidade)
{
    size_t i, tamanho = strlen(entrada);
    if ((tamanho % 2) != 0 || tamanho / 2 >= capacidade) return 0;
    for (i = 0; i < tamanho; i += 2)
    {
        int alto = valorHex(entrada[i]);
        int baixo = valorHex(entrada[i + 1]);
        if (alto < 0 || baixo < 0 || (alto == 0 && baixo == 0)) return 0;
        saida[i / 2] = (char)((alto << 4) | baixo);
    }
    saida[tamanho / 2] = '\0';
    return 1;
}

static int salvarOperacao(const char *operacao, const char *algoritmo,
                          const char *deslocamento, const char *original,
                          const char *resultado)
{
    char caminho[TAM_CAMINHO], originalHex[1000], resultadoHex[1000];
    char dataHora[32];
    time_t agora = time(NULL);
    struct tm *local = localtime(&agora);
    FILE *arquivo;

    if (!caminhoHistorico(caminho)) return 0;
    codificarHex(original, originalHex);
    codificarHex(resultado, resultadoHex);
    if (local != NULL)
        strftime(dataHora, sizeof(dataHora), "%Y-%m-%d %H:%M:%S", local);
    else
        strcpy(dataHora, "data-indisponivel");

    arquivo = fopen(caminho, "a");
    if (arquivo == NULL) return 0;
    if (fprintf(arquivo, "%s|%s|%s|%s|%s|%s\n", operacao, algoritmo,
                deslocamento, dataHora, originalHex, resultadoHex) < 0)
    {
        fclose(arquivo);
        return 0;
    }
    return fclose(arquivo) == 0;
}

static void carregarHistorico(void)
{
    char caminho[TAM_CAMINHO], linha[2200];
    FILE *arquivo;
    if (!caminhoHistorico(caminho)) return;

    arquivo = fopen(caminho, "a+"); /* Cria o arquivo quando ainda nao existe. */
    if (arquivo == NULL) return;
    rewind(arquivo);
    while (fgets(linha, sizeof(linha), arquivo) != NULL)
    {
        char *campos[6], *p = linha;
        int i, valido = 1;
        size_t len = strlen(linha);
        if (len && linha[len - 1] == '\n') linha[--len] = '\0';
        if (len && linha[len - 1] == '\r') linha[--len] = '\0';
        campos[0] = p;
        for (i = 1; i < 6; i++)
        {
            p = strchr(p, '|');
            if (p == NULL) { valido = 0; break; }
            *p++ = '\0';
            campos[i] = p;
        }
        if (!valido || strchr(campos[5], '|') != NULL ||
            totalHistorico >= MAX_HISTORICO || strlen(campos[0]) >= sizeof(operacoes[0]) ||
            strlen(campos[1]) >= sizeof(algoritmos[0]) ||
            strlen(campos[2]) >= sizeof(deslocamentos[0]) ||
            strlen(campos[3]) >= sizeof(datasOperacoes[0])) continue;
        if (!decodificarHex(campos[4], mensagensOriginais[totalHistorico], 500) ||
            !decodificarHex(campos[5], historico[totalHistorico], 500)) continue;
        strcpy(operacoes[totalHistorico], campos[0]);
        strcpy(algoritmos[totalHistorico], campos[1]);
        strcpy(deslocamentos[totalHistorico], campos[2]);
        strncpy(datasOperacoes[totalHistorico], campos[3], sizeof(datasOperacoes[0]) - 1);
        datasOperacoes[totalHistorico][sizeof(datasOperacoes[0]) - 1] = '\0';
        totalHistorico++;
        if (strcmp(campos[0], "Criptografia") == 0) totalCriptografias++;
        else if (strcmp(campos[0], "Descriptografia") == 0) totalDescriptografias++;
        if (strcmp(campos[1], "Cifra de Cesar") == 0) usoCesar++;
        else if (strcmp(campos[1], "Cifra de Substituicao") == 0) usoSubstituicao++;
        else if (strcmp(campos[1], "Cifra de Transposicao") == 0) usoTransposicao++;
    }
    fclose(arquivo);
}

static int esvaziarArquivoHistorico(void)
{
    char caminho[TAM_CAMINHO];
    FILE *arquivo;
    if (!caminhoHistorico(caminho)) return 0;
    arquivo = fopen(caminho, "w");
    if (arquivo == NULL) return 0;
    return fclose(arquivo) == 0;
}



// Criptografa ou descriptografa mensagens usando a Cifra de Cesar.

void cifracesar(char mensagem[],int deslocamento, int descriptografar)
{
    int i;
    if (descriptografar)
    {
        deslocamento = -deslocamento;
    }

    for (i = 0; mensagem[i] != '\0'; i++)
    {

        char letra = mensagem[i];

        if (letra >= 'A' && letra <= 'Z')
        {

            mensagem[i] = 'A' + (letra - 'A' + deslocamento + 26)% 26;
        }
        else if (letra >= 'a' && letra <= 'z')
        {

            mensagem[i] = 'a' + (letra - 'a' + deslocamento + 26)% 26;
        }
    }
}


// Substitui letras conforme uma chave fixa de substituicao.

void cifraSubstituicao(char mensagem[], int descriptografar)
{
    char alfabeto[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char chave[] = "QWERTYUIOPASDFGHJKLZXCVBNM";
    int i,j;

    for(i = 0; mensagem [i] != '\0'; i++)
    {
        char letra = mensagem[i];
        int maiuscula = 1;

        if (letra >= 'a' && letra <= 'z')
        {
            letra = letra - 'a' + 'A';
            maiuscula = 0;

        }

        for (j = 0; j < 26; j++)
        {
            if (!descriptografar && alfabeto[j] == letra)
            {
                mensagem[i] = chave[j];
                if(!maiuscula)
                {
                    mensagem[i] = mensagem[i] - 'A' + 'a';
                }
                break;

            }

            if (descriptografar && chave [j] == letra)
            {
                mensagem[i] = alfabeto[j];
                if (!maiuscula)
                {
                    mensagem[i] = mensagem[i] - 'A' + 'a';

                }
                break;

            }

        }

    }

}


// Altera a posicao dos caracteres em pares.

void cifraTransposicao(char mensagem[])
{
    int i;
    char temporario;

    for (i = 0; mensagem[i] != '\0' && mensagem[i + 1] != '\0'; i+= 2)
    {
        temporario = mensagem[i];
        mensagem[i] = mensagem[i + 1 ];
        mensagem[i + 1] = temporario;

    }

}



// Gerencia a escolha do algoritmo e o processamento das mensagens.


void laboratorio(void)
{
    char mensagem[500];
    int metodo;
    int operacao;
    int deslocamento;
    char mensagemOriginal[500];



    printf("\n========================================\n");
    printf("        LABORATORIO DE MENSAGENS\n");
    printf("========================================\n");
    printf("Escolha um algoritmo para processar\n");
    printf("sua mensagem.\n");
    printf("----------------------------------------\n");

    printf("Escolha o algoritmo:\n");
    printf("1 - Cifra de Cesar\n");
    printf("2 - Cifra de Substituicao\n");
    printf("3 - Cifra de Transposicao\n");
    printf("Escolha: ");

    char entrada[50];

    if (fgets(entrada, sizeof(entrada), stdin) == NULL)
    {
        printf("Erro ao ler opcao.\n");
        return;

    }
    char extra;

    if (sscanf(entrada, " %d %c", &metodo, &extra) != 1)
    {
        printf("Entrada invalida! Escolha 1, 2 ou 3.\n");
        return;
    }

    if (metodo < 1 || metodo > 3 )
    {
        printf("Algoritmo invalido.\n");
        return;

    }

    printf("\n1 - Criptografar\n");
    printf("2 - Descriptografar\n");
    printf("Escolha: ");



    if (fgets(entrada, sizeof(entrada), stdin) == NULL)
    {
        printf("Erro ao ler operacao.\n");

        return;
    }



    if (sscanf(entrada, " %d %c", &operacao, &extra) != 1)
    {
        printf("Entrada invalida! Digite 1 ou 2.\n");
        return;
    }
    if (operacao !=1 && operacao != 2)
    {
        printf("Operacao invalida.\n");
        return;

    }

    if (metodo == 1)
    {
        printf("Digite o deslocamento (1 a 25):");

        if (fgets(entrada, sizeof(entrada), stdin) == NULL ||
                sscanf(entrada, " %d %c", &deslocamento, &extra) != 1)
        {
            printf("Entrada invalida.\n");
            return;
        }

        if (deslocamento < 1 || deslocamento > 25)
        {
            printf("O deslocamento deve ser de 1 a 25.\n");
            return;
        }
    }

    printf("Digite a mensagem (sem acentos):");

    if (fgets(mensagem, sizeof(mensagem), stdin) == NULL)
    {
        printf("Nao foi possivel ler a mensagem.\n");
        return;
    }
    if (strchr(mensagem, '\n') == NULL)
    {
        int caractere = getchar();

        if (caractere != '\n' && caractere != EOF)
        {


            while(caractere != '\n' && caractere != EOF)
            {
                caractere = getchar();
            }


            printf("Mensagem muito longa! Limite de 499 caracteres.\n");
            return;

        }


    }

    mensagem[strcspn(mensagem, "\n")] = '\0';

    if(mensagem[0] == '\n' || mensagem [0] == '\0')
    {
        printf("\nMensagem vazia! Tente novamente.\n");
        return;
    }




    int i;
    int soEspacos = 1;

    for (i = 0; mensagem[i] != '\0'; i++)
    {
        if (mensagem [i] != ' ' && mensagem[i] != '\t')
        {
            soEspacos = 0;
            break;

        }
    }

    if (soEspacos)
    {
        printf("A mensagem nao pode ficar vazia.\n");
        return;

    }



    strcpy(mensagemOriginal, mensagem);

    if (metodo == 1)
    {
        cifracesar(mensagem, deslocamento, operacao == 2);

    }
    else if (metodo == 2 )
    {
        cifraSubstituicao(mensagem, operacao == 2);


    }
    else
    {
        cifraTransposicao(mensagem);
    }

    if (operacao == 1)
    {
        totalCriptografias++;
    }
    else
    {
        totalDescriptografias++;
    }

    if (metodo == 1)
    {
        usoCesar++;

    }
    else if (metodo == 2)
    {
        usoSubstituicao++;

    }
    else
    {
        usoTransposicao++;

    }

    if (metodo == 1)
    {
        printf("\nAlgoritmo utilizado: Cifra de Cesar\n");
    }
    else if (metodo ==2)
    {
        printf("\nAlgoritmo utilizado: Cifra de Substituicao\n");
    }
    else
    {
        printf("\nAlgoritmo utilizado: Cifra de Transposicao\n");
    }
    if (operacao == 1)
    {
        printf("Tipo de processamento: Criptografia\n");
    }
    else
    {
        printf("Tipo de processamento: Descriptografia\n");
    }

    {
        const char *nomeAlgoritmo = metodo == 1 ? "Cifra de Cesar" :
                                    metodo == 2 ? "Cifra de Substituicao" : "Cifra de Transposicao";
        char deslocamentoTexto[10];
        const char *tipo = operacao == 1 ? "Criptografia" : "Descriptografia";
        if (metodo == 1) sprintf(deslocamentoTexto, "%d", deslocamento);
        else strcpy(deslocamentoTexto, "N/A");
        if (!salvarOperacao(tipo, nomeAlgoritmo, deslocamentoTexto, mensagemOriginal, mensagem))
            printf("\nAviso: nao foi possivel gravar a operacao em historico.dat.\n");
    }

    if (totalHistorico < MAX_HISTORICO)
    {

        strcpy(historico[totalHistorico], mensagem);

        strcpy(mensagensOriginais[totalHistorico], mensagemOriginal);

        if (metodo == 1)
        {

            strcpy(algoritmos[totalHistorico], "Cifra de Cesar");

        }
        else if (metodo == 2 )
        {

            strcpy(algoritmos[totalHistorico],"Cifra de Substituicao");

        }
        else
        {
            strcpy(algoritmos[totalHistorico], "Cifra de Transposicao");

        }

        if (operacao == 1)
        {
            strcpy(operacoes[totalHistorico], "Criptografia");
        }
        else
        {
            strcpy(operacoes[totalHistorico], "Descriptografia");
        }

        if (metodo == 1)
        {
            sprintf(deslocamentos[totalHistorico], "%d", deslocamento);
        }
        else
        {
            strcpy(deslocamentos[totalHistorico], "N/A");
        }

        {
            time_t agora = time(NULL);
            struct tm *local = localtime(&agora);
            if (local != NULL)
                strftime(datasOperacoes[totalHistorico], sizeof(datasOperacoes[0]), "%Y-%m-%d %H:%M:%S", local);
            else
                strcpy(datasOperacoes[totalHistorico], "data-indisponivel");
        }

        totalHistorico++;

    }
    else
    {
        printf("\nLimite em memoria atingido! Maximo de %d operacoes consultaveis.\n", MAX_HISTORICO);


    }


    printf("\n====================================\n");
    printf("          PROCESSAMENTO CONCLUIDO\n");
    printf("====================================\n");
    printf("Mensagem original: %s\n", mensagemOriginal);
    printf("Resultado: %s\n", mensagem);
    printf("====================================\n");
}





// Exibe as operacoes armazenadas no historico.


void mostrarHistorico(void)
{
    int i;

    printf("\n== HISTORICO DE OPERACOES ===\n");

    if(totalHistorico == 0)
    {
        printf("\nAinda nao existem operacoes registradas.\n");
        printf("Utilize o Laboratorio de Mensagens para comecar.\n");
        printf("========================================\n");
        return;
    }




    for(i = 0; i < totalHistorico; i ++)
    {
        printf("\nOperacao %d\n", i + 1);
        printf("------------------------------------\n");
        printf("Algoritmo: %s\n", algoritmos[i]);


        if (strcmp(algoritmos[i], "Cifra de Cesar") == 0)
        {
            printf("Deslocamento utilizado: %s\n", deslocamentos[i]);
        }


        printf("Tipo: %s\n", operacoes[i]);
        printf("Data e hora: %s\n", datasOperacoes[i]);
        printf("Mensagem original: %s\n", mensagensOriginais[i]);
        printf("Resultado: %s\n", historico[i]);
        printf("------------------------------------\n");
    }

}


// Exporta os registros do historico para um arquivo TXT.

void exportarHistorico(void)
{
    FILE *arquivo;
    int i;


    if (totalHistorico == 0)
    {
        printf("\nNao existem operacoes para exportar!\n");
        return;
    }

    printf("\nPreparando exportacao de %d operacoes...\n", totalHistorico);
    arquivo = fopen("historico_cripto_lab.txt", "w");

    if (arquivo == NULL)
    {
        printf("\nErro ao criar o arquivo!\n");
        return;
    }

    fprintf(arquivo, "=== CRIPTO LAB ===\n");
    fprintf(arquivo, "HISTORICO DE OPERACOES\n\n");


    for(i = 0; i < totalHistorico; i++)
    {
        fprintf(arquivo, "Operacao %d\n", i + 1);
        fprintf(arquivo, "Algoritmo: %s\n", algoritmos[i]);

        if (strcmp(algoritmos[i], "Cifra de Cesar") == 0)
        {
            fprintf(arquivo, "Deslocamento utilizado: %s\n", deslocamentos[i]);
        }



        fprintf(arquivo, "Tipo: %s\n", operacoes[i]);
        fprintf(arquivo, "Data e hora: %s\n", datasOperacoes[i]);
        fprintf(arquivo, "Mensagem original: %s\n", mensagensOriginais[i]);
        fprintf(arquivo, "Resultado: %s\n\n", historico[i]);
    }

    fclose(arquivo);


    printf("\n========================================\n");
    printf("       EXPORTACAO CONCLUIDA!\n");
    printf("========================================\n");
    printf("Arquivo gerado: historico_cripto_lab.txt\n");
    printf("Total de operacoes exportadas: %d\n", totalHistorico);
    printf("========================================\n");

}




// Solicita confirmacao e limpa os registros armazenados.

void limparHistorico(void)
{
    int confirmacao;
    char resposta[50];
    char extra;

    printf("\n========================================\n");
    printf("          GERENCIAR HISTORICO\n");
    printf("========================================\n");

    if (totalHistorico == 0)
    {
        printf("\nO historico ja esta vazio.\n");
        return;
    }

    printf("Deseja realmente apagar todo o historico?\n");
    printf("1 - Sim\n");
    printf("2 - Nao\n");
    printf("Escolha: ");

    if (fgets(resposta, sizeof(resposta), stdin) == NULL ||
            sscanf(resposta, " %d %c", &confirmacao, &extra) != 1)
    {
        printf("Entrada invalida.\n");
        return;
    }

    if (confirmacao == 1)
    {
        if (esvaziarArquivoHistorico())
        {
            totalHistorico = 0;
            totalCriptografias = totalDescriptografias = 0;
            usoCesar = usoSubstituicao = usoTransposicao = 0;
            printf("\nHistorico apagado com sucesso!\n");
        }
        else
            printf("\nErro ao apagar o arquivo de historico. Os dados foram mantidos.\n");
    }
    else if (confirmacao == 2)
    {
        printf("\nOperacao cancelada.\n");
    }
    else
    {
        printf("\nOpcao invalida.\n");
    }
}


// Apresenta os totais de operacoes e o uso dos algoritmos.

void mostrarEstatisticas(void)
{
    printf("\n====================================\n");
    printf("          ESTATISTICAS DE USO\n");
    printf("====================================\n");

    printf("\nOperacoes realizadas:\n");
    printf("Criptografias: %d\n", totalCriptografias);
    printf("Descriptografias: %d\n", totalDescriptografias);

    printf("\nUtilizacao dos algoritmos:\n");
    printf("Cifra de Cesar: %d\n", usoCesar);
    printf("Cifra de Substituicao: %d\n", usoSubstituicao);
    printf("Cifra de Transposicao: %d\n", usoTransposicao);

    printf("\nTotal geral: %d operacoes\n",
           totalCriptografias + totalDescriptografias);

    printf("====================================\n");
}


// Apresenta instrucoes sobre as funcionalidades do sistema.

void mostrarAjuda(void)
{
    printf("\n========================================\n");
    printf("          CENTRAL DE AJUDA\n");
    printf("========================================\n");
    printf("       SISTEMA CRIPTO LAB - V1.0\n");
    printf("========================================\n");
    printf("Consulte as instrucoes de cada recurso.\n");
    printf("----------------------------------------\n");



    printf("\n1. INTRODUCAO A CRIPTOGRAFIA\n");
    printf("Conheca os conceitos de criptografia e descriptografia.\n");




    printf("\n2. LABORATORIO DE MENSAGENS\n");
    printf("Escolha um algoritmo para criptografar ou descriptografar mensagens.\n");


    printf("\n3. APRENDER OS ALGORITMOS\n");
    printf("Veja como funcionam as cifras de Cesar, Substituicao e Transposicao.\n");

    printf("\n4. HISTORICO DE OPERACOES\n");
    printf("Consulte ate 1000 operacoes armazenadas no sistema.\n");


    printf("\n5. LIMPAR HISTORICO\n");
    printf("Apague os registros armazenados no historico.\n");


    printf("\n6. ESTATISTICAS DE USO\n");
    printf("Consulte a quantidade de operacoes e o uso dos algoritmos.\n");

    printf("\n7. EXPORTAR HISTORICO\n");
    printf("Salve as operacoes registradas em um arquivo TXT.\n");


    printf("\n8. AJUDA DO USUARIO\n");
    printf("Consulta as instrucoes de utilizacao do sistema.\n");


    printf("\n0. SAIR\n");
    printf("Encerre o programa CRIPTO LAB.\n");


    printf("\nOBSERVACAO:\n");
    printf("Este programa possui finalidade educacional.\n");
    printf("Nao utilize as cifras demonstradas para proteger dados reais.\n");
    printf("\nRISCOS DE SEGURANCA NA INTERNET:\n");

    printf("\n1. ENGENHARIA SOCIAL\n");
    printf("Golpistas tentam manipular pessoas para\n");
    printf("obter informacoes confidenciais.\n");

    printf("\n2. ROUBO DE SENHAS\n");
    printf("Senhas fracas ou compartilhadas podem\n");
    printf("facilitar acessos nao autorizados.\n");

    printf("\n3. LINKS MALICIOSOS\n");
    printf("Evite acessar links de origem desconhecida.\n");

    printf("\n4. VAZAMENTO DE DADOS\n");
    printf("Informacoes pessoais podem ser expostas\n");
    printf("quando sistemas ou contas sao invadidos.\n");

    printf("\nRECOMENDACAO:\n");
    printf("Mantenha seus dados protegidos e tenha\n");
    printf("cuidado ao compartilhar informacoes online.\n");



    printf("\n=============================================\n");




}



// Funcao principal: Exibe o menu e controla a navegacao do programa.




int main(void)
{
    int opcao = -1;
    system("color 0A");
    carregarHistorico();

    do
    {
        printf("\n");
        printf("========================================\n");
        printf("              C R I P T O    L A B\n");
        printf("========================================\n");
        printf("      Aprenda Criptografia na Pratica\n");
        printf("========================================\n");
        printf("                 VERSAO 1.0\n");
        printf("----------------------------------------\n");
        printf("             PROJETO ACADEMICO - UNIP\n");
        printf("----------------------------------------\n");
        printf("    Sistema Educacional de Criptografia\n");
        printf("=========================================\n");
        printf("           Bem-vindo ao CRIPTO LAB!\n");
        printf("------------------------------------------\n");
        printf("Explore, aprenda e descubra o mundo da criptografia!\n");
        printf("\n");
        printf("[1] - Introducao a Criptografia\n");
        printf("[2] - Laboratorio de Mensagens\n");
        printf("[3] - Aprender os Algoritmos\n");
        printf("[4] - Historico de Operacoes\n");
        printf("[5] - Limpar Historico\n");
        printf("[6] - Estatisticas de Uso\n");
        printf("[7] - Exportar Historico\n");
        printf("[8] - Ajuda do Usuario\n");
        printf("[0] - Sair\n");
        printf("\n");
        printf("\nEscolha uma Opcao:");

        char entrada[50];

        if (fgets(entrada, sizeof(entrada), stdin) == NULL)
        {
            printf("Erro ao ler opcao.\n");
            break;
        }
        char extra;

        if (sscanf(entrada, " %d %c", &opcao, &extra) != 1)
        {
            printf("Entrada invalida. Digite uma opcao valida.\n");
            continue;
        }

        switch (opcao)
        {

        case 1:
            printf("\n====================================\n");
            printf("       INTRODUCAO A CRIPTOGRAFIA\n");
            printf("====================================\n");

            printf("\nO que e Criptografia?\n");
            printf("E a tecnica de transformar mensagens\n");
            printf("em um formato que dificulta o acesso\n");
            printf("por pessoas nao autorizadas.\n");

            printf("\nO que e Descriptografia?\n");
            printf("E o processo inverso, que permite\n");
            printf("recuperar a mensagem original por\n");
            printf("meio do metodo correto.\n");

            printf("\nObjetivo do CRIPTO LAB:\n");
            printf("Demonstrar, na pratica, como funcionam\n");
            printf("diferentes tecnicas de criptografia.\n");

            printf("\n====================================\n");
            printf("\n========================================\n");
            printf("       BOAS PRATICAS DE SEGURANCA\n");
            printf("========================================\n");

            printf("\n1. Senhas fortes:\n");
            printf("Utilize letras, numeros e simbolos nas senhas.\n");

            printf("\n2. Autenticacao em duas etapas:\n");
            printf("Adicione uma camada extra de protecao as contas.\n");

            printf("\n3. Cuidado com phishing:\n");
            printf("Evite clicar em links e mensagens suspeitas.\n");

            printf("\n4. Protecao de dados:\n");
            printf("Evite compartilhar informacoes pessoais.\n");

            printf("\n5. Uso da criptografia:\n");
            printf("A criptografia ajuda a proteger informacoes,\n");
            printf("mas nao substitui outros cuidados de seguranca.\n");

            break;


        case 2:
            laboratorio();
            break;



        case 3:
            printf("\n========================================\n");
            printf("          CENTRAL DE APRENDIZADO\n");
            printf("========================================\n");
            printf("Conheca os algoritmos utilizados\n");
            printf("no sistema CRIPTO LAB.\n");
            printf("========================================\n");

            printf("\n1- CIFRA DE CESAR\n");
            printf("A Cifra de Cesar substitui cada letra\n");
            printf("por outra posicionada algumas casas\n");
            printf("adiante no alfabeto.\n");


            printf("\nExemplo com deslocamento 3:\n");
            printf("A mensagem OLA vira ROD.\n");
            printf("O vira R, L vira O e A vira D.\n");
            printf("\nTeste outro exemplo:\n");
            printf("Mensagem: CASA\n");
            printf("Deslocamento: 3\n");
            printf("Resultado: FDVD\n");


            printf("\nComo descriptografar?\n");
            printf("Basta voltar o mesmo numero de casas.\n");

            printf("\n2 - CIFRA DE SUBSTITUICAO\n");
            printf("Cada letra e trocada por outra letra\n");
            printf("conforme uma chave predefinida,\n");

            printf("\nExemplo da chave usada no Criptolab:\n");
            printf("A vira Q, B vira W e C vira E.\n");
            printf("Por isso, OLA vira GSQ.\n");
            printf("\nOutro exemplo:\n");
            printf("Mensagem: ABC\n");
            printf("Resultado: QWE\n");

            printf("\n3 - CIFRA DE TRANSPOSICAO\n");
            printf("Esse metodo troca a posicao dos caracteres\n");
            printf("da mensagem seguindo uma regra definida.\n");
            printf("\nExemplo usado no criptolab:\n");
            printf("A mensagem ABCDEF vira BADCFE,\n");
            printf("Os caracteres sao trocados em pares,\n");
            printf("\nVeja como funciona:\n");
            printf("AB vira BA\n");
            printf("CD vira DC\n");
            printf("EF vira FE\n");
            printf("Para descriptografar, basta aplicar\n");
            printf("o mesmo processo novamente.\n");

            printf("\nCRIPTOGRAFIA CLASSICA:\n");
            printf("Utiliza regras simples para transformar\n");
            printf("mensagens, como as cifras apresentadas\n");
            printf("neste laboratorio.\n");

            printf("\nCRIPTOGRAFIA MODERNA:\n");
            printf("Utiliza algoritmos matematicos e chaves\n");
            printf("para proteger informacoes digitais.\n");
            printf("E aplicada em sistemas, aplicativos e\n");
            printf("comunicacoes pela internet.\n");

            printf("\nIMPORTANTE:\n");
            printf("Esses metodos sao educativos e nao\n");
            printf("protegem dados reais com seguranca.\n");


            break;


        case 4:
            mostrarHistorico();
            break;

        case 5:
            limparHistorico();
            break;
        case 0:
            printf("\n====================================\n");
            printf("       OBRIGADO POR UTILIZAR!\n");
            printf("====================================\n");
            printf("       CRIPTO LAB - VERSAO 1.0\n");
            printf("\nContinue aprendendo sobre seguranca\n");
            printf("digital e protecao de informacoes!\n");
            printf("\n       Ate a proxima!\n");
            printf("====================================\n");
            break;

        case 6:
            mostrarEstatisticas();
            break;

        case 7:
            exportarHistorico();
            break;

        case 8:
            mostrarAjuda();
            break;


        default:

            printf("\nOpcao invalida. Tente Novamente.\n");
        }


    }
    while (opcao != 0);


    return 0;

}


























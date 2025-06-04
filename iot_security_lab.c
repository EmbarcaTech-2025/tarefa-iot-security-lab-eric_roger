// Escolha o modo: descomente UMA das linhas abaixo
#define MODO_PUBLISH         // Habilita o modo de publicação (envio de dados)
// #define MODO_SUBSCRIBE    // Habilita o modo de assinatura (recebimento de dados)

#include <string.h>          // Funções de manipulação de strings
#include <stdint.h>          // Tipos inteiros padrão (uint8_t, uint32_t, etc)
#include <stdio.h>           // Funções de entrada/saída padrão (printf, sscanf)
#include <time.h>            // Para obter timestamp atual
#include "pico/stdlib.h"     // Biblioteca padrão do Raspberry Pi Pico
#include "pico/cyw43_arch.h" // Driver WiFi para Pico W
#include "include/wifi_conn.h"   // Funções personalizadas para conexão WiFi
#include "include/mqtt_comm.h"   // Funções personalizadas para comunicação MQTT
#include "include/xor_cipher.h"  // Funções de cifra XOR

#ifdef MODO_SUBSCRIBE
/**
 * Callback chamada automaticamente ao receber mensagem MQTT.
 * 
 * @param topic   Nome do tópico recebido
 * @param payload Ponteiro para os dados recebidos (criptografados)
 * @param len     Tamanho dos dados recebidos
 * 
 * Funcionamento:
 * - Descriptografa a mensagem recebida usando XOR
 * - Faz o parse do JSON para extrair valor e timestamp
 * - Verifica se a mensagem não é repetida (proteção contra replay)
 * - Exibe a leitura ou alerta de replay detectado
 */
uint32_t ultima_timestamp_recebida = 0; // Armazena o último timestamp recebido para evitar replay

void trata_mensagem(const char *topic, const uint8_t *payload, size_t len) 
{
    uint8_t chave = 42; // Mesma chave usada na criptografia do publish
    uint8_t msg[128];   // Buffer para mensagem descriptografada

    // Garante que não ultrapassa o tamanho do buffer
    size_t tam = len < sizeof(msg) - 1 ? len : sizeof(msg) - 1;

    // Descriptografa a mensagem recebida usando XOR
    xor_decrypt(payload, msg, tam, chave);
    msg[tam] = '\0'; // Garante terminação nula para string

    uint32_t nova_timestamp;
    float valor;
    
    // Faz o parse do JSON recebido para extrair valor e timestamp
    if (sscanf((char*)msg, "{\"valor\":%f,\"ts\":%lu}", &valor, &nova_timestamp) != 2) {
        printf("Erro no parse da mensagem!\n");
        return;
    }

    // Verifica se a mensagem não é repetida (proteção contra replay)
    if (nova_timestamp > ultima_timestamp_recebida) 
    {
        ultima_timestamp_recebida = nova_timestamp;
        printf("Nova leitura: %.2f (ts: %lu)\n", valor, nova_timestamp);
    } 
    else 
    {
        printf("Replay detectado (ts: %lu <= %lu)\n", nova_timestamp, ultima_timestamp_recebida);
    }
}
#endif

/**
 * Função principal do programa.
 * 
 * Inicializa o sistema, conecta ao Wi-Fi, configura o cliente MQTT e executa o loop principal.
 * O comportamento depende do modo selecionado (PUBLISH ou SUBSCRIBE).
 */
int main() 
{
    stdio_init_all(); // Inicializa as interfaces padrão (USB serial, etc.)

    // Conecta à rede WiFi com SSID e senha
    connect_to_wifi("Redmi_Note_10", "12345678");

    // Configura o cliente MQTT com ID, IP do broker, usuário e senha
    mqtt_setup("bitdog1", "192.168.115.201", "aluno", "nqcv9982");

#ifdef MODO_PUBLISH
    /**
     * Modo Publisher:
     * - Gera mensagem JSON com valor e timestamp
     * - Criptografa a mensagem usando XOR
     * - Publica a mensagem criptografada no tópico MQTT
     * - Exibe a mensagem criptografada em hexadecimal no terminal
     */
    uint8_t chave = 42; // Chave de criptografia XOR (deve ser igual à do subscriber)
    while (true)
    {
        char buffer[128];                  // Buffer para mensagem JSON
        uint8_t buffer_criptografado[128]; // Buffer para mensagem criptografada
        float valor = 26.5;                // Valor de exemplo do sensor (poderia ser lido de um sensor real)
        uint32_t ts = (uint32_t)time(NULL); // Timestamp atual (segundos desde 1970)

        // Monta mensagem JSON com valor e timestamp
        int len = snprintf(buffer, sizeof(buffer), "{\"valor\":%.2f,\"ts\":%lu}", valor, (unsigned long)ts);

        // Criptografa a mensagem usando XOR
        xor_encrypt((const uint8_t *)buffer, buffer_criptografado, len, chave);

        // Publica mensagem criptografada no tópico MQTT
        mqtt_comm_publish("escola/sala1/temperatura", buffer_criptografado, len);

        // Mostra a mensagem criptografada em hexadecimal no terminal para depuração
        printf("Publicado (criptografado): ");
        for (int i = 0; i < len; ++i) {
            printf("%02X ", buffer_criptografado[i]);
        }
        printf("\n");

        sleep_ms(5000); // Aguarda 5 segundos para próxima publicação
    }
#endif

#ifdef MODO_SUBSCRIBE
    /**
     * Modo Subscriber:
     * - Inscreve-se no tópico e define a função de callback para tratar mensagens recebidas
     * - Mantém o programa rodando em loop
     */
    mqtt_comm_subscribe("escola/sala1/temperatura", trata_mensagem);

    while (true)
    {
        sleep_ms(5000); // Mantém o programa rodando (poderia ser usado para outras tarefas)
    }
#endif

    return 0; // Fim do programa (nunca chega aqui devido ao loop infinito)
}
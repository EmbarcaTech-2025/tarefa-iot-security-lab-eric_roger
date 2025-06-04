// Inclusão das bibliotecas necessárias para MQTT, configuração e funções utilitárias
#include "lwip/apps/mqtt.h"       // Biblioteca MQTT do lwIP (protocolo MQTT)
#include "include/mqtt_comm.h"    // Header com as declarações das funções deste arquivo
#include "lwipopts.h"             // Configurações do lwIP
#include <stdio.h>
#include <string.h>

// Variável global estática para armazenar a instância do cliente MQTT
static mqtt_client_t *client = NULL;

// Ponteiro para a função callback definida pelo usuário para tratar mensagens recebidas
static mqtt_message_cb_t user_msg_cb = NULL;

// Buffer para armazenar o nome do tópico inscrito (usado no callback)
static char subscribed_topic[128] = {0}; 

/**
 * Callback de conexão MQTT
 * 
 * @param client Instância do cliente MQTT
 * @param arg    Argumento opcional (não utilizado)
 * @param status Status da tentativa de conexão
 * 
 * Exibe mensagem de sucesso ou erro ao conectar ao broker.
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) 
{
    if (status == MQTT_CONNECT_ACCEPTED) 
    {
        printf("Conectado ao broker MQTT com sucesso!\n");
    } 
    else 
    {
        printf("Falha ao conectar ao broker, código: %d\n", status);
    }
}

/**
 * Função para configurar e conectar ao broker MQTT
 * 
 * @param client_id  Identificador do cliente MQTT
 * @param broker_ip  Endereço IP do broker (ex: "192.168.1.1")
 * @param user       Nome de usuário para autenticação (pode ser NULL)
 * @param pass       Senha para autenticação (pode ser NULL)
 * 
 * Realiza a conversão do IP, cria o cliente e inicia a conexão.
 */
void mqtt_setup(const char *client_id, const char *broker_ip, const char *user, const char *pass) 
{
    ip_addr_t broker_addr;
    // Converte o IP do broker de string para formato de rede
    if (!ip4addr_aton(broker_ip, &broker_addr)) 
    {
        printf("Erro no IP\n");
        return;
    }

    // Cria uma nova instância de cliente MQTT
    client = mqtt_client_new();
    if (client == NULL) {
        printf("Falha ao criar o cliente MQTT\n");
        return;
    }

    // Preenche as informações do cliente
    struct mqtt_connect_client_info_t ci = 
    {
        .client_id = client_id,
        .client_user = user,
        .client_pass = pass
    };

    // Inicia a conexão com o broker na porta 1883
    mqtt_client_connect(client, &broker_addr, 1883, mqtt_connection_cb, NULL, &ci);
}

/**
 * Callback de confirmação de publicação
 * 
 * @param arg    Argumento opcional
 * @param result Código de resultado da operação
 * 
 * Exibe mensagem de sucesso ou erro ao publicar.
 */
static void mqtt_pub_request_cb(void *arg, err_t result) 
{
    if (result == ERR_OK) 
    {
        printf("Publicação MQTT enviada com sucesso!\n");
    } 
    else 
    {
        printf("Erro ao publicar via MQTT: %d\n", result);
    }
}

/**
 * Função para publicar dados em um tópico MQTT
 * 
 * @param topic Nome do tópico (ex: "sensor/temperatura")
 * @param data  Ponteiro para os dados a serem enviados
 * @param len   Tamanho dos dados
 * 
 * Envia a mensagem MQTT para o broker.
 */
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len) 
{
    // Envia a mensagem para o broker
    err_t status = mqtt_publish(
        client,                // Instância do cliente
        topic,                 // Tópico de publicação
        data,                  // Dados a serem enviados
        len,                   // Tamanho dos dados
        0,                     // QoS 0 (entrega pelo menos uma vez)
        0,                     // Não manter mensagem (retain)
        mqtt_pub_request_cb,   // Função chamada após publicar
        NULL
    );

    if (status != ERR_OK) 
    {
        printf("mqtt_publish falhou ao ser enviada: %d\n", status);
    }
}

/**
 * Callback chamado quando chega uma nova mensagem em um tópico inscrito
 * 
 * @param arg     Argumento opcional
 * @param topic   Nome do tópico recebido
 * @param tot_len Tamanho total da mensagem
 * 
 * Salva o nome do tópico para uso posterior no callback de dados.
 */
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) 
{
    // Salva o nome do tópico recebido
    strncpy(subscribed_topic, topic, sizeof(subscribed_topic) - 1);
    subscribed_topic[sizeof(subscribed_topic) - 1] = '\0';
    printf("Mensagem recebida no tópico: %s\n", topic);
}

/**
 * Callback chamado quando chegam os dados da mensagem MQTT
 * 
 * @param arg   Argumento opcional
 * @param data  Ponteiro para os dados recebidos
 * @param len   Tamanho dos dados recebidos
 * @param flags Flags de controle do MQTT
 * 
 * Chama o callback do usuário, se definido, passando o tópico e os dados.
 */
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) 
{
    // Se o usuário definiu uma função callback, chama ela passando o tópico e os dados recebidos
    if (user_msg_cb) 
    {
        user_msg_cb(subscribed_topic, data, len);
    }
}

/**
 * Função para inscrever-se em um tópico MQTT
 * 
 * @param topic Nome do tópico para inscrição
 * @param cb    Função callback a ser chamada ao receber mensagem
 * 
 * Registra os callbacks e solicita inscrição no tópico desejado.
 */
void mqtt_comm_subscribe(const char *topic, mqtt_message_cb_t cb) 
{
    if (!client) 
    {
        printf("Cliente MQTT não inicializado!\n");
        return;
    }
    user_msg_cb = cb; // Salva o callback do usuário

    // Registra as funções que tratam mensagens recebidas
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

    // Solicita inscrição no tópico desejado
    err_t status = mqtt_subscribe(client, topic, 0, NULL, NULL);
    if (status == ERR_OK) 
    {
        printf("Inscrito no tópico: %s\n", topic);
    } 
    else 
    {
        printf("Falha ao inscrever no tópico: %s (erro %d)\n", topic, status);
    }
}
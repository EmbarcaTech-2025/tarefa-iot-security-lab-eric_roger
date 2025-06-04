#include "lwip/apps/mqtt.h"       // Biblioteca MQTT do lwIP (protocolo MQTT)
#include "include/mqtt_comm.h"    // Header com as declarações das funções deste arquivo
#include "lwipopts.h"             // Configurações do lwIP
#include <stdio.h>
#include <string.h>

// Variável global para guardar o cliente MQTT
static mqtt_client_t *client = NULL;

// Ponteiro para a função callback que será chamada ao receber mensagem
static mqtt_message_cb_t user_msg_cb = NULL;
// Buffer para guardar o nome do tópico inscrito
static char subscribed_topic[128] = {0}; 

// Função chamada automaticamente quando conecta (ou falha) ao broker MQTT
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

// Função para configurar e conectar ao broker MQTT
// Recebe: ID do cliente, IP do broker, usuário e senha
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

// Função chamada automaticamente após tentar publicar uma mensagem
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

// Função para publicar dados em um tópico MQTT
// Recebe: nome do tópico, dados a enviar e tamanho dos dados
void mqtt_comm_publish(const char *topic, const uint8_t *data, size_t len) 
{
    // Envia a mensagem para o broker
    err_t status = mqtt_publish(
        client,
        topic,
        data,
        len,
        0, // QoS 0 (entrega pelo menos uma vez)
        0, // Não manter mensagem (retain)
        mqtt_pub_request_cb, // Função chamada após publicar
        NULL
    );

    if (status != ERR_OK) 
    {
        printf("mqtt_publish falhou ao ser enviada: %d\n", status);
    }
}

// Função chamada quando chega uma nova mensagem em um tópico inscrito
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) 
{
    // Salva o nome do tópico recebido
    strncpy(subscribed_topic, topic, sizeof(subscribed_topic) - 1);
    subscribed_topic[sizeof(subscribed_topic) - 1] = '\0';
    printf("Mensagem recebida no tópico: %s\n", topic);
}

// Função chamada quando chegam os dados da mensagem MQTT
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) 
{
    // Se o usuário definiu uma função callback, chama ela passando o tópico e os dados recebidos
    if (user_msg_cb) 
    {
        user_msg_cb(subscribed_topic, data, len);
    }
}

// Função para inscrever-se em um tópico MQTT
// Recebe: nome do tópico e função callback para tratar mensagens recebidas
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
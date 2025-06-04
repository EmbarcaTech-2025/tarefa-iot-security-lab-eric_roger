# Tarefa: IoT Security Lab - EmbarcaTech 2025

Autores: **Eric Senne Roma**  
         **Roger Melo**

Curso: Residência Tecnológica em Sistemas Embarcados  
Instituição: EmbarcaTech - HBr  
Campinas, 04 de 2025

---

## Descrição do Projeto

Este projeto demonstra uma comunicação segura entre dispositivos embarcados BitDogLab com Raspberry Pi Pico W utilizando o protocolo MQTT sobre Wi-Fi. O objetivo é implementar autenticação no broker, criptografia leve (XOR) e proteção contra ataques de sniffing e replay, seguindo um roteiro prático de segurança em IoT.

### Etapas Implementadas

1. **Conexão Wi-Fi:**  
   O código inicializa o chip Wi-Fi do Pico W e conecta a uma rede protegida usando SSID e senha fornecidos. Isso garante que apenas dispositivos autorizados acessem a rede local.

2. **Setup MQTT com Autenticação:**  
   O cliente MQTT é configurado para se conectar ao broker Mosquitto utilizando autenticação por usuário e senha, impedindo conexões anônimas e aumentando a segurança da rede.

3. **Publicação e Assinatura MQTT:**  
   O dispositivo pode atuar como publisher (publicando dados de sensor) ou subscriber (recebendo e validando dados), alternando entre os modos via diretiva de compilação.

4. **Criptografia Leve (XOR):**  
   Antes do envio, os dados são criptografados com uma cifra XOR simples, dificultando a interceptação e leitura dos dados por sniffers de rede.

5. **Proteção contra Replay:**  
   Cada mensagem publicada inclui um timestamp. O subscriber só aceita mensagens com timestamp maior que o último recebido, descartando tentativas de replay.

---

## Discussão

**Quais técnicas são escaláveis?**  
A autenticação no broker MQTT e o uso de timestamps para proteção contra replay são técnicas escaláveis, pois podem ser aplicadas a qualquer número de dispositivos sem aumento significativo de complexidade. O uso de criptografia leve como XOR é didático, mas para ambientes reais recomenda-se algoritmos mais robustos (como AES), que também são escaláveis com bibliotecas apropriadas.

**Como aplicá-las com várias BitDogLab em rede escolar?**  
Em uma rede escolar, cada BitDogLab pode ser configurada com credenciais únicas ou compartilhadas para autenticação no broker, permitindo controle centralizado de acesso. O uso de tópicos MQTT distintos para cada sala ou grupo facilita a organização e o gerenciamento dos dados. A proteção contra replay e a criptografia podem ser implementadas em todos os dispositivos, garantindo integridade e confidencialidade das mensagens mesmo em ambientes com muitos nós.

---

## Como Executar

1. Configure o Broker Mosquitto com Autenticação

   - No arquivo `mosquitto.conf` do seu computador, adicione:
     ```
     allow_anonymous false
     password_file /etc/mosquitto/passwd   # Ou caminho equivalente no Windows
     listener 1883
     ```
   - No terminal, crie o arquivo de senha (ajuste o caminho conforme seu SO):
     ```bash
     sudo mosquitto_passwd -c /etc/mosquitto/passwd aluno
     ```
   - Inicie o broker Mosquitto apontando para o arquivo de configuração:
     ```bash
     mosquitto -c CAMINHO\PARA\SEU\mosquitto.conf -v
     ```
   - Exemplo no Windows:
     ```bash
     mosquitto -c C:\Users\seu_usuario\caminho\para\mosquitto.conf -v
     ```
2. Configure o Wi-Fi no Código
   
   - No seu código, coloque o SSID e a senha da sua rede e teste a conexão:
     ```c
     connect_to_wifi("Redmiote_10", "12345678");
     ```

3. Descubra o IP do seu computador
   
   - No terminal/cmd, rode:
     ```bash
     ipconfig
     ```
   - Anote o endereço IPv4 (exemplo: `192.168.115.201`).

4. Configure o IP do Broker no Código
   
   - No seu código, coloque o IP anotado (PC vai funcionar como Broker) e a senha que você configurou para o Broker em passwd (exemplo: `nqcv9982`):
     ```c
     mqtt_setup("bitdog1", "192.168.115.201", "aluno", "nqcv9982");
     ```
5. Compile e grave o código nas placas BitDogLab.
6. Use um dispositivo como publisher e outro como subscriber.
7. Monitore o tráfego com Wireshark para observar a diferença entre mensagens em texto claro e criptografadas.

## Etapas:

1. **Teste Publicação Simples (sem criptografia)**
   
   - Use o exemplo abaixo para publicar mensagens:
     ```c
     #include <string.h>
     #include "pico/stdlib.h"
     #include "pico/cyw43_arch.h"
     #include "include/wifi_conn.h"
     #include "include/mqtt_comm.h"

     int main() {
         stdio_init_all();
         connect_to_wifi("Redmi_Note_10", "12345678");
         mqtt_setup("bitdog1", "192.168.115.201", "aluno", "nqcv9982");
         const char *mensagem = "26.8";
         while (true) {
             mqtt_comm_publish("escola/sala1/temperatura", mensagem, strlen(mensagem));
             sleep_ms(5000);
         }
         return 0;
     }
     ```

2. **Teste Recepção Simples (sem criptografia)**
   
   - Use o exemplo abaixo para receber mensagens:
     ```c
     #include <string.h>
     #include "pico/stdlib.h"
     #include "pico/cyw43_arch.h"
     #include "include/wifi_conn.h"
     #include "include/mqtt_comm.h"

     void trata_mensagem(const char *topic, const uint8_t *payload, size_t len) {
         printf("Recebido no tópico %s: %.*s\n", topic, (int)len, payload);
     }

     int main() {
         stdio_init_all();
         connect_to_wifi("Redmi_Note_10", "12345678");
         mqtt_setup("bitdog1", "192.168.115.201", "aluno", "nqcv9982");
         mqtt_comm_subscribe("escola/sala1/temperatura", trata_mensagem);
         while (true) {
             sleep_ms(5000);
         }
         return 0;
     }
     ```

3. **Teste Proteção contra Replay**
   
   - **Publisher** (envia JSON com timestamp):
     ```c
     #include <string.h>
     #include <stdint.h>
     #include <stdio.h>
     #include <time.h>
     #include "pico/stdlib.h"
     #include "pico/cyw43_arch.h"
     #include "include/wifi_conn.h"
     #include "include/mqtt_comm.h"

     int main() {
         stdio_init_all();
         connect_to_wifi("Redmi_Note_10", "12345678");
         mqtt_setup("bitdog1", "192.168.115.201", "aluno", "nqcv9982");
         while (true) {
             char buffer[128];
             float valor = 26.5;
             uint32_t ts = (uint32_t)time(NULL);
             int len = snprintf(buffer, sizeof(buffer), "{\"valor\":%.2f,\"ts\":%lu}", valor, (unsigned long)ts);
             mqtt_comm_publish("escola/sala1/temperatura", (const uint8_t *)buffer, len);
             printf("Publicado: %s\n", buffer);
             sleep_ms(5000);
         }
         return 0;
     }
     ```
   - **Subscriber** (descarta mensagens repetidas):
     ```c
     #include <string.h>
     #include <stdint.h>
     #include <stdio.h>
     #include "pico/stdlib.h"
     #include "pico/cyw43_arch.h"
     #include "include/wifi_conn.h"
     #include "include/mqtt_comm.h"

     uint32_t ultima_timestamp_recebida = 0;

     void trata_mensagem(const char *topic, const uint8_t *payload, size_t len) {
         char msg[128];
         if (len >= sizeof(msg)) len = sizeof(msg) - 1;
         memcpy(msg, payload, len);
         msg[len] = '\0';
         uint32_t nova_timestamp;
         float valor;
         if (sscanf(msg, "{\"valor\":%f,\"ts\":%lu}", &valor, &nova_timestamp) != 2) {
             printf("Erro no parse da mensagem!\n");
             return;
         }
         if (nova_timestamp > ultima_timestamp_recebida) {
             ultima_timestamp_recebida = nova_timestamp;
             printf("Nova leitura: %.2f (ts: %lu)\n", valor, nova_timestamp);
         } else {
             printf("Replay detectado (ts: %lu <= %lu)\n", nova_timestamp, ultima_timestamp_recebida);
         }
     }

     int main() {
         stdio_init_all();
         connect_to_wifi("Redmi_Note_10", "12345678");
         mqtt_setup("bitdog1", "192.168.115.201", "aluno", "nqcv9982");
         mqtt_comm_subscribe("escola/sala1/temperatura", trata_mensagem);
         while (true) {
             sleep_ms(5000);
         }
         return 0;
     }
     ```
     ![image.png](attachment:394b0d98-efc5-4004-89c7-7b45e617e625:image.png)

4. **Teste Criptografia Leve (XOR) com Replay**
   
   - **Publisher** (criptografa mensagem):
     ```c
     #include <string.h>
     #include <stdint.h>
     #include <stdio.h>
     #include <time.h>
     #include "pico/stdlib.h"
     #include "pico/cyw43_arch.h"
     #include "include/wifi_conn.h"
     #include "include/mqtt_comm.h"
     #include "include/xor_cipher.h"

     int main() {
         stdio_init_all();
         connect_to_wifi("Redmi_Note_10", "12345678");
         mqtt_setup("bitdog1", "192.168.115.201", "aluno", "nqcv9982");
         uint8_t chave = 0xAA; // Chave de criptografia XOR
         while (true) {
             char buffer[128];
             uint8_t buffer_criptografado[128];
             float valor = 26.5;
             uint32_t ts = (uint32_t)time(NULL);
             int len = snprintf(buffer, sizeof(buffer), "{\"valor\":%.2f,\"ts\":%lu}", valor, (unsigned long)ts);
             xor_encrypt((const uint8_t *)buffer, buffer_criptografado, len, chave);
             mqtt_comm_publish("escola/sala1/temperatura", buffer_criptografado, len);
             printf("Publicado (criptografado): ");
             for (int i = 0; i < len; ++i) printf("%02X ", buffer_criptografado[i]);
             printf("\n");
             sleep_ms(5000);
         }
         return 0;
     }
     ```
   - **Subscriber** (descriptografa e valida replay):
     ```c
     #include <string.h>
     #include <stdint.h>
     #include <stdio.h>
     #include "pico/stdlib.h"
     #include "pico/cyw43_arch.h"
     #include "include/wifi_conn.h"
     #include "include/mqtt_comm.h"
     #include "include/xor_cipher.h"

     uint32_t ultima_timestamp_recebida = 0;

     void trata_mensagem(const char *topic, const uint8_t *payload, size_t len) {
         uint8_t chave = 0xAA; // Mesma chave usada na criptografia
         uint8_t msg[128];
         size_t tam = len < sizeof(msg) - 1 ? len : sizeof(msg) - 1;
         xor_decrypt(payload, msg, tam, chave);
         msg[tam] = '\0';
         uint32_t nova_timestamp;
         float valor;
         if (sscanf((char*)msg, "{\"valor\":%f,\"ts\":%lu}", &valor, &nova_timestamp) != 2) {
             printf("Erro no parse da mensagem!\n");
             return;
         }
         if (nova_timestamp > ultima_timestamp_recebida) {
             ultima_timestamp_recebida = nova_timestamp;
             printf("Nova leitura: %.2f (ts: %lu)\n", valor, nova_timestamp);
         } else {
             printf("Replay detectado (ts: %lu <= %lu)\n", nova_timestamp, ultima_timestamp_recebida);
         }
     }

     int main() {
         stdio_init_all();
         connect_to_wifi("Redmi_Note_10", "12345678");
         mqtt_setup("bitdog1", "192.168.115.201", "aluno", "nqcv9982");
         mqtt_comm_subscribe("escola/sala1/temperatura", trata_mensagem);
         while (true) {
             sleep_ms(5000);
         }
         return 0;
     }
     ```
     ![image.png](attachment:c39c0cc0-eee5-45d6-94ce-7cade6adc1c2:image.png)

---

---

## 📜 Licença
GNU GPL-3.0.

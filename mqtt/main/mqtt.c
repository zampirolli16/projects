#include <stdio.h> // para funcões de print 
#include <string.h> //para lidar com sitrings
#include "freertos/FreeRTOS.h" //para os delays
#include "freertos/task.h" 
#include "esp_wifi.h" //configuracao do wifi
#include "esp_system.h" // gerencia o sistema da esp32
#include "esp_log.h" //logs no terminal
#include "esp_event.h" // manipular eventos
#include "nvs_flash.h" //memoria nao volatil
#include "esp_netif.h" 
#include "protocol_examples_common.h" //facilita a manipulacao de protoclos(wifi,mqtt etc)
// biblioteca para manipulacao de pilha TCP/IP
#include "lwip/err.h" 
#include "lwip/sys.h" 
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h" // funcoes para a conexao MQTT

#define  SSID ""
#define  SENHA ""

uint32_t MQTT_conexao = 0; //flag para identificar a conexao do MQTT

static const char *TAG1 = "wifi";
static const char *TAG = "MQTT";

int retry_num=0;

// Lidando com os eventos de Wi-fi, verificando o stutus atual da conexao e obtencao do IP
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data){
    switch (event_id)
    {
    case WIFI_EVENT_STA_START: //quando o wifi esta tentando conectar
        esp_wifi_connect(); // conecta a rede
        ESP_LOGI(TAG1, "Tentando conectar ao Wi-Fi...\n");
        break;

    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG1, "Wi-Fi coectado\n"); // conectado ao ponto de acesso(AP)
        break;   

    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG1, "Disconectado do Wi-Fi:Reconectando\n");
        esp_wifi_connect();
        /*if(retry_num<5){
            esp_wifi_connect();
            retry_num++;
            printf("Retrying to Connect...\n");
        }*/

        break;

    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG1, "IP recebido\n");

        break;

    default:
        break;
    }
}

// Funcao que sera chamado no app_main para configurar o wifi
void wifi_initia(void){
    esp_netif_init();
    esp_event_loop_create_default(); //cria um loop dos eventos para que sejam manuseados e enviados 
    esp_netif_create_default_wifi_sta(); //incializacao do wifi para o modo estaocao e trata os eventos
    wifi_init_config_t wifi_default = WIFI_INIT_CONFIG_DEFAULT(); // configuracao padrao do wifi
    esp_wifi_init(&wifi_default); // driver inicializado com os valores padrao
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL); // registra o wifi_event para obter qualquer variacao/evento do wifi
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL); // configura eventos Ip
   
   wifi_config_t wifi_param = {
        .sta = {
            .ssid = SSID,
            .password = SENHA,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA); // configura para o modo estacao
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_param); // coloca os parametros
    esp_wifi_start(); //tenta conectar com as configuracoes salvas
}

// configuracoes do MQTT e seus eventos
static void mqtt_event_handler(esp_mqtt_event_handle_t event){
    //esp_mqtt_event_handle_t e uma struct que receb a struct "event", a qual contem as variaveis do evento MQTT
    //esp_mqtt_event_handle_t event = event_data; //obtem os dados do evento MQTT
    esp_mqtt_client_handle_t client = event->client; //obtem o conteudo do identificador cliente dentro da struct 
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED: // cliente conectado ao broker
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED"); //exibe o lpg
        esp_mqtt_client_subscribe(client, "teste", 0); // Inscreve-se no tópico MQTT
        esp_mqtt_client_publish(client, "teste", "ola da ESP32.....", 0,0,0);
        MQTT_conexao = 1;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED"); // Quando desconectado
        MQTT_conexao = 0;
        break;

    case MQTT_EVENT_SUBSCRIBED: //quando se inscreveru com sucesso no topico
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        break;

    case MQTT_EVENT_UNSUBSCRIBED: // quando cancela a inscricao
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED"); 
        break;
    
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED"); //quando o broker reconhece a publicacao no topico
        break;

    case MQTT_EVENT_DATA: //quando o cliente recebe a mensagem publicada
        ESP_LOGI(TAG, "MQTT_EVENT_DATA"); 
        printf("TOPIC=%.*s\n", event->topic_len, event->topic); //mostra o topico 
        printf("DATA=%.*s\n", event->data_len, event->data); // mostra o dado/mensagem 
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR"); // Em caso de erro
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }


}
esp_mqtt_client_handle_t client = NULL;
static void mqtt_app_start(void){

    const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtt://igbt.eesc.usp.br",
    .credentials = {
        .username = "mqtt",
        .authentication.password = "mqtt_123_abc"
    }
};

    esp_mqtt_client_handle_t client =esp_mqtt_client_init(&mqtt_cfg); //envia os parametros do mqtt para inicializar
    if (client == NULL) {
    ESP_LOGE(TAG, "Falha ao inicializar o cliente MQTT!");
    return;
    }
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);  //coloca o manipulador definido acima
    esp_mqtt_client_start(client); // inicia o cliente mqtt com as configuracoes
}

void Publisher_Task(void *params) // publica a mensagem no topico a cada 15 segundos
{
  while (true)
  {
    if(MQTT_conexao)
    {
        esp_mqtt_client_publish(client, "teste", "Conectado no PC!", 0, 0, 0);
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
  }
}
void app_main(void){
    nvs_flash_init(); // mantem guardada as configuracoes como wifi
    wifi_initia(); //chama a funcao para a incializacao do wifi
    vTaskDelay(10000 /portTICK_PERIOD_MS); // delay para esperar a conexao
    mqtt_app_start(); // inicia a conexao mqtt
    //xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, NULL);
}







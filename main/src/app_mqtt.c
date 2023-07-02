#include "app_mqtt.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "esp_mac.h"

#define MQTT_RX_MAX_LEN 1024
#define MQTT_TOPIC_MAX_LEN 64
#define MQTT_TASK_STACK_SIZE 8*1024

#define MIN(x, y) ((x) > (y)) ? (y) : (x)

static esp_mqtt_client_handle_t mqtt_client_handle = NULL;
static TaskHandle_t app_mqtt_task_handle = NULL;
static esp_mqtt_client_config_t mqtt_cfg;

struct {
    uint8_t data[MQTT_RX_MAX_LEN];
    uint16_t data_len;
    uint8_t topic[MQTT_TOPIC_MAX_LEN];
    uint8_t topic_len;
    int msg_id;
} app_mqtt_client_rx_data;



static void app_mqtt_connected_event(esp_mqtt_client_handle_t client){
    printi("连接成功!");
    //订阅主题
    esp_mqtt_client_subscribe(client,mqtt_cfg.credentials.client_id,0);
    //发布消息
    esp_mqtt_client_publish(client,mqtt_cfg.credentials.client_id,"{}",2,0,0);
}


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    printi("%s:%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    printi("session_present:%d",event->session_present);
    switch ((esp_mqtt_event_id_t)event_id){
    case MQTT_EVENT_CONNECTED:{     //已连接
        app_mqtt_connected_event(client);
        break;
    }
    case MQTT_EVENT_SUBSCRIBED:{    //订阅主题事件
        printi("MQTT订阅主题:");
        printi("\tmsg_id:%d", event->msg_id);
        printi("\ttopic:%.*s", event->topic_len,event->topic);
        printi("\tqos:%d",event->qos);
        printi("\tdata:%.*s",event->data_len,event->data);
        break;
    }
    case MQTT_EVENT_DATA:{          //接收到数据
        app_mqtt_client_rx_data.data_len = MIN(event->data_len, MQTT_RX_MAX_LEN);   
        memcpy(app_mqtt_client_rx_data.data, event->data, app_mqtt_client_rx_data.data_len);
        app_mqtt_client_rx_data.topic_len = MIN(event->topic_len, MQTT_TOPIC_MAX_LEN);
        memcpy(app_mqtt_client_rx_data.topic, event->topic, app_mqtt_client_rx_data.topic_len);
        app_mqtt_client_rx_data.msg_id = event->msg_id;
        xTaskNotify(app_mqtt_task_handle, 0, eNoAction);    //通知任务收到消息
        break;
    }
    default:
        break;
    }
}

static void wifi_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    if (base == IP_EVENT){  //IP事件
        if (event_id == IP_EVENT_STA_GOT_IP){   //获取到IP
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            printi("Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
            esp_mqtt_client_start(mqtt_client_handle);  // 获取到ip则启动mqtt客户端
        }
    }
    else if (base == WIFI_EVENT){   //Wifi事件
        if (event_id == WIFI_EVENT_STA_DISCONNECTED){   //Wifi断开
            esp_mqtt_client_stop(mqtt_client_handle);   // Wifi断开则停止mqtt客户端
        }
    }
}

static void app_mqtt_task(void *arg)
{
    while (1)
    {
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        printi("MQTT收到数据:");
        printi("\tmsg_id:%d", app_mqtt_client_rx_data.msg_id);
        printi("\ttopic:%.*s", app_mqtt_client_rx_data.topic_len, app_mqtt_client_rx_data.topic);
        printi("\tdata:%.*s", app_mqtt_client_rx_data.data_len, app_mqtt_client_rx_data.data);
    }
}
char* app_mqtt_create_id_string(void){
    uint8_t mac[6];
    char *id_string = calloc(1, 32);
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(id_string, "%02x%02X%02X%02x%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return id_string;
}
esp_err_t app_mqtt_client_init(void){
    if (mqtt_client_handle){
        printi("mqtt客户端已初始化!");
        return ESP_OK;
    }
    mqtt_cfg.broker.address.uri = MQTT_CLIENT_URI;
    mqtt_cfg.credentials.client_id = app_mqtt_create_id_string();
    mqtt_client_handle = esp_mqtt_client_init(&mqtt_cfg);
    
    esp_mqtt_client_register_event(mqtt_client_handle, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL); //注册mqtt事件
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);           //注册IP事件
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL); //注册WiFi断开事件
    xTaskCreate(app_mqtt_task, "mqtt_task", MQTT_TASK_STACK_SIZE, NULL, 3, &app_mqtt_task_handle);  //创建MQTT任务
    return ESP_OK;
}

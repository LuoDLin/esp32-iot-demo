#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"
#include "button.h"

#define TAG "APP_DEVICE"
#define DEVICE_INFO(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define DEVICE_ERROR(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)


static void IRAM_ATTR button_isr_handler(void *arg){
    BaseType_t xHigherPriorityTaskWoken = 0;
    button_dev_t *handle = (button_dev_t*)arg;
    int level = gpio_get_level(handle->gpio_num);
    if( level == handle->active_level ){
        handle->status = BUTTON_STATE_PUSH;
        handle->last_push_time = xTaskGetTickCountFromISR();
    }else{
        handle->status = BUTTON_STATE_IDLE;
    }
    xTaskNotifyFromISR(handle->task_handle,(uint32_t)arg,eSetValueWithOverwrite,&xHigherPriorityTaskWoken);
}

static void button_config(button_dev_t* handle){
    static bool init_tag = false;
    if( !init_tag ){
        gpio_install_isr_service(0);
        init_tag = true;
    }
    gpio_config_t gpio_button_config = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pin_bit_mask = (1ULL<<handle->gpio_num),
    };
    if(handle->active_level){
        gpio_button_config.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_button_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    }else{
        gpio_button_config.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_button_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    }
    gpio_config(&gpio_button_config);
    gpio_isr_handler_add(handle->gpio_num, button_isr_handler, (void*)handle);
}

static void button_task(void* args){
    button_dev_t* handle = (button_dev_t*)args;
    bool rtn;
    uint32_t pulNotificationValue;
    while(1){
        rtn = false;
        xTaskNotifyWait(0,0,&pulNotificationValue,portMAX_DELAY);       //等待按下
        if( handle->status == BUTTON_STATE_PUSH ){
            if( handle->cb[BUTTON_EVENT_PUSH] ){
                rtn = handle->cb[BUTTON_EVENT_PUSH](handle,BUTTON_EVENT_PUSH);
                if( rtn ) goto wait_idle;
            }
            if ( handle->long_press_time > 0&&xTaskNotifyWait(0,0,&pulNotificationValue,handle->long_press_time) == pdFALSE)
                if(handle->status == BUTTON_STATE_PUSH) 
                    if( handle->cb[BUTTON_EVENT_LONG_PRESS] ){
                        rtn = handle->cb[BUTTON_EVENT_LONG_PRESS](handle,BUTTON_EVENT_LONG_PRESS);
                        if( rtn ) goto wait_idle;
                    }
        }
wait_idle:
        while(handle->status == BUTTON_STATE_PUSH) xTaskNotifyWait(0,0,&pulNotificationValue,portMAX_DELAY);
        if( !rtn && handle->cb[BUTTON_EVENT_IDLE] ) handle->cb[BUTTON_EVENT_IDLE](handle,BUTTON_EVENT_IDLE);
    }
}

button_dev_t* button_create(gpio_num_t gpio_num,button_active_t active_level,uint32_t task_stack,uint32_t task_prior){
    button_dev_t* handle = malloc(sizeof(button_dev_t));
    memset(handle,0,sizeof(button_dev_t));
    handle->gpio_num = gpio_num;
    handle->active_level = active_level;
    handle->task_stack = task_stack;
    handle->task_prior = task_prior;
    handle->long_press_time = 300;
    button_config(handle);
    char str[16] = "button_01";
    sprintf(str,"button_%02d",handle->gpio_num);
    xTaskCreate(button_task,str,handle->task_stack,(void*)handle,handle->task_prior,&handle->task_handle);
    return handle;
}
void button_set_longPress_tick(button_dev_t * handle,TickType_t tick){
    if( tick > 0 ) handle->long_press_time = tick;
}
void button_event_register(button_dev_t *handle, button_event_t event , button_cb cb){
    if(event == BUTTON_EVENT_ANY) 
        for( int i = 0 ; i < BUTTON_EVENT_COUNT - 1 ; i ++ )
            handle->cb[i] = cb;
    else
        handle->cb[event] = cb; 
}










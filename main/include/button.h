#ifndef _BUTTON_H_
#define _BUTTON_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define BUTTON_EVENT_COUNT 4        //事件数量
typedef enum {
    BUTTON_EVENT_IDLE = 0,
    BUTTON_EVENT_PUSH,
    BUTTON_EVENT_LONG_PRESS,
    BUTTON_EVENT_ANY
} button_event_t;
typedef bool (*button_cb)(void*,button_event_t);
typedef enum {
    BUTTON_ACTIVE_LOW = 0,      /*!<button active level: low level*/
    BUTTON_ACTIVE_HIGH,    /*!<button active level: high level*/
} button_active_t;
typedef enum {
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_PUSH
} button_status_t;
typedef struct{
    gpio_num_t      gpio_num;       //引脚编号
    button_active_t active_level;   //促发方式 
    TickType_t      long_press_time;//长按时间
    uint32_t        task_stack;     //任务堆栈
    uint32_t        task_prior;     //任务等级
    TaskHandle_t    task_handle;    //任务句柄
    button_status_t status;         //当前状态
    TickType_t      last_push_time; //上次按下时间
    button_cb       cb[BUTTON_EVENT_COUNT-1];
}button_dev_t;

button_dev_t* button_create(gpio_num_t gpio_num,button_active_t active_level,uint32_t task_stack,uint32_t task_prior);
void button_event_register(button_dev_t *handle, button_event_t event , button_cb cb);
void button_set_longPress_tick(button_dev_t * handle,TickType_t tick);
#endif
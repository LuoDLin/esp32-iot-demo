#include "button.h"
#include "app_button.h"
#include "app_provision.h"

#define BUTTON_GPIO GPIO_NUM_0          // button gpio
#define BUTTON_TASK_STACK_SIZE 1024 * 4 // button task stack
#define BUTTON_LONG_PRESS_TICK 3000     // button long press time
button_dev_t *button_device;

// 长按检测
bool app_device_button_longPress_event_handler(void *arg, button_event_t event)
{
    if (event == BUTTON_EVENT_LONG_PRESS)
    {
        printi("长按");
        ESP_ERROR_CHECK(app_smartconfig_start());
        return true;
    }
    return false;
}
// 松手检测
bool app_device_idle_event_handler(void *arg, button_event_t event)
{
    if (event == BUTTON_EVENT_IDLE)
    {
    }
    return true;
}

// 按钮初始化
void app_button_init(void)
{
    button_device = button_create(BUTTON_GPIO, BUTTON_ACTIVE_LOW, BUTTON_TASK_STACK_SIZE, 2);
    button_set_longPress_tick(button_device, BUTTON_LONG_PRESS_TICK);
    button_event_register(button_device, BUTTON_EVENT_LONG_PRESS, app_device_button_longPress_event_handler);
    button_event_register(button_device, BUTTON_EVENT_IDLE, app_device_idle_event_handler);
}

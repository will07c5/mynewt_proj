#include <nrf_temp.h>
#include <assert.h>
#include <stdint.h>

#include "ble_temp_sensor.h"

/* Temperature task */
#define TEMP_TASK_PRIO (1)
#define TEMP_STACK_SIZE (256)
static os_stack_t temp_stack[TEMP_STACK_SIZE];
static struct os_task temp_task;

/* Temperature history buffer */
#define TEMP_HIST_COUNT (10)
#define TEMP_SAMPLE_INTERVAL_MS (1000)
static uint32_t temp_sample_interval_ticks = 0;
static uint16_t temp_history[TEMP_HIST_COUNT];
static int temp_history_cur = 0;
static struct os_mutex temp_lock;

/* Returns the internal temperature of the nRF52 in degC (2 decimal places, scaled) */
int16_t
get_temp_measurement(void)
{
    int16_t temp;
    /* Start the temperature measurement. */
    NRF_TEMP->TASKS_START = 1;
    while(NRF_TEMP->EVENTS_DATARDY != TEMP_INTENSET_DATARDY_Set) {};
    /* Temp reading is in units of 0.25degC, so divide by 4 to get in units of degC
     * (scale by 100 to avoid representing as decimal). */
    temp = (nrf_temp_read() * 100) / 4.0;

    return temp;
}

void
append_temp_measurement(uint16_t temp)
{
    LOG(INFO, "appending value=%i\n", temp);
    os_error_t err = os_mutex_pend(&temp_lock, OS_TIMEOUT_NEVER);
    assert(err == 0);

    /* Write new temp value to history */
    temp_history[temp_history_cur] = temp;
    temp_history_cur = (temp_history_cur + 1) % TEMP_HIST_COUNT;

    err = os_mutex_release(&temp_lock);
    assert(err == 0);
    LOG(INFO, "done appending\n");
}

static void
temp_task_func(void *arg)
{
    /* Loop forever sampling temperature */
    while (1) {
        append_temp_measurement(get_temp_measurement());

        os_time_delay(temp_sample_interval_ticks);
    }
}

void
start_temp_measurements(void)
{
    /* Convert sampling interval in ms to ticks */
    int rc = os_time_ms_to_ticks(
        TEMP_SAMPLE_INTERVAL_MS, &temp_sample_interval_ticks);
    assert(rc == 0);

    /* Init a mutex for controlling access to history buffer */
    rc = os_mutex_init(&temp_lock);
    assert(rc == 0);

    /* Init task for periodically sampling temperature */
    os_task_init(&temp_task, "temp_task", temp_task_func, NULL, TEMP_TASK_PRIO,
        OS_WAIT_FOREVER, temp_stack, TEMP_STACK_SIZE);
}
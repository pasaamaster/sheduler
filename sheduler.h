/**
 ******************************************************************************
 * @file    sheduler.h
 * @author
 * @version V0.0.1
 * @date    2017
 * @brief   Cooperative task sheduler with non-blocking delays
 *  - We have an array of tasks
 *  - Each task consists of:
 *      - function pointer (.function)
 *      - time before task execution (.timeLeft)
 *      - task execution perion (.period)
 *  - Each task has several states:
 *      - DISABLED          (when .timeLeft==UINT32_MAX)
 *      - READY to execute  (when .timeLeft==0)
 *      - BLOCKED by delay  (when .timeLeft>0 and .timeLeft<UINT32_MAX)
 *  - Task can be added: routine will find free place in task array, place
 *    task here and return array index as task handler
 *  - Task can be removed: array after removed task will be shifted and last
 *    cell will be erased
 *  - Task can be set READY, DISABLED and BLOCKED.
 *  - Sheduler will analize task array, execure READY tasks and decrease
 *    execute delay in BLOCKED tasks in each cycle
 *  - At the end of each cycle sheduler will find minimal delay of BLOCKED
 *    tasks in array and return it to use in sleep function
 *  - There is also a sheduler test function for use example
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __SHEDULER_H
#define __SHEDULER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

#define SHEDULER_MAX_TASK_NUM   11
#define SHEDULER_DEBUG_CHECK    0 // Disable run-time checks for faster execution
#define SHEDULER_DEBUG          0

// TODO addd tick_MS_VALUE

/* Exported types ------------------------------------------------------------*/

typedef uint32_t Timestamp_t;

struct TaskStr_t {
    void (*function)(void);     // Function to call for task's tick
    uint32_t            timeLeft;   // Time before the execution of tasks
    uint32_t            period;     // Rate at which the task should tick or 0 for non-repeated tasks
    struct TaskStr_t*   prev;
    struct TaskStr_t*   next;
};
typedef struct TaskStr_t* Task_t;

/* Exported macro ------------------------------------------------------------*/

#define RTC_DAY_TICK_NUM   UINT32_MAX

// Dispatcher HAL
#ifndef ERROR
#define ERROR   0
#endif
#ifndef SUCCESS
#define SUCCESS !ERROR
#endif

#define sheduler_getTimestamp()      rtc_getTimeStamp()   // Get timestamp fromRTC for example
#define sheduler_startTimer()        // start RTC for example to wakeup CPU from sleep/stop mode
#define sheduler_sleep()             // sleep(ms)
#define DBG(msg)
#define ERR(msg)

/* Exported functions --------------------------------------------------------*/

void sheduler_init(void);
Timestamp_t sheduler(void);
void sheduler_addTask(Task_t task, void (*func)(void),  Timestamp_t delay, Timestamp_t period);
bool sheduler_removeTask(Task_t task);
bool sheduler_removeThisTask(void); // Note: use only in only-dispatcher-handled functions. Else ERROR.

bool sheduler_readyTask(Task_t task);
bool sheduler_delayTask(Task_t task, Timestamp_t delay);
bool sheduler_delayThisTask(Timestamp_t delay); // Note: use only in only-dispatcher-handled functions. Else ERROR.
bool sheduler_blockTask(Task_t task);
bool sheduler_blockThisTask(void); // Note: use only in only-dispatcher-handled functions. Else ERROR.
bool sheduler_setTaskPeriod(Task_t task, Timestamp_t period);
Timestamp_t sheduler_timestamp(void);
bool sheduler_isBlocked(Task_t task);
bool sheduler_isTask(Task_t task);

void sheduler_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __SHEDULER_H */

/**************************** END OF FILE *************************************/

/**
 ******************************************************************************
 * @file    sheduler.c
 * @author
 * @version V0.0.0
 * @date    2017
 * @brief   Cooperative task sheduler with non-blocking delays
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "sheduler.h"
#include <string.h>
//#include "rtc.h"
//#include "trace_put.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static Task_t       FirstTask, LastTask, CurrentTask;
static Timestamp_t  TimeStamp = 0;
static Timestamp_t  OldTimeStamp = 0;

/* Private macro -------------------------------------------------------------*/

// Note: .timeLeft==0 -> task_ready;  >0 -> task_blocked; ==MAX -> task_disabled
#define TASK_IS_READY(h)        (h->timeLeft == 0)
#define TASK_IS_BLOCKED(h)      (h->timeLeft == UINT32_MAX)
#define TASK_IS_DELAYED(h)      (h->timeLeft > 0)
#define TASK_IS_VALID(h)        (h->function != 0)

#define TASK_SET_READY(h)       h->timeLeft = 0
#define TASK_SET_BLOCKED(h)     h->timeLeft = UINT32_MAX
#define TASK_SET_DELAYED(h,d)   h->timeLeft = d

#define TASK_IS_HANDLE(h)       (h != 0)

#define TASK_EXECUTE(h)         do{ CurrentTask = h; \
                                    h->function(); \
                                    CurrentTask = NULL; }while(0)

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void sheduler_init(void)
{
    FirstTask = LastTask = CurrentTask = 0;
    sheduler_startTimer();
    sheduler_getTimestamp(); // init oldTimeStamp
}

Timestamp_t sheduler(void)
{
    // Get timestamp delta
    TimeStamp = sheduler_getTimestamp();
    Timestamp_t dt = TimeStamp - OldTimeStamp;
    //trace_putDecStr("  dt=", (int32_t)dt, "\n");
    OldTimeStamp = TimeStamp;

    Timestamp_t minDelay = RTC_DAY_TICK_NUM - 1; // max RTC delay
    CurrentTask = FirstTask;
    while (CurrentTask)
    {
        CurrentTask = CurrentTask->next;
        if (!TASK_IS_VALID(CurrentTask))
        {
            ERR(""); // func must be valid. Comment after debug
        }
        if (TASK_IS_BLOCKED(CurrentTask))
        {
            continue;
        }
        // Time sheduler
        if (TASK_IS_DELAYED(CurrentTask))
        {
            if (CurrentTask->timeLeft > dt)
            {
                CurrentTask->timeLeft -= dt; // leave
            }
            else
            {
                TASK_SET_READY(CurrentTask);
            }
        }
        // Run task
        if (TASK_IS_READY(CurrentTask))
        {
            TASK_EXECUTE(CurrentTask);
            if (TASK_IS_READY(CurrentTask))
            {
                if (CurrentTask->period > 0)
                {
                    TASK_SET_DELAYED(CurrentTask, CurrentTask->period);
                }
            }
        }
        // Get min delay
        // TODO in separate loop to find new chenges in task list
        if (TASK_IS_DELAYED(CurrentTask))
        {
            if (minDelay > CurrentTask->timeLeft)
            {
                minDelay = CurrentTask->timeLeft;
            }
        }
    }

    if (minDelay >= RTC_DAY_TICK_NUM)
    {
        trace_putDecStr("errStop=", (int32_t)minDelay, "\n");
    }

    return minDelay;
}

void sheduler_addTask(Task_t task, void (*func)(void),  Timestamp_t delay, Timestamp_t period)
{
#if SHEDULER_DEBUG_CHECK
    if (!func || !task) { ERR("ParamErr"); }
#endif

    task->function = func;
    task->timeLeft = delay;
    task->period = period;

    task->next = NULL;
    if (FirstTask)
    {
        task->prev = LastTask;
        LastTask->next = task;
    }
    else
    {
        task->prev = NULL;
        FirstTask = task;
    }
    LastTask = task;
}

bool sheduler_removeTask(Task_t task)
{
#if SHEDULER_DEBUG_CHECK
    if (!task) { ERR("ParamErr"); }
#endif

    if (task->function)
    {
        task->function = 0;
        if (task->prev)
        {
            task->prev->next = task->next;
        }
        if (task->next)
        {
            task->next->prev = task->prev;
        }
    }
    task->prev = task->next = NULL;

    return SUCCESS;
}

bool sheduler_removeThisTask(void)
{
    return sheduler_removeTask(CurrentTask);
}

bool sheduler_readyTask(Task_t task)
{
#ifdef SHEDULER_ALL_CHECKS
    if (!task || !TASK_IS_VALID(task)) { ERR("Task handle error\n"); }
#endif

    TASK_SET_READY(task);

    return SUCCESS;
}

bool sheduler_delayTask(Task_t task, Timestamp_t delay)
{
#ifdef SHEDULER_ALL_CHECKS
    if (!TASK_IS_HANDLE(handle) || !TASK_IS_VALID(handle))
    {
        PRINTF("Task handle error\n");
        return ERROR;
    }
#endif

    TASK_SET_DELAYED(task, delay);

    return SUCCESS;
}

bool sheduler_delayThisTask(Timestamp_t delay)
{
#ifdef SHEDULER_ALL_CHECKS
    if (!TASK_IS_HANDLE(CurrentTask) || !TASK_IS_VALID(CurrentTask)) { ERR("Task error\n"); }
#endif

    TASK_SET_DELAYED(CurrentTask, delay);

    return SUCCESS;
}

bool sheduler_blockTask(Task_t task)
{
#ifdef SHEDULER_ALL_CHECKS
    if (!TASK_IS_HANDLE(handle) || !TASK_IS_VALID(handle)) { ERR("Task handle error\n"); }
#endif

    TASK_SET_BLOCKED(task);

    return SUCCESS;
}

bool sheduler_blockThisTask(void)
{
#ifdef SHEDULER_ALL_CHECKS
    if (!TASK_IS_HANDLE(handle) || !TASK_IS_VALID(handle)) { ERR("Task handle error\n"); }
#endif

    TASK_SET_BLOCKED(CurrentTask);

    return SUCCESS;
}

bool sheduler_setTaskPeriod(Task_t task, Timestamp_t period)
{
#ifdef SHEDULER_ALL_CHECKS
    if (!TASK_IS_HANDLE(handle) || !TASK_IS_VALID(handle)) { ERR("Task handle error\n"); }
#endif

    task->period = period;

    return SUCCESS;
}

uint32_t sheduler_timestamp(void)
{
    return TimeStamp;
}

bool sheduler_isBlocked(Task_t task)
{
#ifdef SHEDULER_ALL_CHECKS
    if (!TASK_IS_HANDLE(handle)) { ERR("Task handle error\n"); }
#endif

    return TASK_IS_BLOCKED(task);
}

bool sheduler_isTask(Task_t task)
{
    return (task && task->function);
}

/**************************** END OF FILE *************************************/

# sheduler

Small cooperative task sheduler for low power devices.

Cooperative task sheduler with non-blocking delays
 *  We have an array of tasks
 *  Each task consists of:
        - function pointer (.function)
        - time before task execution (.timeLeft)
        - task execution perion (.period)
 *  Each task has several states:
        - DISABLED          (when .timeLeft==UINT32_MAX)
        - READY to execute  (when .timeLeft==0)
        - BLOCKED by delay  (when .timeLeft>0 and .timeLeft<UINT32_MAX)
 *  Task can be added: routine will find free place in task array, place task here and return array index as task handler
 *  Task can be removed: array after removed task will be shifted and last cell will be erased
 *  Task can be set READY, DISABLED and BLOCKED.
 *  Sheduler will analize task array, execure READY tasks and decrease execute delay in BLOCKED tasks in each cycle
 *  At the end of each cycle sheduler will find minimal delay of BLOCKED tasks in array and return it to use in sleep function
 *  There is also a sheduler test function for use example

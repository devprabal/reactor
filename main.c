#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <threads.h>
#include <unistd.h>

#define DBG \
    printf("\nPrabal's debug: %s:%s()L:%d\n", __FILE__, __func__, __LINE__)

typedef struct ReactorEvent
{
    char data[20];
} ReactorEvent;

static thrd_t task_id;
static bool task_run_status = true;
static sem_t queue_sem;
static mtx_t queue_lock;
static ReactorEvent simple_queue[10] = { 0 };
static int q_idx                     = 0;

int task_function(void *arg)
{
    while (task_run_status) {
        DBG;
        sem_wait(&queue_sem);
        if (task_run_status == false) {
            break;
        }
        bool is_event_available = false;
        ReactorEvent event      = { 0 };
        mtx_lock(&queue_lock);
        if (q_idx > 0) {
            event              = simple_queue[0];
            is_event_available = true;
            for (int i = 1; i < q_idx; i++) {
                simple_queue[i - 1] = simple_queue[i];
            }
            q_idx--;
        }
        mtx_unlock(&queue_lock);
        //// perform logic on event after lock to avoid performance wastage in
        /// critical section
        if (is_event_available) {
            DBG;
            printf("\nPrabal's debug: event data = %s\n", event.data);
            is_event_available = false;
        }
    }
    return 0;
}

bool reactor_create(void)
{
    sem_init(
        &queue_sem, 0, 0);  //// 0 permits because 0 events available initally
    mtx_init(&queue_lock, mtx_plain);
    if(thrd_success == thrd_create(&task_id, task_function, NULL)) {
        return true;
    }
    return false;
}

bool reactor_send(ReactorEvent event)
{
    mtx_lock(&queue_lock);
    if (q_idx >= 10) {
        for (int i = 1; i < 10; i++) {
            //// drop the earliest event
            simple_queue[i - 1] = simple_queue[i];
        }
        q_idx = 9;
    }
    simple_queue[q_idx] = event;
    q_idx++;
    printf("\nq_idx = %d\n", q_idx);
    mtx_unlock(&queue_lock);
    sem_post(&queue_sem);  //// +1 event is available
    return true;
}

bool reactor_destroy(void)
{
    int res = -1;
    sleep(1);
    task_run_status = false;
    //// let the thread finish by bluffing that an event is available
    sem_post(&queue_sem);
    thrd_join(task_id, &res);
    DBG;
    mtx_destroy(&queue_lock);
    sem_destroy(&queue_sem);
    printf("\nres=%d\n", res);
    return true;
}

int main(void)
{
    reactor_create();
    ReactorEvent e1  = { .data = "one" };
    ReactorEvent e2  = { .data = "two" };
    ReactorEvent e3  = { .data = "three" };
    ReactorEvent e4  = { .data = "four" };
    ReactorEvent e5  = { .data = "five" };
    ReactorEvent e6  = { .data = "six" };
    ReactorEvent e7  = { .data = "seven" };
    ReactorEvent e8  = { .data = "eight" };
    ReactorEvent e9  = { .data = "nine" };
    ReactorEvent e10 = { .data = "ten" };
    ReactorEvent e11 = { .data = "eleven" };
    ReactorEvent e12 = { .data = "tweleve" };
    reactor_send(e1);
    reactor_send(e2);
    reactor_send(e3);
    reactor_send(e4);
    reactor_send(e5);
    reactor_send(e6);
    reactor_send(e7);
    reactor_send(e8);
    reactor_send(e9);
    reactor_send(e10);
    reactor_send(e11);
    reactor_send(e12);
    reactor_destroy();
    return 0;
}

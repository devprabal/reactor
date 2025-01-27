#include <stdio.h>
#include <threads.h>
#include <unistd.h>
#include <stdbool.h>

#define DBG printf("\nPrabal's debug: %s:%s()L:%d\n", __FILE__, __func__, __LINE__)

typedef struct ReactorEvent {
    char data[20];
} ReactorEvent;

static thrd_t task_id;
static bool task_run_status = true;
static mtx_t event_lock;
static mtx_t queue_lock;
static ReactorEvent simple_queue[10] = {0};
static int q_idx = 0;

int task_function(void* arg)
{
    while(task_run_status) {
        DBG;
        mtx_lock(&event_lock);
        if(task_run_status == false) {
            break;
        }
        mtx_lock(&queue_lock);
        if(q_idx > 0) {
            ReactorEvent event = simple_queue[0];
            for(int i = 1; i < q_idx; i++){
                simple_queue[i-1] = simple_queue[i];
            }
            q_idx--;
            DBG;
            printf("\nPrabal's debug: event data = %s\n", event.data);
        }
        mtx_unlock(&queue_lock);
    }
    return 0;
}

bool reactor_create(void)
{
    mtx_init(&event_lock, mtx_plain);
    mtx_init(&queue_lock, mtx_plain);
    mtx_lock(&event_lock);
    thrd_create(&task_id, task_function, NULL);
    return true;
}

bool reactor_send(ReactorEvent event)
{
    mtx_lock(&queue_lock);
    if(q_idx >= 10) {
        for(int i = 1; i < 10; i++){
            //// drop the earliest event
            simple_queue[i-1] = simple_queue[i];
        }
        q_idx = 9;
    }
    simple_queue[q_idx] = event;
    q_idx++;
    mtx_unlock(&queue_lock);
    mtx_unlock(&event_lock);
    return true;
}

bool reactor_destroy(void)
{
    int res = -1;
    sleep(1);
    task_run_status = false;
    mtx_unlock(&event_lock);
    thrd_join(task_id, &res);
    DBG;
    mtx_destroy(&event_lock);
    printf("\nres=%d\n", res);
    return true;
}

int main(void)
{
    reactor_create();
    ReactorEvent e1 = {.data = "hithere"};
    ReactorEvent e2 = {.data = "yesplease"};
    ReactorEvent e3 = {.data = "nonever"};
    ReactorEvent e4 = {.data = "bye!"};
    sleep(1);
    reactor_send(e1);
    sleep(1);
    reactor_send(e2);
    sleep(1);
    reactor_send(e3);
    sleep(1);
    reactor_send(e4);
    sleep(1);
    reactor_destroy();
    return 0;
}
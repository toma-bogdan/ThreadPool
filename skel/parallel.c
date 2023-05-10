#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "os_list.h"

#define MAX_TASK 10000
#define MAX_THREAD 4

int sum = 0;
pthread_mutex_t traversal_mutex;
pthread_mutex_t mutex;
os_graph_t *graph;

void processNode(void *arg)
{
    // Process the given node
    os_node_t *node = (os_node_t*)arg;
    pthread_mutex_lock(&(traversal_mutex));
    if (graph->visited[node->nodeID] == 0) {
        sum += node->nodeInfo;
        graph->visited[node->nodeID] = 1;
    }
    pthread_mutex_unlock(&(traversal_mutex));

    // Process the nodes's neighbours
    for (int i = 0; i < node->cNeighbours; i++) {
        os_node_t *aux = graph->nodes[node->neighbours[i]];

        pthread_mutex_lock(&(traversal_mutex));
        if (graph->visited[aux->nodeID] == 0) {
            sum += aux->nodeInfo;
            graph->visited[aux->nodeID] = 1;
        }
        pthread_mutex_unlock(&(traversal_mutex));
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./main input_file\n");
        exit(1);
    }

    FILE *input_file = fopen(argv[1], "r");

    if (input_file == NULL) {
        printf("[Error] Can't open file\n");
        return -1;
    }

    graph = create_graph_from_file(input_file);
    if (graph == NULL) {
        printf("[Error] Can't read the graph from file\n");
        return -1;
    }

    pthread_mutex_init(&traversal_mutex,NULL);
    pthread_mutex_init(&mutex,NULL);

    os_threadpool_t *tp = threadpool_create(MAX_TASK,MAX_THREAD);
    for (int i = 0; i < graph->nCount; ++i) {
        pthread_mutex_lock(&mutex);
        if (graph->visited[i] == 0) {
            os_task_t *task = task_create(graph->nodes[i], &processNode);
            add_task_in_queue(tp, task);
        }
        pthread_mutex_unlock(&mutex);
    }
    for (int i = 0; i < tp->num_threads; i++) {
        
        int res = pthread_create(&tp->threads[i], NULL, thread_loop_function, tp);
        
        if (res) {
            printf("Error at initializing thread with id %d\n", i);
            exit(-1);
        }
    }

    int res;
    void *status;
    for (int i = 0; i < tp->num_threads; i++) {
        res = pthread_join(tp->threads[i], &status);

        if (res) {
            printf("Error at waiting thread: %d\n", i);
            exit(-1);
        }
    }
    printf("%d", sum);

    pthread_mutex_destroy(&traversal_mutex);
    pthread_mutex_destroy(&tp->taskLock);
    pthread_mutex_destroy(&mutex);

    return 0;
}

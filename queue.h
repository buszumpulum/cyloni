
typedef struct Entry{
  int id;
  int lamport_clock;
  int meeting_id;
  int cylon_id;
  struct Entry* next;
} queue_entry;


typedef struct{
  int id;
  queue_entry* entries;
} r_queue;

//QUEUE FUNCTIONS

r_queue* queue_init(int id);//TESTED

int queue_free(r_queue* queue);//TESTED

int queue_add(r_queue* queue, int id, int lamport_clock);//TESTED

int queue_set_meeting(r_queue* queue, int id, int meeting);//TESTED
int queue_set_meeting_by_id(r_queue* queue, int id, int meeting);

int queue_set_cylon(r_queue* queue, int send_id, int cylon);//TESTED

void queue_sort(r_queue* queue);//TODO

queue_entry* queue_get(r_queue* queue, int id);//TESTED

queue_entry queue_get_top(r_queue* queue);

int queue_get_top_meeting(r_queue* queue);

int queue_get_id(r_queue* queue, int num);

int queue_get_lamport(r_queue* queue, int num);

int queue_remove(r_queue* queue, int id);//TESTED

int queue_to_array(r_queue* queue, queue_entry array[]);//PROBABLY POINTLESS

int queue_size(r_queue* queue);//TESTED

int queue_top(r_queue* queue, int *id, int *lamport);//TESTED

int queue_position(r_queue* queue, int id);//TESTED

int queue_count_with_meeting(r_queue* queue);

int queue_count_below_limit(r_queue* queue, int cylons);

int queue_clean(r_queue* queue);
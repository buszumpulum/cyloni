
typedef struct{
  int id;
  int lamport_clock;
  int meeting_id;
  int cylon_id;
  queue_entry* next;
} queue_entry;


typedef struct{
  int id;
  queue_entry* entries;
} r_queue;

//QUEUE FUNCTIONS

int queue_init(r_queue* queue, int id);

int queue_free(r_queue* queue);

int queue_add(r_queue* queue, int id, int lamport_clock);

int queue_set_meeting(r_queue* queue, int id, int meeting);

int queue_set_cylon(r_queue* queue, int send_id, int cylon);

void queue_sort(r_queue* queue);

queue_entry* queue_get(r_queue* queue, int id);

int queue_remove(r_queue* queue, int id);

int queue_to_array(r_queue* queue, r_queue[] array);

int queue_size(r_queue* queue);

int queue_top(r_queue* queue);

int queue_position(r_queue* queue, int id);
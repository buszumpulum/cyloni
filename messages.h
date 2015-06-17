typedef struct msg{
  int id;
  int lamport;
  int institute;
  int meeting;
} msg;

typedef struct Msg_Entry {
  msg message;
  int type;
  struct Msg_Entry* next;
} msg_entry;

typedef struct{
  msg_entry* entries;
} m_queue;

//QUEUE FUNCTIONS

m_queue* m_queue_init();

void m_queue_free(m_queue* queue);

void m_queue_insert(m_queue* queue, msg message, int type);

msg m_queue_get(m_queue* queue, int type);

int m_queue_size(m_queue* queue, int type);
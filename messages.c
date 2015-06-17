#include "messages.h"
#include <stdlib.h>

m_queue* m_queue_init()
{
   m_queue* queue;
   queue = malloc(sizeof(m_queue));
   
   if(queue==NULL)
    return queue;
   
   queue->entries=malloc(sizeof(msg_entry));
   queue->entries->type=-1;
   queue->entries->next=NULL;
   return queue;
}

void m_queue_free(m_queue* queue)
{
  msg_entry* field = queue->entries;
  while(field!=NULL)
  {
    msg_entry* rest = field->next;
    free(field);
    field=rest;
  }
  free(queue);
}

void m_queue_insert(m_queue* queue, msg message, int type)
{
  msg_entry* entry = malloc(sizeof(msg_entry));
  
  if(entry==NULL)
    return;
  
  entry->type=type;
  entry->message=message;
  entry->next=NULL;
  
  msg_entry* field = queue->entries;
  
  while(field->next!=NULL)
    field=field->next;
  field->next = entry;
}


msg m_queue_get(m_queue* queue, int type)
{
  msg message;
  message.id=-1;
  msg_entry* field = queue->entries->next;
  while(field->next!=NULL && (field->next->type==type || type==-1))
    field=field->next;
  if(field->next==NULL)
    return message;
  msg_entry* entry = field->next;
  field->next = entry->next;
  
  message=entry->message;
  
  free(entry);
  return message;
}

int m_queue_size(m_queue* queue, int type)
{
  int i=0;
  
  msg_entry* entry = queue->entries;
  if(entry->next==NULL)
    return 0;
  entry=entry->next;
  while(entry!=NULL)
  {
    if(entry->type==type || type==-1)
      i++;
    entry=entry->next;
  }
  
  return i;
}




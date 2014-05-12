#include "vp_list.h"
#include <string.h>

#ifdef NODE_TYPE_COLLECTOR
#define VP_L_START 0
#else
#define VP_L_START 1
#endif

void vp_list_init(struct vp_list *l, rimeaddr_t personal_addr)
{
  int x;
  *l = (struct vp_list){ 0 };

  for ( x = 0; x < VP_LIST_SIZE; x++)
  {
    //in general the lladdr.addr and rimeaddr of a platform should be the same.
    //the specific implementation should be found in the platforms main file
    l->e[x].r.record_addr = rimeaddr_null;
//    l->e[x].r.r_seqid = 0;
//    l->e[x].r.r_est_age = 0;
//    l->e[x].r.r_hops = 0;
//    l->e[x].r.heart_rate = 0;
    l->e[x].r.spo2 = x+1;
    l->e[x].r.bpm = x+2;
    l->e[x].r.temperature = x+3;
//    l->e[x].rank = 0;
//    l->e[x].old_data_heard = 0;
//    l->e[x].repetitions_over_interval = 0;
//    l->e[x].intervals_since_last_send = 0;
//    l->e[x].in_message = 0;
  }


#ifndef NODE_TYPE_COLLECTOR
  l->e[0].r.record_addr = personal_addr;
  l->e[0].rank = 10000;
#endif
}

void vp_list_update_node(struct vp_list *l, struct ripplecomm_record r)
{
  if (r.record_addr.u8[0] == l->e[0].r.record_addr.u8[0] && r.record_addr.u8[1] == l->e[0].r.record_addr.u8[1])
    {
      memcpy(&(l->e[0].r),&r,sizeof(struct ripplecomm_record));
    }
}

void vp_list_to_msg(struct vp_list *l, struct ripplecomm_vp_message *m)
{
  int a[VP_LIST_SIZE];//will be used to rank the indices based on rank value using bubble sort
  int i,j,temp;
  for (i=0;i<(VP_LIST_SIZE);i++)
  {
    a[i]=i;
  }
  /*Bubble sort based on rank*/
  for (i=0;i<(VP_LIST_SIZE-1);++i)
  {
    for (j=0;j<(VP_LIST_SIZE-1);++j)
    {
      if (l->e[a[j]].rank < l->e[a[j+1]].rank)
      {
        temp = a[j+1];
        a[j+1] = a[j];
        a[j] = temp;
      }
    }
  }
  m->header.r_dispatch=RIPPLECOMM_DISPATCH;
  m->header.r_msg_type=VITALPROP_RECORDS;
  for (i=0;i<(VP_MSG_RCD_CNT);i++)
  {
    //l->e[a[i]].r.heart_rate = l->e[a[i]].rank;//test line so we can view rank in place of heart rate
    memcpy(&(m->records[i]),&(l->e[a[i]].r),sizeof(struct ripplecomm_record));
    l->e[a[i]].intervals_since_last_send=0;
  }
}

void vp_list_from_msg(struct vp_list *l, struct ripplecomm_vp_message m)
{
  int x,y;
  for ( x = 0; x < VP_MSG_RCD_CNT; x++)
  {
    //if own or null address, ignore. otherwise, find record, if not found, add record
#ifndef NODE_TYPE_COLLECTOR
    if (!rimeaddr_cmp(&(m.records[x].record_addr),&rimeaddr_null) && !rimeaddr_cmp(&(m.records[x].record_addr),&(l->e[0].r.record_addr)))
#else
    if (!rimeaddr_cmp(&(m.records[x].record_addr),&rimeaddr_null))
#endif//NODE_TYPE_COLLECTOR
    {
      int record_found = 0;
      for ( y = VP_L_START; y < VP_LIST_SIZE; y++)
      {
        if (rimeaddr_cmp(&(m.records[x].record_addr),&(l->e[y].r.record_addr)))
        {
          record_found = 1;
          if (m.records[x].r_seqid > l->e[y].r.r_seqid)//if this is a new seqid
          {
            memcpy(&(l->e[y].r),&(m.records[x]),sizeof(struct ripplecomm_record));
            l->e[y].r.r_hops++;
            l->e[y].old_data_heard=0;
            l->e[y].repetitions_over_interval=1;
            l->e[y].in_message=0;
          }
          if (m.records[x].r_seqid < l->e[y].r.r_seqid)//if we already have a newer seqid
          {
             l->e[y].old_data_heard++;
          }
          //update rank for this record
          update_element_rank(&(l->e[y]));
          break;
        }
      }
      if (record_found == 0)
      {
        int null_found=0;
        for ( y = VP_L_START; y < VP_LIST_SIZE; y++)
        {
          if (rimeaddr_cmp(&rimeaddr_null,&(l->e[y].r.record_addr)))
          {
            //replace first null space that we find with new record
            null_found=1;
            memcpy(&(l->e[y].r),&(m.records[x]),sizeof(struct ripplecomm_record));
            l->e[y].r.r_hops++;
            l->e[y].old_data_heard=0;
            l->e[y].repetitions_over_interval=1;
            l->e[y].in_message=0;
            break;
          }
        }
        if (!null_found)
        {
          //no space left in list so find lowest rank and replace
          int z;
          z=lowest_rank_from_vp_list(l);
          memcpy(&(l->e[z].r),&(m.records[x]),sizeof(struct ripplecomm_record));
          l->e[z].r.r_hops++;
          l->e[z].old_data_heard=0;
          l->e[z].repetitions_over_interval=1;
          l->e[z].in_message=0;
        }
      }
    }
  }
}

void age_vp_list(struct vp_list *l)
{
  int y;
  for ( y = VP_L_START; y < VP_LIST_SIZE; y++)
  {
    l->e[y].r.r_est_age++;
    l->e[y].intervals_since_last_send++;
  }
}

void update_element_rank(struct vp_list_element *e)
{
  int d=0;
  if (rimeaddr_cmp(&rimeaddr_null,&(e->r.record_addr)))
  {
    e->rank = 0;
  }
  else
  {
    if ( e->r.r_est_age < age_rel )
    {
      d=rank_D;
    }
    e->rank = rank_A*e->old_data_heard + rank_B*e->repetitions_over_interval + rank_C*e->intervals_since_last_send + d;
  }
}

void update_all_ranks(struct vp_list *l)
{
  int y;
  for ( y = VP_L_START; y < VP_LIST_SIZE; y++)
  {
    update_element_rank(&(l->e[y]));
  }
}

int lowest_rank_from_vp_list(struct vp_list *l)
{
  int lowest_rank;
  int y,x;
  lowest_rank = l->e[VP_L_START].rank;
  x=1;
  for ( y = (VP_L_START+1); y < VP_LIST_SIZE; y++)
  {
    if (l->e[y].rank < lowest_rank)
    {
      lowest_rank = l->e[y].rank;
      x=y;
    }
  }
  return x;
}

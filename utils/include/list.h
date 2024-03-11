#ifndef __APP_LIST_H__
#define __APP_LIST_H__

typedef struct _node_t {
    struct _node_t * pre;
    struct _node_t * next;
} node_t;

static inline void node_init(node_t * node)
{
    node->pre = node->next = (node_t *)0;
}

static inline node_t * list_node_pre(node_t * node)
{
    return node->pre;
}

static inline node_t * list_node_next(node_t * node)
{
    return node->next;
}

typedef struct _list
{
    node_t * first;
    node_t * last;
    int count;
} list_t;

void list_init(list_t * list);

static inline int list_is_empty(list_t * list)
{
    return list -> count == 0;
}

static inline int list_count(list_t * list)
{
    return list->count;
}

static inline node_t * list_first(list_t * list)
{
    return list->first;
}

static inline node_t * list_last(list_t * list)
{
    return list->last;
}

void list_insert_first(list_t * list, node_t * node);

void list_insert_last(list_t * list, node_t * node);

node_t * list_remove_first(list_t * list);

node_t * list_remove_last(list_t * list);

node_t * list_remove(list_t * list, node_t * node);

//获取到node_name在parent_type中相对地址偏移量
#define offset_int_parent(parent_type, node_name) \
    ((uint32_t)&(((parent_type *)0)->node_name))

//获取到parent_type的地址
#define parent_addr(node, parent_type, node_name) \
    ((uint32_t)node - offset_int_parent(parent_type, node_name))

//转成parent_type*类型
#define list_node_parent(node, parent_type, node_name) \
    ((parent_type *)(node ? parent_addr(node, parent_type, node_name) : 0))


#endif
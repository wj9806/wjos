#include "list.h"

void list_init(list_t * list)
{
    list->first = list->last = (node_t *)0;
    list->count = 0;
}

void list_insert_first(list_t * list, node_t * node)
{
    //设置node
    node->next = list->first;
    node->pre = (node_t*)0;
    
    //设置list
    if (list_is_empty(list))
    {
        list->last = list->first = node;
    }
    else
    {
        list->first->pre = node;
        list->first = node;
    }
    list->count++;
}

void list_insert_last(list_t * list, node_t * node)
{
    //设置node
    node->next = (node_t*)0;
    node->pre = list->last;

        //设置list
    if (list_is_empty(list))
    {
        list->last = list->first = node;
    }
    else
    {
        list->last->next = node;
        list->last = node;
    }
    list->count++;
}

node_t * list_remove_first(list_t * list)
{
    if (list_is_empty(list))
    {
        return (node_t *)0;
    }

    node_t * removed_node = list->first;
    list->first = list->first->next;
    if (list->first == (node_t *)0)
    {
        list->last = (node_t *)0;
    }
    else
    {
        removed_node->next->pre = (node_t *)0;
    }
    list->count--;

    removed_node->pre = removed_node->next = (node_t *)0;
    return removed_node;
}

node_t * list_remove_last(list_t * list)
{
    if (list_is_empty(list))
    {
        return (node_t *)0;
    }

    node_t * removed_node = list->last;
    list->last = list->last->pre;
    if (list->last == (node_t *)0)
    {
        list->first = (node_t *)0;
    }
    else
    {
        removed_node->pre->next = (node_t *)0;
    }
    list->count--;

    removed_node->pre = removed_node->next = (node_t *)0;
    return removed_node;
}

node_t * list_remove(list_t * list, node_t * node)
{
    if (node == list->first)
    {
        list->first = node->next;
    }
    if (node == list->last)
    {
        list->last = node->pre;
    }
    
    if (node->pre)
    {
        node->pre->next = node->next;
    }

    if (node->next)
    {
        node->next->pre = node->pre;
    }

    node->pre = node->next = (node_t *)0;
    list->count--;
    return node;
}
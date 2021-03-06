#ifndef __dlist_h
#define __dlist_h

#include <assert.h>

// 双向循环链表节点, 不包含数据
typedef struct DList_node_base *DList_link;

/**
 * 双向链表节点基类
 *            ___  next      
 *           |   |------>
 *     <-----|___|       
 *       prev
 */
struct DList_node_base {
	DList_link next;
	DList_link prev;
};

// 带哨兵节点的双向循环链表
struct DList_base {
	DList_node_base nil;	// 哨兵节点
};

/**
 * 初始化链表
 *         ____
 *        |    |
 *        V    |
 *   .--[nil]--'
 *   |    A    
 *   |____|     
 */
inline 
void list_init(DList_base &list)
{
	// 初始化nil节点
	list.nil.prev = list.nil.next = &list.nil;
}

// 判断list是否为空, 
inline
bool list_is_empty(const DList_base &list)
{
	return (list.nil.next == &list.nil);
}

/**
 * 在链表的x节点前插入t节点
 *            ___                ___
 *           |   |------------->|   |------>
 *    <------|___|<-------------|___|
 *                                ^-x
 *                  ___        
 *                 |   |------>
 *            <----|___|       
 *                   ^-t       
 * =========================================
 *            ___                ___
 *           |   |------------->|   |------>
 *    <------|___|<-------------|___|
 *             A                 A ^-x
 *             |    ___          |
 *             |   |   |---------'
 *             '---|___|       
 *                   ^-t       
 * =========================================
 *            ___                ___
 *           |   |--.     .---->|   |------>
 *    <------|___|  |     |  .--|___|
 *             A    |     |  |    ^-x
 *             |    V__   |  |
 *             |   |   |--'  |
 *             '---|___|<----'
 *                   ^-t
 */
inline
void list_insert(DList_link x, DList_link t)
{
    assert(x != NULL && t != NULL);
	t->prev = x->prev;
	t->next = x;
	x->prev->next = t;
	x->prev = t;
}

/**
 * 从链表上删除x节点
 *            ___           ___           ___ 
 *           |   |-------->|   |-------->|   |------>
 *    <------|___|<--------|___|<--------|___|
 *                           ^-x       
 * ==================================================
 *                    .--------------------.
 *                    |                    |
 *            ___     |     ___           _V_ 
 *           |   |----'    |   |-------->|   |------>
 *    <------|___|<--------|___|    .----|___|
 *             A             ^-x    |   
 *             |                    |                    
 *             '--------------------'
 */
inline
void list_delete(DList_link x)
{
    assert(x != NULL);
	x->prev->next = x->next;
	x->next->prev = x->prev;
}

/**
 * 从一个链表上将[a,b]之间的节点移动到另外一个链表上x节点前
 *            ___             ___             ___            ___ 
 *           |   |---------->|   |----...--->|   |--------->|   |------>
 *    <------|___|<----------|___|<---...----|___|<---------|___|
 *                             ^-a             ^-b
 *            ___             ___             ___ 
 *           |   |----...--->|   |---------->|   |------>
 *    <------|___|<---...----|___|<----------|___|
 *                                             ^-x       
 * ======================================================================
 *                   .----------------------------------------.
 *                   | .---------------------------------.    |
 *            ___    | |      ___             ___        |   _V_ 
 *           |   |---' | .-->|   |----...--->|   |----.  |  |   |------>
 *    <------|___|<----' | .-|___|<---...----|___|<-. |  '--|___|
 *                       | |   ^-a             ^-b  | |
 *                       | '---------.  .-----------' |
 *                       |           |  | .-----------'     
 *                       '---------. |  | |    
 *            ___             ___  | |  | |   ___ 
 *           |   |----...--->|   |-' |  | '->|   |------>
 *    <------|___|<---...----|___|<--'  '----|___|
 *                                             ^-x       
 */
inline
void list_transfer(DList_link x, DList_link a, DList_link b)
{
    // 先将[a,b]从之前的链表上摘除
    a->prev->next = b->next;
    b->next->prev = a->prev;

    // 再将[a,b]插入到新链表的x节点前
    a->prev = x->prev;
    b->next = x;
    x->prev->next = a;
    x->prev = b;
}

/**
 * 交换两个链表的所有节点(除了nil节点)
 */
inline
void list_swap(DList_base &list1, DList_base &list2)
{
    DList_base temp;
    list_init(temp);

    if (!list_is_empty(list1)) // temp = list1
        list_transfer(&temp.nil, list1.nil.next, list1.nil.prev);
    if (!list_is_empty(list2)) // list1 = list2
        list_transfer(&list1.nil, list2.nil.next, list2.nil.prev);
    if (!list_is_empty(temp))  // list2 = temp
        list_transfer(&list2.nil, temp.nil.next, temp.nil.prev);
}

template <typename T>
struct DList_node : public DList_node_base {
    T value;
};

template <typename T>
struct DList : public DList_base {
};

// 在堆上动态分配一个节点
template <typename T>
DList_node<T> *list_new_node(const T &val)
{
    auto x = new DList_node<T>;
    x->value = val;
    return x;
}

// 将一个节点释放回堆
template <typename T>
void list_free_node(DList_node<T> *x)
{
    delete x;
}

// 在链表头插入节点
template <typename T>
void list_insert_front(DList<T> &list, DList_node<T> *x)
{
    list_insert(list.nil.next, x);
}

// 在链表尾插入节点
template <typename T>
void list_insert_back(DList<T> &list, DList_node<T> *x)
{
    list_insert(&list.nil, x);
}

// 在链表头删除节点
template <typename T>
DList_node<T> *list_delete_front(DList<T> &list)
{
    auto x = list.nil.next;
    list_delete(x);
    return static_cast<DList_node<T> *>(x);
}

// 在链表尾删除节点
template <typename T>
DList_node<T> *list_delete_back(DList<T> &list)
{
    auto x = list.nil.prev;
    list_delete(x);
    return static_cast<DList_node<T> *>(x);
}

// 遍历链表, 为每个节点调用fn
template <typename T, typename Function>
void list_for_each(DList<T> &list, Function fn)
{
    for (auto x = list.nil.next; x != &list.nil; x = x->next)
        fn(static_cast<DList_node<T> *>(x));
}

// 查找值等于val的第一个节点的地址, 
// 如果没有等于val的节点, 返回nil的地址
template <typename T>
DList_node<T> *list_search(DList<T> &list, const T &val)
{
    auto x = list.nil.next; 
    while (x != &list.nil && static_cast<DList_node<T> *>(x)->value != val)
        x = x->next;
    return static_cast<DList_node<T> *>(x); 
}

#endif

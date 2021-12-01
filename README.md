# <span id="11">C Structdata Project</span>
这是使用Clang实现的数据结构代码，创建于9/12/2021，这里面包含了大部分常见的数据结构实现，后面还会继续添加完善；开始这个project的初衷是为了重新学习数据结构，但是为了以后能在某些地方复用这些代码，所以会尽量将这些代码写的通用，这也体现在了这个项目内部，如栈复用了数组代码，树结构调用了栈和队列实现了遍历、插入等操作。如果你打算阅读项目里面的代码，你需要知道下面的一些要点：
  
*	部分代码只适合运行于64位的linux平台，并且内核版本不低于linux3.10
*	所有的实现都不是线程安全，在这里实现线程安全无非就是加锁，既然没有这个需求就不在这里折腾了
*	通过void\*实现数据泛型，在创建数据结构的时候会传入关键字或者数据的size保存以进行后面的操作
*	某种数据结构的实现都只包含了一个.h和一个.c文件，.c文件里面的函数实现基本都是静态函数，其中全大写和"\_\_"开始的函数都是内联函数；在.h文件中提供了一个包含操作接口的结构体，可以通过相关函数获取这个结构体单例进而操作具体的数据结构
  
|线性结构|[SqList](#1)|[SqStack](#2)|[DuCirLinkList](#3)|[DlQueue](#4)|
|:----|:----|:----|:----|:----|
|**树结构**|**[ThrtAVLTree](#5)**|**[RBTree](#6)**|**[B-Tree](#7)**|**[B+Tree](#8)**|
|**图结构**|
  
## <span id="1">SqList</span>
```c
typedef struct SqList{
	void* pElems;
	void* tmpRet;
	size_t e_S;
	size_t length;
	size_t size;
}SqList;
```
类似于C++的vector，SqList是空间连续的线性表，其中pElems为元素存储空间，e_S记录元素结构大小，length为当前数组长度，size为总长度，在空间不足时会自动扩容，tmpRet作为at & erase函数的返回；SqList使用快速排序作为内部排序；.h中提供了SQLIST_FOREACH & SQLIST_FOREACH_REVERSE两个宏方便数组的遍历
## <span id="2">SqStack</span>
```c
typedef struct SqStack{
	SqList* list;
}SqStack;
```
SqStack完全继承了SqList的实现，并在此基础上增加了栈相应的接口函数
## <span id="3">DuCirLinkList</span>
```c
typedef struct DuCirLink{
	void* pElem;
	struct DuCirLink* prior;
	struct DuCirLink* next;
}DuCirLink;
typedef struct DuCirLinkList{
	DuCirLink* link;
	void* tmpRet;
	size_t e_S;
	size_t length;
}DuCirLinkList;
```
这是双向循环链表，tmpRet & e_S & length意同SqList的同名元素，link为头结点，在链表初始化的时候就已经分配好空间，头节点不存储元素，同时作为遍历结束的标志；类似于SqList，DuCirLinkList使用快排作为内部排序，.h文件中同样提供了DULIST_FOREACH & DULIST_FOREACH_REVERSE两个宏实现对链表的快速遍历
## <span id="4">DlQueue</span>
```c
typedef struct DlQueue{
	DuCirLinkList* list;
}DlQueue;
```
类似于SqStack与SqList的关系，DlQueue完全继承于DuCirLinkList，并在此基础上增加了队列相应的接口函数
## <span id="5">ThrtAVLTree</span>
```c
typedef struct AVLNode{
	void* pKey;
	void* pValue;
	struct AVLNode* lchild;
	struct AVLNode* rchild;
	unsigned char height;
	unsigned char ThrtFlag;
}AVLNode;
```
这是带有threaded的平衡二叉树实现
## <span id="6">RBTree</span>
## <span id="7">B-Tree</span>
## <span id="8">B+Tree</span>

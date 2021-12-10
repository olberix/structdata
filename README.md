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
typedef bool(*AVLKeyCompareFuncT)(const void*, const void*);
typedef void(*AVLForEachFuncT)(const void*, void*);
typedef struct AVLTree{
	AVLNode* root;
	AVLNode thrtHead;
	AVLKeyCompareFuncT equalFunc;
	AVLKeyCompareFuncT lessFunc;
	size_t keySize;
	size_t valSize;
}AVLTree;
```
平衡二叉搜索树(Balanced Binary Tree/Height-Balanced Tree)，AVLNode包含键值对和左右孩子指针，新增height字段记录结点的高度用于计算Balanced Factor，其中叶子结点高度恒为1，非叶子结点取左右孩子最大高度+1，AVLTree创建的时候需要传入自定义的比较函数和键值大小  

AVLTree是严格平衡的二叉排序树，因为平衡因子的绝对值不会超过1，所以叶子结点只会出现在层数最大的两层，可以发现，当数据达到一定量的时候，层数越大的数据离散程度就越小，下几次的插入或删除操作所发生旋转的概率就会变低，或者旋转的次数变少；值得注意的是，$\color{rgb(0,248,0)}{AVLTree的插入删除操作均不需暴力回退到根节点}$，插入时，若回溯中结点的高度不变，则停止回溯，因为是插入操作，结点高度不变，证明以此结点为根的子树已经平衡，高度不变故而后面的结点也就没有了回溯的必要；而删除时，若回溯结点高度不变**且**平衡因子绝对值不超过1时停止回溯，因为此时以此结点为根的子树已然平衡，加之高度不变便停止回溯  

ThrtAVLTree是带有中序threaded的平衡二叉树实现，AVLNode中新增线索标志ThrtFlag用于判断结点的左右指向是孩子结点还是前驱后继结点；线索是对结点空指针的合理利用，以二叉链表实现的二叉树为例，必有空指针数=2n-(n-1)=n+1个，这些空指针用于指向前驱或者后继结点形成线索；不过线索这个概念除了可以加深理解之外好像实际用处并不大，如果是用于遍历，以栈遍历为例，栈和线索遍历时间复杂度都是O(n)，唯一比栈遍历好的就是空间复杂度达到常数级别O(1)，但是对于可以运行树结构的机器来说，O(1)和O(log(2, N))的空间复杂度没啥区别，如果是用于查找前驱和后继结点，能利用上线索的也只有叶子结点或者单孩子结点，优势也不明显；加上线索比较局限，一次只能建立一种次序线索，插入和删除结点时，也同样需要进行维护，不过为了加深认知，还是在AVLTree中加入了线索实现。  

关于AVLTree与RBTree的对比，RBTree好像并不能完全替代AVLTree，对于插入，它们的效率对比其实有点依赖输入数据，对于一组顺序的数据来说，RBTree必然优于AVLTree，因为此时AVLTree总是进行单支插入，但如果数据随机，AVLTree发生旋转的概率会大大减少，而RBTree可能需要继续进行着色操作，甚至会因为维护自身特性进行必要的旋转；不过对于删除，因为RBTree自身的特性，大部分情况下应该都优于AVLTree。总而言之，对于查询和随机插入较多的环境，AVLTree优于RBTree，对于顺序插入和删除较多又或者比较综合的环境，RBTree优于AVLTree

[参考链接：]()&nbsp;[AVL树基础篇](https://mp.weixin.qq.com/s?__biz=MzA4NDE4MzY2MA==&amp;mid=2647521381&amp;idx=1&amp;sn=796ac1eda0eaefadfb57a1b9742bcec0&amp;chksm=87d24766b0a5ce70a18acca20a130a14c16fb56a716d1c0e1fbe0acf23915a1b8aaf509f3850&scene=178&cur_album_id=1338152221988585473#rd)&nbsp;&nbsp;[AVL树删除篇](https://mp.weixin.qq.com/s?__biz=MzA4NDE4MzY2MA==&amp;mid=2647521508&amp;idx=1&amp;sn=ff0751a1a49a48450757b53978fcbef8&amp;chksm=87d247e7b0a5cef1f5f581cfa843b68021a51e979ee49b2b947cf394c613b4701ac07a8e8a76&scene=178&cur_album_id=1338152221988585473#rd)
## <span id="6">RBTree</span>
## <span id="7">B-Tree</span>
## <span id="8">B+Tree</span>

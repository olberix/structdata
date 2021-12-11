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
这是双向循环链表，tmpRet & e_S & length意同SqList的同名元素，link为头结点，在链表初始化的时候就已经分配好空间，头结点不存储元素，同时作为遍历结束的标志；类似于SqList，DuCirLinkList使用快排作为内部排序，.h文件中同样提供了DULIST_FOREACH & DULIST_FOREACH_REVERSE两个宏实现对链表的快速遍历
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

AVLTree是严格平衡的二叉排序树，因为平衡因子的绝对值不会超过1，所以叶子结点只会出现在层数最大的两层，可以发现，当数据达到一定量的时候，层数越大的数据离散程度就越小，下几次的插入或删除操作所发生旋转的概率就会变低，或者旋转的次数变少；值得注意的是，[**AVLTree的插入删除操作均不需暴力回退到根结点**]()，插入时，若回溯中结点的高度不变，则停止回溯，因为是插入操作，结点高度不变，证明以此结点为根的子树已经平衡，高度不变故而后面的结点也就没有了回溯的必要；而删除时，若回溯结点高度不变**且**平衡因子绝对值不超过1时停止回溯，因为此时以此结点为根的子树已然平衡，加之高度不变便停止回溯  

ThrtAVLTree是带有中序threaded的平衡二叉树实现，AVLNode中新增线索标志ThrtFlag用于判断结点的左右指向是孩子结点还是前驱后继结点；线索是对结点空指针的合理利用，以二叉链表实现的二叉树为例，必有空指针数=2n-(n-1)=n+1个，这些空指针用于指向前驱或者后继结点形成线索；不过线索这个概念除了可以加深理解之外好像实际用处并不大，如果是用于遍历，以栈遍历为例，栈和线索遍历时间复杂度都是O(n)，唯一比栈遍历好的就是空间复杂度达到常数级别O(1)，但是对于可以运行树结构的机器来说，O(1)和O(log(2, N))的空间复杂度没啥区别，如果是用于查找前驱和后继结点，能利用上线索的也只有叶子结点或者单孩子结点，优势也不明显；加上线索比较局限，一次只能建立一种次序线索，插入和删除结点时，也同样需要进行维护，不过为了加深认知，还是在AVLTree中加入了线索实现  

关于AVLTree与RBTree的对比，RBTree好像并不是绝对比AVLTree高效，对于插入，它们的效率对比其实有点依赖输入数据，对于一组顺序的数据来说，RBTree必然优于AVLTree，因为此时AVLTree总是进行单支插入，但如果数据随机，AVLTree发生旋转的概率会大大减少，而RBTree可能需要继续进行着色操作，甚至会因为维护自身特性进行必要的旋转；对于删除，虽然RBTree最多仅需3次旋转，但仍然可能需要进行着色，加之AVLTree也不是总是需要回溯到根结点，所以也不见得一定比AVLTree快。总而言之，对于查询和随机插入较多的环境，AVLTree优于RBTree，对于顺序插入又或者比较综合的环境，RBTree一定优于AVLTree?

[**参考链接：**]()&nbsp;[AVL树基础篇](https://mp.weixin.qq.com/s?__biz=MzA4NDE4MzY2MA==&amp;mid=2647521381&amp;idx=1&amp;sn=796ac1eda0eaefadfb57a1b9742bcec0&amp;chksm=87d24766b0a5ce70a18acca20a130a14c16fb56a716d1c0e1fbe0acf23915a1b8aaf509f3850&scene=178&cur_album_id=1338152221988585473#rd)&nbsp;&nbsp;[AVL树删除篇](https://mp.weixin.qq.com/s?__biz=MzA4NDE4MzY2MA==&amp;mid=2647521508&amp;idx=1&amp;sn=ff0751a1a49a48450757b53978fcbef8&amp;chksm=87d247e7b0a5cef1f5f581cfa843b68021a51e979ee49b2b947cf394c613b4701ac07a8e8a76&scene=178&cur_album_id=1338152221988585473#rd)
## <span id="6">RBTree</span>
```c
static const unsigned char RB_RED = 0x0;
static const unsigned char RB_BLACK = 0x1;
typedef struct RBNode{
	void* pKey;
	void* pValue;
	struct RBNode* lchild;
	struct RBNode* rchild;
	struct RBNode* parent;
	unsigned char color;
}RBNode;
typedef bool(*RBKeyCompareFuncT)(const void*, const void*);
typedef void(*RBForEachFuncT)(const void*, void*);
typedef struct RBTree{
	RBNode* root;
	RBKeyCompareFuncT equalFunc;
	RBKeyCompareFuncT lessFunc;
	size_t keySize;
	size_t valSize;
}RBTree;
```
红黑树(Red Black Tree)是一棵自平衡的二叉排序树，它具有以下特点：  

+	根结点恒为黑色，非根结点为黑色或者红色
+	红色结点不能路径上相邻
+	以任意结点为根的子树，所有路径都包含了数量相等的黑色结点
  
与AVLTree的实现类似，RBTree创建时同样需要传入自定义比较函数和键值大小，RBNode新增父指针parent和颜色标志color，因为插入总是以红色结点插入，所以将红色的标志值定义为0，使结点清零初始化便为红色结点，同时将黑色标志值定义为1，交换颜色的时候异或取反便可；RBTree的起源是从4阶B树得到启发，要彻底理解RBTree最好先熟悉 [B-Tree](#7)  

RBTree引理：一棵有N个结点的红黑树高度h<=2log(2,N+1)，证明：  

1. 将RBTree合并为2-3-4树(红色结点向黑色父结点合并)，令合并前高度为h，合并后高度为h'，则h<=2h'，当某分支红黑结点各一半时等号成立
2. 合并后可得N>=2^h'-1，故h'<=log(2,N+1)，当不存在红色结点时等号成立 (其实这一步可以省略理解，因为带有N个结点的二叉树，必然有N+1个空指针(不算父指针)，RBTree合并为2-3-4树后，每条分支等高，直接得出h'<=log(2,N+1))
3. 因为h<=2h'，故h<=2log(2,N+1)，所以RBTree的查找复杂度为log(2, N)

RBTree插入和删除步骤：  
![insert](https://github.com/ccencon/structdata/blob/main/images/rbtree_insert.png)
![erase](https://github.com/ccencon/structdata/blob/main/images/rbtree_erase.png)  
[**参考链接：**]()&nbsp;[红黑树上篇](https://mp.weixin.qq.com/s?__biz=MzA4NDE4MzY2MA==&mid=2647521575&idx=1&sn=844d6bcdf8cdce683d7d837d4933c788&chksm=87d24624b0a5cf32a119334268184987b6f254de7edfd35ae19c88999789ff70e24a5881bf68&scene=178&cur_album_id=1338152221988585473#rd)&nbsp;&nbsp;[红黑树中篇](https://mp.weixin.qq.com/s?__biz=MzA4NDE4MzY2MA==&mid=2647521642&idx=1&sn=dda12824118e46c17374333062e37a6a&chksm=87d24669b0a5cf7f5bf8e2614deb224d15c10cd474d24c17e9354743b7a5eb72fc89469e6330&scene=178&cur_album_id=1338152221988585473#rd)&nbsp;&nbsp;[红黑树下篇](https://mp.weixin.qq.com/s?__biz=MzA4NDE4MzY2MA==&mid=2647521911&idx=1&sn=2e0a8b636cff515c5471f9a1bb2c3022&chksm=87d24574b0a5cc62e19d1f78b429a78b855948c5a3c7a98f7aca8372a674d17419c47f34a6c8&scene=178&cur_album_id=1338152221988585473#rd)&nbsp;&nbsp;[通过2-3-4树理解红黑树](https://zhuanlan.zhihu.com/p/269069974)
## <span id="7">B-Tree</span>
## <span id="8">B+Tree</span>

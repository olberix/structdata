# <span id="11">C Structdata Project</span>
这是使用Clang实现的数据结构代码，创建于9/12/2021，这里面包含了大部分常见的数据结构实现，后面还会继续添加完善；开始这个project的初衷是为了重新学习数据结构，但是为了以后能在某些地方复用这些代码，所以会尽量将这部分代码写得通用，这也体现在了这个项目内部，如栈复用了数组代码，树结构调用了栈和队列实现了遍历、插入等操作。如果你打算阅读项目里面的代码，你需要知道下面的一些要点：
  
*	部分代码只适合运行于64位的linux平台，并且内核版本不低于linux3.10
*	所有的实现都不是线程安全
*	通过void\*实现数据泛型，在创建数据结构的时候会传入键值的size保存以进行后面的操作
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

AVLTree是严格平衡的二叉排序树，因为平衡因子的绝对值不会超过1，所以叶子结点只会出现在层数最大的两层，可以发现，当数据达到一定量的时候，层数越大的数据离散程度就越小，下几次的插入或删除操作所发生旋转的概率就会变低，或者旋转的次数变少；值得注意的是，[**AVLTree的插入删除操作均不需暴力回退到根结点**]()，插入时，若回溯中结点的高度不变，则停止回溯，因为是插入操作，结点高度不变，证明以此结点为根的子树已经平衡；而删除时，若回溯结点高度不变**且**平衡因子绝对值不超过1时停止回溯，因为此时以此结点为根的子树已然平衡，高度不变便停止回溯  

ThrtAVLTree是带有中序threaded的平衡二叉树实现，AVLNode中新增线索标志ThrtFlag用于判断结点的左右指向是孩子结点还是前驱后继结点；线索是对结点空指针的合理利用，以二叉链表实现的二叉树为例，必有空指针数=2n-(n-1)=n+1个，这些空指针用于指向前驱或者后继结点形成线索；个人感觉线索这个概念除了可以加深理解之外好像实际用处已经不大，如果是用于遍历，以栈遍历为例，栈和线索遍历时间复杂度都是O(n)，唯一优于栈遍历的就是空间复杂度达到常数级别O(1)，但是对于可以运行树结构的机器来说，O(1)和O(log(2, N))的空间复杂度没有太多区别，如果是用于查找前驱和后继结点，能利用上线索的也只有叶子结点或者单孩子结点，对于稳定的O(log(2, N))查找优势也不明显；线索本身比较局限，一次只能建立一种次序线索，插入和删除结点时，也同样需要进行维护，不过为了加深理解，还是在AVLTree中加入了线索实现  

在大部分认知中，[RBTree](#6)优于AVLTree，但这个观点好像并不能那么绝对，对于插入，它们的效率对比其实有点依赖输入数据，对于一组顺序的数据来说，RBTree必然优于AVLTree，因为此时AVLTree总是进行单支插入，但如果数据随机，AVLTree发生旋转的概率会大大减少，而RBTree可能需要继续进行着色操作，甚至会因为维护自身特性进行必要的旋转；对于删除，虽然RBTree最多仅需3次旋转，但仍然可能需要进行着色，加之AVLTree也不是总是需要回溯到根结点，所以也不见得一定比AVLTree快。总而言之，对于查询和随机插入较多的环境，AVLTree必然优于RBTree，对于顺序插入又或者比较综合的环境，RBTree一定优于AVLTree?  

AVLTree的删除步骤中为了保持高度平衡可能需要进行旋转操作，但在实现AVLTree的过程中不难发现，如果不是为了时刻维护这个特性，这个旋转操作有点多余，因为下一次或几次的插入或者删除有可能会抵消这个高度差，那么这几次的回溯旋转就完全没有了必要；如果不能抵消，或许可以加入另外一个因子判定何时进行平衡操作？如果将来要对AVLTree进行优化，一定是从这方面下手（RBTree好像就是这样的优化？总感觉还能找到更加简洁的描述和更加高效的实现）  

[**参考链接：**]()&nbsp;[AVL树基础篇](https://mp.weixin.qq.com/s/POX8QV9JFrRcAi-q-sJvOA)&nbsp;&nbsp;[AVL树删除篇](https://mp.weixin.qq.com/s/9no2Ge0hWo1lZHRm_JS0hA)
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

RBTree引理：一棵有N个结点的红黑树高度h<=2log(2, N+1)，证明：  

1. 将RBTree合并为2-3-4树(红色结点向黑色父结点合并)，令合并前高度为h，合并后高度为h'，则h<=2h'，当某分支红黑结点各一半时等号成立
2. 合并后可得N>=2^h'-1，故h'<=log(2, N+1)，当不存在红色结点时等号成立 (其实这一步可以省略理解，因为带有N个结点的二叉树，必然有N+1个空指针(不算父指针)，RBTree合并为2-3-4树后，每条分支等高，直接得出h'<=log(2, N+1))
3. 因为h<=2h'，故h<=2log(2, N+1)，所以RBTree的查找复杂度为log(2, N)

RBTree插入和删除步骤：  
<span id="6-1">![insert](https://github.com/ccencon/structdata/blob/main/images/rbtree_insert.png)</span>  
<span id="6-2">![erase](https://github.com/ccencon/structdata/blob/main/images/rbtree_erase.png)</span>  

[**参考链接：**]()&nbsp;[红黑树上篇](https://mp.weixin.qq.com/s/DXh93cQaKRgsKccmoQOAjQ)&nbsp;&nbsp;[红黑树中篇](https://mp.weixin.qq.com/s/tnbbvgPyqz0pEpA76rn_1g)&nbsp;&nbsp;[红黑树下篇](https://mp.weixin.qq.com/s/oAyiRC_O-N5CHqAjt2va9w)&nbsp;&nbsp;[通过2-3-4树理解红黑树](https://zhuanlan.zhihu.com/p/269069974)
## <span id="7">B-Tree</span>
```c
typedef struct HeaderNode{
	off_t rootPointer;
	size_t keySize;
	size_t valSize;
	int pageSize;
}HeaderNode;
typedef ssize_t BNodeST;
typedef struct BNode{
	off_t* childPointers;
	void* pKey;
	void* pValue;
	off_t selfPointer;//标记这个node在文件的偏移位置,为0时代表新插入的node,还没有写入文件
	BNodeST size;
}BNode;
typedef bool(*BKeyCompareFuncT)(const void*, const void*);
typedef void(*BForEachFuncT)(const void*, const void*);
typedef struct BTree{
	HeaderNode head;
	BKeyCompareFuncT equalFunc;
	BKeyCompareFuncT lessFunc;
	void* tmpRet;
	int fd;
	unsigned short maxNC;
	unsigned short minNC;
}BTree;
```
这是基于外存实现的B树（聚簇实现），HeaderNode存储了B树的元信息，位于文件开头并独占一页空间，其中记录包括根结点，键值大小和页大小；BNode是B树结点结构，存储了关键字个数，键值对，孩子指针和自身指针，自身指针不存入文件，在写入文件或从文件读取时才被赋值；BTree是B树结构，包含了比较函数，文件描述符，最大最小关键字数等信息  

B树实现依赖了文件系统，除了基本的文件操作外，需要另外注意两个接口函数[**fallocate**](https://blog.csdn.net/weixin_36145588/article/details/78822837)和[**lseek**](https://www.zhihu.com/question/407305048/answer/1358083582)，fallocate函数可以对文件进行打洞并归还磁盘空间，这是本文B树实现结点删除的基础；lseek函数在linux3.10之后的文件系统版本提供了另外两个参数SEEK_DATA和SEEK_HOLE（编译时需要加上宏_GNU_SOURCE），其中SEEK_HOLE可以使文件偏移到指定位置开始的第一个空洞位置，如果没有空洞则返回文件尾，新结点插入时便通过此参数找到空洞位置进行写入，使文件变得充分紧凑；需要特别注意的是，SEEK_HOLE总是以磁盘块为单位进行空洞查找（猜测是直接遍历inode中的block数组？），而B树结点大小往往以页大小为基准，所以向文件写入一个结点时，剩下的页空间需要以0进行填充  

![BTree](https://github.com/ccencon/structdata/blob/main/images/BTree.png)  

通过SEEK_HOLE查找写入地址的方式并不高效，为了使下一次得到正确的地址必须在每次新写入结点时同步磁盘数据，这种方式不仅IO利用率低，耗时也急剧增加，如果改用位图的实现便可消除这个影响，[B+Tree](#8)的实现便采用了这种方法  

B树是一种平衡多路查找树，特征如下：  

+	任意分支等高，叶子结点永远位于同一层（这里的叶子结点是实际存在的树结点，事实上很多教材在介绍B树的时候把叶子结点看作查找失败的NULL结点，这或许是来源于作者的思路？但B树好像并不需要按照这种方式理解，判断一个结点是否为叶子结点时也只需判断第一个孩子指针是否为空）
+	除叶子结点外一个含有N个关键字的结点必然有N+1个孩子，B树的阶为结点可以容纳最大关键字数+1
+	linux对外存与内存的数据交换以页为单位，所以B树结点大小通常等于页大小；除根结点外，结点关键字数位于区间[t-1, 2t-1]，t>=2，其中t为B树的最小度数；计算结点最大关键字数通常为 ⌊page_size/(key_size+value_size+point_size)⌋（根据实现方式有少许区别，比如这个实现在每页起始写入了4字节的关键字数）；关键字数区间[t-1, 2t-1]可以保证B树结点分裂或者合并时依然保持B树特性
+	任意结点中任意两个关键字k1，k2之间的关键字和孩子结点的关键字必然大于k1，小于k2
+	B树增删改查的时间复杂度log(2t-1, N) ~ log(t-1, N)
  
下面是B树的插入和删除步骤，同时将红黑树转化为2-3-4树进行比较：  

[**插入步骤：**]()  

1. 若插入后的结点关键字数不大于2t-1，则插入成功，否则转到步骤2
    - 红黑树总是以红色结点插入，若插入后关键字数不大于3，会有三种情况：
    	1. 此结点为根结点，设置为黑色
    	2. 父结点为黑色，这种情况不需要做处理
    	3. 父结点为红色，这时候叔叔结点必为NULL结点或者黑色结点，这种情况旋转后变色便可（叔叔结点为NULL结点是最开始处理的叶子结点情况，为黑色结点时是结点分离上升后的情况）
2. 以⌊t⌋为分界将结点拆分为左中右3部分，左边部分保留在原结点中，右边部分移动到新结点，将中间部分和指向新结点的指针上升到父结点中，然后继续进行步骤1判断；若没有父结点，则产生了新的根结点
    - 相当于将父结点和叔叔结点设置为黑色，祖父结点设置为红色，转到步骤1判断

B树插入方法并不唯一，各种方法大同小异，文中实现是采用提前分裂的方式进行插入，去除了回溯的步骤，但是在顺序插入的时候，这种方式会造成绝大部分的结点只包含最小个数的关键字从而增加结点个数和树深度，所以这种实现方式并不推荐  

[**删除步骤：**]()  

1. 若结点执行删除或关键字下移操作后关键字数仍大于等于t-1，则删除成功，否则转到步骤2
	- 执行删除或下移操作后必为2-或者3-结点，相当于红黑树删除的是红色结点（如果删除的是黑色结点另外一个红色结点自动变为黑色）
	- 叶子结点删除操作对应红黑树中下面两种情况： 
		1. 删除结点为红色，直接删除 [**C2**](#6-2)
		2. 删除结点为黑色，此时必然有且只有一个红色孩子，将此孩子结点替换删除结点并将颜色设置为黑 [**C1**](#6-2)
2. 若结点相邻兄弟关键字数大于t-1，则将兄弟结点最小（或最大）的关键字上移到父结点，并将父结点对应关键字下移到此结点，若此结点不是叶子，还需移动对应孩子指针；若相邻兄弟关键字数均等于t-1则转到步骤3
	- 这种情况相当于红黑树中兄弟结点是黑色结点且有红色孩子，这时候旋转变色即可 [**C5**](#6-2)
3. 父结点对应关键字下移并将此结点与某个兄弟结点合并，父结点关键字数减一，若父结点为根结点且关键字数为0，重设根结点，否则将父结点转到步骤1判断
	- 下移的关键字相当于红黑树中对兄弟结点和父结点重新着色的情况，因为下移关键字必为红黑树中的红色结点，下移到新结点后变为黑色，左右关键字变为红色，相当于删除图 [**C4**](#6-2)，如果下移关键字是黑色结点则需进行 [**C3**](#6-2)步骤将下移结点转化为红色结点

同样的，B树删除方法也不唯一，比如步骤2和步骤3，不是非得判断两个相邻兄弟都不能借才合并，可以在第一个相邻兄弟不够借之后便选择与之合并，这样树结点数还能减少1，不过因为父结点关键字数减1，可能会增加回溯的路程，所以具体的选择就见仁见智了  

[**参考链接：**]()&nbsp;[什么是B树](https://mp.weixin.qq.com/s/Q29CgcnnudePQ0l2UyshUw)&nbsp;&nbsp;[MySQL背后的数据结构及算法原理](http://blog.codinglabs.org/articles/theory-of-mysql-index.html)&nbsp;&nbsp;[基于外存（磁盘）的B+树实现](https://zhuanlan.zhihu.com/p/67374506)
## <span id="8">B+Tree</span>
```c
typedef unsigned short __keynode_size_t;
typedef unsigned short __value_size_t;
typedef struct BPMetaNode{
	unsigned long long rows;//实际数据行数
	off_t rootPointer;//根结点指针,初始为-1
	off_t firstPointer;//第一个叶子结点指针,初始为-1
	off_t fileSize;//元文件大小
	size_t keySize;//键大小
	size_t valSize;//值大小
	size_t indexBitMapEdge;//位图字节分界,前面为索引文件位图,后面为数据文件位图
	int metaPageSize;//meta页大小
	int indexPageSize;//索引页大小
	int dataPageSize;//数据页大小
	int dataPageBitBytes;//数据页位图字节数
	__value_size_t maxDC;//数据页最大行数
	__keynode_size_t maxNC;//结点最大关键字数
	__keynode_size_t minNC;//结点最小关键字数
}BPMetaNode;
typedef struct BPNode{
	void* pKey;
	off_t* childPointers;//分配空间size+1,最后一个为next指针
	off_t selfPointer;//与B树不同,这个初始为-1
	__keynode_size_t size;//结点关键字数
	unsigned char isLeaf;//0为内部结点,1为叶子结点
}BPNode;
typedef bool(*BPKeyCompareFuncT)(const void*, const void*);
typedef void(*BPForEachFuncT)(const void*, const void*);
typedef struct BPTree{
	char META_FILENAME[4096];
	char INDEX_FILENAME[4096];
	char DATA_FILENAME[4096];
	BPMetaNode meta;
	BPKeyCompareFuncT equalFunc;
	BPKeyCompareFuncT lessFunc;
	void* tmpRet;
	char* tmpWriteStr;
	char* metaMap;//元文件映射
	int fdMeta;
	int fdIndex;
	int fdData;
}BPTree;
```
这同样是基于外存实现的B+树（非聚簇实现），在上面[B-Tree](#7)的实现中，IO利用率并不理想，这里结合位图的方式，实现了更高的存储效率。同样的，B+树新创建的时候会创建3个文件：元文件，索引文件和数据文件；元文件中，因为其特殊性采用了[mmap](https://www.cnblogs.com/huxiao-tee/p/4660352.html#_labelTop)映射方式进行文件存取，文件起始用一页空间记录B+树的meta信息，剩余空间记录了索引和数据的地址位信息，同样以一页为单位，通过索引页容纳关键字数和数据页容纳数据行数计算出具体的分界a，即前a个字节是索引位图，剩余字节是数据位图；索引文件中，每一页分别记录了是否叶子，关键字数，关键字行，地址行等信息，结点分裂时，通过元文件具体bit位计算出空闲页位置并将bit位置1，结点删除时，将整个索引页打洞归还磁盘空间并将元文件对应bit位置0；数据文件中，每一页分别记录了当前页已用数据行数，数据行位图所占字节，数据行位图和实际的数据行，插入新数据时，通过元文件找到未写满的数据页地址，再通过当前数据位图找到空闲的数据行写入，如果当前页写满，便将元文件对应bit位置1，删除数据后，若整个数据页已经空闲，则打洞当前页，若删除前整页已经写满，则将元文件对应bit位置0，插入和删除操作同样需要更新当前页数据行位图  

![B+Tree](https://github.com/ccencon/structdata/blob/main/images/B+Tree.png)  

当一棵B+树经历了多次的增删操作后，很容易会产生一些页空洞，每个数据页空闲行数也可能增多并且索引中逻辑上相邻的数据也不能很好的在数据文件中物理上相邻，当进行范围查找时，磁盘的读取和预读功能便很有可能利用不上，从而导致IO次数的增加，效率变低；所以在这个B+树的实现中提供了重建功能，类似mysql的[optimize](https://cloud.tencent.com/developer/article/1653643)，可以通过调用重建功能实现最优B+树结构  

B+树与B-树的差异：  

+	有n棵子树的结点含有n个关键字，叶子结点会多出一个或者两个指针域指向左右兄弟结点
+	所有叶子结点包含了全部关键字信息，而内部结点仅作索引作用，每个关键字都为其对应子树最大（小）关键字
+	为了防止B+树出现无意义的单关键字结点，故B+树的结点最小关键字数必须大于等于2，当根结点为叶子结点时根结点不受此约束
+	内部结点和叶子结点计算容纳最大关键字数的方法存在差异（尤其体现在聚簇实现中），所以B+树可以存在两个不同的阶a和b，阶a用于描述内部结点，阶b用于描述叶子结点

[**插入步骤：**]()  

1. 若插入关键字比整棵B+树的最大（小）关键字大（小），则需更改路径上每个结点的最大(小)关键字
2. 插入后若关键字数不多于结点最大关键字数，插入结束，否则转到步骤3
3. 分裂结点，然后有下面两种情况：
	+	存在父结点，则更新父结点关键字，父结点关键数+1，对父结点转到步骤2判断
	+	若不存在父结点，则增加新的根结点，插入结束

[**删除步骤：**]()  

1. 若删除关键字是结点中最大（小）关键字，则需更新父结点对应关键字，并对父结点重复此步骤直至根结点
2. 删除后若关键字数不少于结点最少关键字数，删除结束，否则转到步骤3
3. 若结点为根结点，分下面两种情况，否则转到步骤4：
	+	根结点为叶子结点，若结点关键字数为0，重置整棵B+树；删除结束
	+	根结点为内部结点，若结点关键字数小于2，重设根结点；删除结束
4. 左右兄弟有多余关键字，向左右兄弟借，删除结束；否则转到步骤5
5. 合并结点，更新父结点关键字，父结点关键字数-1，对父结点转到步骤2判断
# <span id="11">C Structdata</span>
这是使用Clang实现的数据结构代码，创建于2021/9/12，这里面包含了大部分常见的数据结构实现，后面还会继续添加完善；开始这个project的初衷是为了重新学习数据结构，但是为了以后能在某些地方复用这些代码，所以会尽量将这部分代码写得通用，这也体现在了这个项目内部，如栈结构复用了数组代码，树结构引用了栈和队列，哈希结构引用了链表和红黑树等
  
*	部分代码只适合运行于64位的linux平台，并且内核版本不低于linux3.10
*	所有的实现都不是线程安全
*	通过void\*实现数据泛型，在创建数据结构的时候会传入键值大小，相关操作函数等
*	某种数据结构的实现都只包含了一个.h和一个.c文件，.c文件里面的函数实现大部分为静态函数，其中以全大写或"\_\_"开始命名的函数为内部函数，不提供外部使用；在.h文件中提供了一个包含操作接口的结构体，可以通过相关函数获取这个结构体单例进而操作具体的数据结构

|线性结构|[SqList](#1)|[SqStack](#2)|[DuCirLinkList](#3)|[DlQueue](#4)|**[SkipList](#10)**|**[PriorityQueue](#PriorityQueue)**|
|:----|:----|:----|:----|:----|:----|:----|
|**树结构**|**[ThrtAVLTree](#5)**|**[RBTree](#6)**|**[B-Tree](#7)**|**[B+Tree](#8)**|
|**其他结构**|**[HashTable](#9)**|**[UnDirectedGraph](#UnDirectedGraph)**|**[DirectedGraph](#DirectedGraph)**|**[Graph](#Graph)**|

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
	unsigned char height;/*节点的最大高度,叶子节点恒为1,用于计算Balanced factor*/
	unsigned char ThrtFlag;
}AVLNode;

typedef struct AVLTree{
	AVLNode* root;
	AVLNode thrtHead;
	CmnCompareFunc equalFunc;
	CmnCompareFunc lessFunc;
	void* tmpRet;
	size_t keySize;
	size_t valSize;
	size_t tree_size;
}AVLTree;
```
平衡二叉搜索树(Balanced Binary Tree/Height-Balanced Tree)，AVLNode包含键值对和左右孩子指针，新增height字段记录结点的高度用于计算Balanced Factor，其中叶子结点高度恒为1，非叶子结点取左右孩子最大高度+1，AVLTree创建的时候需要传入默认或自定义的比较函数和键值大小  

AVLTree是严格平衡的二叉排序树，因为平衡因子的绝对值不会超过1，所以叶子结点只会出现在层数最大的两层，可以发现，当数据达到一定量的时候，层数越大的数据离散程度就越小，下几次的插入或删除操作所发生旋转的概率就会变低，或者旋转的次数变少；值得注意的是，[**AVLTree的插入删除操作均不需暴力回退到根结点**]()，插入时，若回溯中结点的高度不变，则停止回溯，因为是插入操作，结点高度不变，证明以此结点为根的子树已经平衡；而删除时，若回溯结点高度不变**且**平衡因子绝对值不超过1时停止回溯，因为此时以此结点为根的子树已然平衡，高度不变便停止回溯  

ThrtAVLTree是带有中序threaded的平衡二叉树实现，AVLNode中新增线索标志ThrtFlag用于判断结点的左右指向是孩子结点还是前驱后继结点；线索是对结点空指针的合理利用，以二叉链表实现的二叉树为例，必有空指针数$2n-(n-1)=n+1$个，这些空指针用于指向前驱或者后继结点形成线索；个人感觉线索这个概念除了可以加深理解之外好像实际用处已经不大，如果是用于遍历，以栈遍历为例，栈和线索遍历时间复杂度都是O(n)，唯一优于栈遍历的就是空间复杂度达到常数级别O(1)，但是对于可以运行树结构的机器来说，O(1)和O($log_2 N$)的空间复杂度没有太多区别，如果是用于查找前驱和后继结点，能利用上线索的也只有叶子结点或者单孩子结点，对于稳定的O($log_2 N$)查找优势也不明显；线索本身比较局限，一次只能建立一种次序线索，插入和删除结点时，也同样需要进行维护，不过为了加深理解，还是在AVLTree中加入了线索实现  

在大部分认知中，[RBTree](#6)优于AVLTree，但这个观点好像并不能那么绝对，对于插入，它们的效率对比其实有点依赖输入数据，对于一组顺序的数据来说，RBTree必然优于AVLTree，因为此时AVLTree总是进行单支插入，但如果数据随机，AVLTree发生旋转的概率会大大减少，而RBTree可能需要继续进行着色操作，甚至会因为维护自身特性进行必要的旋转；对于删除，虽然RBTree最多仅需3次旋转，但仍然可能需要进行着色，加之AVLTree也不是总是需要回溯到根结点，所以也不见得一定比AVLTree快。总而言之，对于查询和随机插入较多的环境，AVLTree必然优于RBTree，对于顺序插入又或者比较综合的环境，RBTree一定优于AVLTree?  

AVLTree的删除步骤中为了保持高度平衡可能需要进行旋转操作，但在实现AVLTree的过程中可以发现，如果不是为了时刻维护这个特性，这个旋转操作有点多余，因为下一次或几次的插入或者删除有可能会抵消这个高度差，那么这几次的回溯旋转就完全没有了必要；如果不能抵消，或许可以加入另外一个因子判定何时进行平衡操作？如果将来要对AVLTree进行优化，一定是从这方面下手（RBTree好像就是这样的优化？总感觉还能找到更加简洁的描述和更加高效的实现）  

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

typedef struct RBTree{
	RBNode* root;
	CmnCompareFunc equalFunc;
	CmnCompareFunc lessFunc;
	void* tmpRet;
	size_t keySize;
	size_t valSize;
	size_t tree_size;
}RBTree;
```
红黑树(Red Black Tree)是一棵自平衡的二叉排序树，它具有以下特点：  

+	根结点恒为黑色，非根结点为黑色或者红色
+	红色结点不能路径上相邻
+	以任意结点为根的子树，所有路径都包含了数量相等的黑色结点
  
与AVLTree的实现类似，RBTree创建时同样需要传入默认或自定义比较函数和键值大小，RBNode新增父指针parent和颜色标志color，因为插入总是以红色结点插入，所以将红色的标志值定义为0，使结点清零初始化便为红色结点，同时将黑色标志值定义为1，交换颜色的时候异或取反便可；RBTree的起源是从4阶B树得到启发，要彻底理解RBTree最好先熟悉 [B-Tree](#7)  

RBTree引理：一棵有$N$个结点的红黑树高度$h<=2log_2 {(N+1)}$，证明：  

1. 将RBTree合并为2-3-4树(红色结点向黑色父结点合并)，令合并前高度为$h$，合并后高度为$h'$，则$h<=2h'$，当某分支红黑结点各一半时等号成立
2. 合并后可得$N>=2^{h'}-1$，故$h'<=log_2 {(N+1)}$，当不存在红色结点时等号成立，其实这一步可以省略理解，因为带有$N$个结点的二叉树，必然有$N+1$个空指针(不算父指针)，RBTree合并为2-3-4树后，每条分支等高，直接得出$h'<=log_2 {(N+1)}$
3. 因为$h<=2h'$，故$h<=2log_2 {(N+1)}$，所以RBTree的查找复杂度为$log_2 N$

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

typedef struct BNode{
	off_t* childPointers;
	void* pKey;
	void* pValue;
	off_t selfPointer;//标记这个node在文件的偏移位置,为0时代表新插入的node,还没有写入文件
	ssize_t size;
}BNode;

typedef struct BTree{
	HeaderNode head;
	CmnCompareFunc equalFunc;
	CmnCompareFunc lessFunc;
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
+	B树增删改查的时间复杂度$log_{2t-1} N$ ~ $log_{t-1} N$
  
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
	size_t rows;//实际数据行数
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

typedef struct BPTree{
	char META_FILENAME[4096];
	char INDEX_FILENAME[4096];
	char DATA_FILENAME[4096];
	BPMetaNode meta;
	CmnCompareFunc equalFunc;
	CmnCompareFunc lessFunc;
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

[**参考链接：**]()&nbsp;[B+树介绍](https://zhuanlan.zhihu.com/p/54102723)&nbsp;&nbsp;[B+树插入与删除](https://mp.weixin.qq.com/s/N3kCheEhJZJ1UZ4p7n-OQQ)

## <span id="9">HashTable</span>
```c
#define BUCKETTYPE_NIL 0x0
#define BUCKETTYPE_ORIGIN 0x1
#define BUCKETTYPE_LIST 0x2
#define BUCKETTYPE_RBTREE 0x3
#define LOADFACTOR 0.75f

static const size_t HASHINITSIZE = 8;//哈希表初始长度
static const size_t TRANSTOTREESIZE = 6;//链表转化为红黑树的元素个数

typedef struct HashEntry{
	void* pKey;
	void* pValue;
}HashEntry;

typedef union __HashBucket{
	HashEntry entry;
	DuCirLinkList* entry_list;
	RBTree* entry_tree;
}__HashBucket;

typedef struct HashBucket{
	__HashBucket bucket;
	unsigned char type;
}HashBucket;

typedef struct HashTable{
	SqList* list;
	void* tmpRet;
	HashFuncTT hashFunc;
	CmnCompareFunc lessFunc;
	CmnCompareFunc equalFunc;
	size_t keySize;
	size_t valSize;
	size_t table_size;
	size_t maxLoadFactor;
	size_t elem_count;
}HashTable;
```
散列表最值得讨论的永远都有 [如何构造一个好的散列函数（或多个）]()和 [如何高效处理冲突]()；不同语言或不同标准库的实现方法各有特色，总体可以归纳为下：

1. 对于一组源数据，散列函数会尽可能使其输出均匀分布，而大多数散列函数的实现都采用了LCG算法，或类似变种
2. 散列数组的扩容方式大体分为两种：倍数扩充，如MSVC12.0的STL实现和java的HashMap；素数扩充，如SGI-STL3.0和c#的HashTable；在往桶中存放数据的时候，往往需要按照桶的数量再次取模，LCG算法存在周期，而模一个质数能使这个周期达到最大，这也是素数扩充的理论基础；但如果散列函数已经足够随机，模的数是否为质数影响已然不大
3. 冲突处理，总体分为开放地址法和链地址法；MSVC12.0中实现unordered_map便采用了开放地址法，这种方法可以极大的提高cpu cache命中率，就算在冲突的情况下也能快速得到数据，但如果冲突过多，存取效率会不如链地址法；jdk1.8的HashMap采用了链地址法，在冲突时会自动转化为链表，在链表元素数目达到8且不执行扩容机制时自动转化为红黑树，红黑树元素数目降为6，在resize或remove满足相应条件时再次退化成链表，中间有一个7的缓冲防止频繁转换

本文HashTable的实现借鉴了以上方法，散列函数用的是MSVC中的_Hash_seq(xstddef:_Hash_seq)，但与其满载扩容不同，这里设定了负载因子为0.75，与java-HashMap一致，散列数组初始长度为16，按照4倍容量扩充，在达到256时按照2倍扩充，类似(xhash:_Check_size)；而冲突处理同样借鉴了java-HashMap，在冲突时转化为链表，元素个数达到6时链表转化为红黑树，但与之不同，在下一次rehash之前，当桶转化为红黑树时不会再退化成链表，转化为链表时不会再退化成普通的entry；即使通过删除元素使冲突位置不再冲突，这时候也只是多出一次比较，但却可以避免相互转化的风险  

[**参考链接：**]()&nbsp;[LCG算法介绍](https://www.cnblogs.com/vancasola/p/9942583.html)&nbsp;&nbsp;[LCG算法深入理解](https://zhuanlan.zhihu.com/p/36301602)&nbsp;&nbsp;[伪随机数发生器](https://www.zhihu.com/question/35365618/answer/991302922)&nbsp;&nbsp;[哈希函数模质数](https://www.cnblogs.com/cryingrain/p/11144225.html)&nbsp;&nbsp;[lua hash函数](https://blog.codingnow.com/2020/05/lua_hash.html)&nbsp;&nbsp;[HashMap中红黑树退化成链表的条件](https://blog.csdn.net/qq_45369827/article/details/114930945)

## <span id="10">SkipList</span>
```c
#define SKIPLIST_MAXLEVEL 32
#define SKIPLIST_P 0.25f

#define SKL_BEGIN(L) (L)->header->levelInfo[0].forward
#define SKL_LAST(L) (L)->header->backward
#define SKL_HEAD(L) (L)->header
#define SKL_END(L) SKL_HEAD(L)

typedef struct SkipListNode{
	void* pKey;
	struct SkipListNode* backward;
	unsigned short level;
	struct SkipListLevel{
		struct SkipListNode* forward;
		size_t span;
	}levelInfo[];
}SkipListNode;

typedef struct SkipList{
	SkipListNode* header;
	CmnCompareFunc equalFunc;
	CmnCompareFunc lessFunc;
	size_t length;
	size_t keySize;
	unsigned short level;
}SkipList;
```
这是基于双向循环链表实现的SkipList-Set，header是跳表的头结点，保存了每层的第一个结点指针，同时作为循环结束的标志；SkipListNode借鉴了Redis中跳表的结点数据结构实现，即增加一个span数据域表示当前结点层跳过了多少个结点，在计算排名相关操作时可以使时间复杂度与键查找保持一致；跳表的层数非均匀分布，遵循以下算法：
```c
unsigned char __random_level()
{
	unsigned char level = 1;
	while((1.0f * rand()) / RAND_MAX <= SKIPLIST_P && level < SKIPLIST_MAXLEVEL)
		level++;
	return level;
}
```
通过计算可知每个结点平均层数为$\frac{1}{1-p}$，具体证明点击[这里](https://zhuanlan.zhihu.com/p/23370124)；而跳表时间复杂度的计算，实际就是对跳表最长查找路径的计算，因为每层最长查找路径恒为$\frac{1}{p}$，要使查找路径最长，必然是每一层的查找都走到最后一个结点，而跳表层数为$log_\frac{1}{p} N$，故时间复杂度为$\frac{1}{p}·log_\frac{1}{p} N$，即为$log_\frac{1}{p} N$；这里的p与Redis中的实现保持一致，取值0.25，此时结点平均层数为1.33，时间复杂度为$log_4 N$

跳表与二叉树比较，有如下优点：  

1. 占用空间少，跳表要达到二叉树一样的效率，p的取值必然要小于等于0.5，这时候平均指针数即平均层数必然小于等于2
2. 效率更加灵活，以p取值0.25为例，平均时间复杂度$2log_4 N$，与$log_2 N$一致
3. 范围查找即为线性查找，清晰方便，而二叉树需要进行较为复杂的中序遍历
4. 实现简单，调试方便，这来源于作者原话(They are simpler to implement, debug, and so forth.)，不过对于实现简单这点我不太苟同，加入了span的跳表实现复杂度已经和二叉树相差无几了

跳表的缺点也很明显，因为层数的随机性，当数据量没有达到一个量级的时候，实际的层数分布很有可能与理论分布相差甚远，这时候效率会远远低于二叉树

[**参考链接：**]()&nbsp;[Redis为什么用跳表而不用平衡树](https://zhuanlan.zhihu.com/p/23370124)&nbsp;&nbsp;[漫画：什么是跳表](https://zhuanlan.zhihu.com/p/53975333?ivk_sa=1024320u)&nbsp;&nbsp;[图解：什么是跳表](https://mp.weixin.qq.com/s/gGL4vghqhIy_Gzcfah3FTw)  

## <span id="PriorityQueue">PriorityQueue</span>
```c
typedef struct PriorityQueue{
	SqList* list;
	CmnCompareFunc topFunc;
}PriorityQueue;
```

PriorityQueue是通过二叉堆实现的优先队列，继承了SqList的实现，并在此基础上提供了队列接口；在概念上，二叉堆分为大顶（根）堆和小顶（根）堆，但在实际编程的时候，通常是通过相关比较函数确定堆的上下分布；对二叉堆的结点有两个重要的操作，向下调整（堆化）和向上调整（为啥向上调整就不能叫堆化？），其中向上调整仅需比较自身与父结点，向下调整需比较自身和左右孩子结点  

[**参考链接：**]()&nbsp;[图解：什么是二叉堆](https://mp.weixin.qq.com/s/wVrklsni7WyuCQkNlbrbUQ)  

## <span id="UnDirectedGraph">UnDirectedGraph</span>
```
#define UG_MAX_VERTEX_NUM 15
#define UG_GEN_EDGE_RATE 3500
#define UG_MAX_WEIGHT 100
typedef struct UGEdgeNode{
	struct UGEdgeNode* ilink;
	struct UGEdgeNode* jlink;
	int ivex;
	int jvex;
	int weight;
}UGEdgeNode;

typedef struct UGVertexNode{
	int data;
	UGEdgeNode* firstEdge;
}UGVertexNode;

typedef struct UGraph{
	UGVertexNode adjmulist[UG_MAX_VERTEX_NUM];
	int vexNum;
	int edgeNum;
}UGraph;
```
这是使用邻接多重表实现的无向图，由于图结构侧重的是数据之间的关系并不关心数据存储，所以无向图和下面有向图的实现均使用int型表达顶点数据和边权重；UnDirectedGraph实现了无向图大部分算法，其中包括无向图随机生成，图的展示，DFS/BFS，增加/删除一条边，判断连通性，是否有环，最小生成树的两种经典算法和最短路径的三种经典算法等  

> 图的生成

图的随机生成其实就是对边的生成，这取决于实际需求；比如使两两顶点之间按照一定概率$p$生成边，只需在生成新顶点的时候，按照这个概率遍历新顶点与其他所有已生成顶点即可，这样任意两个顶点之间存在边的概率都为$p$，总执行次数$\frac{n(n-1)}{2}$，时间复杂度O($n^2$)；而文中为了实现对图路径方面更加严密的测试，会将$p$乘以已生成顶点与新顶点的序号比值，即期望顶点序号相隔越远的顶点存在边的概率越低

> DFS/BFS

DFS(Depth First Search)深度优先搜索，BFS(Breadth First Search)广度优先搜索，是对图的两种遍历方式，采用邻接矩阵实现的图，边的生成顺序不影响对顶点的遍历顺序，而采用（逆）邻接表，十字链表，邻接多重表实现的图，边的生成顺序会直接影响到遍历后的顶点序列，因为指针顺序不同；比较特殊的是，十字链表，邻接多重表存储的是一条边，可以采用DFS/BFS对存储的边结构进行遍历，而对边的遍历，所产生的顶点序列也不一样，在这种结构中，对边和顶点的DFS/BFS本质的区别就是对边表结点不同数据域的DFS/BFS

> 连通性

判断无向图的连通性可以采用DFS BFS 并查集 warshell等方法，这几种算法本质大同小异；DFS/BFS从某顶点开始进行遍历，如果遍历结束时遍历了所有顶点，则图连通；并查集类似，查看所有顶点是否具有相同的根，如果存储结构采用邻接多重表，这种方式反而有点画蛇添足；warshell算法思想是构造一个新的邻接矩阵，如果A顶点和B顶点连通，则以A或B为顶点的边继承B或A的所有连通关系，结束时判断矩阵中所有数值是否大于0即可  

而判断有向图的强连通性同样可以采用上述方法，不过是对所有顶点进行暴力计算，而大多数判断有向图强连通性的算法其实都是基于以上算法的优化；其中比较出名的是Kosaraju算法，不过个人认为Kosaraju算法思路比较笨重，而网上所看到的实现都没有跳出它的思路框架，更加简洁的思路和实现应该是：  

1. 选定一个顶点A，对A的出度进行遍历，用并查集U记录
2. 如果U没有具有相同的根，则有向图不强连通，否则转到步骤3
3. 重置U，对A的入度进行反向遍历，并查集U记录
4. 如果U具有相同的根，结合步骤2则可说明：从A可以到达其他所有顶点，从其他所有顶点也可以到达A，即任意两个顶点相互强连通

> 环

判断无向图是否有环可以采用以下方法：  

1. DFS/BFS，对边遍历，遇到重复边则存在环
2. 并查集，对边遍历，在将一条边的两个顶点插入并查集前如果这两个顶点已具有相同根，则存在环
3. 拓扑思想，拓扑排序是对于有向图来说，但这里可以借鉴它的思想，首先弹出度为1的顶点，将自身与所有邻接顶点的度减1，重复此步骤直至最后，检查是否弹出了所有顶点

判断有向图是否有环同样可以采用以上方法，其中以拓扑排序最为经典

> 最小生成树

+ 克鲁斯卡尔(Kruskal)  
Kruskal算法先将所有边权重从小到大排序，依次从边集中取出当前权重最小的边（可用优先队列优化），如果新取的边会组成环（并查集检测），则舍弃，直至取够n-1条边；采用堆优化的时间复杂度为O($elog_2 e$)
+ 普里姆(Prim)  
相对Kruskal“加边法”，Prim可以看作“加点法”，算法思想是既定两个集合A和B，A初始为空，B初始储存了所有顶点，从B中任选一个顶点移动到A，然后重复下面步骤：遍历集合A所有顶点的边，选出集合A中到集合B中最短边，并将对应邻接点从B加入到A，直至边数为n-1；由于遍历了图中所有顶点，而每一次遍历实质都是遍历当前顶点的所有边，所以时间复杂度为O($n^2$)

> 最短路径

+ 迪杰斯特拉(Dijkstra)  
Dijkstra算法是求某源点到其他所有顶点的最短路径，算法思想有点类似最小生成树中的Prim算法，初始既定一个路径向量数组P（用于记录顶点在路径上的后继顶点），一个权重数组W（用于记录顶点到所求顶点v的路径权重），集合A（初始为空），集合B（初始仅包含v），重复执行以下步骤：在B中选出在权重数组中最小的顶点u，将u移动到A，将u所有不在A中的邻接点放入B，并刷新他们在W中的权重和在P中的后继结点；当集合A包含所有顶点时，v到其他顶点的最短路径就已求得；Dijktra算法同样可以采用优先队列进行优化，时间复杂度为O($n^2$)

+ 弗洛伊德(Floyd)  
Floyd算法是求每一对顶点之间的最短路径，基于公式$D^{(k)}[i][j]=Min\{D^{(k-1)}[i][j], D^{(k-1)}[i][k] + D^{(k-1)}[k][j]\}$，算法思路如下（摘自《数据结构C语言版第2版》）：  
		1. 在$v_i$和$v_j$之间加入$v_0$，比较$(v_i, v_j)$和$(v_i, 0, v_j)$的路径长度，取较短者作为$(v_i, v_j)$新的路径长度  
		2. 在$v_i$和$v_j$之间加入$v_1$，比较$(v_i, v_j)$和$(v_i, ..., v_k)+(v_k, ..., v_j)$的路径长度，取较短者更新  
		3. 重复上面步骤直至加入所有顶点比较  
可以看出，方阵$D^{(k)}$的计算基于动态规划思想，每一次的计算都是基于前一次计算所得的最短路径；从这点看，教材的思路便具有了迷惑性，教材把每次加入比较顶点与计算次序相绑定，科学一点的理解应该是每次加入不同的中转点，比较上一次计算的路径长度与经过中转点的路径长度；比如假设有一个array[4]={4,2,1,3}保存了所有顶点序列，则公式可以表示为$D^{(k)}[i][j]=Min\{D^{(k-1)}[i][j], D^{(k-1)}[i][array[k]] + D^{(k-1)}[array[k]][j]\}$，不过在实际编程中，肯定是利用原公式的技巧性，但思路还需按照动态规划的思想去理解

## <span id="DirectedGraph">DirectedGraph</span>

## <span id="Graph">Graph</span>
图的概念实在太多，为防止复习时翻书，现摘抄如下，以下内容摘自《数据结构C语言版第2版》，部分摘自 [图解：什么是图](https://mp.weixin.qq.com/s/ZP8OnqftqCr9q3wjBg6TMA)  

> 图的定义

图$G$(Graph)由两个集合$V$和$E$组成，记为$G=(V,E)$，其中$V$是顶点(vertex)的有穷非空集合，$E$是$V$中顶点偶对的有穷集合，这些顶点偶对称为边(edge)；边分为两种，无向边和有向边，无向边用无序偶$(x,y)$表示，其中$(x,y)$和$(y,x)$表示同一条边；有向边也称为弧(arc)，用有序偶$<x,y>$表示，$x$为弧尾，$y$为弧头，$x$指向$y$，其中$<x,y>$和$<y,x>$是两个顶点之间的不同有向边  

> 图的基本术语

+ [**子图(subgraph)**]() 假设有两个图$G=(V,E)$和$G'=(V',E')$，如果$V'⊆V$且$E'⊆E$，则称$G'$为$G$的子图
+ [**简单图**]() 在图中，若不存在顶点到自身的边（自环），且同一条边不重复出现，则称这样的图为简单图
+ [**无向完全图**]() 对于无向图，如果任意两个顶点之间都存在边，则称该图为无向完全图，此时边数为$n(n-1)/2$
+ [**有向完全图**]() 对于有向图，如果任意两个顶点之间都存在互为相反的两条弧，则称该图为无向完全图，此时边数为$n(n-1)$
+ [**稀疏图和稠密图**]() 稀疏图和稠密图是相对模糊的概念，通常认为边数或弧数小于$n·log_2 N$的图为稀疏图，反之为稠密图
+ [**权和网**]() 图的每条边可以标上具有某种含义的数值，该数值称为该边上的权(weight)，这些权可以表示从一个顶点到另外一点顶点的距离或耗费或其他；这种带权的图称为网(network)
+ [**邻接点(adjacent)**]() 对于无向图，如果边$(v,v')∈E$，则称顶点$v$和$v'$互为邻接点
+ [**度(degree) 入度(indegree) 出度(outdegree)**]() 顶点$v$的度是指和$v$相关联的边的数目，记作$TD(v)$；对于有向图，顶点$v$的度分为入度和出度，入度是以顶点$v$为头的弧的数目，记作$ID(v)$，出度以顶点$v$为尾的弧的数目，记作$OD(v)$，$TD(v)=ID(v)+OD(v)$，$e=\frac{1}{2}\sum_{i=0}^n TD(v_i)$
+ [**路径(path)和路径长度**]() 路径是从一个顶点到另外一个顶点的顶点序列，在有向图中，路径是有向的；路径长度是一条路径上经过的边或弧的数目
+ [**回路或环(cycle)**]() 第一个顶点和最后一个顶点相同的路径称为回路或环
+ [**简单路径 简单回路或简单环**]() 序列中顶点不重复出现的路径称为简单路径；除第一个顶点和最后一个顶点外，其余顶点不重复出现的回路称为简单回路或简单环
+ [**连通 连通图(connected graph) 连通分量(connected component)**]() 在无向图$G$中，如果顶点$v_1$和顶点$v_2$有路径，则称$v_1$和$v_2$是连通的；如果对于图中的任意两个顶点都是连通的，则图$G$称为连通图；而连通分量，则是无向图的极大连通子图，连通图只有一个极大连通子图，就是连通图本身，而非连通图会有多个极大连通子图
+ [**强连通(strongly connected) 强连通图 强连通分量**]() 概念类似上，区别是强连通是对于有向图来说的；同样的，强连通图只有一个极大强连通子图，就是强连通图本身，而非强连通图会有多个极大强连通子图
+ [**生成树(spanning tree)**]() 连通图中，一个极小连通子图，它含有图中的全部顶点，但只有足以构成一棵树的$n-1$条边，这样的连通子图称为连通图的生成树；如果一个图有$n$个顶点和小于$n-1$条边，则是非连通图，如果多于$n-1$条边，则一定有环，但是有$n-1$条边的图不一定是生成树，以下是生成树的一些性质：
	+ 一个连通图可以有多个生成树，它们都只有构成图的$n$个顶点和构成一棵树的$n-1$条边
	+ 生成树中不存在环，往生成树中添加一条边必然会构成环
	+ 移除生成树任意一条边都会导致图的不连通，这是生成树即极小连通子图的特性
	+ 对于包含$n$个顶点的无向完全图最多包含$n^{n-2}$棵生成树，这是Cayley公式，具体证明看 [经典证明：Prüfer编码与Cayley公式](http://www.matrix67.com/blog/archives/682#comment-1221689)
+ [**最小代价生成树(minimum cost spanning tree)**]() 在一个连通网所有生成树中，各边的代价之和最小的那棵树称为该连通网的最小代价生成树，简称最小生成树
+ [**有向树和生成森林(spanning forest)**]() 有一个顶点入度为0，其余顶点入度均为1的有向图称为有向树；一个有向图的生成森林是由若干棵有向树组成，含有图中的全部顶点，但只有足以构成若干棵不相交的有向树的弧
+ [**DGA图(directed acycline graph)**]() 有向无环图
+ [**AOV-网(activity on vertex network)**]() 用顶点表示活动，用弧表示活动间优先关系的DGA图称为AOV-网
+ [**AOE-网(activity on edge network)**]() 用顶点表示事件，弧表示活动，权表示活动的消耗的DGA图称为AOE-网

> 图的存储方式

+ 邻接矩阵(Adjacency Matrix)

	+ 优点
		1. 便于判断两个顶点之间是否有边，通过$A[i][j]$的值来判断
		2. 便于计算各个顶点的度；对于无向图，邻接矩阵第$i$行元素个数就是顶点$i$的度；对于有向图，第$i$行元素个数就是顶点$i$的出度，第$i$列元素个数就是顶点$i$的出度
	+ 缺点
		1. 由于用顺序数组存储边，所以不便于增加和删除顶点
		2. 不便于统计边的数目，需要扫描邻接矩阵所有元素才能统计完毕，时间复杂度为O($n^2$)
		3. 空间复杂度高；如果是有向图，$n$个顶点需要$n^2$个单元存储边；如果是无向图，因其邻接矩阵是对称的，所以对规模较大的邻接矩阵可以采用压缩存储的方法，仅存储下三角（或上三角）的元素，这样需要$n(n-1)/2$个单元即可；但无论是以何种方式存储，邻接矩阵表示法的空间复杂度均为O($n^2$)，这对于稀疏图而言尤其浪费空间

+ 邻接表(adjacency list)和逆邻接表(inverse adjacency list)

	+ 优点
		1. 便于增加和删除顶点
		2. 便于统计边的数目，按顶点表顺序扫描所有边表可得到边的数目，时间复杂度为O($n+e$)
		3. 空间效率高；对于一个具有$n$个顶点$e$条边的图$G$，若$G$是无向图，则在邻接表表示中有$n$个顶点和$2e$个边表结点；若$G$是有向图，则在它的邻接表表示或逆邻接表表示中均有$n$个顶点表结点和$e$个边表结点；因此，邻接表或逆邻接表表示的空间复杂度为O($n+e$)，适合表示稀疏图；对于稠密图，考虑到邻接表中需要附加链域，因此常采取邻接矩阵表示法
	+ 缺点
		1. 不便于判断顶点之间是否有边，要判定$v_i$和$v_j$之间是否有边，就需扫描第$i$个边表，最坏情况下要耗费O($n$)时间
		2. 不便于计算有向图各个顶点的度；对于无向图，在邻接表表示中顶点$v_i$的度是第$i$个边表中结点个数；在有向图的邻接表中，第$i$个边表结点个数是顶点$v_i$的出度，但求$v_i$的入度较困难，需遍历各顶点的边表；若有向图采用逆邻接表表示，则与邻接表表示相反，求顶点的入度容易，而求顶点的出度较难

+ 十字链表(orthogonal list)

	+ 十字链表是有向图的另一种链式存储结构，可以看作邻接表和逆邻接表的组合体，可以容易求出顶点的出度和入度，且不存在重复边，在某些有向图的应用中，十字链表是很有用的工具

+ 邻接多重表(adjacency multilist)

	+ 类似十字链表，邻接多重表是无向图的另一种链式存储结构，边表结点存储的是一条边，更加关注的是对边的操作

+ 边集数组(edgeset array)

	+ 边集数组是由两个一维数组构成，一个是存储顶点的信息，另一个存储边的信息，这个边数组每个元素由一条边的起点下标(begin)，终点下标(end)和权(weight)组成；带权图（网）另一种存储结构是边集数组，它适用于一些以边为主的操作

邻接矩阵，邻接表和逆邻接表对于有向图和无向图都适用，但他们更关注顶点的操作，其中邻接矩阵容易判断两个顶点之间是否有边，顺序操作逻辑清晰，但不便于增加和删除结点，也不便于统计边的数目，空间利用率较低，邻接表和逆邻接表反之；十字链表和邻接多重表更加关注的是对边的操作，比如对搜索过的边作记号或者删除一条边等，其中十字链表适用于有向图，邻接多重表适用于无向图；边集数组适用于带权图对边的操作，但不便于通过顶点到边的映射
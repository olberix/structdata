/*
*红黑树引理:一棵有n个内部结点的红黑树的高度h<=2log(2, n+1),证明:
*1.将红黑树合并为2-3-4树,则n>=2^bh(N)-1,得bh(N)<=log(2,n+1),当不存在红色结点时等号成立
*	(其实这一步可以省略理解,因为有n个结点的二叉树,必然有n+1个NULL结点,2-3-4树属于满二叉树,直接得出bh(N)<=log(2,n+1))
*2.因为h<=2bh(N),当某个分支上除叶子结点外红黑结点各一半时等号成立
*3.所以h<=2log(2,n+1),增删改查时间复杂度为log(2,n)
*/
#ifndef _RB_TREE__
#define _RB_TREE__
#include "../common/common.h"
#include <stdbool.h>

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

typedef struct RBTreeOp{
	RBTree* (*create)(size_t, size_t, CmnCompareFunc, CmnCompareFunc);
	void (*clear)(RBTree*);
	void (*destroy)(RBTree**);
	void (*level_order_traverse)(RBTree*, UnorderedForEachFunc_Mutable, void*);/*保留层次遍历,用于调试*/
	void (*traverse)(RBTree*, UnorderedForEachFunc_Mutable, void*);
	void (*insert)(RBTree*, const void*, const void*);
	void (*erase)(RBTree*, const void*);
	const void* (*at)(RBTree*, const void*);
	void (*change)(RBTree*, const void*, const void*);
	size_t (*size)(RBTree*);
}RBTreeOp;

extern const RBTreeOp* GetRBTreeOpStruct();
#define RBTree() (*(GetRBTreeOpStruct()))
#endif
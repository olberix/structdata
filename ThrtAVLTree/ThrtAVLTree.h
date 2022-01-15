#ifndef _AVL_TREE__
#define _AVL_TREE__
#include "../common/common.h"
#include <stdbool.h>

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

typedef struct AVLTreeOp{
	AVLTree* (*create)(size_t, size_t, CmnCompareFunc, CmnCompareFunc);
	void (*clear)(AVLTree*);
	void (*destroy)(AVLTree**);
	void (*pre_order_traverse)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*in_order_traverse)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*post_order_traverse)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*pre_order_traverse_st)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*in_order_traverse_st)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*post_order_traverse_st)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*in_order_traverse_thrt)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*level_order_traverse)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*traverse)(AVLTree*, UnorderedForEachFunc_Mutable, void*);
	void (*insert)(AVLTree*, const void*, const void*);
	void (*erase)(AVLTree*, const void*);
	const void* (*at)(AVLTree*, const void*);
	void (*change)(AVLTree*, const void*, const void*);
	size_t (*size)(AVLTree*);
}AVLTreeOp;

extern const AVLTreeOp* GetAVLTreeOpStruct();
#define AVLTree() (*(GetAVLTreeOpStruct()))
#endif
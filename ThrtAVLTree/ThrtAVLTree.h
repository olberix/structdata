#ifndef _AVL_TREE__
#define _AVL_TREE__
#include "../common.h"
#include <stdbool.h>

typedef struct AVLNode{
	void* pKey;
	void* pValue;
	struct AVLNode* lchild;
	struct AVLNode* rchild;
	unsigned char height;/*节点的最大高度,叶子节点恒为1,用于计算Balanced factor*/
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

typedef struct AVLTreeOp{
	AVLTree* (*create)(size_t, size_t, AVLKeyCompareFuncT, AVLKeyCompareFuncT);
	void (*clear)(AVLTree*);
	void (*destroy)(AVLTree**);
	void (*pre_order_traverse)(AVLTree*, AVLForEachFuncT);
	void (*in_order_traverse)(AVLTree*, AVLForEachFuncT);
	void (*post_order_traverse)(AVLTree*, AVLForEachFuncT);
	void (*pre_order_traverse_st)(AVLTree*, AVLForEachFuncT);
	void (*in_order_traverse_st)(AVLTree*, AVLForEachFuncT);
	void (*post_order_traverse_st)(AVLTree*, AVLForEachFuncT);
	void (*in_order_traverse_thrt)(AVLTree*, AVLForEachFuncT);
	void (*level_order_traverse)(AVLTree*, AVLForEachFuncT);
	void (*traverse)(AVLTree*, AVLForEachFuncT);
	void (*insert)(AVLTree*, const void*, const void*);
	void (*erase)(AVLTree*, const void*);
}AVLTreeOp;

extern const AVLTreeOp* GetAVLTreeOpStruct();
#define AVLTree() (*(GetAVLTreeOpStruct()))
#endif
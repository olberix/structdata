#include "RBTree.h"
#include "../SqStack/SqStack.h"
#include "../DlQueue/DlQueue.h"
#include <string.h>

#define ROOTCREATE(tree, root, pkey, pValue)\
	POINTCREATE_INIT(RBNode*, (root), RBNode, sizeof(RBNode));\
	POINTCREATE(EMPTYDEF, (root)->pKey, void, (tree)->keySize);\
	POINTCREATE(EMPTYDEF, (root)->pValue, void, (tree)->valSize);\
	memcpy((root)->pKey, pKey, (tree)->keySize);\
	memcpy((root)->pValue, pValue, (tree)->valSize);

static RBTree* create(size_t keySize, size_t valSize, CmnCompareFunc equalFunc, CmnCompareFunc lessFunc)
{
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC, __FILE__, __LINE__);
	size_t tr_s = sizeof(RBTree);
	POINTCREATE_INIT(RBTree*, ret, RBTree, tr_s);
	POINTCREATE(EMPTYDEF, ret->tmpRet, void, valSize);
	ret->keySize = keySize;
	ret->valSize = valSize;
	ret->equalFunc = equalFunc;
	ret->lessFunc = lessFunc;
	return ret;
}

static void __clear(RBNode* root)
{
	ROOTCHECK(root);
	__clear(root->lchild);
	__clear(root->rchild);
	RELEASENODE(root);
}

static inline void clear(RBTree* tree)
{
	__clear(tree->root);
	tree->root = NULL;
}

static inline void destroy(RBTree** stree)
{
	clear(*stree);
	FREE((*stree)->tmpRet);
	FREE(*stree);
}

/*层级遍历*/
static void level_order_traverse(RBTree* tree, UnorderedForEachFunc_Mutable func, void* args)
{
	RBNode* root = tree->root;
	ROOTCHECK(root);
	DlQueue* queue = DlQueue().create(sizeof(RBNode*));
	DlQueue().push(queue, &root);
	while(!DlQueue().empty(queue)){
		root = TOCONSTANT(RBNode*, DlQueue().pop(queue));
		func(root->pKey, root->pValue, args);
#ifdef DEBUG
		printf("%d\n", root->color);
#endif
		RBNode* tmpRoot = root->lchild;
		if (tmpRoot)
			DlQueue().push(queue, &tmpRoot);
		tmpRoot = root->rchild;
		if (tmpRoot)
			DlQueue().push(queue, &tmpRoot);
	}
	DlQueue().destroy(&queue);
}

/*中序栈遍历*/
static inline void traverse(RBTree* tree, UnorderedForEachFunc_Mutable func, void* args)
{
	RBNode* root = tree->root;
	ROOTCHECK(root);
	SqStack* stack = SqStack().create(sizeof(RBNode*), NULL);
	while(root || !SqStack().empty(stack)){
		if (root){
			SqStack().push(stack, &root);
			root = root->lchild;
		}
		else{
			root = TOCONSTANT(RBNode*, SqStack().pop(stack));
			func(root->pKey, root->pValue, args);
			root = root->rchild;
		}
	}
	SqStack().destroy(&stack);
}

static inline void right_rotation(RBTree* tree, RBNode* root)
{
	RBNode* P = root->parent;
	RBNode* L = root->lchild;
	RBNode* LR = L->rchild;
	root->parent = L;
	root->lchild = LR;
	L->rchild = root;
	if (LR)
		LR->parent = root;
	if (P){
		L->parent = P;
		if (P->lchild == root)
			P->lchild = L;
		else
			P->rchild = L;
	}
	else{
		L->parent = NULL;
		tree->root = L;
	}
}

static inline void left_rotation(RBTree* tree, RBNode* root)
{
	RBNode* P = root->parent;
	RBNode* R = root->rchild;
	RBNode* RL = R->lchild;
	root->parent = R;
	root->rchild = RL;
	R->lchild = root;
	if (RL)
		RL->parent = root;
	if (P){
		R->parent = P;
		if (P->rchild == root)
			P->rchild = R;
		else
			P->lchild = R;
	}
	else{
		R->parent = NULL;
		tree->root = R;
	}
}

/*非递归的实现更好*/
static void do_balance_insert(RBTree* tree, RBNode* root)
{
	if (tree->root == root){
		root->color = RB_BLACK;
		return;
	}
	else if (root->parent->color == RB_BLACK){
		return;
	}
	else{
		RBNode* P = root->parent;
		RBNode* G = P->parent;
		if (G->lchild == P){
			RBNode* U = G->rchild;
			if (!U || U->color == RB_BLACK){
				if (P->rchild == root)
					left_rotation(tree, P);
				right_rotation(tree, G);
				G->parent->color = G->color;
				G->color ^= 0x1;
			}
			else{
				P->color = U->color = RB_BLACK;
				G->color = RB_RED;
				do_balance_insert(tree, G);
			}
		}
		else{
			RBNode* U = G->lchild;
			if (!U || U->color == RB_BLACK){
				if (P->lchild == root)
					right_rotation(tree, P);
				left_rotation(tree, G);
				G->parent->color = G->color;
				G->color ^= 0x1;
			}
			else{
				P->color = U->color = RB_BLACK;
				G->color = RB_RED;
				do_balance_insert(tree, G);
			}
		}
	}
}

static void insert(RBTree* tree, const void* pKey, const void* pValue)
{
	RBNode* root = tree->root;
	if (!root){
		ROOTCREATE(tree, newRoot, pKey, pValue);
		root = tree->root = newRoot;
		root->color = RB_BLACK;
		tree->tree_size = 1;
		return;
	}
	while(true){
		if (tree->equalFunc(root->pKey, pKey)){
			memcpy(root->pValue, pValue, tree->valSize);
			return;
		}
		if (tree->lessFunc(root->pKey, pKey)){
			if (root->lchild)
				root = root->lchild;
			else{
				ROOTCREATE(tree, newRoot, pKey, pValue);
				root->lchild = newRoot;
				newRoot->parent = root;
				do_balance_insert(tree, newRoot);
				tree->tree_size++;
				return;
			}
		}
		else{
			if (root->rchild)
				root = root->rchild;
			else{
				ROOTCREATE(tree, newRoot, pKey, pValue);
				root->rchild = newRoot;
				newRoot->parent = root;
				do_balance_insert(tree, newRoot);
				tree->tree_size++;
				return;
			}
		}
	}
}

static inline void reset_parent_point(RBTree* tree, RBNode* P, RBNode* rlc, RBNode* C)
{
	if (P){//非根节点
		if (P->lchild == rlc)
			P->lchild = C;
		else
			P->rchild = C;
	}//根节点
	else
		tree->root = C;
}

/*非递归实现*/
static inline void do_balance_erase(RBTree* tree, RBNode* rlc)
{
	RBNode* P = rlc->parent;
	//红色结点-无孩子
	if (rlc->color == RB_RED){
		reset_parent_point(tree, P, rlc, NULL);
		return;
	}
	RBNode* C = rlc->lchild;
	if (!C)
		C = rlc->rchild;
	//黑色结点-仅有一个(红色)孩子
	if (C){
		reset_parent_point(tree, P, rlc, C);
		C->parent = P;
		C->color = RB_BLACK;
		return;
	}
	//黑色结点-无孩子
	RBNode* pivot = rlc;
	while(true){
		if (!P)//根节点,黑高减一
			break;
		//S是P左孩子
		if (P->rchild == pivot){
			RBNode* S = P->lchild;
			//S为黑色
			if (S->color == RB_BLACK){
				RBNode* SL = S->lchild;
				//SL为红色
				if (SL && SL->color == RB_RED){
					right_rotation(tree, P);
					SL->color ^= 0x1;
					S->color = P->color;
					P->color = RB_BLACK;
					break;
				}
				RBNode* SR = S->rchild;
				//SR为红色(SL,SR同为红色时可以选择前后两步任意操作,不同选择会导致红黑树表现不同)
				if (SR && SR->color == RB_RED){
					left_rotation(tree, S);
					right_rotation(tree, P);
					SR->color = P->color;
					P->color = RB_BLACK;
					break;
				}
				//SL,SR均为黑色
				S->color = RB_RED;
				if (P->color == RB_RED){//P为红色
					P->color = RB_BLACK;
					break;
				}
				pivot = P;//P为黑色,递归判断
				P = P->parent;
			}
			//S为红色,递归判断(转化为S为黑色的情况)
			else{
				right_rotation(tree, P);
				P->color ^= 0x1;
				S->color ^= 0x1;
			}
		}
		//S是P右孩子
		else{
			RBNode* S = P->rchild;
			//S为黑色
			if (S->color == RB_BLACK){
				RBNode* SR = S->rchild;
				//SR为红色
				if (SR && SR->color == RB_RED){
					left_rotation(tree, P);
					SR->color ^= 0x1;
					S->color = P->color;
					P->color = RB_BLACK;
					break;
				}
				RBNode* SL = S->lchild;
				//SL为红色(SR,SL同为红色时可以选择前后两步任意操作,不同选择会导致红黑树表现不同)
				if (SL && SL->color == RB_RED){
					right_rotation(tree, S);
					left_rotation(tree, P);
					SL->color = P->color;
					P->color = RB_BLACK;
					break;
				}
				//SR,SL均为黑色
				S->color = RB_RED;
				if (P->color == RB_RED){//P为红色
					P->color = RB_BLACK;
					break;
				}
				pivot = P;//P为黑色,递归判断
				P = P->parent;
			}
			//S为红色,递归判断(转化为S为黑色的情况)
			else{
				left_rotation(tree, P);
				P->color ^= 0x1;
				S->color ^= 0x1;
			}
		}
	}
	reset_parent_point(tree, rlc->parent, rlc, NULL);
}

static void erase(RBTree* tree, const void* pKey)
{
	RBNode* root = tree->root;
	while(root){
		if (tree->equalFunc(root->pKey, pKey)){
			RBNode* rlc = root->rchild;
			if (rlc){
				while(rlc->lchild)
					rlc = rlc->lchild;
				memcpy(root->pKey, rlc->pKey, tree->keySize);
				memcpy(root->pValue, rlc->pValue, tree->valSize);
			}
			else
				rlc = root;
			do_balance_erase(tree, rlc);
			tree->tree_size--;
			RELEASENODE(rlc);
			return;
		}
		if (tree->lessFunc(root->pKey, pKey))
			root = root->lchild;
		else
			root = root->rchild;
	}
}

const void* at(RBTree* tree, const void* pKey)
{
	RBNode* root = tree->root;
	while(root){
		if (tree->equalFunc(root->pKey, pKey))
			return root->pValue;
		if (tree->lessFunc(root->pKey, pKey))
			root = root->lchild;
		else
			root = root->rchild;
	}
	return NULL;
}

void change(RBTree* tree, const void* pKey, const void* pValue)
{
	insert(tree, pKey, pValue);
}

static inline size_t size(RBTree* tree)
{
	return tree->tree_size;
}

inline const RBTreeOp* GetRBTreeOpStruct()
{
	static const RBTreeOp OpList = {
		.create = create,
		.clear = clear,
		.destroy = destroy,
		.level_order_traverse = level_order_traverse,
		.traverse = traverse,
		.insert = insert,
		.erase = erase,
		.at = at,
		.change = change,
		.size = size,
	};
	return &OpList;
}
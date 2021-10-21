#include "RBTree.h"
#include "../SqStack/SqStack.h"
#include "../DlQueue/DlQueue.h"
#include <string.h>

#define ROOTCREATE(tree, root, pkey, pValue)\
	POINTCREATE(RBNode*, (root), RBNode, sizeof(RBNode));\
	memset((root), 0, sizeof(RBNode));\
	POINTCREATE(EMPTYDEF, (root)->pKey, void, (tree)->keySize);\
	POINTCREATE(EMPTYDEF, (root)->pValue, void, (tree)->valSize);\
	memcpy((root)->pKey, pKey, (tree)->keySize);\
	memcpy((root)->pValue, pValue, (tree)->valSize);

static RBTree* create(size_t keySize, size_t valSize, RBKeyCompareFuncT equalFunc, RBKeyCompareFuncT lessFunc)
{
	CONDCHECK(keySize > 0 && valSize > 0, STATUS_SIZEERROR);
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC);
	size_t tr_s = sizeof(RBTree);
	POINTCREATE(RBTree*, ret, RBTree, tr_s);
	memset(ret, 0, tr_s);
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
	FREE(*stree);
}

/*层级遍历*/
static void level_order_traverse(RBTree* tree, RBForEachFuncT func)
{
	RBNode* root = tree->root;
	ROOTCHECK(root);
	DlQueue* queue = DlQueue().create(sizeof(RBNode*));
	DlQueue().push(queue, &root);
	while(!DlQueue().empty(queue)){
		root = TOCONSTANT(RBNode*, DlQueue().pop(queue));
		func(root->pKey, root->pValue);
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
}

/*中序栈遍历*/
static inline void traverse(RBTree* tree, RBForEachFuncT func)
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
			func(root->pKey, root->pValue);
			root = root->rchild;
		}
	}
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
		return;
	}
	while(root){
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
				return;
			}
		}
	}
}

static void erase(RBTree* tree, const void* pKey)
{

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
	};
	return &OpList;
}
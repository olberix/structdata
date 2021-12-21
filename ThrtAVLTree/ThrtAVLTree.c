#include "ThrtAVLTree.h"
#include "../SqStack/SqStack.h"
#include "../DlQueue/DlQueue.h"
#include <string.h>

#define ROOTCREATE(tree, root, pkey, pValue)\
	POINTCREATE_INIT(AVLNode*, (root), AVLNode, sizeof(AVLNode));\
	(root)->height = 1;\
	POINTCREATE(EMPTYDEF, (root)->pKey, void, (tree)->keySize);\
	POINTCREATE(EMPTYDEF, (root)->pValue, void, (tree)->valSize);\
	memcpy((root)->pKey, pKey, (tree)->keySize);\
	memcpy((root)->pValue, pValue, (tree)->valSize);

static AVLTree* create(size_t keySize, size_t valSize, AVLKeyCompareFuncT equalFunc, AVLKeyCompareFuncT lessFunc)
{
	CONDCHECK(keySize > 0 && valSize > 0, STATUS_SIZEERROR, __FILE__, __LINE__);
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC, __FILE__, __LINE__);
	size_t tr_s = sizeof(AVLTree);
	POINTCREATE_INIT(AVLTree*, ret, AVLTree, tr_s);
	ret->thrtHead.lchild = &ret->thrtHead;
	ret->thrtHead.rchild = &ret->thrtHead;
	ret->thrtHead.ThrtFlag = 0x3;
	ret->keySize = keySize;
	ret->valSize = valSize;
	ret->equalFunc = equalFunc;
	ret->lessFunc = lessFunc;
	return ret;
}

static inline AVLNode* L_CHILD(AVLNode* root)
{
	if (root->ThrtFlag & 0x1)
		return NULL;
	return root->lchild;
}

static inline AVLNode* R_CHILD(AVLNode* root)
{
	if (root->ThrtFlag & 0x2)
		return NULL;
	return root->rchild;
}

static void __clear(AVLNode* root)
{
	ROOTCHECK(root);
	__clear(L_CHILD(root));
	__clear(R_CHILD(root));
	RELEASENODE(root);
}

static inline void clear(AVLTree* tree)
{
	__clear(tree->root);
	tree->root = NULL;
	tree->thrtHead.lchild = &tree->thrtHead;
	tree->thrtHead.rchild = &tree->thrtHead;
}

static inline void destroy(AVLTree** stree)
{
	clear(*stree);
	FREE(*stree);
}

/*前序遍历-递归*/
static void __pre_order_traverse(AVLNode* root, AVLForEachFuncT func)
{
	ROOTCHECK(root);
	func(root->pKey, root->pValue);
	__pre_order_traverse(L_CHILD(root), func);
	__pre_order_traverse(R_CHILD(root), func);
}

/*中序遍历-递归*/
static void __in_order_traverse(AVLNode* root, AVLForEachFuncT func)
{
	ROOTCHECK(root);
	__in_order_traverse(L_CHILD(root), func);
	func(root->pKey, root->pValue);
	__in_order_traverse(R_CHILD(root), func);
}

/*后序遍历-递归*/
static void __post_order_traverse(AVLNode* root, AVLForEachFuncT func)
{
	ROOTCHECK(root);
	__post_order_traverse(L_CHILD(root), func);
	__post_order_traverse(R_CHILD(root), func);
	func(root->pKey, root->pValue);
}

/*前序遍历-递归*/
static inline void pre_order_traverse(AVLTree* tree, AVLForEachFuncT func)
{
	__pre_order_traverse(tree->root, func);
}

/*中序遍历-递归*/
static inline void in_order_traverse(AVLTree* tree, AVLForEachFuncT func)
{
	__in_order_traverse(tree->root, func);
}

/*后序遍历-递归*/
static inline void post_order_traverse(AVLTree* tree, AVLForEachFuncT func)
{
	__post_order_traverse(tree->root, func);
}

/*前序遍历-栈*/
static void pre_order_traverse_st(AVLTree* tree, AVLForEachFuncT func)
{
	AVLNode* root = tree->root;
	ROOTCHECK(root);
	SqStack* stack = SqStack().create(sizeof(AVLNode*), NULL);
	while(root || !SqStack().empty(stack)){
		if (root){
			func(root->pKey, root->pValue);
			SqStack().push(stack, &root);
			root = L_CHILD(root);
		}
		else{
			root = TOCONSTANT(AVLNode*, SqStack().pop(stack));
			root = R_CHILD(root);
		}
	}
	SqStack().destroy(&stack);
}

/*中序遍历-栈*/
static void in_order_traverse_st(AVLTree* tree, AVLForEachFuncT func)
{
	AVLNode* root = tree->root;
	ROOTCHECK(root);
	SqStack* stack = SqStack().create(sizeof(AVLNode*), NULL);
	while(root || !SqStack().empty(stack)){
		if (root){
			SqStack().push(stack, &root);
			root = L_CHILD(root);
		}
		else{
			root = TOCONSTANT(AVLNode*, SqStack().pop(stack));
			func(root->pKey, root->pValue);
			root = R_CHILD(root);
		}
	}
	SqStack().destroy(&stack);
}

/*后序遍历-栈*/
static void post_order_traverse_st(AVLTree* tree, AVLForEachFuncT func)
{
	AVLNode* root = tree->root;
	ROOTCHECK(root);
	SqStack* stack = SqStack().create(sizeof(AVLNode*), NULL);
	AVLNode* tmpRoot = NULL;/*记录退栈时访问的结点*/
	while(root || !SqStack().empty(stack)){
		if (root){
			SqStack().push(stack, &root);
			root = L_CHILD(root);
		}
		else{
			root = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
			if (R_CHILD(root) && R_CHILD(root) != tmpRoot)
				root = R_CHILD(root);
			else{
				SqStack().pop(stack);
				func(root->pKey, root->pValue);
				tmpRoot = root;
				root = NULL;
			}
		}
	}
	SqStack().destroy(&stack);
}

/*中序遍历-线索*/
static void in_order_traverse_thrt(AVLTree* tree, AVLForEachFuncT func)
{
	AVLNode* root = tree->root;
	ROOTCHECK(root);
	for(root = tree->thrtHead.rchild; root != &(tree->thrtHead);){
		func(root->pKey, root->pValue);
		if (R_CHILD(root)){
			root = R_CHILD(root);
			while(L_CHILD(root))
				root = L_CHILD(root);
		}
		else
			root = root->rchild;
	}
}

/*层级遍历*/
static void level_order_traverse(AVLTree* tree, AVLForEachFuncT func)
{
	AVLNode* root = tree->root;
	ROOTCHECK(root);
	DlQueue* queue = DlQueue().create(sizeof(AVLNode*));
	DlQueue().push(queue, &root);
	while(!DlQueue().empty(queue)){
		root = TOCONSTANT(AVLNode*, DlQueue().pop(queue));
		func(root->pKey, root->pValue);
		AVLNode* tmpRoot = L_CHILD(root);
		if (tmpRoot)
			DlQueue().push(queue, &tmpRoot);
		tmpRoot = R_CHILD(root);
		if (tmpRoot)
			DlQueue().push(queue, &tmpRoot);
	}
	DlQueue().destroy(&queue);
}

static inline void traverse(AVLTree* tree, AVLForEachFuncT func)
{
	in_order_traverse_st(tree, func);
}

static inline unsigned char _H(AVLNode* root)
{
	if (!root)
		return 0;
	return root->height;
}

static inline void REFRESH_H(AVLNode* root)
{
	unsigned char lh = _H(L_CHILD(root));
	unsigned char rh = _H(R_CHILD(root));
	root->height = (lh > rh ? lh : rh) + 1;
}

static inline void right_rotation(AVLTree* tree, AVLNode* pivot, AVLNode* pivotParent)
{
	AVLNode* lc = L_CHILD(pivot);
	AVLNode* lc_rc = R_CHILD(lc);
	lc->rchild = pivot;
	if (lc_rc)
		pivot->lchild = lc_rc;
	else{
		lc->ThrtFlag ^= 0x2;
		pivot->lchild = lc;
		pivot->ThrtFlag |= 0x1;
	}
	REFRESH_H(pivot);
	REFRESH_H(lc);
	if (pivotParent){
		if (L_CHILD(pivotParent) == pivot)
			pivotParent->lchild = lc;
		else
			pivotParent->rchild = lc;
	}
	else
		tree->root = lc;
}

static inline void left_rotation(AVLTree* tree, AVLNode* pivot, AVLNode* pivotParent)
{
	AVLNode* rc = R_CHILD(pivot);
	AVLNode* rc_lc = L_CHILD(rc);
	rc->lchild = pivot;
	if (rc_lc)
		pivot->rchild = rc_lc;
	else{
		rc->ThrtFlag ^= 0x1;
		pivot->rchild = rc;
		pivot->ThrtFlag |= 0x2;
	}
	REFRESH_H(pivot);
	REFRESH_H(rc);
	if (pivotParent){
		if (L_CHILD(pivotParent) == pivot)
			pivotParent->lchild = rc;
		else
			pivotParent->rchild = rc;
	}
	else
		tree->root = rc;
}

static inline void do_balance_insert(SqStack* stack, AVLTree* tree, const void* pKey)
{
	while(!SqStack().empty(stack)){
		AVLNode* tmpRoot = TOCONSTANT(AVLNode*, SqStack().pop(stack));
		unsigned char lh = _H(L_CHILD(tmpRoot));
		unsigned char rh = _H(R_CHILD(tmpRoot));
		unsigned char nh = (lh > rh ? lh : rh) + 1;
		if (nh == _H(tmpRoot))/*插入后高度不变或旋转后高度不变,则后面结点无需判断,直接break*/
			break;
		tmpRoot->height = nh;
		char factor = lh - rh;
		if (factor > 1){
			if (tree->lessFunc(L_CHILD(tmpRoot)->pKey, pKey)){
				AVLNode* parent = NULL;
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				right_rotation(tree, tmpRoot, parent);
			}
			else{
				left_rotation(tree, L_CHILD(tmpRoot), tmpRoot);
				AVLNode* parent = NULL;
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				right_rotation(tree, tmpRoot, parent);
			}
		}
		else if(factor < -1){
			if (tree->lessFunc(R_CHILD(tmpRoot)->pKey, pKey)){
				right_rotation(tree, R_CHILD(tmpRoot), tmpRoot);
				AVLNode* parent = NULL;
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				left_rotation(tree, tmpRoot, parent);
			}
			else{
				AVLNode* parent = NULL;
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				left_rotation(tree, tmpRoot, parent);
			}
		}
	}
}

static inline void insert_left_child(AVLNode* pivot, AVLNode* newRoot)
{
	AVLNode* tmpRoot = pivot->lchild;
	newRoot->lchild = tmpRoot;
	newRoot->rchild = pivot;
	newRoot->ThrtFlag = 0x3;
	if (tmpRoot->ThrtFlag & 0x2)
		tmpRoot->rchild = newRoot;
	pivot->lchild = newRoot;
	pivot->ThrtFlag ^= 0x1;
}

static inline void insert_right_child(AVLNode* pivot, AVLNode* newRoot)
{
	AVLNode* tmpRoot = pivot->rchild;
	newRoot->lchild = pivot;
	newRoot->rchild = tmpRoot;
	newRoot->ThrtFlag = 0x3;
	if (tmpRoot->ThrtFlag & 0x1)
		tmpRoot->lchild = newRoot;
	pivot->rchild = newRoot;
	pivot->ThrtFlag ^= 0x2;
}

/*
*更加省头发的做法是设置一个标志判断是否可以进行中序线索遍历,标志为false便调用中序递归或者中序栈遍历并生成线索,将标志设置为true
*在insert或者erase的时候再将标志置为false让下一次进行中序遍历重新生成线索
*这里选择了复杂的实现方式(折磨自己?),在insert&erase的时候进行线索更新,但总体效率应该高不了多少
*AVL树的栈遍历更加高效,空间复杂度因为平衡的原因可以达到logN,避免了BST斜树的情况,故此在traverse中直接调用栈遍历而不是线索遍历
*可以发现,对一组随机数据以AVL树的形式插入时,插入的数据越多(即层数越大),发生旋转的概率就越低
*因为层数越大的数据越多且离散程度也越小,从而使得后继每一次插入能比较均匀的分布在数据量最多的层次上
*/
static void insert(AVLTree* tree, const void* pKey, const void* pValue)
{
	AVLNode* root = tree->root;
	if (!root){
		ROOTCREATE(tree, newRoot, pKey, pValue);
		root = tree->root = newRoot;
		root->lchild = root->rchild = &(tree->thrtHead);
		root->ThrtFlag = 0x3;
		tree->thrtHead.lchild = tree->thrtHead.rchild = root;
		return;
	}
	SqStack* stack = SqStack().create(sizeof(AVLNode*), NULL);/*记录访问路径*/
	while(true){
		SqStack().push(stack, &root);
		if (tree->equalFunc(root->pKey, pKey)){
			memcpy(root->pValue, pValue, tree->valSize);
			break;
		}
		if (tree->lessFunc(root->pKey, pKey)){
			if (L_CHILD(root))
				root = L_CHILD(root);
			else{
				ROOTCREATE(tree, newRoot, pKey, pValue);
				insert_left_child(root, newRoot);
				do_balance_insert(stack, tree, pKey);
				break;
			}
		}
		else{
			if (R_CHILD(root))
				root = R_CHILD(root);
			else{
				ROOTCREATE(tree, newRoot, pKey, pValue);
				insert_right_child(root, newRoot);
				do_balance_insert(stack, tree, pKey);
				break;
			}
		}
	}
	SqStack().destroy(&stack);
}

/*以结点高度最大的孩子和孙子为基准进行旋转,其实就是插入的逆过程*/
static inline void do_balance_erase(SqStack* stack, AVLTree* tree)
{
	while(!SqStack().empty(stack)){
		AVLNode* tmpRoot = TOCONSTANT(AVLNode*, SqStack().pop(stack));
		unsigned char lh = _H(L_CHILD(tmpRoot));
		unsigned char rh = _H(R_CHILD(tmpRoot));
		unsigned char nh = (lh > rh ? lh : rh) + 1;
		char factor = lh - rh;
		if (tmpRoot->height == nh){//写红黑树删除的时候突然想到,AVL树的删除好像也不需要回溯到到根节点
			if (factor <= 1 && factor >= -1)
				break;
		}
		else
			tmpRoot->height = nh;
		if (factor > 1){
			AVLNode* lc = L_CHILD(tmpRoot);
			AVLNode* parent = NULL;
			if (_H(L_CHILD(lc)) >= _H(R_CHILD(lc))){/*>逻辑上也行,不过>=在=的情况下可以少一次旋转*/
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				right_rotation(tree, tmpRoot, parent);
			}
			else{
				left_rotation(tree, lc, tmpRoot);
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				right_rotation(tree, tmpRoot, parent);
			}
		}
		else if(factor < -1){
			AVLNode* rc = R_CHILD(tmpRoot);
			AVLNode* parent = NULL;
			if (_H(R_CHILD(rc)) >= _H(L_CHILD(rc))){
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				left_rotation(tree, tmpRoot, parent);
			}
			else{
				right_rotation(tree, rc, tmpRoot);
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				left_rotation(tree, tmpRoot, parent);
			}
		}
	}
}

static inline void delete_leaf_node(AVLTree* tree, AVLNode* root, AVLNode* parent)
{
	if (parent){
		if (L_CHILD(parent) == root)
			parent->ThrtFlag ^= 0x1;
		else
			parent->ThrtFlag ^= 0x2;
	}
	if (!R_CHILD(root->lchild))
		root->lchild->rchild = root->rchild;
	if (!L_CHILD(root->rchild))
		root->rchild->lchild = root->lchild;
	if(!parent)
		tree->root = NULL;

	RELEASENODE(root);
}

/*可以跟delete_leaf_node合在一起,但感觉没必要,还是这样分开明确一点*/
static inline void delete_single_node(AVLTree* tree, AVLNode* root, AVLNode* parent)
{
	AVLNode* rootChild = NULL;
	if (!R_CHILD(root)){
		rootChild = L_CHILD(root);
		AVLNode* leftMax = rootChild;
		while(R_CHILD(leftMax))
			leftMax = R_CHILD(leftMax);
		leftMax->rchild = root->rchild;
		if (!L_CHILD(root->rchild))
			root->rchild->lchild = leftMax;
	}
	else{
		rootChild = R_CHILD(root);
		AVLNode* rightMin = rootChild;
		while(L_CHILD(rightMin))
			rightMin = L_CHILD(rightMin);
		rightMin->lchild = root->lchild;
		if (!R_CHILD(root->lchild))
			root->lchild->rchild = rightMin;
	}
	if (parent){
		if (L_CHILD(parent) == root)
			parent->lchild = rootChild;
		else
			parent->rchild = rootChild;
	}
	else
		tree->root = rootChild;
	RELEASENODE(root);
}

/*
*先额外空间保存key值然后复制pValue,删除替换结点后再复制key值到原结点
*这种实现方式其实有点奇怪,比较符合逻辑的做法应该是通过更改指针指向实现
*但是AVL树的实现中加入了线索,如果采用更改指针指向做法实在太过繁杂,其实不涉及线程安全,下面这种做法没啥毛病
*/
static void erase(AVLTree*, const void*);
static inline void delete_binary_node(AVLTree* tree, AVLNode* root)
{
	AVLNode* rightMin = R_CHILD(root);
	while(L_CHILD(rightMin))
		rightMin = L_CHILD(rightMin);
	POINTCREATE(void*, tmpKey, void, tree->keySize);
	memcpy(tmpKey, rightMin->pKey, tree->keySize);
	memcpy(root->pValue, rightMin->pValue, tree->valSize);
	erase(tree, tmpKey);
	memcpy(root->pKey, tmpKey, tree->keySize);
	FREE(tmpKey);
}

static void erase(AVLTree* tree, const void* pKey)
{
	AVLNode* root = tree->root;
	ROOTCHECK(root);
	SqStack* stack = SqStack().create(sizeof(AVLNode*), NULL);
	while(true){
		if (tree->equalFunc(root->pKey, pKey)){
			if (_H(root) == 1){
				AVLNode* parent = NULL;
				if (!SqStack().empty(stack))
					parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
				delete_leaf_node(tree, root, parent);
				do_balance_erase(stack, tree);
			}
			else{
				if (L_CHILD(root) && R_CHILD(root))
					delete_binary_node(tree, root);
				else{
					AVLNode* parent = NULL;
					if (!SqStack().empty(stack))
						parent = TOCONSTANT(AVLNode*, SqStack().get_top(stack));
					delete_single_node(tree, root, parent);
					do_balance_erase(stack, tree);
				}
			}
			break;
		}
		if (tree->lessFunc(root->pKey, pKey)){
			if (L_CHILD(root)){
				SqStack().push(stack, &root);
				root = L_CHILD(root);
			}
			else
				break;
		}
		else{
			if (R_CHILD(root)){
				SqStack().push(stack, &root);
				root = R_CHILD(root);
			}
			else
				break;
		}
	}
	SqStack().destroy(&stack);
}

const void* at(AVLTree* tree, const void* pKey)
{
	AVLNode* root = tree->root;
	while(root){
		if (tree->equalFunc(root->pKey, pKey))
			return root->pValue;
		if (tree->lessFunc(root->pKey, pKey))
			root = L_CHILD(root);
		else
			root = R_CHILD(root);
	}
	return NULL;
}

void change(AVLTree* tree, const void* pKey, const void* pValue)
{
	insert(tree, pKey, pValue);
}

inline const AVLTreeOp* GetAVLTreeOpStruct()
{
	static const AVLTreeOp OpList = {
		.create = create,
		.clear = clear,
		.destroy = destroy,
		.pre_order_traverse = pre_order_traverse,
		.in_order_traverse = in_order_traverse,
		.post_order_traverse = post_order_traverse,
		.pre_order_traverse_st = pre_order_traverse_st,
		.in_order_traverse_st = in_order_traverse_st,
		.post_order_traverse_st = post_order_traverse_st,
		.in_order_traverse_thrt = in_order_traverse_thrt,
		.level_order_traverse = level_order_traverse,
		.traverse = traverse,
		.insert = insert,
		.erase = erase,
		.at = at,
		.change = change,
	};
	return &OpList;
}
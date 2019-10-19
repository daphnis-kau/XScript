//=========================================================================================
// TRBTree.h 
// 定义一个嵌入式红黑树类，参考《算法导论》第三部分 第13章
// 优点：插入和删除节点不需要进行内存的分配和释放，节点操作速度极快
//		 节点可以自删除，不需要知道所处的树
// 缺点：数据需要从指定的节点类继承
// 柯达昭
// 2017-06-16
//=========================================================================================
#ifndef __TGAMMA_RBTREE_H__
#define __TGAMMA_RBTREE_H__

#include "common/Help.h"

namespace Gamma
{
	template<typename ImpClass>
	class TRBTree
	{
		const TRBTree& operator= ( const TRBTree& );
		TRBTree( const TRBTree& );
		enum { eRed, eBlack, eDoubleBlack };
	public:
		typedef ImpClass CNode;
		typedef TRBTree<CNode> CRBTree;

		class CRBTreeNode
		{
			bool				m_bRootNode;
			int8				m_nNodeColor;
			CRBTreeNode*		m_pLeftChild;
			CRBTreeNode*		m_pRightChild;

			union 
			{
				CRBTreeNode*	m_pParent;
				CRBTree*		m_pTree;
			};

			friend class TRBTree<ImpClass>;

			// 红黑树的左旋
			void LeftRotate()
			{
				assert( m_pRightChild );
				if( m_bRootNode )
					m_pTree->m_pRootNode = m_pRightChild;
				else if( this == m_pParent->m_pLeftChild )
					m_pParent->m_pLeftChild = m_pRightChild;
				else
					m_pParent->m_pRightChild = m_pRightChild;

				CRBTreeNode* pRight = m_pRightChild;
				pRight->m_pParent = m_pParent;
				pRight->m_bRootNode = m_bRootNode;
				m_bRootNode = false;
				m_pParent = pRight;
				m_pRightChild = pRight->m_pLeftChild;
				if( m_pRightChild )
					m_pRightChild->m_pParent = this;
				pRight->m_pLeftChild = this;
			}

			// 红黑树的右旋
			void RightRotate()
			{
				assert( m_pLeftChild );
				if( m_bRootNode )
					m_pTree->m_pRootNode = m_pLeftChild;
				else if( this == m_pParent->m_pLeftChild )
					m_pParent->m_pLeftChild = m_pLeftChild;
				else
					m_pParent->m_pRightChild = m_pLeftChild;

				CRBTreeNode* pLeft = m_pLeftChild;
				pLeft->m_pParent = m_pParent;
				pLeft->m_bRootNode = m_bRootNode;
				m_bRootNode = false;
				m_pParent = pLeft;
				m_pLeftChild = pLeft->m_pRightChild;
				if( m_pLeftChild )
					m_pLeftChild->m_pParent = this;
				pLeft->m_pRightChild = this;
			}

			void ReplaceBy( CRBTreeNode* pNode )
			{
				CRBTreeNode* pParent = m_pParent;
				CRBTreeNode* pLeftChild = m_pLeftChild;
				CRBTreeNode* pRightChild = m_pRightChild;
				bool bRootNode = m_bRootNode;
				int8 nNodeColor = m_nNodeColor;
				if( bRootNode )
					m_pTree->m_pRootNode = pNode;
				else if( this == pParent->m_pLeftChild )
					pParent->m_pLeftChild = pNode;
				else
					pParent->m_pRightChild = pNode;
				if( pLeftChild )
					pLeftChild->m_pParent = pNode;
				if( pRightChild )
					pRightChild->m_pParent = pNode;
				pNode->m_bRootNode = bRootNode;
				pNode->m_nNodeColor = nNodeColor;
				pNode->m_pParent = pParent;
				pNode->m_pLeftChild = pLeftChild;
				pNode->m_pRightChild = pRightChild;
				m_bRootNode = false;
				m_nNodeColor = eBlack;
				m_pParent = NULL;
				m_pLeftChild = NULL;
				m_pRightChild = NULL;
			}

		public:
			CRBTreeNode() 
				: m_bRootNode( false )
				, m_nNodeColor( eBlack )
				, m_pParent( NULL )
				, m_pLeftChild( NULL )
				, m_pRightChild( NULL )
			{
			}

			~CRBTreeNode()
			{ 
				Remove();
			}

			bool IsBlack() const
			{
				return m_nNodeColor != eRed;
			}

			bool IsInTree() const
			{ 
				return m_pParent != NULL; 
			}

			void Remove()
			{
				if( !IsInTree() )
					return;

				// 哨兵节点，临时用
				struct CSentryNode : public CRBTreeNode
				{
					~CSentryNode()
					{
						if( !m_pParent )
							return;
						CRBTreeNode* pCurNode = this;
						assert( m_nNodeColor == eBlack );
						assert( !m_pLeftChild && !m_pRightChild );
						if( m_bRootNode )
							m_pTree->m_pRootNode = NULL;
						else if( m_pParent->m_pLeftChild == pCurNode )
							m_pParent->m_pLeftChild = NULL;
						else if( m_pParent->m_pRightChild == pCurNode )
							m_pParent->m_pRightChild = NULL;
						m_pParent = NULL;
					}
				};

				// 哨兵节点，方便删除操作，析构时自动从树上移除
				CSentryNode SentryNode;

				// 如果有两个子节点，则和后序（Next）节点进行交换，
				// 直到其中一个子节点为空。
				while( m_pLeftChild && m_pRightChild )
				{
					CRBTreeNode* pNext = GetNext();

					// 调换位置，但是保留CRBTreeNode的属性;

					// 先用SentryNode把pNode替换出来
					ReplaceBy( &SentryNode );

					// 再用pNode替换pNext
					pNext->ReplaceBy( this );

					// 再用pNext替换SentryNode
					SentryNode.ReplaceBy( pNext );
				}

				CRBTreeNode* pParent = m_pParent;
				CRBTreeNode* pNode = m_pLeftChild ? m_pLeftChild : m_pRightChild;
				if( !pNode )
					pNode = &SentryNode;

				if( m_bRootNode )
					m_pTree->m_pRootNode = pNode;
				else if( pParent->m_pLeftChild == this )
					pParent->m_pLeftChild = pNode;
				else if( pParent->m_pRightChild == this )
					pParent->m_pRightChild = pNode;

				pNode->m_nNodeColor += m_nNodeColor == eBlack;
				pNode->m_bRootNode = m_bRootNode;
				pNode->m_pParent = pParent;

				m_pParent = NULL;
				m_pLeftChild = NULL;
				m_pRightChild = NULL;

				while( !pNode->m_bRootNode && pNode->m_nNodeColor == eDoubleBlack )
				{
					pParent = pNode->GetParent();

					// 若 “x”是“它父节点的左孩子”，则设置 “w”为“x的叔叔”(即x为它父节点的右孩子) 
					if( pNode == pParent->m_pLeftChild )
					{
						CRBTreeNode* pBrother = pParent->m_pRightChild;

						// Case 1: x是“黑+黑”节点，x的兄弟节点是红色。(此时x的父节点和x的兄弟节点的子节点都是黑节点)。
						//  原理：交换兄弟和父节点颜色后左旋，
						//		是在保持红黑树的每条分支的黑节点数量不变的情况下将兄弟节点变为黑色
						if( pBrother->m_nNodeColor == eRed )
						{
							// 不会改变红黑树每条分支的黑节点数量
							pBrother->m_nNodeColor = eBlack;
							pParent->m_nNodeColor = eRed;
							pParent->LeftRotate();
							continue;
						}

						// 侄子的颜色
						CRBTreeNode* pLeftNephew = pBrother->m_pLeftChild;
						CRBTreeNode* pRightNephew = pBrother->m_pRightChild;
						int8 nLeftNephewColor = pLeftNephew ? pLeftNephew->m_nNodeColor : eBlack;
						int8 nRightNephewColor = pRightNephew ? pRightNephew->m_nNodeColor : eBlack; 
						// Case 2: x是“黑+黑”节点，x的兄弟节点是黑色，x的兄弟节点的两个孩子都是黑色。
						//	原理：共同抽取兄弟节点与当前节点的一个黑到父节点去
						if( nLeftNephewColor == eBlack && nRightNephewColor == eBlack )
						{
							pBrother->m_nNodeColor--;
							pNode->m_nNodeColor--;
							pParent->m_nNodeColor++;
							pNode = pParent;
							continue;
						}

						// Case 3: x是“黑+黑”节点，x的兄弟节点是黑色；x的兄弟节点的左孩子是红色，右孩子是黑色的。
						//  原理：交换兄弟节点和兄弟节点的左孩子的颜色，然后对x的兄弟节点进行右旋，
						//		目标是让兄弟节点的右孩子变成红色节点进入第四步
						if( nLeftNephewColor == eRed && nRightNephewColor == eBlack )
						{
							pBrother->m_nNodeColor = eRed;
							pLeftNephew->m_nNodeColor = eBlack;
							pBrother->RightRotate();
							continue;
						}

						// Case 4: x是“黑+黑”节点，x的兄弟节点是黑色；x的兄弟节点的右孩子是红色的。
						//  原理：因为右孩子为红色，所以父节点进行左旋后为了保持红黑树的每条分支的黑节点数量不变，
						//   需要进行以下颜色改变，则红黑树正好平衡，将黑+黑拆开了
						//   (01) 将x父节点颜色 赋值给 x的兄弟节点。
						//   (02) 将x兄弟节点的右子节设为“黑色”。
						//   (03) 将x父节点设为“黑色”。
						//   (04) 将当前节点设为“黑色”。
						pParent->LeftRotate();
						pBrother->m_nNodeColor = pParent->m_nNodeColor;
						pRightNephew->m_nNodeColor = eBlack;
						pParent->m_nNodeColor = eBlack;
						pNode->m_nNodeColor = eBlack;
					}
					else
					{
						CRBTreeNode* pBrother = pParent->m_pLeftChild;

						// Case 1: x是“黑+黑”节点，x的兄弟节点是红色。(此时x的父节点和x的兄弟节点的子节点都是黑节点)。
						//  原理：交换兄弟和父节点颜色后右旋，
						//		是在保持红黑树的每条分支的黑节点数量不变的情况下将兄弟节点变为黑色
						if( pBrother->m_nNodeColor == eRed )
						{
							// 不会改变红黑树每条分支的黑节点数量
							pBrother->m_nNodeColor = eBlack;
							pParent->m_nNodeColor = eRed;
							pParent->RightRotate();
							continue;
						}

						// 侄子的颜色
						CRBTreeNode* pLeftNephew = pBrother->m_pLeftChild;
						CRBTreeNode* pRightNephew = pBrother->m_pRightChild;
						int8 nLeftNephewColor = pLeftNephew ? pLeftNephew->m_nNodeColor : eBlack;
						int8 nRightNephewColor = pRightNephew ? pRightNephew->m_nNodeColor : eBlack; 
						// Case 2: x是“黑+黑”节点，x的兄弟节点是黑色，x的兄弟节点的两个孩子都是黑色。
						//	原理：共同抽取兄弟节点与当前节点的一个黑到父节点去
						if( nLeftNephewColor == eBlack && nRightNephewColor == eBlack )
						{
							pBrother->m_nNodeColor--;
							pNode->m_nNodeColor--;
							pParent->m_nNodeColor++;
							pNode = pParent;
							continue;
						}

						// Case 3: x是“黑+黑”节点，x的兄弟节点是黑色；x的兄弟节点的右孩子是红色，左孩子是黑色的。
						//  原理：交换兄弟节点和兄弟节点的右孩子的颜色，然后对x的兄弟节点进行左旋，
						//		目标是让兄弟节点的右孩子变成红色节点进入第四步
						if( nRightNephewColor == eRed && nLeftNephewColor == eBlack )
						{
							pBrother->m_nNodeColor = eRed;
							pRightNephew->m_nNodeColor = eBlack;
							pBrother->LeftRotate();
							continue;
						}

						// Case 4: x是“黑+黑”节点，x的兄弟节点是黑色；x的兄弟节点的左孩子是红色的。
						//  原理：因为左孩子为红色，所以父节点进行右旋后为了保持红黑树的每条分支的黑节点数量不变，
						//   需要进行以下颜色改变，则红黑树正好平衡，将黑+黑拆开了
						//   (01) 将x父节点颜色 赋值给 x的兄弟节点。
						//   (02) 将x兄弟节点的左子节设为“黑色”。
						//   (03) 将x父节点设为“黑色”。
						//   (04) 将当前节点设为“黑色”。
						pParent->RightRotate();
						pBrother->m_nNodeColor = pParent->m_nNodeColor;
						pLeftNephew->m_nNodeColor = eBlack;
						pParent->m_nNodeColor = eBlack;
						pNode->m_nNodeColor = eBlack;
					}
				}

				if( !pNode->m_bRootNode )
					return;
				pNode->m_nNodeColor = eBlack;
			}

			CNode* GetParent() const
			{
				return m_bRootNode ? NULL : static_cast<CNode*>( m_pParent );
			}

			CNode* GetLeftChild() const
			{
				return static_cast<CNode*>( m_pLeftChild );
			}

			CNode* GetRightChild() const
			{
				return static_cast<CNode*>( m_pRightChild );
			}

			CNode* GetPre() const
			{
				if( !m_pLeftChild )
				{
					const CRBTreeNode* pNext = this;
					CRBTreeNode* pParent = pNext->GetParent();
					while( pParent && pNext == pParent->m_pLeftChild )
					{
						pNext = pParent;
						pParent = pNext->GetParent();
					}
					return static_cast<CNode*>( pParent );
				}

				CRBTreeNode* pNext = m_pLeftChild;
				while( pNext->m_pRightChild )
					pNext = pNext->m_pRightChild;
				return static_cast<CNode*>( pNext );
			}
			
			CNode* GetNext() const
			{
				if( !m_pRightChild )
				{
					const CRBTreeNode* pPre = this;
					CRBTreeNode* pParent = pPre->GetParent();
					while( pParent && pPre == pParent->m_pRightChild )
					{
						pPre = pParent;
						pParent = pPre->GetParent();
					}
					return static_cast<CNode*>( pParent );
				}

				CRBTreeNode* pPre = m_pRightChild;
				while( pPre->m_pLeftChild )
					pPre = pPre->m_pLeftChild;
				return static_cast<CNode*>( pPre );
			}
		};

	public:
		class iterator
		{
			CRBTreeNode* m_pNode;
		public:
			iterator() : m_pNode( NULL ){}
			iterator( CRBTreeNode* pNode ) : m_pNode( pNode ){}
			iterator( const iterator& rhs ) : m_pNode( rhs.m_pNode ){}
			iterator operator= ( CRBTreeNode* pNode ) { m_pNode = pNode; return *this; }
			iterator operator= ( const iterator& rhs ) { m_pNode = rhs.m_pNode; return *this; }
			bool operator == ( CRBTreeNode* pNode ) const { return m_pNode == pNode; }
			bool operator == ( const iterator& rhs ) const { return m_pNode == rhs.m_pNode; }
			bool operator != ( CRBTreeNode* pNode ) const { return m_pNode != pNode; }
			bool operator != ( const iterator& rhs ) const { return m_pNode != rhs.m_pNode; }
			iterator& operator++() { m_pNode = m_pNode ? m_pNode->GetNext() : NULL; return *this; }
			iterator operator++( int ) { iterator i = *this; ++*this; return i; }
			iterator& operator--() { m_pNode = m_pNode ? m_pNode->GetPre() : NULL; return *this; }
			iterator operator--( int ) { iterator i = *this; ++*this; return i; }
			CNode& operator* () const { return *static_cast<CNode*>( m_pNode ); }
		};

		TRBTree()
			: m_pRootNode( NULL )
		{
		}

		~TRBTree()
		{
			assert( IsEmpty() );
		}

		bool IsEmpty() const
		{
			return !m_pRootNode;
		}

		uint32 Size() const
		{
			uint32 nSize = 0;			
			for( CRBTreeNode* pNode = GetFirst(); pNode; pNode = pNode->GetNext() )
				nSize++;				
			return nSize;
		}

		CNode& Insert( CNode& Node )
		{
			assert( !static_cast<CRBTreeNode&>( Node ).IsInTree() );
			Node.m_pLeftChild = NULL;
			Node.m_pRightChild = NULL;

			// 插入根节点
			if( m_pRootNode == NULL )
			{
				m_pRootNode = &Node;
				Node.m_pTree = this;
				Node.m_nNodeColor = eBlack;
				Node.m_bRootNode = true;
				return Node;
			}

			Node.m_nNodeColor = eRed;
			Node.m_bRootNode = false;
			CRBTreeNode* pNode = m_pRootNode;

			// 插入二叉查找树
			while( true )
			{
				if( Node < *static_cast<CNode*>( pNode ) )
				{
					if( !pNode->m_pLeftChild )
					{
						pNode->m_pLeftChild = &Node;
						static_cast<CRBTreeNode&>( Node ).m_pParent = pNode;
						break;
					}
					pNode = pNode->m_pLeftChild;
				}
				else
				{
					if( !pNode->m_pRightChild )
					{
						pNode->m_pRightChild = &Node;
						static_cast<CRBTreeNode&>( Node ).m_pParent = pNode;
						break;
					}
					pNode = pNode->m_pRightChild;
				}
			}

			pNode = &Node;
			CRBTreeNode* pParent = static_cast<CRBTreeNode&>( Node ).m_pParent;
			// 如果父节点和当前节点都为红色，则纠正
			while( pNode && pNode->m_nNodeColor == eRed && pParent->m_nNodeColor == eRed )
			{
				CRBTreeNode* pGrandpa = pParent->m_pParent;
				CRBTreeNode* pUncle = pGrandpa->m_pLeftChild == pParent ? 
					pGrandpa->m_pRightChild : pGrandpa->m_pLeftChild;

				// Case 1	当前节点的父节点是红色，且当前节点的祖父节点的另一个子节点（叔叔节点）也是红色。	
				// 	(01) 将“父节点”设为黑色。
				// 	(02) 将“叔叔节点”设为黑色。
				//  (03) 如果“祖父节点”是根节点，这时调整完成，所有分支上的黑色节点增加1。
				// 	(04) 如果“祖父节点”不是根节点，将“祖父节点”设为“红色”。
				// 	(05) 将“祖父节点”设为“当前节点”(红色节点)；即，之后继续对“当前节点”进行操作。
				if( pUncle && pUncle->m_nNodeColor == eRed )
				{
					pParent->m_nNodeColor = eBlack;
					pUncle->m_nNodeColor = eBlack;
					if( pGrandpa->m_bRootNode )
						break;
					pGrandpa->m_nNodeColor = eRed;
					pNode = pGrandpa;
					pParent = pNode->m_pParent;
					continue;
				}

				if( pParent == pGrandpa->m_pLeftChild )
				{
					// Case 2	当前节点的父节点是红色，叔叔节点是黑色，且当前节点是其父节点的右孩子	
					// 	(01) 将“父节点”作为“新的当前节点”。
					// 	(02) 以“新的当前节点”为支点进行左旋。
					if( pNode == pParent->m_pRightChild )
					{
						pParent->LeftRotate();
						pNode = pParent;
						pParent = pParent->m_pParent;
						continue;
					}

					// Case 3	当前节点的父节点是红色，叔叔节点是黑色，且当前节点是其父节点的左孩子	
					// 	(01) 将“父节点”设为“黑色”。
					// 	(02) 将“祖父节点”设为“红色”。
					// 	(03) 以“祖父节点”为支点进行右旋。
					pParent->m_nNodeColor = eBlack;
					pGrandpa->m_nNodeColor = eRed;
					pGrandpa->RightRotate();
					pNode = pGrandpa->m_pRightChild;
					pParent = pGrandpa;
				}
				else
				{
					// Case 2	当前节点的父节点是红色，叔叔节点是黑色，且当前节点是其父节点的右孩子	
					// 	(01) 将“父节点”作为“新的当前节点”。
					// 	(02) 以“新的当前节点”为支点进行左旋。
					if( pNode == pParent->m_pLeftChild )
					{
						pParent->RightRotate();
						pNode = pParent;
						pParent = pParent->m_pParent;
						continue;
					}

					// Case 3	当前节点的父节点是红色，叔叔节点是黑色，且当前节点是其父节点的左孩子	
					// 	(01) 将“父节点”设为“黑色”。
					// 	(02) 将“祖父节点”设为“红色”。
					// 	(03) 以“祖父节点”为支点进行右旋。
					pParent->m_nNodeColor = eBlack;
					pGrandpa->m_nNodeColor = eRed;
					pGrandpa->LeftRotate();
					pNode = pGrandpa->m_pLeftChild;
					pParent = pGrandpa;
				}
			}

			if( m_pRootNode->m_nNodeColor == eBlack )
				return Node;
			m_pRootNode->m_nNodeColor = eBlack;
			return Node;
		}

		CNode* GetFirst() const
		{
			if( !m_pRootNode )
				return NULL;
			CRBTreeNode* pNode = m_pRootNode;
			while( pNode->m_pLeftChild )
				pNode = pNode->m_pLeftChild;
			return static_cast<CNode*>( pNode );
		}

		CNode* GetLast() const
		{
			if( !m_pRootNode )
				return NULL;
			CRBTreeNode* pNode = m_pRootNode;
			while( pNode->m_pRightChild )
				pNode = pNode->m_pRightChild;
			return static_cast<CNode*>( pNode );
		}

		CNode* GetRoot() const
		{
			return static_cast<CNode*>( m_pRootNode );
		}

		template<class KeyType>
		CNode* LowerBound( const KeyType& key ) const
		{
			if( !m_pRootNode )
				return NULL;

			CRBTreeNode* pNode = m_pRootNode;
			while( true )
			{
				if( key < (KeyType)*static_cast<CNode*>( pNode ) )
				{
					if( !pNode->m_pLeftChild )
						return static_cast<CNode*>( pNode );
					pNode = pNode->m_pLeftChild;
				}
				else if( *static_cast<CNode*>( pNode ) < key )
				{
					if( !pNode->m_pRightChild )
						return static_cast<CNode*>( pNode->GetNext() );
					pNode = pNode->m_pRightChild;
				}
				else
				{
					while( pNode->GetPre() && !( *static_cast<CNode*>( pNode->GetPre() ) < key ) )
						pNode = static_cast<CNode*>( pNode->GetPre() );
					return static_cast<CNode*>( pNode );
				}
			}
		}

		template<class KeyType>
		CNode* UpperBound( const KeyType& key ) const
		{
			if( !m_pRootNode )
				return NULL;

			CNode* pNode = static_cast<CNode*>( m_pRootNode );
			while( true )
			{
				if( key < (KeyType)*static_cast<CNode*>( pNode ) )
				{
					if( !pNode->m_pLeftChild )
						return static_cast<CNode*>( pNode );
					pNode = static_cast<CNode*>( pNode->m_pLeftChild );
				}
				else
				{
					if( !pNode->m_pRightChild )
						return static_cast<CNode*>( pNode->GetNext() );
					pNode = static_cast<CNode*>( pNode->m_pRightChild );
				}
			}

			return pNode;
		}

		template<class KeyType>
		CNode* Find( const KeyType& key ) const
		{
			CNode* pNode = LowerBound( key );
			return !pNode || key < (KeyType)*pNode ? NULL : pNode;
		}

		bool empty() const
		{
			return IsEmpty();
		}

		iterator begin()
		{
			return iterator( GetFirst() );
		}

		iterator end()
		{
			return iterator();
		}

		iterator rbegin()
		{
			return iterator( GetLast() );
		}

		iterator rend()
		{
			return iterator();
		}

		template<class KeyType>
		iterator lower_bound( const KeyType& key )
		{
			return iterator( LowerBound( key ) );
		}

		template<class KeyType>
		iterator upper_bound( const KeyType& key )
		{
			return iterator( UpperBound( key ) );
		}

		template<class KeyType>
		iterator find( const KeyType& key )
		{
			return iterator( Find( key ) );
		}

		uint32 size() const
		{
			return Size();
		}

	private:
		CRBTreeNode*	m_pRootNode;
	};

	template<typename DataType>
	class TGammaRBSetNode : 
		public TRBTree< TGammaRBSetNode<DataType> >::CRBTreeNode
	{
	protected:
		DataType	m_Data;
	public:
		TGammaRBSetNode( const DataType& Data ) : m_Data( Data ){}
		operator const DataType&() { return m_Data; }
		bool operator< ( const DataType& r ) { return m_Data < r; }
		const DataType& Get() const { return m_Data; }
	};

	template<typename KeyType, typename DataType>
	class TGammaRBMapNode : 
		public TRBTree< TGammaRBMapNode<KeyType, DataType> >::CRBTreeNode
	{
	protected:
		KeyType		m_Key;
		DataType	m_Data;
	public:
		TGammaRBMapNode( const KeyType& Key, const DataType& Data ) : m_Key( Key ), m_Data( Data ){}
		operator const KeyType&() { return m_Key; }
		bool operator< ( const KeyType& r ) { return m_Key < r; }
		const KeyType& GetKey() const { return m_Key; }
		const DataType& GetData() const { return m_Data; }
		void SetData( const DataType& Data ) { m_Data = Data; }
	};
}

#endif

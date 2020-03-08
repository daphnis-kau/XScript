/**@file  		TRBTree.h 
* @brief		Red-black tree
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
* @note			Chapter 13 of Introduction to Algorithms: Red-Black Trees
*				No memory need to be allocated/freed when insert a node to \n
*				or remove node from a TRBTree. Nodes can be removed directly \n
*				without knowing the parent TRBTree. An object can exist in \n
*				multiple trees by inheritance from multiple subclasses of \n
*				CRBTreeNode.
*/
#ifndef __XS_RBTREE_H__
#define __XS_RBTREE_H__

#include "common/Help.h"

namespace XS
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

				// Sentinel node, temporary variable
				struct CSentinelNode : public CRBTreeNode
				{
					~CSentinelNode()
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

				// Sentinel node, auto be remove from tree when destructed.
				CSentinelNode SentinelNode;

				// If the node has two children, so we exchange the node 
				// with the follow node until one child of the node is null
				while( m_pLeftChild && m_pRightChild )
				{
					CRBTreeNode* pNext = GetNext();

					//Exchange node's but keep property in previous place

					// Replace the node by sentinel node
					ReplaceBy( &SentinelNode );

					// Replace next node by current node
					pNext->ReplaceBy( this );

					// Sentinel node by next node
					SentinelNode.ReplaceBy( pNext );
				}

				CRBTreeNode* pParent = m_pParent;
				CRBTreeNode* pNode = m_pLeftChild ? m_pLeftChild : m_pRightChild;
				if( !pNode )
					pNode = &SentinelNode;

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

				// Loop while the node color is double black
				while( !pNode->m_bRootNode && pNode->m_nNodeColor == eDoubleBlack )
				{
					pParent = pNode->GetParent();

					// If the node is left child, so rotate it's brother as it's uncle.
					if( pNode == pParent->m_pLeftChild )
					{
						CRBTreeNode* pBrother = pParent->m_pRightChild;

						// Case 1: The node is double black and it's brother is red.
						//			Color of the node's parent and brother are black:
						// Theory: Rotate around the parent after the parent exchange 
						//			color with the node's brother.
						if( pBrother->m_nNodeColor == eRed )
						{
							// Number of black nodes of all paths in right branch isn't changed.
							pBrother->m_nNodeColor = eBlack;
							pParent->m_nNodeColor = eRed;
							pParent->LeftRotate();
							continue;
						}

						// Color of nephews
						CRBTreeNode* pLeftNephew = pBrother->m_pLeftChild;
						CRBTreeNode* pRightNephew = pBrother->m_pRightChild;
						int8 nLeftNephewColor = pLeftNephew ? pLeftNephew->m_nNodeColor : eBlack;
						int8 nRightNephewColor = pRightNephew ? pRightNephew->m_nNodeColor : eBlack; 
						// Case 2: The node is double black and it's brother is black.
						//			Color of two nephew's are black:
						// Theory: Mark the node and brother node with red and mark parent with double black.
						if( nLeftNephewColor == eBlack && nRightNephewColor == eBlack )
						{
							pBrother->m_nNodeColor--;
							pNode->m_nNodeColor--;
							pParent->m_nNodeColor++;
							pNode = pParent;
							continue;
						}

						// Case 3: The node is double black and it's brother is black.
						//			Left child of brother is red and other is black:
						// Theory: Brother exchange color with brother's left child and 
						//			do right rotation around itself.
						if( nLeftNephewColor == eRed && nRightNephewColor == eBlack )
						{
							pBrother->m_nNodeColor = eRed;
							pLeftNephew->m_nNodeColor = eBlack;
							pBrother->RightRotate();
							continue;
						}

						// Case 4: The node is double black and it's brother is black.
						//			Right child of brother is red:
						//   (01) Mark color of brother with parent's color 
						//   (02) Mark right child of brother with black
						//   (03) Mark parent with black
						//   (04) Mark current node with black
						pParent->LeftRotate();
						pBrother->m_nNodeColor = pParent->m_nNodeColor;
						pRightNephew->m_nNodeColor = eBlack;
						pParent->m_nNodeColor = eBlack;
						pNode->m_nNodeColor = eBlack;
					}
					else
					{
						CRBTreeNode* pBrother = pParent->m_pLeftChild;

						// Case 1: Current node is double black, the brother is red.
						if( pBrother->m_nNodeColor == eRed )
						{
							pBrother->m_nNodeColor = eBlack;
							pParent->m_nNodeColor = eRed;
							pParent->RightRotate();
							continue;
						}

						// Color of nephews
						CRBTreeNode* pLeftNephew = pBrother->m_pLeftChild;
						CRBTreeNode* pRightNephew = pBrother->m_pRightChild;
						int8 nLeftNephewColor = pLeftNephew ? pLeftNephew->m_nNodeColor : eBlack;
						int8 nRightNephewColor = pRightNephew ? pRightNephew->m_nNodeColor : eBlack; 
						// Case 2: Current node is double black, the brother is black, 
						//			Color of brother's children are black
						if( nLeftNephewColor == eBlack && nRightNephewColor == eBlack )
						{
							pBrother->m_nNodeColor--;
							pNode->m_nNodeColor--;
							pParent->m_nNodeColor++;
							pNode = pParent;
							continue;
						}

						// Case 3: Current node is double black, the brother is black, 
						//			left child of brother is black and right child is red
						if( nRightNephewColor == eRed && nLeftNephewColor == eBlack )
						{
							pBrother->m_nNodeColor = eRed;
							pRightNephew->m_nNodeColor = eBlack;
							pBrother->LeftRotate();
							continue;
						}

						// Case 4: Current node is double black, the brother is black, 
						//			left child of brother is red
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

			// Insert root node
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

			// Insert to tree
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
			// When parent node and child node are red, we then fix them up by rotating nodes.
			while( pNode && pNode->m_nNodeColor == eRed && pParent->m_nNodeColor == eRed )
			{
				CRBTreeNode* pGrandpa = pParent->m_pParent;
				CRBTreeNode* pUncle = pGrandpa->m_pLeftChild == pParent ? 
					pGrandpa->m_pRightChild : pGrandpa->m_pLeftChild;

				// Case 1	Parent node and uncle node are red	
				// 	(01) Mark parent with black
				// 	(02) Mark uncle with black
				//  (03) If grandparent is root, fix-up completed
				// 	(04) Otherwise, mark grandparent with red
				// 	(05) Set grandparent as current node and continue next loop
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

				// Parent is left child of grandparent
				if( pParent == pGrandpa->m_pLeftChild )
				{
					// Case 2	Parent is red, uncle is black, current node is right child	
					// 	(01) Set parent as new current node
					// 	(02) Do left rotation around new current node and continue next loop
					if( pNode == pParent->m_pRightChild )
					{
						pParent->LeftRotate();
						pNode = pParent;
						pParent = pParent->m_pParent;
						continue;
					}

					// Case 3	Parent is red, uncle is black, current node is left child	
					// 	(01) Mark parent with black
					// 	(02) Mark grandparent with red
					// 	(03) Do right rotation around grandparent
					pParent->m_nNodeColor = eBlack;
					pGrandpa->m_nNodeColor = eRed;
					pGrandpa->RightRotate();
					pNode = pGrandpa->m_pRightChild;
					pParent = pGrandpa;
				}
				// Parent is left right of grandparent
				else
				{
					// Case 2	Parent is red, uncle is black, current node is right child	
					// 	(01) Set parent as new current node
					// 	(02) Do left rotation around new current node and continue next loop
					if( pNode == pParent->m_pLeftChild )
					{
						pParent->RightRotate();
						pNode = pParent;
						pParent = pParent->m_pParent;
						continue;
					}

					// Case 3	Parent is red, uncle is black, current node is left child	
					// 	(01) Mark parent with black
					// 	(02) Mark grandparent with red
					// 	(03) Do right rotation around grandparent
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
	class TRBSetNode : 
		public TRBTree< TRBSetNode<DataType> >::CRBTreeNode
	{
	protected:
		DataType	m_Data;
	public:
		TRBSetNode( const DataType& Data ) : m_Data( Data ){}
		operator const DataType&() { return m_Data; }
		bool operator< ( const DataType& r ) { return m_Data < r; }
		const DataType& Get() const { return m_Data; }
	};

	template<typename KeyType, typename DataType>
	class TRBMapNode : 
		public TRBTree< TRBMapNode<KeyType, DataType> >::CRBTreeNode
	{
	protected:
		KeyType		m_Key;
		DataType	m_Data;
	public:
		TRBMapNode( const KeyType& Key, const DataType& Data ) : m_Key( Key ), m_Data( Data ){}
		operator const KeyType&() { return m_Key; }
		bool operator< ( const KeyType& r ) { return m_Key < r; }
		DataType& GetData() { return m_Data; }
		const KeyType& GetKey() const { return m_Key; }
		const DataType& GetData() const { return m_Data; }
		void SetData( const DataType& Data ) { m_Data = Data; }
	};
}

#endif

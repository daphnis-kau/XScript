//=========================================================================================
// TList.h 
// 定义一个双向List类
// 优点：插入和删除节点不需要进行内存的分配和释放，节点操作速度极快
//		 节点可以自删除，不需要知道所处的链
// 缺点：数据需要从指定的节点类继承
// 柯达昭
// 2008-02-27
//=========================================================================================
#ifndef __XS_LIST_H__
#define __XS_LIST_H__

#include "common/Help.h"

namespace XS
{

	template<typename ImpClass>
	class TList
	{
		const TList& operator= ( const TList& );
		TList( const TList& );
	public:
		typedef ImpClass CNode;
		class CListNode
		{
			CListNode*	m_pPreNode;
			CListNode*	m_pNextNode;
			friend class TList<CNode>;

		public:
			CListNode() : m_pPreNode(NULL), m_pNextNode(NULL)
			{
			}

			~CListNode()
			{ 
				Remove();
			}

			bool IsInList() const
			{ 
				return m_pPreNode != NULL; 
			}
			//只是从链表中脱离  delete需要自己调用
			void Remove()
			{
				if( !IsInList() )
					return;
				m_pPreNode->m_pNextNode = m_pNextNode;
				m_pNextNode->m_pPreNode = m_pPreNode;
				m_pPreNode = NULL;
				m_pNextNode = NULL;
			}

			CNode* GetPre() const
			{
				return m_pPreNode && m_pPreNode->m_pPreNode ? static_cast<CNode*>( m_pPreNode ) : NULL;
			}

			CNode* GetNext() const
			{
				return m_pNextNode && m_pNextNode->m_pNextNode ? static_cast<CNode*>( m_pNextNode ) : NULL;
			}

			void InsertBefore( CListNode& Node )
			{
				assert( !Node.IsInList() );
				assert( &Node != this );

				Node.m_pPreNode  = m_pPreNode;
				Node.m_pNextNode = this;
				m_pPreNode->m_pNextNode = &Node;
				m_pPreNode = &Node;
			}

			void InsertAfter( CListNode& Node )
			{
				assert( !Node.IsInList() );
				assert( &Node != this );

				Node.m_pNextNode  = m_pNextNode;
				Node.m_pPreNode = this;
				m_pNextNode->m_pPreNode = &Node;
				m_pNextNode = &Node;
			}
		};

	public:
		class iterator
		{
			CNode* m_pNode;
		public:
			iterator() : m_pNode( NULL ){}
			iterator( CNode* pNode ) : m_pNode( pNode ){}
			iterator( const iterator& rhs ) : m_pNode( rhs.m_pNode ){}
			iterator operator= ( CNode* pNode ) { m_pNode = pNode; return *this; }
			iterator operator= ( const iterator& rhs ) { m_pNode = rhs.m_pNode; return *this; }
			bool operator == ( CNode* pNode ) const { return m_pNode == pNode; }
			bool operator == ( const iterator& rhs ) const { return m_pNode == rhs.m_pNode; }
			bool operator != ( CNode* pNode ) const { return m_pNode != pNode; }
			bool operator != ( const iterator& rhs ) const { return m_pNode != rhs.m_pNode; }
			iterator& operator++() { m_pNode = m_pNode ? m_pNode->CListNode::GetNext() : NULL; return *this; }
			iterator operator++( int ) { iterator i = *this; ++*this; return i; }
			iterator& operator--() { m_pNode = m_pNode ? m_pNode->CListNode::GetPre() : NULL; return *this; }
			iterator operator--( int ) { iterator i = *this; ++*this; return i; }
			CNode& operator* () const { return *m_pNode; }
		};

		TList()
		{
			m_NodeHead.m_pNextNode = &m_NodeTail;
			m_NodeTail.m_pPreNode = &m_NodeHead;
		}

		~TList()
		{
			assert( IsEmpty() );
			m_NodeHead.m_pNextNode = NULL;
			m_NodeTail.m_pPreNode = NULL;
		}

		bool IsEmpty() const
		{
			return m_NodeHead.m_pNextNode == &m_NodeTail;
		}

		void PushFront( CListNode& Node )
		{
			InsertAfter( Node, NULL );
		}

		void PushBack( CListNode& Node )
		{
			InsertBefore( Node, NULL );
		}

		void InsertBefore( CListNode& Node, CListNode* pNodePos )
		{
			assert( !Node.IsInList() );
			assert( &Node != pNodePos );

			pNodePos = pNodePos ? pNodePos : &m_NodeTail;
			Node.m_pPreNode  = pNodePos->m_pPreNode;
			Node.m_pNextNode = pNodePos;
			pNodePos->m_pPreNode->m_pNextNode = &Node;
			pNodePos->m_pPreNode = &Node;
		}

		void InsertAfter( CListNode& Node, CListNode* pNodePos )
		{
			assert( !Node.IsInList() );
			assert( &Node != pNodePos );

			pNodePos = pNodePos ? pNodePos : &m_NodeHead;
			Node.m_pNextNode  = pNodePos->m_pNextNode;
			Node.m_pPreNode = pNodePos;
			pNodePos->m_pNextNode->m_pPreNode = &Node;
			pNodePos->m_pNextNode = &Node;
		}

		CNode* GetFirst() const
		{
			return IsEmpty() ? NULL : static_cast<CNode*>( m_NodeHead.m_pNextNode );
		}

		CNode* GetLast() const
		{
			return IsEmpty() ? NULL : static_cast<CNode*>( m_NodeTail.m_pPreNode );
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
	private:
		CListNode	m_NodeHead;
		CListNode	m_NodeTail;
	};

	template<typename DataType>
	class TListNode : 
		public TList< TListNode<DataType> >::CListNode
	{
		DataType m_Data;
	public:
		TListNode() {}
		TListNode( const DataType& v ) : m_Data( v ) {}
		const DataType& Get() { return m_Data; }
		void Set( const DataType& v ) { m_Data = v; }
	};
}

#endif

//=========================================================================================
// TTinyList.h 
// 定义一个紧凑的List类
// 优点：整个类实例体积很小，只有4个字节
//		 插入和删除节点不需要进行内存的分配和释放，节点操作速度极快
//		 节点可以自删除，不需要知道所处的链
// 缺点：数据需要从指定的节点类继承
//		 插入只能在链头插入
//		 遍历自能从链头开始
// 功能推荐：
//		 这个类特别适合用于基于格子或者区域的对象链表的管理，通常对象链表特性如下：
//		 @不关心对象在链里面的位置
//		 @因为格子很多，所以链的实例体积太大会占用大量内存
//		 @对象移动频繁，需要经常插入和删除
// 柯达昭
// 2008-01-27
//=========================================================================================
#ifndef __TTINY_LIST_H__
#define __TTINY_LIST_H__

#include "common/Help.h"


namespace XS
{
	template<typename ImpClass>
	class TTinyList
	{
		TTinyList( const TTinyList& );
		const TTinyList& operator = ( const TTinyList& );
	public:		
		typedef ImpClass CNode;
		class CTinyListNode
		{
			CNode**	m_ppPreNode;
			CNode*	m_pNextNode;
			friend class TTinyList<CNode>;

		public:
			CTinyListNode() : m_ppPreNode(NULL), m_pNextNode(NULL)
			{
			}

			~CTinyListNode()
			{ 
				Remove();
			}

			bool IsInList()
			{ 
				return m_ppPreNode != NULL; 
			}

			void Remove()
			{
				if( m_ppPreNode )
					*m_ppPreNode = m_pNextNode;
				if( m_pNextNode )
					static_cast<CTinyListNode*>( m_pNextNode )->m_ppPreNode = m_ppPreNode;
				m_ppPreNode = NULL;
				m_pNextNode = NULL;
			}

			CNode* GetNext() const
			{
				return m_pNextNode;
			}
		};

		void _InsertAfter( CTinyListNode& Node, CTinyListNode* pLearder )
		{
			assert( !Node.IsInList() );
			assert( &Node != pLearder );

			CNode*& pNode = pLearder ? pLearder->m_pNextNode : m_pNodeHead;
			Node.m_ppPreNode = &pNode;
			Node.m_pNextNode = pNode;
			if( pNode )
				static_cast<CTinyListNode*>( pNode )->m_ppPreNode = &Node.m_pNextNode;
			pNode = static_cast<CNode*>( &Node );
		}

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
			iterator& operator++() { m_pNode = m_pNode ? m_pNode->CTinyListNode::GetNext() : NULL; return *this; }
			iterator operator++( int ) { iterator i = *this; ++*this; return i; }
			CNode& operator* () const { return *m_pNode; }
		};

		TTinyList() : m_pNodeHead(NULL)
		{
		}

		~TTinyList()
		{
			assert( !m_pNodeHead );
		}

		void InsertAfter( CNode& Node, CNode* pLearder )
		{
			_InsertAfter( Node, pLearder );
		}

		void PushFront( CNode& Node )
		{
			_InsertAfter( Node, NULL );
		}

		CNode* GetFirst() const
		{
			return m_pNodeHead;
		}
		
		CNode* GetPreNode( CTinyListNode* pNode ) const
		{
			if( pNode == m_pNodeHead )
				return NULL;
			void* pPreNode = pNode->m_ppPreNode - 1;
			return static_cast<CNode*>( (CTinyListNode*)pPreNode );
		}

		iterator begin()
		{
			return iterator( GetFirst() );
		}

		iterator end()
		{
			return iterator();
		}

	private:
		CNode*	m_pNodeHead;
	};
	
	template<typename DataType>
	class TTinyListNode : 
		public TTinyList< TTinyListNode<DataType> >::CTinyListNode
	{
		DataType m_Data;
	public:
		const DataType& Get() { return m_Data; }
		void Set( const DataType& v ) { m_Data = v; }
	};
}

#endif

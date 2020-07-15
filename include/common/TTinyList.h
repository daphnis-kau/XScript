/**@file  		TList.h
* @brief		Single link list
* @author		Daphnis Kau
* @date			2019-06-24
* @version		V1.0
* @note			The sizeof TTinyList is very small, it's size is equal to \n
*				the sizeof void*. No memory need to be allocated/freed \n
*				when insert a node to or remove node from a TTinyList. \n
*				Nodes can be removed directly without knowing the parent \n
*				TTinyList. An object can exist in multiple lists by \n
*				inheritance from multiple subclasses of CTinyListNode.
*/
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
			CTinyListNode() : m_ppPreNode(nullptr), m_pNextNode(nullptr)
			{
			}

			~CTinyListNode()
			{ 
				Remove();
			}

			bool IsInList()
			{ 
				return m_ppPreNode != nullptr; 
			}

			void Remove()
			{
				if( m_ppPreNode )
					*m_ppPreNode = m_pNextNode;
				if( m_pNextNode )
					static_cast<CTinyListNode*>( m_pNextNode )->m_ppPreNode = m_ppPreNode;
				m_ppPreNode = nullptr;
				m_pNextNode = nullptr;
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
			iterator() : m_pNode( nullptr ){}
			iterator( CNode* pNode ) : m_pNode( pNode ){}
			iterator( const iterator& rhs ) : m_pNode( rhs.m_pNode ){}
			iterator operator= ( CNode* pNode ) { m_pNode = pNode; return *this; }
			iterator operator= ( const iterator& rhs ) { m_pNode = rhs.m_pNode; return *this; }
			bool operator == ( CNode* pNode ) const { return m_pNode == pNode; }
			bool operator == ( const iterator& rhs ) const { return m_pNode == rhs.m_pNode; }
			bool operator != ( CNode* pNode ) const { return m_pNode != pNode; }
			bool operator != ( const iterator& rhs ) const { return m_pNode != rhs.m_pNode; }
			iterator& operator++() { m_pNode = m_pNode ? m_pNode->CTinyListNode::GetNext() : nullptr; return *this; }
			iterator operator++( int ) { iterator i = *this; ++*this; return i; }
			CNode& operator* () const { return *m_pNode; }
		};

		TTinyList() : m_pNodeHead(nullptr)
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
			_InsertAfter( Node, nullptr );
		}

		CNode* GetFirst() const
		{
			return m_pNodeHead;
		}
		
		CNode* GetPreNode( CTinyListNode* pNode ) const
		{
			if( pNode == m_pNodeHead )
				return nullptr;
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

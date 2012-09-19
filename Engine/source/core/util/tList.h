//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _TORQUE_LIST_
#define _TORQUE_LIST_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

namespace Torque
{

/// A list template class.
/// A classic list class similar to the STL list class. The list
/// class supports fast insert and erase operations, but slow dynamic
/// access. The list is circular and the iterator supports iterating
/// past the end() or rend() in order to loop around.
/// @ingroup UtilContainers
template<typename Type>
class List
{
   struct Link;
   struct PrivatePersist {};  ///< Used instead of a (void *) for type-safety of persistent iterators

public:
   /// A PersistentIter is used for the special cases where you need to store an iterator for
   /// efficiency reasons.  The _Iterator class handles the conversion to/from PersistentIter.
   /// @note The data in the node this points to could go away, so only use these in cases where
   ///  you control the allocation/deallocation of the nodes in the list.
   typedef  PrivatePersist *PersistentIter;

   // Iterator support
   template<typename U,typename E>
   class _Iterator
   {
   public:
      typedef U  ValueType;
      typedef U* Pointer;
      typedef U& Reference;

      _Iterator();
      _Iterator(PersistentIter &iter) { _link = (Link *)iter; }
 
      operator PersistentIter() const { return (PrivatePersist *)_link; }

      _Iterator& operator++();
      _Iterator  operator++(int);
      _Iterator& operator--();
      _Iterator  operator--(int);
      bool operator==(const _Iterator&) const;
      bool operator!=(const _Iterator&) const;
      U* operator->() const;
      U& operator*() const;

   private:
      friend class List;

      E* _link;
      _Iterator(E*);
   };

   // types
   typedef Type        ValueType;
   typedef Type&       Reference;
   typedef const Type& ConstReference;

   typedef _Iterator<Type,Link>              Iterator;
   typedef _Iterator<const Type,const Link>  ConstIterator;

   typedef S32         DifferenceType;
   typedef U32         SizeType;

   // initialization
   List();
   ~List();
   List(const List& p);

   // management
   U32  getSize() const;               ///< Return the number of elements
   void resize(U32);                   ///< Set the list size
   void clear();                       ///< Empty the List
   bool isEmpty() const;               ///< Node count == 0?

   // insert elements
   Iterator insert(U32 index, const Type& = Type());  ///< Insert new element at the given index
   Iterator insert(Iterator, const Type& = Type());   ///< Insert at the given iter
   Iterator pushFront(const Type& = Type());    ///< Insert at first element
   Iterator pushBack(const Type& = Type());     ///< Insert after the last element

   // erase elements
   void erase(U32);                    ///< Preserves element order
   void erase(Iterator);               ///< Preserves element order
   void popFront();                    ///< Remove the first element
   void popBack();                     ///< Remove the last element

   // forward Iterator access
   Iterator begin();                   ///< _Iterator to first element
   ConstIterator begin() const;        ///< _Iterator to first element
   Iterator end();                     ///< _Iterator to last element + 1
   ConstIterator end() const;          ///< _Iterator to last element + 1

   // reverse Iterator access
   Iterator rbegin();                  ///< _Iterator to first element - 1
   ConstIterator rbegin() const;       ///< _Iterator to first element - 1
   Iterator rend();                    ///< _Iterator to last element
   ConstIterator rend() const;         ///< _Iterator to last element

   // type access
   Type&       first();                ///< First element
   const Type& first() const;          ///< First element
   Type&       last();                 ///< Last element
   const Type& last() const;           ///< Last element

   // operators
   void operator=(const List& p);
   Type& operator[](U32);
   const Type& operator[](U32) const;

private:
   struct Link
   {
      Link* next;
      Link* prev;
      Link() {}
      Link(Link* p,Link* n): next(n),prev(p) {}
   };

   struct Node: Link
   {
      Type value;
      Node() {}
      Node(Link* p,Link* n,const Type& x): Link(p,n),value(x) {}
   };

   Link _head;
   U32 _size;

   Link* _node(U32 index) const;
   void _destroy();
};

template<class Type> List<Type>::List()
{
   _size = 0;
   _head.next = &_head;
   _head.prev = &_head;
}

template<class Type> List<Type>::List(const List& p)
{
   _size = 0;
   _head.next = &_head;
   _head.prev = &_head;
   *this = p;
}

template<class Type> List<Type>::~List()
{
   _destroy();
}


//-----------------------------------------------------------------------------

template<class Type> typename List<Type>::Link* List<Type>::_node(U32 index) const
{
   Iterator itr(_head.next);
   while (index--)
      itr++;
   return itr._link;
}

template<class Type> void List<Type>::_destroy()
{
   for (Iterator itr(begin()); itr != end(); )
   {
      Node* n = static_cast<Node*>((itr++)._link);
      delete n;
   }
}


//-----------------------------------------------------------------------------
// management

template<class Type> inline U32 List<Type>::getSize() const
{
   return _size;
}

template<class Type> void List<Type>::resize(U32 size)
{
   if (size > _size)
   {
      while (_size != size)
         pushBack(Type());
   }
   else
   {
      while (_size != size)
         popBack();
   }
}

template<class Type> void List<Type>::clear()
{
   _destroy();
   _head.next = &_head;
   _head.prev = &_head;
   _size = 0;
}

template<class Type> inline bool List<Type>::isEmpty() const
{
   return _size == 0;
}


//-----------------------------------------------------------------------------
// add Nodes

template<class Type> typename List<Type>::Iterator List<Type>::insert(Iterator itr, const Type& x)
{
   Link* node = itr._link;
   Node* n = new Node(node->prev,node,x);
   node->prev->next = n;
   node->prev = n;
   _size++;
   return n;
}

template<class Type> inline typename List<Type>::Iterator List<Type>::insert(U32 index, const Type& x)
{
   return insert(_node(index),x);
}

template<class Type> inline typename List<Type>::Iterator List<Type>::pushFront(const Type& x)
{
   return insert(_head.next,x);
}

template<class Type> inline typename List<Type>::Iterator List<Type>::pushBack(const Type& x)
{
   return insert(&_head,x);
}


//-----------------------------------------------------------------------------
// remove elements

template<class Type> void List<Type>::erase(Iterator itr)
{
   Node* node = static_cast<Node*>(itr._link);
   node->prev->next = node->next;
   node->next->prev = node->prev;
   delete node;
   _size--;
}

template<class Type> inline void List<Type>::erase(U32 index)
{
   erase(_node(index));
}

template<class Type> inline void List<Type>::popFront()
{
   erase(_head.next);
}

template<class Type> inline void List<Type>::popBack()
{
   erase(_head.prev);
}


//-----------------------------------------------------------------------------
// Iterator access

template<class Type> inline typename List<Type>::Iterator List<Type>::begin()
{
   return _head.next;
}

template<class Type> inline typename List<Type>::ConstIterator List<Type>::begin() const
{
   return _head.next;
}

template<class Type> inline typename List<Type>::Iterator List<Type>::end()
{
   return &_head;
}

template<class Type> inline typename List<Type>::ConstIterator List<Type>::end() const
{
   return &_head;
}

template<class Type> inline typename List<Type>::Iterator List<Type>::rbegin()
{
   return _head.prev;
}

template<class Type> inline typename List<Type>::ConstIterator List<Type>::rbegin() const
{
   return _head.prev;
}

template<class Type> inline typename List<Type>::Iterator List<Type>::rend()
{
   return &_head;
}

template<class Type> inline typename List<Type>::ConstIterator List<Type>::rend() const
{
   return &_head;
}


//-----------------------------------------------------------------------------
// type access

template<class Type> inline Type& List<Type>::first()
{
   return static_cast<Node*>(_head.next)->value;
}

template<class Type> inline const Type& List<Type>::first() const
{
   return static_cast<Node*>(_head.next)->value;
}

template<class Type> inline Type& List<Type>::last()
{
   return static_cast<Node*>(_head.prev)->value;
}

template<class Type> inline const Type& List<Type>::last() const
{
   return static_cast<Node*>(_head.prev)->value;
}


//-----------------------------------------------------------------------------
// operators

template<class Type> void List<Type>::operator=(const List& p)
{
   if (_size >= p._size)
   {
      // Destroy the elements we're not going to use and copy the rest
      while (_size != p._size)
         popBack();
   }

   U32 count = _size;
   ConstIterator src = p.begin();
   Iterator dst = begin();
   while (count--)
      *dst++ = *src++;
   while (src != p.end())
      pushBack(*src++);
}

template<class Type> inline Type& List<Type>::operator[](U32 index)
{
   return static_cast<Node*>(_node(index))->value;
}

template<class Type> inline const Type& List<Type>::operator[](U32 index) const
{
   return static_cast<Node*>(_node(index))->value;
}

//-----------------------------------------------------------------------------
// _Iterator class

template<class Type> template<class U,typename E>
inline List<Type>::_Iterator<U,E>::_Iterator()
{
   _link = 0;
}

template<class Type> template<class U,typename E>
inline List<Type>::_Iterator<U,E>::_Iterator(E* ptr)
{
   _link = ptr;
}

// The checking for _MSC_VER on the ++/-- operators is due to a bug with the VS2008 compiler
// recheck this and remove if fixed with VS2008 SP1

template<class Type> template<class U,typename E>
#ifdef _MSC_VER
inline typename List<Type>:: _Iterator<U,E>& List<Type>::_Iterator<U,E>::operator++()
#else
inline typename List<Type>::template _Iterator<U,E>& List<Type>::_Iterator<U,E>::operator++()
#endif
{
   _link = _link->next;
   return *this;
}

template<class Type> template<class U,typename E>
#ifdef _MSC_VER
inline typename List<Type>:: _Iterator<U,E> List<Type>::_Iterator<U,E>::operator++(int)
#else
inline typename List<Type>::template _Iterator<U,E> List<Type>::_Iterator<U,E>::operator++(int)
#endif
{
   _Iterator itr(*this);
   _link = _link->next;
   return itr;
}

template<class Type> template<class U,typename E>
#ifdef _MSC_VER
inline typename List<Type>:: _Iterator<U,E>& List<Type>::_Iterator<U,E>::operator--()
#else
inline typename List<Type>::template _Iterator<U,E>& List<Type>::_Iterator<U,E>::operator--()
#endif
{
   _link = _link->prev;
   return *this;
}

template<class Type> template<class U,typename E>
#ifdef _MSC_VER
inline typename List<Type>:: _Iterator<U,E> List<Type>::_Iterator<U,E>::operator--(int)
#else
inline typename List<Type>::template _Iterator<U,E> List<Type>::_Iterator<U,E>::operator--(int)
#endif
{
   _Iterator itr(*this);
   _link = _link->prev;
   return itr;
}

template<class Type> template<class U,typename E>
inline bool List<Type>::_Iterator<U,E>::operator==(const _Iterator& b) const
{
   return _link == b._link;
}

template<class Type> template<class U,typename E>
inline bool List<Type>::_Iterator<U,E>::operator!=(const _Iterator& b) const
{
   return !(_link == b._link);
}

template<class Type> template<class U,typename E>
inline U* List<Type>::_Iterator<U,E>::operator->() const
{
   // Can't use static_cast because of const / non-const versions of Iterator.
   // Could use reflection functions, but C-style cast is easier in this case.
   return &((Node*)_link)->value;
}

template<class Type> template<class U,typename E>
inline U& List<Type>::_Iterator<U,E>::operator*() const
{
   // Can't use static_cast because of const / non-const versions of Iterator.
   // Could use reflection functions, but C-style cast is easier in this case.
   return ((Node*)_link)->value;
}

}  // Namespace

#endif // _TORQUE_LIST_


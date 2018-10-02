// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2018 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

namespace Eigen {

template<typename XprType,typename Derived>
class IndexedBasedStlIteratorBase
{
public:
  typedef Index difference_type;
  typedef std::random_access_iterator_tag iterator_category;

  IndexedBasedStlIteratorBase() : mp_xpr(0), m_index(0) {}
  IndexedBasedStlIteratorBase(XprType& xpr, Index index) : mp_xpr(&xpr), m_index(index) {}

  void swap(IndexedBasedStlIteratorBase& other) {
    std::swap(mp_xpr,other.mp_xpr);
    std::swap(m_index,other.m_index);
  }

  Derived& operator++() { ++m_index; return derived(); }
  Derived& operator--() { --m_index; return derived(); }

  Derived operator++(int) { Derived prev(derived()); operator++(); return prev;}
  Derived operator--(int) { Derived prev(derived()); operator--(); return prev;}

  friend Derived operator+(const IndexedBasedStlIteratorBase& a, Index b) { Derived ret(a.derived()); ret += b; return ret; }
  friend Derived operator-(const IndexedBasedStlIteratorBase& a, Index b) { Derived ret(a.derived()); ret -= b; return ret; }
  friend Derived operator+(Index a, const IndexedBasedStlIteratorBase& b) { Derived ret(b.derived()); ret += a; return ret; }
  friend Derived operator-(Index a, const IndexedBasedStlIteratorBase& b) { Derived ret(b.derived()); ret -= a; return ret; }
  
  Derived& operator+=(Index b) { m_index += b; return derived(); }
  Derived& operator-=(Index b) { m_index -= b; return derived(); }

  difference_type operator-(const IndexedBasedStlIteratorBase& other) const { eigen_assert(mp_xpr == other.mp_xpr);return m_index - other.m_index; }

  bool operator==(const IndexedBasedStlIteratorBase& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index == other.m_index; }
  bool operator!=(const IndexedBasedStlIteratorBase& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index != other.m_index; }
  bool operator< (const IndexedBasedStlIteratorBase& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index <  other.m_index; }
  bool operator<=(const IndexedBasedStlIteratorBase& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index <= other.m_index; }
  bool operator> (const IndexedBasedStlIteratorBase& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index >  other.m_index; }
  bool operator>=(const IndexedBasedStlIteratorBase& other) { eigen_assert(mp_xpr == other.mp_xpr); return m_index >= other.m_index; }

protected:

  Derived& derived() { return static_cast<Derived&>(*this); }
  const Derived& derived() const { return static_cast<const Derived&>(*this); }

  XprType *mp_xpr;
  Index m_index;
};

template<typename XprType>
class DenseStlIterator : public IndexedBasedStlIteratorBase<XprType, DenseStlIterator<XprType> >
{
public:
  typedef typename XprType::Scalar value_type;

protected:

  enum {
    has_direct_access = (internal::traits<XprType>::Flags & DirectAccessBit) ? 1 : 0,
    has_write_access  = internal::is_lvalue<XprType>::value
  };

  typedef IndexedBasedStlIteratorBase<XprType,DenseStlIterator> Base;
  using Base::m_index;
  using Base::mp_xpr;

  typedef typename internal::conditional<bool(has_direct_access), const value_type&, const value_type>::type read_only_ref_t;

public:
  
  typedef typename internal::conditional<bool(has_write_access), value_type *, const value_type *>::type pointer;
  typedef typename internal::conditional<bool(has_write_access), value_type&, read_only_ref_t>::type reference;
  
  DenseStlIterator() : Base() {}
  DenseStlIterator(XprType& xpr, Index index) : Base(xpr,index) {}

  reference operator*()         const { return   (*mp_xpr)(m_index);   }
  reference operator[](Index i) const { return   (*mp_xpr)(m_index+i); }
  pointer   operator->()        const { return &((*mp_xpr)(m_index)); }
};

template<typename XprType,typename Derived>
void swap(IndexedBasedStlIteratorBase<XprType,Derived>& a, IndexedBasedStlIteratorBase<XprType,Derived>& b) {
  a.swap(b);
}

template<typename Derived>
inline DenseStlIterator<Derived> DenseBase<Derived>::begin()
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived);
  return DenseStlIterator<Derived>(derived(), 0);
}

template<typename Derived>
inline DenseStlIterator<const Derived> DenseBase<Derived>::begin() const
{
  return cbegin();
}

template<typename Derived>
inline DenseStlIterator<const Derived> DenseBase<Derived>::cbegin() const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived);
  return DenseStlIterator<const Derived>(derived(), 0);
}

template<typename Derived>
inline DenseStlIterator<Derived> DenseBase<Derived>::end()
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived);
  return DenseStlIterator<Derived>(derived(), size());
}

template<typename Derived>
inline DenseStlIterator<const Derived> DenseBase<Derived>::end() const
{
  return cend();
}

template<typename Derived>
inline DenseStlIterator<const Derived> DenseBase<Derived>::cend() const
{
  EIGEN_STATIC_ASSERT_VECTOR_ONLY(Derived);
  return DenseStlIterator<const Derived>(derived(), size());
}

template<typename XprType, DirectionType Direction>
class SubVectorStlIterator : public IndexedBasedStlIteratorBase<XprType, SubVectorStlIterator<XprType,Direction> >
{
protected:

  enum { is_lvalue  = internal::is_lvalue<XprType>::value };

  typedef IndexedBasedStlIteratorBase<XprType,SubVectorStlIterator> Base;
  using Base::m_index;
  using Base::mp_xpr;

  typedef typename internal::conditional<Direction==Vertical,typename XprType::ColXpr,typename XprType::RowXpr>::type SubVectorType;
  typedef typename internal::conditional<Direction==Vertical,typename XprType::ConstColXpr,typename XprType::ConstRowXpr>::type ConstSubVectorType;

public:
  typedef typename internal::conditional<bool(is_lvalue), SubVectorType, ConstSubVectorType>::type value_type;
  typedef value_type* pointer;
  typedef value_type  reference;
  
  SubVectorStlIterator() : Base() {}
  SubVectorStlIterator(XprType& xpr, Index index) : Base(xpr,index) {}

  reference operator*()         const { return   (*mp_xpr).template subVector<Direction>(m_index);   }
  reference operator[](Index i) const { return   (*mp_xpr).template subVector<Direction>(m_index+i); }
  pointer   operator->()        const { return &((*mp_xpr).template subVector<Direction>(m_index)); }
};

template<typename XprType, DirectionType Direction>
class SubVectorsProxy
{
public:
  typedef SubVectorStlIterator<XprType,       Direction> iterator;
  typedef SubVectorStlIterator<const XprType, Direction> const_iterator;

  SubVectorsProxy(XprType& xpr) : m_xpr(xpr) {}

  iterator        begin() const { return iterator      (m_xpr, 0); }
  const_iterator cbegin() const { return const_iterator(m_xpr, 0); }

  iterator        end()   const { return iterator      (m_xpr, m_xpr.template subVectors<Direction>()); }
  const_iterator cend()   const { return const_iterator(m_xpr, m_xpr.template subVectors<Direction>()); }

protected:
  XprType& m_xpr;
};

template<typename Derived>
SubVectorsProxy<Derived,Vertical> DenseBase<Derived>::allCols()
{ return SubVectorsProxy<Derived,Vertical>(derived()); }

template<typename Derived>
SubVectorsProxy<const Derived,Vertical> DenseBase<Derived>::allCols() const
{ return SubVectorsProxy<const Derived,Vertical>(derived()); }

template<typename Derived>
SubVectorsProxy<Derived,Horizontal> DenseBase<Derived>::allRows()
{ return SubVectorsProxy<Derived,Horizontal>(derived()); }

template<typename Derived>
SubVectorsProxy<const Derived,Horizontal> DenseBase<Derived>::allRows() const
{ return SubVectorsProxy<const Derived,Horizontal>(derived()); }

} // namespace Eigen

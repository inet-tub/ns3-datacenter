/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: John Abraham <john.abraham.in@gmail.com>
 */

#ifndef TIMEVALUE_H
#define TIMEVALUE_H


#include <map>
#include <ostream>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include "log.h"
#include <QtGlobal>


namespace netanim
{

template <class T>
class TimeValue
{
public:
  TimeValue ();
  TimeValue (const TimeValue & other);
  TimeValue <T> & operator= (const TimeValue <T> & rhs);
  typedef std::multimap<qreal, T> TimeValue_t;
  typedef std::pair<qreal, T> TimeValuePair_t;
  typedef std::pair<typename TimeValue_t::const_iterator, typename TimeValue_t::const_iterator> TimeValueIteratorPair_t;
  typedef enum
  {
    GOOD,
    UNDERRUN,
    OVERRUN
  } TimeValueResult_t;

  void add (qreal t, T value);
  void systemReset ();
  TimeValueResult_t setCurrentTime (qreal t);
  typename TimeValue_t::const_iterator Begin ();
  typename TimeValue_t::const_iterator End ();

  T getCurrent ();
  T get (qreal tUpperBound, TimeValueResult_t & result);
  TimeValueIteratorPair_t getRange (qreal lowerBound, qreal upperBound);
  TimeValueIteratorPair_t getNext (TimeValueResult_t & result);
  std::string toString ();
  void setLookBack (qreal lookBack);
  bool isEnd ();
  uint32_t getCount ();
  void rewind ();

private:
  TimeValue_t m_timeValues;
  typename TimeValue<T>::TimeValue_t::const_iterator m_currentIterator;
  typename TimeValue<T>::TimeValue_t::const_iterator m_getIterator;
  qreal m_lookBack;
  void rewindCurrentIterator ();
};

template <class T>
TimeValue<T>::TimeValue (): m_lookBack (0)
{

}

template <class T>
TimeValue<T>::TimeValue (const TimeValue & other)
{
  for (typename TimeValue<T>::TimeValue_t::const_iterator i = other.m_timeValues.begin ();
       i != other.m_timeValues.end ();
       ++i)
    {
      m_timeValues.insert (TimeValuePair_t (i->first, i->second));
    }
  if (!m_timeValues.empty ())
    {
      m_currentIterator = m_timeValues.begin ();
      m_getIterator = m_timeValues.begin ();

    }
}


template <class T>
TimeValue <T> &
TimeValue<T>::operator= (const TimeValue <T> & other)
{
  for (typename TimeValue<T>::TimeValue_t::const_iterator i = other.m_timeValues.begin ();
      i != other.m_timeValues.end ();
      ++i)
    {
      m_timeValues.insert (TimeValuePair_t (i->first, i->second));
      //m_timeValues[i->first] = i->second;
    }
  if (!m_timeValues.empty ())
    {
      m_currentIterator = m_timeValues.begin ();
      m_getIterator = m_timeValues.begin ();

    }
  return *this;
}

template <class T>
typename TimeValue<T>::TimeValue_t::const_iterator
TimeValue<T>::Begin ()
{
  return m_timeValues.begin ();
}

template <class T>
typename TimeValue<T>::TimeValue_t::const_iterator
TimeValue<T>::End ()
{
  return m_timeValues.end ();
}


template <class T>
void
TimeValue<T>::rewindCurrentIterator ()
{
  m_currentIterator = m_timeValues.begin ();
}

template <class T>
void
TimeValue<T>::add (qreal t, T value)
{
  bool wasEmpty = m_timeValues.empty ();
  m_timeValues.insert (TimeValuePair_t (t, value));
  if (wasEmpty)
    {
      m_currentIterator = m_timeValues.begin ();
      m_getIterator = m_timeValues.begin ();
    }
}


template <class T>
bool
TimeValue<T>::isEnd ()
{
  return m_currentIterator == m_timeValues.end ();
}


template <class T>
void
TimeValue<T>::systemReset ()
{
  m_timeValues.clear ();
}

template <class T>
typename TimeValue<T>::TimeValueIteratorPair_t
TimeValue<T>::getRange (qreal lowerBound, qreal upperBound)
{
  setCurrentTime (lowerBound);
  typename TimeValue_t::const_iterator lowerIterator = m_currentIterator;
  typename TimeValue_t::const_iterator tempIterator = m_currentIterator;
  while (tempIterator != m_timeValues.end ())
    {
      if (tempIterator->first > upperBound)
        {
          --tempIterator;
          break;
        }
      ++tempIterator;
    }
  TimeValueIteratorPair_t pp (lowerIterator, tempIterator);
  return pp;
}



template <class T>
typename TimeValue<T>::TimeValueIteratorPair_t
TimeValue<T>::getNext (TimeValueResult_t & result)
{
  result = GOOD;
  TimeValueIteratorPair_t pp =  m_timeValues.equal_range (m_getIterator->first);
  //std::cout << "First:" << m_getIterator->first;
  //fflush (stdout);
  if (m_getIterator == m_timeValues.end ())
    {
      result = OVERRUN;
    }
  else
    {
      m_getIterator = m_timeValues.upper_bound (m_getIterator->first);
    }
  return pp;

}


template <class T>
T
TimeValue<T>::get (qreal tUpperBound, TimeValueResult_t & result)
{
  //logQString (QString ("m_getIterator->first:") + QString::number (m_getIterator->first) + " t:" + QString::number (tUpperBound));
  //  std::cout << "First:" << m_getIterator->first;
  //  fflush (stdout);
  result = GOOD;
  T v = m_getIterator->second;
  if ( (m_getIterator == m_timeValues.end ()) || (m_getIterator->first > tUpperBound))
    {
      result = OVERRUN;
    }
  else
    {
      ++m_getIterator;
    }
  return v;
}

template <class T>
T
TimeValue<T>::getCurrent ()
{
  if (m_currentIterator == m_timeValues.end ())
    {
      return T (m_timeValues.rbegin ()->second);
    }
  return m_currentIterator->second;
}



template <class T>
void
TimeValue<T>::setLookBack (qreal lookBack)
{
  m_lookBack = lookBack;
}

template <class T>
typename TimeValue<T>::TimeValueResult_t
TimeValue<T>::setCurrentTime (qreal t)
{
  TimeValueResult_t result = GOOD;
  if (m_timeValues.empty ())
    {
      result = UNDERRUN;
    }

  bool skipIteration = false;
  if (result == GOOD)
    {
      t = t - m_lookBack;
      t = qMax (t, 0.0);
      if ( (!t) || (t < m_currentIterator->first))
        {
          skipIteration = true;
          //logQString (QString ("m_currentIterator->first:") + QString::number (m_currentIterator->first) + " t:" + QString::number (t));
          rewindCurrentIterator ();
          if (t < m_currentIterator->first)
            {
              result = UNDERRUN;
            }
          else
            {
              result = GOOD;
            }
        }
    }
  if (result == GOOD && (!skipIteration))
    {
      typename TimeValue<T>::TimeValue_t::const_iterator i = m_currentIterator;
      for ( ;
            i != m_timeValues.end ();
            ++i)
        {
          //logQString (QString ("i->first:") + QString::number (i->first) + " t:" + QString::number (t));
          if (i->first > t)
            {
              --m_currentIterator;
              result = GOOD;
              break;
            }
          else if (qFuzzyCompare (i->first, t))
            {
              result = GOOD;
              break;
            }
          else
            {
              ++m_currentIterator;
            }
        }
      if (i == m_timeValues.end ())
        {
          result = OVERRUN;
        }
    }
  m_getIterator = m_currentIterator;
  //logQString (QString ("ENd m_currentIterator->first:") + QString::number ( m_currentIterator->first) + " t:" + QString::number (t));
  //logQString (QString ("ENd m_getIterator->first:") + QString::number ( m_getIterator->first) + " t:" + QString::number (t));
  return result;
}

template <class T>
std::string
TimeValue<T>::toString ()
{
  std::ostringstream os;
  for (typename TimeValue<T>::TimeValue_t::const_iterator i = m_timeValues.begin ();
      i != m_timeValues.end ();
     )
    {
      TimeValueIteratorPair_t pp =  m_timeValues.equal_range (i->first);
      for (typename TimeValue<T>::TimeValue_t::const_iterator j = pp.first;
          j != pp.second;
          ++j)
        {
          os << j->first;
        }
      i = m_timeValues.upper_bound (i->first);
    }
  return os.str();
}

template <class T>
void
TimeValue<T>::rewind ()
{
  rewindCurrentIterator ();
}

template <class T>
uint32_t
TimeValue<T>::getCount ()
{
  return m_timeValues.size ();
}

} // namespace netanim
#endif // TIMEVALUE_H

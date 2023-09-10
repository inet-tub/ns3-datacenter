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


#ifndef TABLE_H
#define TABLE_H

#include "common.h"
namespace netanim {
class Table: public QWidget
{
Q_OBJECT
public:
  Table ();
  void setHeaderList (QStringList headerList);
  void addRow (QStringList rowContents, bool autoAdjust = false);
  void addCell (uint32_t cellIndex, QString value);
  void incrRowCount ();
  void removeAllRows ();
  void adjust ();
  void clear ();
  QString stringListToRowString (QStringList strList);
private:
  QTableWidget * m_table;
  QVBoxLayout * m_vLayout;
  QPushButton * m_exportTableButton;
  QStringList m_headerList;

protected slots:
  void exportButtonClickedSlot ();

};

}

#endif // TABLE_H

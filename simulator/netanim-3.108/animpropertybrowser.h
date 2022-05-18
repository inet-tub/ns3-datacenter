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
 * Contributions: Dmitrii Shakshin <d.shakshin@gmail.com> (Open Source and Linux Laboratory http://dev.osll.ru/)
 */

#ifndef ANIMPROPERTYBROWSER_H
#define ANIMPROPERTYBROWSER_H
#include "common.h"
#include <QtTreePropertyBrowser>
#include <QtIntPropertyManager>
#include <QtSpinBoxFactory>
#include <QtLineEditFactory>
#include <QtDoubleSpinBoxFactory>
#include <QTableWidget>
#include "filepathmanager.h"
#include "fileeditfactory.h"
#include "table.h"


namespace netanim {
class AnimPropertyBroswer: public QWidget
{
Q_OBJECT
public:
  static AnimPropertyBroswer * getInstance ();
  void postParse ();
  void systemReset ();
  void show (bool show);
  void setCurrentNodeId (uint32_t currentNodeId);
  void showNodePositionTable (bool show);
  void refresh ();
private:
  typedef QVector <QtProperty *> QtPropertyVector_t;
  AnimPropertyBroswer ();
  void reset ();
  void setupNodeProperties ();
  void setupBackgroundProperties ();
  void setupManagers ();
  void setupFactories ();
  void refreshBackgroundProperties ();
  QVBoxLayout * m_vboxLayout;
  QtAbstractPropertyBrowser * m_nodeBrowser;
  QtAbstractPropertyBrowser * m_backgroundBrowser;
  Table * m_nodePosTable;
  QComboBox * m_mode;
  QComboBox * m_nodeIdSelector;

  QtProperty * m_nodeIdProperty;
  QtProperty * m_nodeSysIdProperty;
  QtProperty * m_nodeDescriptionProperty;
  QtProperty * m_nodeXProperty;
  QtProperty * m_nodeYProperty;
  QtProperty * m_nodeColorProperty;
  QtProperty * m_nodeSizeProperty;
  QtProperty * m_fileEditProperty;
  QtProperty * m_ipv4AddressGroupProperty;
  QtProperty * m_ipv6AddressGroupProperty;
  QtProperty * m_macAddressGroupProperty;
  QtProperty * m_nodePositionGroupProperty;
  QtPropertyVector_t m_ipv4AddressVectorProperty;
  QtPropertyVector_t m_ipv6AddressVectorProperty;
  QtPropertyVector_t m_macAddressVectorProperty;
  QtProperty * m_showNodeTrajectoryProperty;

  QtPropertyVector_t m_nodeCounterDoubleProperty;
  QtPropertyVector_t m_nodeCounterUint32Property;


  // Background
  QtProperty * m_backgroundFileEditProperty;
  QtProperty * m_backgroundX;
  QtProperty * m_backgroundY;
  QtProperty * m_backgroundScaleX;
  QtProperty * m_backgroundScaleY;
  QtProperty * m_backgroundOpacity;

  uint32_t m_currentNodeId;

  QtIntPropertyManager * m_intManager;
  QtStringPropertyManager * m_stringManager;
  QtDoublePropertyManager * m_doubleManager;
  QtDoublePropertyManager * m_backgroundDoubleManager;

  QtColorPropertyManager * m_colorManager;
  FilePathManager * m_filePathManager;
  QtGroupPropertyManager * m_nodePositionManager;
  QtGroupPropertyManager * m_ipv4AddressManager;
  QtGroupPropertyManager * m_ipv6AddressManager;

  QtGroupPropertyManager * m_macAddressManager;
  QtStringPropertyManager * m_staticStringManager;
  QtBoolPropertyManager * m_boolManager;

  QtDoubleSpinBoxFactory * m_doubleSpinBoxFactory;
  QtSpinBoxFactory * m_spinBoxFactory;
  FileEditFactory * m_fileEditFactory;
  QtLineEditFactory * m_lineEditFactory;
  QtCheckBoxFactory * m_checkBoxFactory;



  typedef std::map <QtProperty *, QString> PropertyIdMap_t;
  PropertyIdMap_t m_propertyId;
private slots:
  void nodeIdSelectorSlot (QString newIndex);
  void valueChangedSlot (QtProperty*, QString);
  void valueChangedSlot (QtProperty*, double);
  void valueChangedSlot (QtProperty*, QColor);
  void valueChangedSlot (QtProperty* ,bool);
  void modeChangedSlot (QString mode);

};

} // namespace netanim
#endif // ANIMPROPERTYBROWSER_H

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
}

SOURCES += \
    main.cpp \
    log.cpp \
    fatal-error.cpp \
    fatal-impl.cpp \
    logqt.cpp \
    resizeableitem.cpp \
    animnode.cpp \
    animatorscene.cpp \
    animpacket.cpp \
    netanim.cpp \
    animatormode.cpp \
    mode.cpp \
    animxmlparser.cpp \
    animatorview.cpp \
    animlink.cpp \
    animresource.cpp \
    statsview.cpp \
    statsmode.cpp \
    routingxmlparser.cpp \
    routingstatsscene.cpp \
    interfacestatsscene.cpp \
    flowmonxmlparser.cpp \
    flowmonstatsscene.cpp \
    textbubble.cpp \
    qtpropertybrowser/src/qtvariantproperty.cpp \
    qtpropertybrowser/src/qttreepropertybrowser.cpp \
    qtpropertybrowser/src/qtpropertymanager.cpp \
    qtpropertybrowser/src/qtpropertybrowserutils.cpp \
    qtpropertybrowser/src/qtpropertybrowser.cpp \
    qtpropertybrowser/src/qtgroupboxpropertybrowser.cpp \
    qtpropertybrowser/src/qteditorfactory.cpp \
    qtpropertybrowser/src/qtbuttonpropertybrowser.cpp \
    animpropertybrowser.cpp \
    qtpropertybrowser/src/filepathmanager.cpp \
    qtpropertybrowser/src/fileeditfactory.cpp \
    qtpropertybrowser/src/fileedit.cpp \
    packetsmode.cpp \
    packetsview.cpp \
    packetsscene.cpp \
    graphpacket.cpp \
    table.cpp \
    countertablesscene.cpp \
    qcustomplot.cpp
HEADERS += \
    log.h \
    fatal-error.h \
    fatal-impl.h \
    abort.h \
    assert.h \
    logqt.h \
    animatorconstants.h \
    resizeableitem.h \
    animnode.h \
    common.h \
    animatorscene.h \
    timevalue.h \
    animpacket.h \
    netanim.h \
    animatormode.h \
    animatorview.h \
    mode.h \
    animxmlparser.h \
    animevent.h \
    animlink.h \
    animresource.h \
    statsview.h \
    statsmode.h \
    statisticsconstants.h \
    routingxmlparser.h \
    routingstatsscene.h \
    interfacestatsscene.h \
    flowmonxmlparser.h \
    flowmonstatsscene.h \
    textbubble.h \
    qtpropertybrowser/src/QtVariantPropertyManager \
    qtpropertybrowser/src/QtVariantProperty \
    qtpropertybrowser/src/qtvariantproperty.h \
    qtpropertybrowser/src/QtVariantEditorFactory \
    qtpropertybrowser/src/QtTreePropertyBrowser \
    qtpropertybrowser/src/qttreepropertybrowser.h \
    qtpropertybrowser/src/QtTimePropertyManager \
    qtpropertybrowser/src/QtTimeEditFactory \
    qtpropertybrowser/src/QtStringPropertyManager \
    qtpropertybrowser/src/QtSpinBoxFactory \
    qtpropertybrowser/src/QtSliderFactory \
    qtpropertybrowser/src/QtSizePropertyManager \
    qtpropertybrowser/src/QtSizePolicyPropertyManager \
    qtpropertybrowser/src/QtSizeFPropertyManager \
    qtpropertybrowser/src/QtScrollBarFactory \
    qtpropertybrowser/src/QtRectPropertyManager \
    qtpropertybrowser/src/QtRectFPropertyManager \
    qtpropertybrowser/src/qtpropertymanager.h \
    qtpropertybrowser/src/qtpropertybrowserutils_p.h \
    qtpropertybrowser/src/qtpropertybrowser.h \
    qtpropertybrowser/src/QtProperty \
    qtpropertybrowser/src/QtPointPropertyManager \
    qtpropertybrowser/src/QtPointFPropertyManager \
    qtpropertybrowser/src/QtLocalePropertyManager \
    qtpropertybrowser/src/QtLineEditFactory \
    qtpropertybrowser/src/QtKeySequencePropertyManager \
    qtpropertybrowser/src/QtKeySequenceEditorFactory \
    qtpropertybrowser/src/QtIntPropertyManager \
    qtpropertybrowser/src/QtGroupPropertyManager \
    qtpropertybrowser/src/QtGroupBoxPropertyBrowser \
    qtpropertybrowser/src/qtgroupboxpropertybrowser.h \
    qtpropertybrowser/src/QtFontPropertyManager \
    qtpropertybrowser/src/QtFontEditorFactory \
    qtpropertybrowser/src/QtFlagPropertyManager \
    qtpropertybrowser/src/QtEnumPropertyManager \
    qtpropertybrowser/src/QtEnumEditorFactory \
    qtpropertybrowser/src/qteditorfactory.h \
    qtpropertybrowser/src/QtDoubleSpinBoxFactory \
    qtpropertybrowser/src/QtDoublePropertyManager \
    qtpropertybrowser/src/QtDateTimePropertyManager \
    qtpropertybrowser/src/QtDateTimeEditFactory \
    qtpropertybrowser/src/QtDatePropertyManager \
    qtpropertybrowser/src/QtDateEditFactory \
    qtpropertybrowser/src/QtCursorPropertyManager \
    qtpropertybrowser/src/QtCursorEditorFactory \
    qtpropertybrowser/src/QtColorPropertyManager \
    qtpropertybrowser/src/QtColorEditorFactory \
    qtpropertybrowser/src/QtCheckBoxFactory \
    qtpropertybrowser/src/QtCharPropertyManager \
    qtpropertybrowser/src/QtCharEditorFactory \
    qtpropertybrowser/src/QtButtonPropertyBrowser \
    qtpropertybrowser/src/qtbuttonpropertybrowser.h \
    qtpropertybrowser/src/QtBrowserItem \
    qtpropertybrowser/src/QtBoolPropertyManager \
    qtpropertybrowser/src/QtAbstractPropertyManager \
    qtpropertybrowser/src/QtAbstractPropertyBrowser \
    qtpropertybrowser/src/QtAbstractEditorFactoryBase \
    animpropertybrowser.h \
    qtpropertybrowser/src/filepathmanager.h \
    qtpropertybrowser/src/fileeditfactory.h \
    qtpropertybrowser/src/fileedit.h \
    packetsmode.h \
    packetsview.h \
    packetsscene.h \
    graphpacket.h \
    table.h \
    countertablesscene.h \
    qcustomplot.h


INCLUDEPATH += qtpropertybrowser/src
DEFINES += NS3_LOG_ENABLE

RESOURCES += \
    resources.qrc \
    qtpropertybrowser/src/qtpropertybrowser.qrc

OTHER_FILES += \
    qtpropertybrowser/src/qtpropertybrowser.pri

macx {
 CONFIG -= app_bundle
 QMAKESPEC = macx-g++
}


/****************************************************************************
** Meta object code from reading C++ file 'backendcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/backendcontroller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'backendcontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN17BackendControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto BackendController::qt_create_metaobjectdata<qt_meta_tag_ZN17BackendControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BackendController",
        "statusChanged",
        "",
        "status",
        "processStarted",
        "processId",
        "processFinished",
        "exitCode",
        "crashed",
        "outputReceived",
        "line",
        "errorOccurred",
        "message"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'processStarted'
        QtMocHelpers::SignalData<void(qint64)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 5 },
        }}),
        // Signal 'processFinished'
        QtMocHelpers::SignalData<void(int, bool)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 }, { QMetaType::Bool, 8 },
        }}),
        // Signal 'outputReceived'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 12 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BackendController, qt_meta_tag_ZN17BackendControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BackendController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17BackendControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17BackendControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17BackendControllerE_t>.metaTypes,
    nullptr
} };

void BackendController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BackendController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->statusChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->processStarted((*reinterpret_cast<std::add_pointer_t<qint64>>(_a[1]))); break;
        case 2: _t->processFinished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 3: _t->outputReceived((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (BackendController::*)(const QString & )>(_a, &BackendController::statusChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (BackendController::*)(qint64 )>(_a, &BackendController::processStarted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (BackendController::*)(int , bool )>(_a, &BackendController::processFinished, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (BackendController::*)(const QString & )>(_a, &BackendController::outputReceived, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (BackendController::*)(const QString & )>(_a, &BackendController::errorOccurred, 4))
            return;
    }
}

const QMetaObject *BackendController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BackendController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17BackendControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BackendController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void BackendController::statusChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void BackendController::processStarted(qint64 _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void BackendController::processFinished(int _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void BackendController::outputReceived(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void BackendController::errorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}
QT_WARNING_POP

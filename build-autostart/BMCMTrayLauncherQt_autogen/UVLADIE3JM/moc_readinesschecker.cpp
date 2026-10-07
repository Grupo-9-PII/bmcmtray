/****************************************************************************
** Meta object code from reading C++ file 'readinesschecker.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/readinesschecker.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'readinesschecker.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16ReadinessCheckerE_t {};
} // unnamed namespace

template <> constexpr inline auto ReadinessChecker::qt_create_metaobjectdata<qt_meta_tag_ZN16ReadinessCheckerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ReadinessChecker",
        "checking",
        "",
        "message",
        "ready",
        "QUrl",
        "url",
        "timedOut"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'checking'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'ready'
        QtMocHelpers::SignalData<void(const QUrl &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Signal 'timedOut'
        QtMocHelpers::SignalData<void(const QUrl &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ReadinessChecker, qt_meta_tag_ZN16ReadinessCheckerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ReadinessChecker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReadinessCheckerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReadinessCheckerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16ReadinessCheckerE_t>.metaTypes,
    nullptr
} };

void ReadinessChecker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ReadinessChecker *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->checking((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->ready((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 2: _t->timedOut((*reinterpret_cast<std::add_pointer_t<QUrl>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ReadinessChecker::*)(const QString & )>(_a, &ReadinessChecker::checking, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReadinessChecker::*)(const QUrl & )>(_a, &ReadinessChecker::ready, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReadinessChecker::*)(const QUrl & )>(_a, &ReadinessChecker::timedOut, 2))
            return;
    }
}

const QMetaObject *ReadinessChecker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ReadinessChecker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ReadinessCheckerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ReadinessChecker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void ReadinessChecker::checking(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ReadinessChecker::ready(const QUrl & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ReadinessChecker::timedOut(const QUrl & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP

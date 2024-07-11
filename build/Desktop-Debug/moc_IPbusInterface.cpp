/****************************************************************************
** Meta object code from reading C++ file 'IPbusInterface.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../IPbusInterface.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'IPbusInterface.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IPbusTarget_t {
    QByteArrayData data[20];
    char stringdata0[171];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IPbusTarget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IPbusTarget_t qt_meta_stringdata_IPbusTarget = {
    {
QT_MOC_LITERAL(0, 0, 11), // "IPbusTarget"
QT_MOC_LITERAL(1, 12, 5), // "error"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 9), // "errorType"
QT_MOC_LITERAL(4, 29, 10), // "noResponse"
QT_MOC_LITERAL(5, 40, 7), // "message"
QT_MOC_LITERAL(6, 48, 13), // "IPbusStatusOK"
QT_MOC_LITERAL(7, 62, 9), // "reconnect"
QT_MOC_LITERAL(8, 72, 11), // "checkStatus"
QT_MOC_LITERAL(9, 84, 4), // "sync"
QT_MOC_LITERAL(10, 89, 13), // "writeRegister"
QT_MOC_LITERAL(11, 103, 7), // "address"
QT_MOC_LITERAL(12, 111, 4), // "data"
QT_MOC_LITERAL(13, 116, 13), // "syncOnSuccess"
QT_MOC_LITERAL(14, 130, 6), // "setBit"
QT_MOC_LITERAL(15, 137, 1), // "n"
QT_MOC_LITERAL(16, 139, 8), // "clearBit"
QT_MOC_LITERAL(17, 148, 10), // "writeNbits"
QT_MOC_LITERAL(18, 159, 5), // "nbits"
QT_MOC_LITERAL(19, 165, 5) // "shift"

    },
    "IPbusTarget\0error\0\0errorType\0noResponse\0"
    "message\0IPbusStatusOK\0reconnect\0"
    "checkStatus\0sync\0writeRegister\0address\0"
    "data\0syncOnSuccess\0setBit\0n\0clearBit\0"
    "writeNbits\0nbits\0shift"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IPbusTarget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   99,    2, 0x06 /* Public */,
       4,    1,  104,    2, 0x06 /* Public */,
       4,    0,  107,    2, 0x26 /* Public | MethodCloned */,
       6,    0,  108,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,  109,    2, 0x0a /* Public */,
       8,    0,  110,    2, 0x0a /* Public */,
       9,    0,  111,    2, 0x0a /* Public */,
      10,    3,  112,    2, 0x0a /* Public */,
      10,    2,  119,    2, 0x2a /* Public | MethodCloned */,
      14,    3,  124,    2, 0x0a /* Public */,
      14,    2,  131,    2, 0x2a /* Public | MethodCloned */,
      16,    3,  136,    2, 0x0a /* Public */,
      16,    2,  143,    2, 0x2a /* Public | MethodCloned */,
      17,    5,  148,    2, 0x0a /* Public */,
      17,    4,  159,    2, 0x2a /* Public | MethodCloned */,
      17,    3,  168,    2, 0x2a /* Public | MethodCloned */,
      17,    2,  175,    2, 0x2a /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 3,    2,    2,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt, QMetaType::Bool,   11,   12,   13,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt,   11,   12,
    QMetaType::Void, QMetaType::UChar, QMetaType::UInt, QMetaType::Bool,   15,   11,   13,
    QMetaType::Void, QMetaType::UChar, QMetaType::UInt,   15,   11,
    QMetaType::Void, QMetaType::UChar, QMetaType::UInt, QMetaType::Bool,   15,   11,   13,
    QMetaType::Void, QMetaType::UChar, QMetaType::UInt,   15,   11,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt, QMetaType::UChar, QMetaType::UChar, QMetaType::Bool,   11,   12,   18,   19,   13,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt, QMetaType::UChar, QMetaType::UChar,   11,   12,   18,   19,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt, QMetaType::UChar,   11,   12,   18,
    QMetaType::Void, QMetaType::UInt, QMetaType::UInt,   11,   12,

       0        // eod
};

void IPbusTarget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IPbusTarget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->error((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< errorType(*)>(_a[2]))); break;
        case 1: _t->noResponse((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->noResponse(); break;
        case 3: _t->IPbusStatusOK(); break;
        case 4: _t->reconnect(); break;
        case 5: _t->checkStatus(); break;
        case 6: _t->sync(); break;
        case 7: _t->writeRegister((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 8: _t->writeRegister((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 9: _t->setBit((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 10: _t->setBit((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 11: _t->clearBit((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 12: _t->clearBit((*reinterpret_cast< quint8(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        case 13: _t->writeNbits((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< quint8(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 14: _t->writeNbits((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3])),(*reinterpret_cast< quint8(*)>(_a[4]))); break;
        case 15: _t->writeNbits((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])),(*reinterpret_cast< quint8(*)>(_a[3]))); break;
        case 16: _t->writeNbits((*reinterpret_cast< quint32(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (IPbusTarget::*)(QString , errorType );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IPbusTarget::error)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (IPbusTarget::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IPbusTarget::noResponse)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (IPbusTarget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IPbusTarget::IPbusStatusOK)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IPbusTarget::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_IPbusTarget.data,
    qt_meta_data_IPbusTarget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IPbusTarget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IPbusTarget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IPbusTarget.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int IPbusTarget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void IPbusTarget::error(QString _t1, errorType _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IPbusTarget::noResponse(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 3
void IPbusTarget::IPbusStatusOK()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

#ifndef tst_insertproxymodel_h__
#define tst_insertproxymodel_h__

#include <QObject>
#include <QList>
class QAbstractItemModel;
class tst_InsertProxyModel : public QObject{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testExcept() { throw(1); }
    void testCrash() { ((QObject*)(nullptr))->objectName(); }
private:
    QList<QAbstractItemModel*> m_models;
};
#endif // tst_insertproxymodel_h__
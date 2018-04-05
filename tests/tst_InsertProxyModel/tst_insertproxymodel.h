#ifndef tst_insertproxymodel_h__
#define tst_insertproxymodel_h__

#include <QObject>
#include <QList>
class QAbstractItemModel;
class tst_InsertProxyModel : public QObject{
    Q_OBJECT
private Q_SLOTS:
    void testCommitSubclass();
    void testCommitSubclass_data();
    void testCommitSlot();
    void testCommitSlot_data();
    void testSourceInsertRow();
private:
    QAbstractItemModel* createListModel();
    QAbstractItemModel* createTableModel();
    QAbstractItemModel* createTreeModel();
};
#endif // tst_insertproxymodel_h__
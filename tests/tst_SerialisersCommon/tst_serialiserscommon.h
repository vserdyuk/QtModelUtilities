#ifndef tst_serialisers_common_h__
#define tst_serialisers_common_h__
#include <QModelIndex>
#include <QStringListModel>
#include <random>
#ifdef QT_GUI_LIB
#    include <QStandardItemModel>
#endif
class AbstractModelSerialiser;
class AbstractStringSerialiser;
class tst_SerialiserCommon
{
protected:
    virtual void basicSaveLoadData(QObject *parent);
    void saveLoadByteArray(AbstractModelSerialiser *serialiser, const QAbstractItemModel *sourceModel, QAbstractItemModel *destinationModel,
                           bool multiRole = true, bool checkHeaders = true) const;
    void saveLoadFile(AbstractModelSerialiser *serialiser, const QAbstractItemModel *sourceModel, QAbstractItemModel *destinationModel,
                      bool multiRole = true, bool checkHeaders = true) const;
    void saveLoadString(AbstractStringSerialiser *serialiser, const QAbstractItemModel *sourceModel, QAbstractItemModel *destinationModel,
                        bool multiRole = true, bool checkHeaders = true) const;
    void checkModelEqual(const QAbstractItemModel *a, const QAbstractItemModel *b, const QModelIndex &aParent = QModelIndex(),
                         const QModelIndex &bParent = QModelIndex(), bool checkHeaders = true) const;
    QAbstractItemModel *createStringModel(QObject *parent = nullptr);
#ifdef QT_GUI_LIB
    void insertBranch(QAbstractItemModel *model, const QModelIndex &parent, bool multiRoles, int subBranches);
    QAbstractItemModel *createComplexModel(bool tree, bool multiRoles, QObject *parent = nullptr);
#endif
    std::default_random_engine generator;
};
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
Q_DECLARE_METATYPE(const QAbstractItemModel *);
#endif
#endif

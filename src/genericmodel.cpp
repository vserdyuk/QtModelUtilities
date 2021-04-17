/****************************************************************************\
   Copyright 2021 Luca Beldi
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
\****************************************************************************/
#include "private/genericmodel_p.h"
#include "genericmodel.h"
#include <QDateTime>

GenericModelItem::GenericModelItem(GenericModel* model)
    : GenericModelItem(static_cast<GenericModelItem *>(nullptr))
{
    m_model = model;
}

GenericModelItem::GenericModelItem(GenericModelItem *par)
    : parent(par)
    , flags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled)
    , m_colCount(0)
    , m_rowCount(0)
    , m_row(-1)
    , m_column(-1)
    , m_rowSpan(1)
    , m_colSpan(1)
{
    if(par)
        m_model = par->m_model;
}

GenericModelItem::~GenericModelItem()
{
    qDeleteAll(children);
}

GenericModelItem *GenericModelItem::childAt(int row, int col) const
{
    const int childIndex = (row * m_colCount) + col;
    return children.at(childIndex);
}

int GenericModelItem::columnCount() const
{
    return m_colCount;
}

int GenericModelItem::rowCount() const
{
    return m_rowCount;
}

void GenericModelItem::insertColumns(int column, int count)
{
    if (m_rowCount > 0) {
        if (m_colCount == 0) {
            Q_ASSERT(column == 0);
            Q_ASSERT(children.isEmpty());
            const int childrenSize = count * m_rowCount;
            children.reserve(childrenSize);
            for (int j = 0; j < childrenSize; ++j) {
                auto newChild = new GenericModelItem(this);
                newChild->m_column = j % count;
                newChild->m_row = j / count;
                children.append(newChild);
            }
        } else {
            for (int i = column + (m_colCount * (m_rowCount - 1)); i >= 0; i -= m_colCount) {
                Q_ASSERT((i - column) % m_colCount == 0);
                const int rowIndex = (i - column) / m_colCount;
                if(rowIndex<0)
                    continue;
                for (int j = i; j < i - column + m_colCount; ++j)
                    children.at(j)->m_column += count;
                children.insert(i, count, nullptr);
                for (int j = 0; j < count; ++j) {
                    auto newChild = new GenericModelItem(this);
                    newChild->m_column = column + j;
                    newChild->m_row = rowIndex;
                    children[i + j] = newChild;
                }
            }
        }
    }
    m_colCount += count;
#ifdef QT_DEBUG
    Q_ASSERT(m_colCount*m_rowCount==children.size());
    for(int i=0;i<children.size();++i){
        Q_ASSERT(children.at(i)->row()==i / m_colCount);
        Q_ASSERT(children.at(i)->column()==i % m_colCount);
    }
#endif
}

void GenericModelItem::removeColumns(int column, int count)
{
    if (m_rowCount > 0) {
        for (int i = column + (m_colCount * (m_rowCount - 1)); i >= 0; i -= qMax(1, m_colCount)) {
            for (int j = i + count; j < i - column + m_colCount; ++j)
                children.at(j)->m_column -= count;
            const auto endRemoveIter = children.begin() + i + count;
            const auto startRemoveIter = children.begin() + i;
            qDeleteAll(startRemoveIter, endRemoveIter);
            children.erase(startRemoveIter, endRemoveIter);
        }
    }
    m_colCount -= count;
#ifdef QT_DEBUG
    Q_ASSERT(m_colCount*m_rowCount==children.size());
    for(int i=0;i<children.size();++i){
        Q_ASSERT(children.at(i)->row()==i / m_colCount);
        Q_ASSERT(children.at(i)->column()==i % m_colCount);
    }
#endif
}

void GenericModelItem::insertRows(int row, int count)
{
    if (m_colCount > 0) {
        for (auto i = children.begin() + (row * m_colCount), iEnd = children.end(); i != iEnd; ++i)
            (*i)->m_row += count;
        children.insert(row * m_colCount, count * m_colCount, nullptr);
        for (int i = row * m_colCount; i < (row + count) * m_colCount; ++i) {
            auto newChild = new GenericModelItem(this);
            newChild->m_column = i % m_colCount;
            newChild->m_row = i / m_colCount;
            children[i] = newChild;
        }
    }
    m_rowCount += count;
#ifdef QT_DEBUG
    Q_ASSERT(m_colCount*m_rowCount==children.size());
    for(int i=0;i<children.size();++i){
        Q_ASSERT(children.at(i)->row()==i / m_colCount);
        Q_ASSERT(children.at(i)->column()==i % m_colCount);
    }
#endif
}

void GenericModelItem::removeRows(int row, int count)
{
    if (m_colCount > 0) {
        Q_ASSERT((row + count) * m_colCount <= children.size());
        const auto endRemoveIter = children.begin() + ((row + count) * m_colCount);
        const auto startRemoveIter = children.begin() + (row * m_colCount);
        for (auto i = endRemoveIter, iEnd = children.end(); i != iEnd; ++i)
            (*i)->m_row -= count;
        qDeleteAll(startRemoveIter, endRemoveIter);
        children.erase(startRemoveIter, endRemoveIter);
    }
    m_rowCount -= count;
#ifdef QT_DEBUG
    Q_ASSERT(m_colCount*m_rowCount==children.size());
    for(int i=0;i<children.size();++i){
        Q_ASSERT(children.at(i)->row()==i / m_colCount);
        Q_ASSERT(children.at(i)->column()==i % m_colCount);
    }
#endif
}

int GenericModelItem::row() const
{
    return m_row;
}

int GenericModelItem::column() const
{
    return m_column;
}

void GenericModelItem::setMergeDisplayEdit(bool val)
{
    GenericModelPrivate::setMergeDisplayEdit(val, data);
    for (int i = 0, maxI = children.size(); i < maxI; ++i)
        children[i]->setMergeDisplayEdit(val);
}

QSize GenericModelItem::span() const
{
    return QSize(m_rowSpan, m_colSpan);
}

void GenericModelItem::setSpan(const QSize &sz)
{
    m_rowSpan = sz.height();
    m_colSpan = sz.width();
}

void GenericModelItem::sortChildren(int column, int role, Qt::SortOrder order, bool recursive)
{
    sortChildren(column,role,order,recursive,m_model->persistentIndexList());
}

void GenericModelItem::sortChildren(int column, int role, Qt::SortOrder order, bool recursive, const QModelIndexList& persistentIndexes)
{
    Q_ASSERT(column>=0);
    if(children.isEmpty() || column>=m_colCount)
        return;
    if(recursive){
        for(GenericModelItem* item : children)
            item->sortChildren(column,role,order,recursive,persistentIndexes);
    }
    QVector<GenericModelItem*> columnChildren;
    columnChildren.reserve(m_rowCount);
    for(int i=column,maxI=children.size();i<maxI;i+=m_colCount){
        GenericModelItem* columnChild= children.at(i);
        columnChildren.append(columnChild);
    }
    const auto lessComparison = [role](const GenericModelItem* a, const GenericModelItem* b)->bool{
        return GenericModelPrivate::isVariantLessThan(a->data.value(role),b->data.value(role));
    };
    const auto greaterComparison = [role](const GenericModelItem*  a, const GenericModelItem* b)->bool{
        return GenericModelPrivate::isVariantLessThan(b->data.value(role),a->data.value(role));
    };
    if(order==Qt::AscendingOrder)
        std::stable_sort(columnChildren.begin(),columnChildren.end(),lessComparison);
    else
        std::stable_sort(columnChildren.begin(),columnChildren.end(),greaterComparison);

    QVector<GenericModelItem*> newChildren;
    newChildren.reserve(children.size());
    QModelIndexList changedPersistentIndexesFrom, changedPersistentIndexesTo;
    for(int toRow=0,maxRow=columnChildren.size();toRow<maxRow;++toRow){
        GenericModelItem* const columnChild = columnChildren.at(toRow);
        const int fromRow = columnChild->m_row;
        if(toRow==fromRow)
            continue;
        for(int i=m_colCount*fromRow;i<m_colCount*(fromRow+1);++i){
            GenericModelItem* const iChild = children.at(i);
            QModelIndex fromIdx = m_model->createIndex(iChild->m_row,iChild->m_column,iChild);
            if(persistentIndexes.contains(fromIdx)){
                changedPersistentIndexesFrom.append(fromIdx);
                changedPersistentIndexesTo.append(m_model->createIndex(toRow,iChild->m_column,iChild));
            }
            iChild->m_row=toRow;
            newChildren.append(iChild);
        }
    }
    Q_ASSERT(newChildren.size()==children.size());
    Q_ASSERT(std::all_of(newChildren.constBegin(),newChildren.constEnd(),[](const GenericModelItem* a)->bool{return a!=nullptr;}));
    children=newChildren;
    m_model->changePersistentIndexList(changedPersistentIndexesFrom, changedPersistentIndexesTo);
}

bool GenericModelPrivate::isVariantLessThan(const QVariant &left, const QVariant &right)
{
    if (left.userType() == QVariant::Invalid)
        return false;
    if (right.userType() == QVariant::Invalid)
        return true;
    switch (left.userType()) {
    case QVariant::Int:
        return left.toInt() < right.toInt();
    case QVariant::UInt:
        return left.toUInt() < right.toUInt();
    case QVariant::LongLong:
        return left.toLongLong() < right.toLongLong();
    case QVariant::ULongLong:
        return left.toULongLong() < right.toULongLong();
    case QMetaType::Float:
        return left.toFloat() < right.toFloat();
    case QVariant::Double:
        return left.toDouble() < right.toDouble();
    case QVariant::Char:
        return left.toChar() < right.toChar();
    case QVariant::Date:
        return left.toDate() < right.toDate();
    case QVariant::Time:
        return left.toTime() < right.toTime();
    case QVariant::DateTime:
        return left.toDateTime() < right.toDateTime();
    case QVariant::String:
    default:
        return left.toString().compare(right.toString()) < 0;
    }
}

GenericModelPrivate::~GenericModelPrivate()
{
    delete root;
}

GenericModelPrivate::GenericModelPrivate(GenericModel *q)
    : q_ptr(q)
    , root(new GenericModelItem(q))
    , m_mergeDisplayEdit(true)
    , sortRole(Qt::DisplayRole)
{
    Q_ASSERT(q_ptr);
}

GenericModelItem *GenericModelPrivate::itemForIndex(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return root;
    Q_ASSERT(idx.model() == q_func());
    return static_cast<GenericModelItem *>(idx.internalPointer());
}

QModelIndex GenericModelPrivate::indexForItem(GenericModelItem *item) const
{
    Q_Q(const GenericModel);
    if (item == root)
        return QModelIndex();
    return q->createIndex(item->row(), item->column(), item);
}

void GenericModelPrivate::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (!parent.isValid())
        hHeaderData.insert(column, count, RolesContainer());
    GenericModelItem *item = itemForIndex(parent);
    item->insertColumns(column, count);
}

void GenericModelPrivate::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!parent.isValid())
        vHeaderData.insert(row, count, RolesContainer());
    GenericModelItem *item = itemForIndex(parent);
    item->insertRows(row, count);
}

void GenericModelPrivate::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (!parent.isValid())
        hHeaderData.erase(hHeaderData.begin() + column, hHeaderData.begin() + column + count);
    GenericModelItem *item = itemForIndex(parent);
    item->removeColumns(column, count);
}

void GenericModelPrivate::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!parent.isValid())
        vHeaderData.erase(vHeaderData.begin() + row, vHeaderData.begin() + row + count);
    GenericModelItem *item = itemForIndex(parent);
    item->removeRows(row, count);
}

/*!
Constructs a new model with the given \a parent.
*/
GenericModel::GenericModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_dptr(new GenericModelPrivate(this))
{ }

/*!
\internal
*/
GenericModel::GenericModel(GenericModelPrivate &dptr, QObject *parent)
    : QAbstractItemModel(parent)
    , m_dptr(&dptr)
{ }

/*!
Destructor
*/
GenericModel::~GenericModel()
{
    delete m_dptr;
}

/*!
\reimp
*/
bool GenericModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_ASSERT(!parent.isValid() || parent.model() == this);
    if (count <= 0 || column < 0 || column > columnCount(parent))
        return false;
    beginInsertColumns(parent, column, column + count - 1);
    Q_D(GenericModel);
    d->insertColumns(column, count, parent);
    endInsertColumns();
    return true;
}

/*!
\reimp
*/
bool GenericModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT(!parent.isValid() || parent.model() == this);
    if (count <= 0 || row < 0 || row > rowCount(parent))
        return false;
    beginInsertRows(parent, row, row + count - 1);
    Q_D(GenericModel);
    d->insertRows(row, count, parent);
    endInsertRows();
    return true;
}

/*!
\reimp
*/
bool GenericModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_ASSERT(!parent.isValid() || parent.model() == this);
    if (count <= 0 || column < 0 || column + count - 1 >= columnCount(parent))
        return false;
    beginRemoveColumns(parent, column, column + count - 1);
    Q_D(GenericModel);
    d->removeColumns(column, count, parent);
    endRemoveColumns();
    return true;
}

/*!
\reimp
*/
bool GenericModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT(!parent.isValid() || parent.model() == this);
    if (count <= 0 || row < 0 || row + count - 1 >= rowCount(parent))
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    Q_D(GenericModel);
    d->removeRows(row, count, parent);
    endRemoveRows();
    return true;
}

/*!
\reimp
*/
QMap<int, QVariant> GenericModel::itemData(const QModelIndex &index) const
{
    if (!index.isValid())
        return QMap<int, QVariant>();
    Q_ASSERT(index.model() == this);
    Q_D(const GenericModel);
    if (!d->m_mergeDisplayEdit)
        return convertFromContainer<QMap<int, QVariant>>(d->itemForIndex(index)->data);
    RolesContainer rawData = d->itemForIndex(index)->data;
    const auto displayIter = rawData.constFind(Qt::DisplayRole);
    if (displayIter != rawData.constEnd())
        rawData[Qt::EditRole] = displayIter.value();
    return convertFromContainer<QMap<int, QVariant>>(rawData);
}

/*!
\reimp
*/
QModelIndex GenericModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_ASSERT(!parent.isValid() || parent.model() == this);
    if (row < 0 || column < 0)
        return QModelIndex();
    Q_D(const GenericModel);
    GenericModelItem *parentItem = d->itemForIndex(parent);
    if (row >= parentItem->rowCount() || column >= parentItem->columnCount())
        return QModelIndex();
    return createIndex(row, column, parentItem->childAt(row, column));
}

/*!
\reimp
*/
int GenericModel::columnCount(const QModelIndex &parent) const
{
    Q_ASSERT(!parent.isValid() || parent.model() == this);
    Q_D(const GenericModel);
    return d->itemForIndex(parent)->columnCount();
}

/*!
\reimp
*/
int GenericModel::rowCount(const QModelIndex &parent) const
{
    Q_ASSERT(!parent.isValid() || parent.model() == this);
    Q_D(const GenericModel);
    return d->itemForIndex(parent)->rowCount();
}

/*!
\reimp
*/
QVariant GenericModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    Q_ASSERT(index.model() == this);
    Q_D(const GenericModel);
    const GenericModelItem *const item = d->itemForIndex(index);
    if (d->m_mergeDisplayEdit && role == Qt::EditRole)
        role = Qt::DisplayRole;
    return item->data.value(role);
}

/*!
\reimp
*/
Qt::ItemFlags GenericModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    Q_ASSERT(index.model() == this);
    Q_D(const GenericModel);
    return d->itemForIndex(index)->flags;
}

/*!
Set the item flags for a specific index
*/
bool GenericModel::setFlags(const QModelIndex &index, Qt::ItemFlags flags)
{
    if (!index.isValid())
        return false;
    Q_ASSERT(index.model() == this);
    Q_D(GenericModel);
    d->itemForIndex(index)->flags = flags;
    dataChanged(index, index);
    return true;
}

/*!
\reimp
*/
bool GenericModel::hasChildren(const QModelIndex &parent) const
{
    Q_ASSERT(!parent.isValid() || parent.model() == this);
    Q_D(const GenericModel);
    GenericModelItem *const item = d->itemForIndex(parent);
    return item->rowCount() > 0 && item->columnCount() > 0;
}

/*!
\reimp
*/
QVariant GenericModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < 0)
        return QVariant();
    Q_D(const GenericModel);
    if (d->m_mergeDisplayEdit && role == Qt::EditRole)
        role = Qt::DisplayRole;
    if (orientation == Qt::Horizontal) {
        if (section >= columnCount())
            return QVariant();
        return d->hHeaderData.at(section).value(role);
    }
    if (section >= rowCount())
        return QVariant();
    return d->vHeaderData.at(section).value(role);
}

/*!
\reimp
*/
bool GenericModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (section < 0)
        return false;
    RolesContainer *sectionContainer = nullptr;
    Q_D(GenericModel);
    if (orientation == Qt::Horizontal) {
        if (section >= columnCount())
            return false;
        sectionContainer = &(d->hHeaderData[section]);

    } else {
        if (section >= rowCount())
            return false;
        sectionContainer = &(d->vHeaderData[section]);
    }
    Q_ASSERT(sectionContainer);
    if (d->m_mergeDisplayEdit && role == Qt::EditRole)
        role = Qt::DisplayRole;
    const auto roleIter = sectionContainer->find(role);
    if (roleIter == sectionContainer->end()) {
        if (value.isValid()) {
            sectionContainer->insert(role, value);
            headerDataChanged(orientation, section, section);
        }
        return true;
    }
    if (!value.isValid()) {
        sectionContainer->erase(roleIter);
        headerDataChanged(orientation, section, section);
        return true;
    }
    if (roleIter.value() != value) {
        roleIter.value() = value;
        headerDataChanged(orientation, section, section);
    }
    return true;
}

/*!
\reimp
*/
QModelIndex GenericModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    Q_ASSERT(index.model() == this);
    Q_D(const GenericModel);
    return d->indexForItem(d->itemForIndex(index)->parent);
}

/*!
\reimp
*/
bool GenericModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    Q_ASSERT(index.model() == this);
    Q_D(GenericModel);
    GenericModelItem *const item = d->itemForIndex(index);
    if (d->m_mergeDisplayEdit && role == Qt::EditRole)
        role = Qt::DisplayRole;
    QVector<int> rolesToEmit{role};
    if (d->m_mergeDisplayEdit && role == Qt::DisplayRole)
        rolesToEmit.append(Qt::EditRole);
    const auto roleIter = item->data.find(role);
    if (roleIter == item->data.end()) {
        if (value.isValid()) {
            item->data.insert(role, value);
            dataChanged(index, index, rolesToEmit);
        }
        return true;
    }
    if (!value.isValid()) {
        item->data.erase(roleIter);
        dataChanged(index, index, rolesToEmit);
        return true;
    }
    if (value != roleIter.value()) {
        roleIter.value() = value;
        dataChanged(index, index, rolesToEmit);
    }
    return true;
}

/*!
\reimp
\details If mergeDisplayEdit is true (the default) and roles contains valuer for both Qt::EditRole and Qt::DisplayRole the latter will prevail
*/
bool GenericModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    if (!index.isValid())
        return false;
    Q_ASSERT(index.model() == this);
    Q_D(GenericModel);
    GenericModelItem *const item = d->itemForIndex(index);
    RolesContainer newData = convertToContainer(roles);
    if (d->m_mergeDisplayEdit) {
        const auto editIter = newData.find(Qt::EditRole);
        const auto displayIter = newData.find(Qt::DisplayRole);
        if (displayIter == newData.end()) {
            if (editIter != newData.end()) {
                newData.insert(Qt::DisplayRole, editIter.value());
                newData.erase(editIter);
            }
        } else if (editIter != newData.end()) {
            newData.erase(editIter);
        }
    }
    if (item->data != newData) {
        item->data = std::move(newData);
        dataChanged(index, index);
    }
    return true;
}

/*!
\reimp
*/
void GenericModel::sort(int column, Qt::SortOrder order)
{
    sort(column,QModelIndex(),order);
}

/*!
\brief Sorts all children of \a parent by \a column in the given \a order.
\details The role used for the sorting is determined by sortRole.
If \a recursive is set to true the sorting will propagate down the hierarchy of the model
*/
void GenericModel::sort(int column, const QModelIndex &parent, Qt::SortOrder order, bool recursive)
{
    if(column<0 || column>=columnCount(parent) || rowCount(parent)==0)
        return;
    QList<QPersistentModelIndex> parents;
    if(parent.isValid())
        parents.append(parent);
    layoutAboutToBeChanged(parents,QAbstractItemModel::VerticalSortHint);
    Q_D(GenericModel);
    d->itemForIndex(parent)->sortChildren(column,d->sortRole,order,recursive);
    layoutChanged(parents,QAbstractItemModel::VerticalSortHint);
}

/*!
\reimp
\details At the moment no view provided by Qt supports this
*/
QSize GenericModel::span(const QModelIndex &index) const
{
    if (!index.isValid())
        return QSize();
    Q_ASSERT(index.model() == this);
    Q_D(const GenericModel);
    return d->itemForIndex(index)->span();
}

/*!
\brief Set the amount of rows and columns an item should occupy.
\details At the moment no view provided by Qt supports this
*/
bool GenericModel::setSpan(const QModelIndex &index, const QSize &size)
{
    if (!index.isValid() || size.isEmpty())
        return false;
    Q_ASSERT(index.model() == this);
    Q_D(GenericModel);
    d->itemForIndex(index)->setSpan(size);
    const QModelIndex parIdx = index.parent();
    const QModelIndex bottomRight = this->index(qMin(index.row() + size.height(), rowCount(parIdx) - 1),
                                                qMin(index.column() + size.width(), columnCount(parIdx) - 1), parIdx);
    dataChanged(index, bottomRight);
    return true;
}

/*!
\property GenericModel::mergeDisplayEdit
\accessors %mergeDisplayEdit(), setMergeDisplayEdit()
\notifier mergeDisplayEditChanged()
\brief This property determines if the Qt::DisplayRole and Qt::EditRole should be merged in the extra row/column
\details By default the two roles are one and the same you can use this property to separate them.
If there's any data in the role when you set this property to true it will be duplicated for both roles.
If there is data both in Qt::DisplayRole and Qt::EditRole when you set this property to false Qt::DisplayRole will prevail.
*/
bool GenericModel::mergeDisplayEdit() const
{
    Q_D(const GenericModel);
    return d->m_mergeDisplayEdit;
}

void GenericModel::setMergeDisplayEdit(bool val)
{
    Q_D(GenericModel);
    if (d->m_mergeDisplayEdit == val)
        return;
    d->setMergeDisplayEdit(val);
    d->m_mergeDisplayEdit = val;
    mergeDisplayEditChanged(d->m_mergeDisplayEdit);
    d->signalAllChanged({Qt::DisplayRole, Qt::EditRole});
}

/*!
\property QSortFilterProxyModel::sortRole
\accessors %sortRole(), setSortRole()
\notifier sortRoleChanged()
\brief the item role that is used to query the source model's data when sorting items

The default value is Qt::DisplayRole.
*/
int GenericModel::sortRole() const
{
    Q_D(const GenericModel);
    return d->sortRole;
}
void GenericModel::setSortRole(int role)
{
    Q_D(GenericModel);
    if (d->sortRole == role)
        return;
    d->sortRole = role;
    sortRoleChanged(role);
}

void GenericModelPrivate::signalAllChanged(const QVector<int> &roles, const QModelIndex &parent)
{
    Q_Q(GenericModel);
    const int rowCnt = q->rowCount(parent);
    const int colCnt = q->columnCount(parent);
    for (int i = 0; i < rowCnt; ++i) {
        for (int j = 0; j < colCnt; ++j) {
            const QModelIndex currPar = q->index(i, j, parent);
            if (q->hasChildren(currPar))
                signalAllChanged(roles, currPar);
        }
    }
    q->dataChanged(q->index(0, 0, parent), q->index(rowCnt - 1, colCnt - 1, parent), roles);
}

void GenericModelPrivate::setMergeDisplayEdit(bool val)
{
    root->setMergeDisplayEdit(val);
    for (auto i = vHeaderData.begin(), iEnd = vHeaderData.end(); i != iEnd; ++i)
        setMergeDisplayEdit(val, *i);
    for (auto i = hHeaderData.begin(), iEnd = hHeaderData.end(); i != iEnd; ++i)
        setMergeDisplayEdit(val, *i);
}

void GenericModelPrivate::setMergeDisplayEdit(bool val, RolesContainer &container)
{
    const auto displayIter = container.constFind(Qt::DisplayRole);
    if (val) {
        if (displayIter != container.constEnd()) {
            container.remove(Qt::EditRole);
            return;
        }
        const auto editIter = container.find(Qt::EditRole);
        if (editIter != container.constEnd()) {
            container.insert(Qt::DisplayRole, editIter.value());
            container.erase(editIter);
        }
    } else if (displayIter != container.constEnd())
        container.insert(Qt::EditRole, displayIter.value());
}

/*!
\class GenericModel
\brief This proxy model provides an extra row and column to handle user insertions
\details This proxy will add an extra row, column or both to allow users to insert new sections with a familiar interface.

You can use setInsertDirection to determine whether to show an extra row or column. By default, this model will behave as QIdentityProxyModel

You can either call commitRow/commitColumn or reimplement validRow/validColumn to decide when a row/column should be added to the main model.

\warning Only flat models are supported. Branches of a tree will be hidden by the proxy
*/

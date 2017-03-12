/****************************************************************************
** Copyright (C) 2016 Olaf Japp
**
** This file is part of AnimationMaker.
**
**  AnimationMaker is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  AnimationMaker is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with AnimationMaker.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "timelinemodel.h"
#include "animationscene.h"
#include "keyframe.h"
#include "rectangle.h"
#include <QPixmap>

TimelineModel::TimelineModel()
{
    QVariant rootData = "Root";
    QVariant data;

    m_rootItem = new TreeItem(rootData, data);
    m_lastkeyframe = 0;
}

void TimelineModel::reset()
{
    beginResetModel();
    m_rootItem->children()->clear();
    m_lastkeyframe = 0;
    endResetModel();
}

void TimelineModel::addKeyFrame(ResizeableItem *item, QString propertyName, qreal value, int time)
{
    bool found = false;
    TreeItem *treeChildItem = NULL;

    TreeItem *treeItem = searchChild(m_rootItem, item);
    if(treeItem)
    {
        found = true;
        treeChildItem = searchChild(treeItem, propertyName);
    }
    else
    {
        treeItem = new TreeItem(item->id(), qVariantFromValue((void *) item), m_rootItem, 1);
        connect(item, SIGNAL(idChanged(ResizeableItem *, QString)), this, SLOT(idChanged(ResizeableItem *, QString)));

        beginInsertRows(QModelIndex(), m_rootItem->childCount() - 1, m_rootItem->childCount() - 1);
        m_rootItem->appendChild(treeItem);
        endInsertRows();
        emit keyframeAdded(item, NULL);
    }

    KeyFrame *keyframe = new KeyFrame();
    keyframe->setValue(QVariant(value));
    keyframe->setTime(time);
    item->addKeyframe(propertyName, keyframe);
    if(treeChildItem)
    {
        QVariant var = treeChildItem->data(1);
        QList<KeyFrame*> *list = (QList<KeyFrame*>*) var.value<void *>();
        list->append(keyframe);
        emit keyframeAdded(NULL, NULL);
    }
    else
    {
        QList<KeyFrame*> *list = new QList<KeyFrame*>();
        list->append(keyframe);
        treeChildItem = new TreeItem(propertyName, qVariantFromValue((void *) list), treeItem, 2);
        beginInsertRows(createIndex(treeItem->row(), 0, treeItem), treeItem->childCount() - 1, treeItem->childCount() - 1);
        treeItem->appendChild(treeChildItem);
        endInsertRows();
        emit keyframeAdded(item, propertyName);
    }

    m_lastkeyframe = m_lastkeyframe < time ? time : m_lastkeyframe;
}

void TimelineModel::keyframeAdded(ResizeableItem * item, QString propertyName, KeyFrame *key)
{
    bool found = false;
    TreeItem *treeChildItem = NULL;

    TreeItem *treeItem = searchChild(m_rootItem, item);
    if(treeItem)
    {
        found = true;
        treeChildItem = searchChild(treeItem, propertyName);
    }
    else
    {
        treeItem = new TreeItem(item->id(), qVariantFromValue((void *) item), m_rootItem, 1);
        connect(item, SIGNAL(idChanged(ResizeableItem *, QString)), this, SLOT(idChanged(ResizeableItem *, QString)));
    }

    if(treeChildItem)
    {
        QVariant var = treeChildItem->data(1);
        QList<KeyFrame*> *list = (QList<KeyFrame*>*) var.value<void *>();
        list->append(key);
    }
    else
    {
        QList<KeyFrame*> *list = new QList<KeyFrame*>();
        list->append(key);
        treeChildItem = new TreeItem(propertyName, qVariantFromValue((void *) list), treeItem, 2);
        beginInsertRows(createIndex(treeItem->row(), 0, treeItem), treeItem->childCount() - 1, treeItem->childCount() - 1);
        treeItem->appendChild(treeChildItem);
        endInsertRows();
    }
    if(!found)
    {
        beginInsertRows(QModelIndex(), m_rootItem->childCount() - 1, m_rootItem->childCount() - 1);
        m_rootItem->appendChild(treeItem);
        endInsertRows();
    }
    m_lastkeyframe = m_lastkeyframe < key->time() ? key->time() : m_lastkeyframe;
}

int TimelineModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    else
        return m_rootItem->columnCount();
}

QVariant TimelineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DecorationRole)
        return QVariant();//QPixmap(":/images/rect.png");

    if (role == Qt::DisplayRole)
    {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item->data(0);
    }

    if (role == Qt::UserRole)
    {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item->data(1);
    }

    if (role == Qt::UserRole + 1)
    {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        return item->data(2);
    }

    return QVariant();
}

Qt::ItemFlags TimelineModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant TimelineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->data(section);

    return QVariant();
}

QModelIndex TimelineModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TimelineModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TimelineModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void TimelineModel::idChanged(ResizeableItem *item, QString value)
{
    TreeItem *treeItem = searchChild(m_rootItem, item);
    if(treeItem)
    {
        if(value.isEmpty())
            value = getItemTypeName(item);
        treeItem->setData(0, value);
        this->dataChanged(index(0,0), index(1,0));
    }
}

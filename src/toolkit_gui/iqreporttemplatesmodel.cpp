#include "iqreporttemplatesmodel.h"

#include <QApplication>
#include <QFontMetrics>

#include "base/tools.h"

//IQReportTemplatesModel::IQReportTemplatesModel(QObject *parent)
//    : QAbstractItemModel(parent),
//      templates_(insight::ResultReportTemplates::globalInstance())
//{
//}

//QVariant IQReportTemplatesModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//    if (orientation==Qt::Horizontal && role==Qt::DisplayRole)
//    {
//      switch (section)
//      {
//        case 0: return "Template label";
//      }
//    }
//    return QVariant();
//}

//QModelIndex IQReportTemplatesModel::index(int row, int column, const QModelIndex &parent) const
//{
//    if (!parent.isValid())
//    {
//      if (row>=0 && row<templates_.size())
//      {
//        return createIndex(row, column, nullptr);
//      }
//    }
//    return QModelIndex();
//}

//QModelIndex IQReportTemplatesModel::parent(const QModelIndex &index) const
//{
//    return QModelIndex();
//}

//int IQReportTemplatesModel::rowCount(const QModelIndex &parent) const
//{
//    return templates_.size();
//}

//int IQReportTemplatesModel::columnCount(const QModelIndex &parent) const
//{
//    return 1;
//}

//QVariant IQReportTemplatesModel::data(const QModelIndex &index, int role) const
//{
//    if (index.isValid())
//    {
//        auto i=templates_.begin();
//        std::advance(i, index.row());

//        if (role==Qt::DisplayRole)
//        {
//            switch (index.column())
//            {
//                case 0: return QString::fromStdString( i->first );
//            }
//        }
//        if ( role==Qt::FontRole &&
//                i==templates_.defaultItemIterator() )
//        {
//            QFont font(QApplication::font());
//            font.setBold(true);
//            return font;
//        }

//    }

//    return QVariant();
//}

//void IQReportTemplatesModel::addTemplate(const QString& filePath)
//{
//    boost::filesystem::path fp(filePath.toStdString());
//    auto key=fp.filename().stem().string();

//    int inew=insight::predictInsertionLocation(templates_, key);
//    beginInsertRows(QModelIndex(), inew, inew);

//    templates_.addTemplateFromFile(key, fp);

//    endInsertRows();
//}

//void IQReportTemplatesModel::setDefaultTemplate(const QModelIndex &index)
//{
//    if (index.isValid())
//    {
//        auto iold = templates_.defaultItemIterator();
//        auto indexold = this->index( std::distance(templates_.cbegin(), iold), 0, QModelIndex());

//        auto i=templates_.begin();
//        std::advance(i, index.row());
//        templates_.setDefaultItem(i->first);

//        Q_EMIT dataChanged(index, index);
//        Q_EMIT dataChanged(indexold, indexold);
//    }
//}

//void IQReportTemplatesModel::removeTemplate(const QModelIndex &index)
//{
//    if (index.isValid())
//    {
//        auto i=templates_.begin();
//        std::advance(i, index.row());

//        insight::assertion(
//                    i!=templates_.defaultItemIterator(),
//                    "cannot remove default template! Please set a new default template first.");

//        insight::assertion(
//                    i->first!="builtin",
//                    "cannot remove builtin template!");

//        beginRemoveRows(QModelIndex(), index.row(), index.row());
//        templates_.erase(i);
//        endRemoveRows();
//    }
//}

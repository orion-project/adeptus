#include <QComboBox>
#include <QDateTimeEdit>
#include <QPlainTextEdit>

#include "bugitemdelegate.h"
#include "bugmanager.h"

BugItemDelegate::BugItemDelegate(QObject *parent) : QItemDelegate(parent)
{
}

void BugItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    switch (index.column())
    {
    case COL_SUMMARY:
    case COL_EXTRA:
    {
        QPlainTextEdit *textEditor = qobject_cast<QPlainTextEdit*>(editor);
        if (textEditor)
            textEditor->setPlainText(index.data().toString());
        break;
    }
    case COL_CREATED:
    case COL_UPDATED:
    {
        QDateTimeEdit *dateEditor = qobject_cast<QDateTimeEdit*>(editor);
        if (dateEditor)
            dateEditor->setDateTime(index.data().toDateTime());
        break;
    }
    default:
    {
        QComboBox *dictEditor = qobject_cast<QComboBox*>(editor);
        if (dictEditor)
            WidgetHelper::selectId(dictEditor, index.data());
    }
    }
}

void BugItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    switch (index.column())
    {
    case COL_SUMMARY:
    case COL_EXTRA:
    {
        QPlainTextEdit *textEditor = qobject_cast<QPlainTextEdit*>(editor);
        if (textEditor)
            model->setData(index, textEditor->toPlainText().trimmed());
        break;
    }
    case COL_CREATED:
    case COL_UPDATED:
    {
        QDateTimeEdit *dateEditor = qobject_cast<QDateTimeEdit*>(editor);
        if (dateEditor)
            model->setData(index, dateEditor->dateTime());
        break;
    }
    default:
    {
        QComboBox *dictEditor = qobject_cast<QComboBox*>(editor);
        if (dictEditor)
            model->setData(index, WidgetHelper::selectedId(dictEditor));
    }
    }
}

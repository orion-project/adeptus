#ifndef ISSUETEXTEDIT_H
#define ISSUETEXTEDIT_H

#include <QPlainTextEdit>

class IssueTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit IssueTextEdit(QWidget *parent = nullptr);

    bool canInsertFromMimeData(const QMimeData* source) const override;
    void insertFromMimeData(const QMimeData* source) override;
    void dropEvent(QDropEvent *event) override;

    QString cleanFiles();

private:
    void pasteImage(const QImage& img);
    void pasteFile(const QMimeData* source);

    QStringList _generatedFiles;
};

#endif // ISSUETEXTEDIT_H

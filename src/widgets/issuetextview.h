#ifndef ISSUE_TEXT_VIEW_H
#define ISSUE_TEXT_VIEW_H

#include <QTextBrowser>

class IssueTextView : public QTextBrowser
{
    Q_OBJECT

public:
    explicit IssueTextView(QWidget *parent = nullptr);

private slots:
    void linkClicked(const class QUrl&);
};

QString sanitizeHtml(const QString& s);

#endif // ISSUE_TEXT_VIEW_H

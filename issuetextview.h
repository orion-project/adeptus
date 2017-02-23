#ifndef ISSUETEXTVIEW_H
#define ISSUETEXTVIEW_H

#include <QTextBrowser>

class IssueTextView : public QTextBrowser
{
    Q_OBJECT

public:
    explicit IssueTextView(QWidget *parent = 0);

private slots:
    void linkClicked(const class QUrl&);
};

QString sanitizeHtml(const QString& s);

#endif // ISSUETEXTVIEW_H

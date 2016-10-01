#include "startpage.h"
#include "widgets/OriLabels.h"

#include <QBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QPaintEvent>

namespace Adeptus {

StartPageItem::StartPageItem(const QString& text, const QPixmap& pixmap) : QWidget(0)
{
    QLabel *icon = new QLabel;
    icon->setPixmap(pixmap);

    auto label = new Ori::Widgets::ClickableLabel(text);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(icon);
    layout->addWidget(label);
    setLayout(layout);
}


StartPage::StartPage(QWidget *parent) : QWidget(parent)
{
    _background = QPixmap(":/about/bug");

    QGridLayout *layoutContent = new QGridLayout;
    layoutContent->setVerticalSpacing(24);
    layoutContent->setHorizontalSpacing(24);
    layoutContent->addWidget(new StartPageItem("reZonator", QPixmap(":/misc/calc")), 0, 0);
    layoutContent->addWidget(new StartPageItem("Spectrum", QPixmap(":/misc/calc")), 0, 1);
    layoutContent->addWidget(new StartPageItem("Adeptus", QPixmap(":/misc/calc")), 1, 0);
    layoutContent->addWidget(new StartPageItem("Herodotus", QPixmap(":/misc/calc")), 1, 1);

    QHBoxLayout *layoutCenter = new QHBoxLayout;
    layoutCenter->addStretch();
    layoutCenter->addLayout(layoutContent);
    layoutCenter->addStretch();
    layoutCenter->addStretch();

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addStretch();
    layoutMain->addLayout(layoutCenter);
    layoutMain->addStretch();
    setLayout(layoutMain);
}

void StartPage::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter p(this);
    QRect r = this->rect();
    p.drawPixmap(r.right() - _background.width(), r.top(),
                 _background.width(), _background.height(),
                 _background);
}

} // namespace Adeptus

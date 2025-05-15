#include <QPropertyAnimation>
#include "spoiler.h"

Spoiler::Spoiler(const QString & title, const int animationDuration, QWidget *parent) : QWidget(parent), animationDuration(animationDuration) {

    toggleButton.setStyleSheet(
        "QToolButton {"
        " font-size: 18px;"
        "  border: none;"
        "  color: black;"
        "  background-color: transparent;"
        "  padding: 4px;"
        "  font-weight: normal;"
        "}"
        );
    toggleButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toggleButton.setArrowType(Qt::ArrowType::RightArrow);
    toggleButton.setText(title);
    toggleButton.setCheckable(true);
    toggleButton.setChecked(false);

    headerLine.setFrameShape(QFrame::HLine);
    headerLine.setFrameShadow(QFrame::Sunken);
    headerLine.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    headerLine.setVisible(true);

    // Add Play Button
    playButton.setIcon(QIcon(":/assets/play.png")); // Use your play icon
    playButton.setStyleSheet(
        "QToolButton {"
        " border: none;"
        " padding: 4px;"
        "}"
        "QToolButton:hover {"
        " background-color: #e0e0e0;"
        "}"
        );
    playButton.setCursor(Qt::PointingHandCursor);
    playButton.setVisible(true);
    playButton.setIcon(QIcon(":/assets/play.png"));

    // qDebug() << "Icon exists:" << QFile::exists(":/assets/play.png");

    playButton.setIconSize(QSize(16, 16));

    contentArea.setStyleSheet(
        "QScrollArea {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
        );

    contentArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);


    // start out collapsed
    contentArea.setMaximumHeight(0);
    contentArea.setMinimumHeight(0);

    // let the entire widget grow and shrink with its content
    toggleAnimation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    toggleAnimation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    toggleAnimation.addAnimation(new QPropertyAnimation(&contentArea, "maximumHeight",this));


    // don't waste space
    mainLayout.setVerticalSpacing(0);
    mainLayout.setContentsMargins(0, 0, 0, 0);
    int row = 0;

    // a horizontal layout for the header row so that play button comes aligned
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->addWidget(&toggleButton, 0, Qt::AlignLeft);
    headerLayout->addStretch(1); // This pushes the play button to the right
    headerLayout->addWidget(&playButton, 0, Qt::AlignRight);

    // Add the header layout and the rest to the main vertical layout
    mainLayout.addLayout(headerLayout, row++, 0, 1, 3);
    mainLayout.addWidget(&headerLine, row++, 0, 1, 3);
    mainLayout.addWidget(&contentArea, row, 0, 1, 3);


    setLayout(&mainLayout);


    QObject::connect(&toggleButton, &QToolButton::clicked, [this](bool checked) {
        toggleButton.setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
        toggleAnimation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        toggleAnimation.start();
    });


    connect(&toggleAnimation, &QAnimationGroup::finished, [this]() {
        if (!parentWidget()) return; // Critical safety check
        parentWidget()->updateGeometry();
        parentWidget()->adjustSize();

        if (QScrollArea* sa = qobject_cast<QScrollArea*>(parentWidget()->parentWidget())) {
            sa->setWidgetResizable(true);
            sa->widget()->adjustSize();
        }
    });

}

void Spoiler::setContentLayout(QLayout & contentLayout) {
    delete contentArea.layout();
    contentArea.setLayout(&contentLayout);

    contentLayout.invalidate();
    contentArea.updateGeometry();

    const auto collapsedHeight = sizeHint().height() - contentArea.maximumHeight();
    auto contentHeight = contentLayout.sizeHint().height();

    for (int i = 0; i < toggleAnimation.animationCount() - 1; ++i) {
        QPropertyAnimation * spoilerAnimation = static_cast<QPropertyAnimation *>(toggleAnimation.animationAt(i));
        spoilerAnimation->setDuration(animationDuration);
        spoilerAnimation->setStartValue(collapsedHeight);
        spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
    }

    QPropertyAnimation * contentAnimation = static_cast<QPropertyAnimation *>(toggleAnimation.animationAt(toggleAnimation.animationCount() - 1));
    contentAnimation->setDuration(animationDuration);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);

    this->setMinimumHeight(collapsedHeight);
    this->setMaximumHeight(collapsedHeight);
    this->updateGeometry();
}

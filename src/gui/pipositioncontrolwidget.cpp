#include <QGroupBox>
#include <QPushButton>
#include <QMessageBox>

#include "pipositioncontrolwidget.h"

PIPositionControlWidget::PIPositionControlWidget(
    PIDevice *device, QWidget *parent) :
    QWidget(parent), device(device)
{
    setupUI();
    appendDummyRows(1);
}

void PIPositionControlWidget::appendDummyRows(const int number)
{
    for (int i = 0; i < number; ++i) {
        appendRow(i, "1");
    }
}

void PIPositionControlWidget::setupUI()
{
    grid = new QGridLayout();

    QGroupBox *gb = new QGroupBox(device->getVerboseName());
    gb->setLayout(grid);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(gb);
    setLayout(vLayout);

    device->connectedState()->assignProperty(gb, "enabled", true);
    device->disconnectedState()->assignProperty(gb, "enabled", false);

    connect(device, &PIDevice::connected,
            this, &PIPositionControlWidget::updateUIConnect);
    connect(device, &PIDevice::newPositions,
            this, &PIPositionControlWidget::updateValues);
}

void PIPositionControlWidget::appendRow(const int row, const QString &name)
{
    int col = 0;
    grid->addWidget(new QLabel(QString("Axis %1").arg(name)), row, col++, 1, 1);
    QLabel *currentPos = new QLabel("0.000");
    currentPosLabelMap[name.at(0)] = currentPos;
    grid->addWidget(currentPos, row, col++, 1, 1);
    DoubleSpinBox *sb = new DoubleSpinBox();
    double min = -10;
    double max = 10;
    if (device->isConnected()) {
        min = device->getTravelRangeLowEnd(name).at(0);
        max = device->getTravelRangeHighEnd(name).at(0);
    }
    sb->setRange(min, max);
    sb->setDecimals(3);
    grid->addWidget(sb, row, col++, 1, 1);

    QFrame *line;
    line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    grid->addWidget(line, row, col++, 1, 1);

    QDoubleSpinBox *stepSpinBox = new QDoubleSpinBox();
    stepSpinBox->setRange(0, max);
    stepSpinBox->setValue(0.1);
    grid->addWidget(stepSpinBox, row, col++, 1, 1);

    QPushButton *plusPushButton = new QPushButton("+");
    grid->addWidget(plusPushButton, row, col++, 1, 1);

    QPushButton *minusPushButton = new QPushButton("-");
    grid->addWidget(minusPushButton, row, col++, 1, 1);

    connect(sb, &DoubleSpinBox::returnPressed, this, [ = ](){
        move(name, sb->value());
    });

    connect(plusPushButton, &QPushButton::clicked, this, [ = ](){
        moveRelative(name, stepSpinBox->value());
    });
    connect(minusPushButton, &QPushButton::clicked, this, [ = ](){
        moveRelative(name, -stepSpinBox->value());
    });
}

void PIPositionControlWidget::move(const QString &name, const double pos)
{
    try {
        device->move(name, &pos);
    }
    catch (std::runtime_error e) {
        QMessageBox::critical(nullptr, "Error", e.what());
    }
}
void PIPositionControlWidget::moveRelative(const QString &name,
                                           const double pos)
{
    try {
        device->moveRelative(name, &pos);
    }
    catch (std::runtime_error e) {
        QMessageBox::critical(nullptr, "Error", e.what());
    }
}

void PIPositionControlWidget::updateUIConnect()
{
    clear();
    QString axes = device->getAxisIdentifiers();
    for (int i = 0; i < axes.length(); ++i) {
        appendRow(i, axes.at(i));
    }
}

void PIPositionControlWidget::updateValues(
    const QString &axes, const QVector<double> &pos)
{
    for (int i = 0; i < axes.length(); ++i) {
        currentPosLabelMap.value(axes.at(i))->setText(
            QString("%1").arg(pos.at(i), 0, 'f', 3));
    }
}

void PIPositionControlWidget::clear()
{
    QLayoutItem *child;
    while ((child = grid->takeAt(0))) {
        QWidget *w = child->widget();
        if (w) {
            delete w;
        }
        delete child;
    }
    currentPosLabelMap.clear();
}

#include "ClockWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTime>
#include <QDate>
#include <QAction>
#include <QApplication>
#include <QStyle>

ClockWidget::ClockWidget(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
    , m_trayIcon(new QSystemTrayIcon(this))
    , m_isClickThrough(false)
    , m_opacity(255)
{
    setupUi();
    setupTrayIcon();

    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&ClockWidget::update));
    m_timer->start(100); // Update frequently for smooth second hand if needed, or 1000ms
    
    resize(300, 300);
}

ClockWidget::~ClockWidget() {
}

void ClockWidget::setupUi() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle("Swiss Clock");
}

void ClockWidget::setupTrayIcon() {
    // Use a standard icon for now, or draw one
    // Draw a custom icon to ensure visibility on all DEs
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::white);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawEllipse(2, 2, 60, 60);
    painter.setBrush(Qt::red);
    painter.drawEllipse(28, 28, 8, 8); // Center dot
    painter.end();

    m_trayIcon->setIcon(QIcon(pixmap));
    
    m_trayMenu = new QMenu(this);
    
    QAction *stayOnTopAction = m_trayMenu->addAction("Stay on Top");
    stayOnTopAction->setCheckable(true);
    stayOnTopAction->setChecked(true);
    connect(stayOnTopAction, &QAction::toggled, this, &ClockWidget::toggleStayOnTop);

    QAction *clickThroughAction = m_trayMenu->addAction("Click Through");
    clickThroughAction->setCheckable(true);
    connect(clickThroughAction, &QAction::toggled, this, &ClockWidget::toggleClickThrough);
    
    QMenu *opacityMenu = m_trayMenu->addMenu("Opacity");
    // Simple preset opacities for now
    QAction *op100 = opacityMenu->addAction("100%");
    connect(op100, &QAction::triggered, [this](){ setOpacity(100); });
    QAction *op75 = opacityMenu->addAction("75%");
    connect(op75, &QAction::triggered, [this](){ setOpacity(75); });
    QAction *op50 = opacityMenu->addAction("50%");
    connect(op50, &QAction::triggered, [this](){ setOpacity(50); });
    QAction *op25 = opacityMenu->addAction("25%");
    connect(op25, &QAction::triggered, [this](){ setOpacity(25); });

    m_trayMenu->addSeparator();
    
    QMenu *sizeMenu = m_trayMenu->addMenu("Size");
    QAction *sSmall = sizeMenu->addAction("Small (200px)");
    connect(sSmall, &QAction::triggered, [this](){ setScale(200); });
    QAction *sMedium = sizeMenu->addAction("Medium (300px)");
    connect(sMedium, &QAction::triggered, [this](){ setScale(300); });
    QAction *sLarge = sizeMenu->addAction("Large (400px)");
    connect(sLarge, &QAction::triggered, [this](){ setScale(400); });
    QAction *sXLarge = sizeMenu->addAction("Extra Large (600px)");
    connect(sXLarge, &QAction::triggered, [this](){ setScale(600); });

    m_trayMenu->addSeparator();
    m_trayMenu->addAction("Quit", qApp, &QCoreApplication::quit);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->show();
}

void ClockWidget::wheelEvent(QWheelEvent *event) {
    if (m_isClickThrough) return; // Should not happen if transparent for input, but safety check
    
    int delta = event->angleDelta().y();
    int newSize = width();
    
    if (delta > 0) {
        newSize += 20;
    } else {
        newSize -= 20;
    }
    
    setScale(newSize);
    event->accept();
}

void ClockWidget::setScale(int size) {
    size = qBound(100, size, 1000); // Limit size
    resize(size, size);
}

void ClockWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Handle Opacity
    painter.setOpacity(m_opacity / 255.0);

    int side = qMin(width(), height());
    painter.translate(width() / 2, height() / 2);
    painter.scale(side / 200.0, side / 200.0);

    drawFace(painter);
    drawHands(painter);
}

void ClockWidget::drawFace(QPainter &painter) {
    // Draw white face
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawEllipse(QPoint(0, 0), 98, 98);

    // Draw ticks
    painter.setPen(QPen(Qt::black, 3));
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(88, 0, 96, 0);
        painter.rotate(30.0);
    }

    painter.setPen(QPen(Qt::black, 1));
    for (int i = 0; i < 60; ++i) {
        if (i % 5 != 0)
            painter.drawLine(92, 0, 96, 0);
        painter.rotate(6.0);
    }
    
    // Draw Logo (Custom Image)
    painter.save();
    QPixmap logo(":/logo.png");
    if (!logo.isNull()) {
        // Scale logo to fit nicely
        // Target width approx 40px?
        int targetWidth = 40;
        int targetHeight = logo.height() * targetWidth / logo.width();
        
        // Position: Below 12:00. 12:00 is at (0, -98). Center is (0,0).
        // Let's place it at y = -40 centered
        QRect targetRect(-targetWidth/2, -50, targetWidth, targetHeight);
        
        painter.drawPixmap(targetRect, logo);
    }
    painter.restore();

    // Draw Day of Month
    painter.save();
    painter.rotate(0); // Reset rotation from loop if needed, but loop ends at 360/0
    // Actually loop ends at 360, so we are at 0.
    
    // Position: Above 6:00. 6:00 is at (0, 98) in our coordinate system?
    // No, we are rotated. Let's reset transform to draw text easily or just calculate position.
    // The coordinate system is centered at (0,0).
    // 6:00 is at (0, +radius).
    // Let's draw a small box at (0, 60)
    
    QDate date = QDate::currentDate();
    QString dayText = QString::number(date.day());
    
    // Font
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(14);
    painter.setFont(font);
    
    // Calculate text rect
    QFontMetrics fm(font);
    int textWidth = fm.horizontalAdvance(dayText);
    int textHeight = fm.height();
    QRect textRect(-textWidth/2, 50, textWidth, textHeight);
    
    // Draw box (optional, but looks nice)
    // painter.setPen(Qt::NoPen);
    // painter.setBrush(Qt::lightGray);
    // painter.drawRect(textRect.adjusted(-2, -1, 2, 1));
    
    painter.setPen(Qt::black);
    painter.drawText(textRect, Qt::AlignCenter, dayText);
    
    painter.restore();
}

void ClockWidget::drawHands(QPainter &painter) {
    QTime time = QTime::currentTime();

    // Hour hand
    painter.save();
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawRect(-3, -60, 6, 80); // Simple block hand
    painter.restore();

    // Minute hand
    painter.save();
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawRect(-2, -80, 4, 100);
    painter.restore();

    // Second hand (Red with circle)
    painter.save();
    // Smooth movement for second hand? Swiss clocks often have smooth sweep or stop-to-go.
    // Let's do standard tick for now, maybe smooth later.
    painter.rotate(6.0 * time.second()); 
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    painter.drawRect(-1, -60, 2, 80);
    painter.drawEllipse(QPoint(0, -60), 5, 5); // Circle at tip
    painter.drawEllipse(QPoint(0, 0), 3, 3); // Center cap
    painter.restore();
}

void ClockWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void ClockWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void ClockWidget::contextMenuEvent(QContextMenuEvent *event) {
    m_trayMenu->exec(event->globalPos());
}

void ClockWidget::toggleStayOnTop(bool checked) {
    Qt::WindowFlags flags = windowFlags();
    if (checked) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    show(); // Re-show is needed after changing flags
}

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#endif

void ClockWidget::toggleClickThrough(bool checked) {
    m_isClickThrough = checked;
    
    // Standard Qt flag (might work on Wayland or some WMs)
    Qt::WindowFlags flags = windowFlags();
    if (checked) {
        flags |= Qt::WindowTransparentForInput;
    } else {
        flags &= ~Qt::WindowTransparentForInput;
    }
    setWindowFlags(flags);
    show();

#ifdef Q_OS_LINUX
    // X11 specific fix using XFixes
    if (auto *x11Native = qApp->nativeInterface<QNativeInterface::QX11Application>()) {
        Display *dpy = x11Native->display();
        Window win = winId();
        
        if (checked) {
            XRectangle rect;
            XserverRegion region = XFixesCreateRegion(dpy, &rect, 1);
            XFixesSetWindowShapeRegion(dpy, win, ShapeInput, 0, 0, region);
            XFixesDestroyRegion(dpy, region);
        } else {
            XFixesSetWindowShapeRegion(dpy, win, ShapeInput, 0, 0, None);
        }
    }
#endif
}

void ClockWidget::setOpacity(int value) {
    // value is 0-100
    m_opacity = (value * 255) / 100;
    update();
}

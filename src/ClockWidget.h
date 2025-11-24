#pragma once

#include <QWidget>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>

class ClockWidget : public QWidget {
    Q_OBJECT

public:
    explicit ClockWidget(QWidget *parent = nullptr);
    ~ClockWidget() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override; // Added wheelEvent
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void toggleStayOnTop(bool checked);
    void toggleClickThrough(bool checked);
    void setOpacity(int value); // 0-100
    void setScale(int size); // size in pixels

private:
    void setupUi();
    void setupTrayIcon();
    void drawFace(QPainter &painter);
    void drawHands(QPainter &painter);

    QTimer *m_timer;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    
    QPoint m_dragPosition;
    bool m_isClickThrough;
    int m_opacity; // 0-255
};

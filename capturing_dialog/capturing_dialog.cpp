
#include "capturing_dialog.h"


#include <QMessageBox>
#include <QDateTime>
#include <QGridLayout>


CapturingDialog::CapturingDialog(QWidget *parent) :
        QDialog(parent)
{
    SetupUI(this);
    RetranslateUi(this);
    InitResizeParam();
}

CapturingDialog::~CapturingDialog()
{
}

void CapturingDialog::SetupUI(QDialog *dialog) {

    if (dialog->objectName().isEmpty())
               dialog->setObjectName(QString::fromUtf8("capturingDialog"));
    dialog->resize(600, 400);
    dialog->setStyleSheet(QString::fromUtf8(""));

    capture_button_ = new QPushButton(dialog);
    capture_button_->setObjectName(QString::fromUtf8("captureButton"));
    capture_button_->setGeometry(QRect(20, 20, 75, 23));

    QMetaObject::connectSlotsByName(dialog);

    //设置无边框透明
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    QPalette dialog_palette = palette();
    dialog_palette.setColor (QPalette::Background, QColor (0, 0 , 0, 0));
    dialog_palette.setColor (QPalette::Foreground, QColor (0,0,0,0));
    dialog->setPalette(dialog_palette);
    // setAttribute(Qt::WA_TranslucentBackground);
    //dialog->setWindowOpacity(0.5);//窗口中的所有东西都透明
}

void CapturingDialog::RetranslateUi(QDialog *dialog) {
    dialog->setWindowTitle(QApplication::translate("CapturingDialog", "CapturingDialog", nullptr));


    capture_button_->setText(QApplication::translate("CapturingDialog", "Capture", nullptr));
}

void CapturingDialog::on_captureButton_clicked() {



    QRect capture_rect = this->geometry();
    capture_rect  = capture_rect.adjusted(capture_border_margin_, capture_border_margin_,
                        -capture_border_margin_, -capture_border_margin_);

    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap p = screen->grabWindow(QApplication::desktop()->winId(),
                                   capture_rect.x(), capture_rect.y(),
                                   capture_rect.width(),capture_rect.height());

    QString filePathName = "widget";
    filePathName += QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz");
    filePathName += ".png";
    I << "save path :" << filePathName;

    if(!p.save(filePathName,"png"))
    {
        D <<"save widget screen failed" << filePathName;
    }
//图片保存路径，mac在编译好的软件，右键，打开包里面，其他系统在软件执行目录。
    QMessageBox::information(this,"success, Img path:",filePathName);
}


void CapturingDialog::InitResizeParam() {

    desktop_p = QApplication::desktop();

    left_button_down_ = false;
    resize_ = false;
    move_ = false;

    //resize mark
    mark_wide_ = 6;
    mark_margin_board_ = 4;
    mark_length_ = 20;
    resize_detect_width_ = 20;
    resize_border_line_wide_ = 2;

    resize_border_margin_ = mark_wide_ + mark_margin_board_ + resize_border_line_wide_/2;
    capture_border_margin_ = mark_wide_ + mark_margin_board_ + resize_border_line_wide_;
    mark_color_ = QColor(255, 0, 0, 255);

    resize_boder_color_ =  QColor(0, 0, 255, 255);
    setMouseTracking(true);
}

void CapturingDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    //view border
    painter.setPen(QPen(resize_boder_color_, resize_border_line_wide_));
    QRect frame_rect = this->rect();
    frame_rect  = frame_rect.adjusted(resize_border_margin_, resize_border_margin_,
                        -resize_border_margin_, -resize_border_margin_);
    painter.drawRect(frame_rect);

    //marker
    DrawResizeMark(painter);
}

void CapturingDialog::mousePressEvent(QMouseEvent *event)
{
    E << "mousePressEvent";
    if (event->button() == Qt::LeftButton) {
        left_button_down_ = true;
        resize_ = false;
        move_ = false;

        this->cursor_relative_window_pos_at_btn_down_ = event->pos();
        this->cursor_global_pos_at_btn_down_ = event->globalPos();
        this->left_mouse_down_window_rect_ = this->rect();
    }
}

void CapturingDialog::mouseMoveEvent(QMouseEvent * event)
{
    if (resize_) {
        HandleResize();
        return;
    }

    if(move_) {
        move(event->globalPos() - cursor_relative_window_pos_at_btn_down_);
        return;
    }

    QPoint clientCursorPos = event->pos();
    QRect r = this->rect();
    QRect resizeInnerRect(resize_detect_width_,
                        resize_detect_width_,
                        r.width() - 2*resize_detect_width_,
                        r.height() - 2*resize_detect_width_);
    if(r.contains(clientCursorPos) && !resizeInnerRect.contains(clientCursorPos)) { //调整窗体大小
        ResizeType resize_type = GetResizeType(clientCursorPos);
        resize_type_ = resize_type;
        setCursor(Qt::ArrowCursor);
        update();

        if (left_button_down_) {
            resize_ = true;
            resize_type_ = resize_type;
            HandleResize();
        }
    }
    else { //移动窗体
        if (resize_type_ != None) {
            resize_type_ = None;
            update();
        }

        setCursor(Qt::OpenHandCursor);
        if (left_button_down_) {
          setCursor(Qt::ClosedHandCursor);
          move_ = true;
          move(event->globalPos() - cursor_relative_window_pos_at_btn_down_);
        }
    }
}

void CapturingDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if(move_) {
        LimitMovePos(event->globalPos());
        setCursor(Qt::ArrowCursor);
    }

    left_button_down_ = false;
    resize_ = false;
    move_ = false;

    resize_type_ = None;

    update();

    //设置了透明以后，会有严重残影
    //https://www.cnblogs.com/findumars/p/6411273.html
    //[Qt]不带标题栏（FramelessWindowHint）的窗体移动及调整大小
    //https://blog.csdn.net/luols/article/details/48733721
    //Qt实现半透明、无边框、可自由移动、不规则的窗体
    //https://blog.csdn.net/qq_37385181/article/details/82894077

}

void CapturingDialog::leaveEvent(QEvent *) {

    if(left_button_down_ == false) {
        resize_type_ = None;
         repaint();
    }
}


void CapturingDialog::LimitMovePos(QPoint button_up_cursor_global_pos)
{
    //up left corner point of the window
    QPoint current_pos_of_window =
            button_up_cursor_global_pos - cursor_relative_window_pos_at_btn_down_;

    int window_left = current_pos_of_window.x()  + resize_border_margin_;
    int window_top = current_pos_of_window.y() + resize_border_margin_;
    int window_right = current_pos_of_window.x() + this->width() - resize_border_margin_*2;
    int window_bottom = current_pos_of_window.y() + this->height() - resize_border_margin_*2;

    if(window_left < desktop_p->x()) {
        current_pos_of_window.setX(desktop_p->x() - resize_border_margin_);
    } else if (window_right > desktop_p->width()) {
        current_pos_of_window.setX(desktop_p->width()-this->width() + resize_border_margin_);
    }

    if(window_top < desktop_p->y()) {
        current_pos_of_window.setY(desktop_p->y() -  resize_border_margin_);
    } else if(window_bottom > desktop_p->height()) {
        current_pos_of_window.setY(desktop_p->height() - this->height() + resize_border_margin_);
    }

    move(current_pos_of_window);
}

ResizeType CapturingDialog::GetResizeType(QPoint clientPos)
{
    if (clientPos.y() <= resize_detect_width_) {
        if (clientPos.x() <= resize_detect_width_)
            return LeftTop;
        else if (clientPos.x() >= this->width() - resize_detect_width_)
            return RightTop;
        else
            return Top;
    }
    else if (clientPos.y() >= this->height() - resize_detect_width_) {
        if (clientPos.x() <= resize_detect_width_)
            return LeftBottom;
        else if (clientPos.x() >= this->width() - resize_detect_width_)
            return RightBottom;
        else
            return Bottom;
    }
    else {
        if (clientPos.x() <= resize_detect_width_)
            return Left;
        else
            return Right;
    }
}

void CapturingDialog::HandleResize()
{
    int xdiff = QCursor::pos().x() - cursor_global_pos_at_btn_down_.x();
    int ydiff = QCursor::pos().y() - cursor_global_pos_at_btn_down_.y();
    switch (resize_type_)
    {
        case Right:{
            int resize_value = left_mouse_down_window_rect_.width()+xdiff;
            if(resize_value < resize_detect_width_*2) break;
            resize(resize_value, this->height());
            break;
        }
        case Left:{
            int resize_value = left_mouse_down_window_rect_.width()-xdiff;
            if(resize_value < resize_detect_width_*2) break;
            resize(resize_value, this->height());
            move(cursor_global_pos_at_btn_down_.x()+xdiff, this->y());
            break;
        }
        case Bottom:{
            int resize_value = left_mouse_down_window_rect_.height()+ydiff;
            if(resize_value < resize_detect_width_*2) break;
            resize(this->width(), resize_value);
            break;
        }
        case Top:{
            int resize_value = left_mouse_down_window_rect_.height()-ydiff;
            if(resize_value < resize_detect_width_*2) break;
            resize(this->width(), resize_value);
            move(this->x(), cursor_global_pos_at_btn_down_.y()+ydiff);
            break;
        }
        case RightBottom:{
            int resize_value_w = left_mouse_down_window_rect_.width() + xdiff;
            int resize_value_h = left_mouse_down_window_rect_.height() + ydiff;
            if(resize_value_w < resize_detect_width_*2 || resize_value_h < resize_detect_width_*2) break;
            resize(resize_value_w, resize_value_h);
            break;
        }
        case RightTop:{
            int resize_value_w = left_mouse_down_window_rect_.width()+xdiff;
            int resize_value_h = left_mouse_down_window_rect_.height()-ydiff;
            if(resize_value_w < resize_detect_width_*2 || resize_value_h < resize_detect_width_*2) break;
            resize(resize_value_w, resize_value_h);
            move(this->x(), cursor_global_pos_at_btn_down_.y()+ydiff);
            break;
        }
        case LeftTop:{
            int resize_value_w = left_mouse_down_window_rect_.width()-xdiff;
            int resize_value_h = left_mouse_down_window_rect_.height()-ydiff;
            if(resize_value_w < resize_detect_width_*2 || resize_value_h < resize_detect_width_*2) break;
            resize(resize_value_w, resize_value_h);
            move(cursor_global_pos_at_btn_down_.x()+xdiff, cursor_global_pos_at_btn_down_.y()+ydiff);
            break;
        }
        case LeftBottom:{
            int resize_value_w = left_mouse_down_window_rect_.width()-xdiff;
            int resize_value_h = left_mouse_down_window_rect_.height()+ydiff;
            if(resize_value_w < resize_detect_width_*2 || resize_value_h < resize_detect_width_*2) break;
            resize(resize_value_w, resize_value_h);
            move(cursor_global_pos_at_btn_down_.x()+xdiff, this->y());
            break;
        }
        default:break;
    }
}

void CapturingDialog::DrawResizeMark(QPainter &painter)
{
    QRect frame_rect = this->rect();


    int f_width = frame_rect.width() - mark_margin_board_;
    int f_height = frame_rect.height() - mark_margin_board_;
    int f_x = frame_rect.x() + mark_margin_board_;
    int f_y = frame_rect.y() + mark_margin_board_;


    int f_half_width = frame_rect.width()/2;
    int f_half_height = frame_rect.height()/2;


    switch (resize_type_) {
        case Right:{
            QRect mark = QRect( f_width - mark_wide_,
                                f_half_height - mark_length_,
                                mark_wide_,
                                mark_length_ + mark_length_);
            painter.fillRect(mark, mark_color_);
            break;
        }
        case Left:{
            QRect mark = QRect( f_x,
                                f_half_height - mark_length_,
                                mark_wide_,
                                mark_length_ + mark_length_);
            painter.fillRect(mark, mark_color_);
            break;
        }
        case Bottom:{
            QRect mark = QRect( f_half_width - mark_length_,
                                f_height - mark_wide_,
                                mark_length_ + mark_length_,
                                mark_wide_);
            painter.fillRect(mark, mark_color_);
            break;
        }
        case Top:{
            QRect mark = QRect( f_half_width - mark_length_,
                                f_y,
                                mark_length_ + mark_length_,
                                mark_wide_);
            painter.fillRect(mark, mark_color_);
            break;
        }
        case RightBottom:{
            QRect mark1 = QRect( f_width - mark_wide_,
                                f_height - mark_length_,
                                mark_wide_,
                                mark_length_);
            QRect mark2 = QRect( f_width - mark_length_,
                                f_height - mark_wide_,
                                mark_length_,
                                mark_wide_);
            painter.fillRect(mark1, mark_color_);
            painter.fillRect(mark2, mark_color_);
            break;
        }
        case RightTop:{
            QRect mark1 = QRect( f_width - mark_length_,
                                f_y,
                                mark_length_,
                                mark_wide_);
            QRect mark2 = QRect( f_width - mark_wide_,
                                f_y,
                                mark_wide_,
                                mark_length_);
            painter.fillRect(mark1, mark_color_);
            painter.fillRect(mark2, mark_color_);
            break;
        }
        case LeftTop:{
            QRect mark1 = QRect(f_x,
                                f_y,
                                mark_length_,
                                mark_wide_);
            QRect mark2 = QRect(f_x,
                                f_y,
                                mark_wide_,
                                mark_length_);
            painter.fillRect(mark1, mark_color_);
            painter.fillRect(mark2, mark_color_);
            break;
        }
        case LeftBottom:{
            QRect mark1 = QRect(f_x,
                                f_height - mark_length_,
                                mark_wide_,
                                mark_length_);
            QRect mark2 = QRect(f_x,
                                f_height - mark_wide_,
                                mark_length_,
                                mark_wide_);
            painter.fillRect(mark1, mark_color_);
            painter.fillRect(mark2, mark_color_);
            break;
        }
        default:{ break; }
    }
}



#include "player.h"

Player::Player(QWidget *parent,std::string _name,PLAYER_TYPE _TYPE) : QMainWindow(parent),name(_name),TYPE(_TYPE){
    timer.start(1000); // 1 second interval
    connect(&timer, &QTimer::timeout, this, &Player::updateProgress);
    connect(this,&Player::actionSignal,this,&Player::actionSlots);
    centrallayout = new QVBoxLayout();

    video=std::make_unique<Video>(name);
    video->push_All_Packets();
    SDL_Window *sdlWindow = video->screen;

    inital(sdlWindow);

}

void Player::play(){
    video->play();
}

void Player::inital(SDL_Window *sdlWindow){
    // 创建一个 SDL 窗口来播放视频
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdlWindow, &wmInfo);

    centrallayout->setSpacing(0);



    sublayout = new QVBoxLayout();

    // 将 SDL 窗口嵌入到 Qt 窗口中
    screen_widget = QWidget::createWindowContainer(QWindow::fromWinId((WId)wmInfo.info.win.window));
    centrallayout->addWidget(screen_widget);
    //screen_widget->installEventFilter(this);//安装事件过滤器







    totalTime=video->duration/1000;
    sublayout->addWidget(&progressBar);
    progressBar.setRange(0, totalTime);
    progressBar.setFixedHeight(20);
    progressBar.setStyleSheet("QProgressBar { border: 1px solid grey; border-radius: 5px; text-align: center; }"
                              "QProgressBar::chunk { background-color: lightblue; width: 5px; }");


    resize(*video->width, *video->height+20+15+40);



    // 时间显示
    QHBoxLayout *timeLayout = new QHBoxLayout();
    //currentTimeLabel.setFixedSize(50, 10);
    currentTimeLabel.setAlignment(Qt::AlignLeft);

    timeLayout->addWidget(&currentTimeLabel);

    totalTimeLabel.setAlignment(Qt::AlignRight);
    totalTimeLabel.setFixedSize(50, 15);
    timeLayout->addWidget(&totalTimeLabel);

    sublayout->addLayout(timeLayout);





    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    buttonLayout->addWidget(spacer);


    // 暂停按钮
    pauseButton = new QPushButton("暂停");
    pauseButton->setFixedSize(40, 40);
    pauseButton->setStyleSheet("QPushButton { background-color: orange; }");
    buttonLayout->addWidget(pauseButton);
    connect(pauseButton, &QPushButton::clicked, this, &Player::pauseVideo);
    // 倍速按钮
    increaseSpeedButton = new QPushButton("faster");
    decreaseSpeedButton = new QPushButton("slower");
    increaseSpeedButton->setFixedSize(40, 20);
    decreaseSpeedButton->setFixedSize(40, 20);
    increaseSpeedButton->setStyleSheet("QPushButton { background-color: white; color: black; font-weight: bold; }");
    decreaseSpeedButton->setStyleSheet("QPushButton { background-color: white; color: black; font-weight: bold; }");
    buttonLayout->addWidget(increaseSpeedButton);
    buttonLayout->addWidget(decreaseSpeedButton);
    connect(increaseSpeedButton, &QPushButton::clicked, this, &Player::increaseSpeed);
    connect(decreaseSpeedButton, &QPushButton::clicked, this, &Player::decreaseSpeed);

    // 将按钮布局添加到主布局中
    sublayout->addLayout(buttonLayout);






    centrallayout->addLayout(sublayout);

    // 设置主布局
    centralWidget.setLayout(centrallayout);
    setCentralWidget(&centralWidget);
}



void Player::keyPressEvent(QKeyEvent *event){
    // 获取按下的键的键码
    int key = event->key();
    SDL_Event sevent;
    sevent.type = SDL_KEYDOWN;
    if (key == Qt::Key_Up) {
        sevent.key.keysym.sym = SDLK_UP;
    } else if (key == Qt::Key_Down) {
        sevent.key.keysym.sym = SDLK_DOWN;
    } else if (key == Qt::Key_Left) {
        sevent.key.keysym.sym = SDLK_LEFT;
    } else if (key == Qt::Key_Right) {
        sevent.key.keysym.sym = SDLK_RIGHT;
    }else if (key == Qt::Key_5) {
        sevent.key.keysym.sym = SDLK_5;
        hide_sub_widgets();
    }else if (key == Qt::Key_8) {
        sevent.key.keysym.sym = SDLK_UP;
    }else if (key == Qt::Key_2) {
        sevent.key.keysym.sym = SDLK_DOWN;
    }else if (key == Qt::Key_4) {
        sevent.key.keysym.sym = SDLK_LEFT;
    }else if (key == Qt::Key_6) {
        sevent.key.keysym.sym = SDLK_RIGHT;
    }else if (key == Qt::Key_Space) {
        sevent.key.keysym.sym = SDLK_SPACE;
    }else if (key == Qt::Key_P) {
        sevent.key.keysym.sym = SDLK_SPACE;
    }else if (key == Qt::Key_0) {
        sevent.key.keysym.sym = SDLK_SPACE;
    } else {
        sevent.key.keysym.sym = static_cast<SDL_Keycode>(key);
    }
    SDL_PushEvent(&sevent);
}

void Player::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        // Calculate the new value based on the mouse click position
        int64_t newValue = static_cast<int64_t>((static_cast<float>(event->x()) / progressBar.width()) * totalTime);
        progressBar.setValue(newValue);
        time_shaft=newValue;
    }
}

void Player::closeEvent(QCloseEvent *event){
    // Emit custom signal before closing
    SDL_Event sevent;
    sevent.type = SDL_QUIT;
    SDL_PushEvent(&sevent);
    emit exitSignal();
    // Call the base class implementation
    QMainWindow::closeEvent(event);
    exit(0);
}

void Player::resizeEvent(QResizeEvent *event){
    // Keep aspect ratio when resizing
    QSize newSize = event->size();
    int newWidth, newHeight;

    if (newSize.width() > newSize.height()) {
        newWidth = newSize.width();
        newHeight = static_cast<int>(newSize.width() * (static_cast<float>(screen_widget->height()) / screen_widget->width()));
    } else {
        newWidth = static_cast<int>(newSize.height() * (static_cast<float>(screen_widget->width()) / screen_widget->height()));
        newHeight = newSize.height();
    }

    // Resize the external widget (replace with SDL video resize logic)
    screen_widget->resize(newWidth, newHeight);
    *video->height=newHeight;
    *video->width=newWidth;

    // Call the base class implementation
    QMainWindow::resizeEvent(event);
}

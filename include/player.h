#ifndef PLAYER_H
#define PLAYER_H

#include <QMainWindow>
#include <QWidget>
#include <QtWidgets>
#include <SDL_syswm.h>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QMouseEvent>
#include <QTimer>
#include <QLCDNumber>
#include "video.h"

class Player : public QMainWindow
{
    Q_OBJECT
public:
    Player(QWidget *parent = nullptr,std::string name = "1.mp4");



    QVBoxLayout *centrallayout;
    QWidget centralWidget;
    QVBoxLayout *sublayout;
    //QWidget subwidget;

    QProgressBar progressBar;
    QWidget *screen_widget;
    QWidget *spacer;
    QPushButton *pauseButton;
    QPushButton *increaseSpeedButton;
    QPushButton *decreaseSpeedButton;
    QLabel currentTimeLabel;
    QLabel totalTimeLabel;

    QTimer timer;
    std::unique_ptr<Video> video;
    std::string name;

    int64_t totalTime;

    bool is_hide=false;

    ~Player(){
        delete screen_widget;
        delete centrallayout;
    }
    void play(){video->play();}
    void inital(SDL_Window *sdlWindow);

private:
    void hide_sub_widgets(){
        is_hide=!is_hide;
        progressBar.setVisible(!is_hide);
        spacer->setVisible(!is_hide);
        pauseButton->setVisible(!is_hide);
        increaseSpeedButton->setVisible(!is_hide);
        decreaseSpeedButton->setVisible(!is_hide);
        currentTimeLabel.setVisible(!is_hide);
        totalTimeLabel.setVisible(!is_hide);
    }

private slots:
    void updateProgress(){
        progressBar.setValue(time_shaft);
        QTime currentTime = QTime(0, 0).addSecs(time_shaft/1000);
        QTime totalTimeQ = QTime(0, 0).addSecs(totalTime/1000);
        currentTimeLabel.setText(currentTime.toString("hh:mm:ss"));
        totalTimeLabel.setText(totalTimeQ.toString("hh:mm:ss"));
    }
    void pauseVideo() {
        SDL_Event sdlEvent;
        sdlEvent.type = SDL_KEYDOWN;
        sdlEvent.key.keysym.sym = SDLK_SPACE;
        SDL_PushEvent(&sdlEvent);
    }
    void increaseSpeed() {
        SDL_Event sdlEvent;
        sdlEvent.type = SDL_KEYDOWN;
        sdlEvent.key.keysym.sym = SDLK_UP;
        SDL_PushEvent(&sdlEvent);
    }

    void decreaseSpeed() {
        SDL_Event sdlEvent;
        sdlEvent.type = SDL_KEYDOWN;
        sdlEvent.key.keysym.sym = SDLK_DOWN;
        SDL_PushEvent(&sdlEvent);
    }


protected:


    void keyPressEvent(QKeyEvent *event) override ;

    void mousePressEvent(QMouseEvent *event) override ;

    void closeEvent(QCloseEvent *event) override ;

    void resizeEvent(QResizeEvent *event) override ;
};
//#include "player.moc"

#endif // PLAYER_H

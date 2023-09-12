#ifndef FEN_IMMERSION_H
#define FEN_IMMERSION_H

#include <QWidget>

namespace Ui {
class fen_Immersion;
}

class fen_Immersion : public QWidget
{
    Q_OBJECT

public:
    explicit fen_Immersion(QWidget *parent = nullptr);
    ~fen_Immersion();
    void setValue(double dImmersion);

private:
    Ui::fen_Immersion *ui;
};

#endif // FEN_IMMERSION_H

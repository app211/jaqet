#ifndef CHOOSEITEMDIALOG_H
#define CHOOSEITEMDIALOG_H

#include <QDialog>
#include "scrapers/scraper.h"

namespace Ui {
class ChooseItemDialog;
}

class ChooseItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseItemDialog(QWidget *parent = 0);
    void setList(FilmPrtList result);
    void setList(ShowPtrList shows);
    ~ChooseItemDialog();
    FilmPtr getSelectedFilm() const;
    ShowPtr getSelectedShow() const;

private:
    Ui::ChooseItemDialog *ui;
    FilmPrtList films;
    ShowPtrList shows;

};

#endif // CHOOSEITEMDIALOG_H
